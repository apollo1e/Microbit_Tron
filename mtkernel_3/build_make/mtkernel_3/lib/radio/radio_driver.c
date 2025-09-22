#include <tk/tkernel.h>
#include <tm/tmonitor.h>
#include <tstdlib.h>
#include <stdint.h>
#include "radio_driver.h"

/* Nordic device headers (CMSIS) */
#include <nrf.h>
#include <tk/device.h>
#include <tk/syslib.h>                /* EnableInt/DisableInt, tk_def_int */

/* ---- Route the RADIO vector to μT-Kernel external interrupt dispatcher ---- */
extern void Excep_Interrupt(void);    /* provided by μT-Kernel */
void Excep_RADIO(void) { Excep_Interrupt(); }


/* μT-Kernel external interrupt number for RADIO (Cortex-M: IRQn + 16) */
#define RADIO_INTNO (16 + RADIO_IRQn)


volatile uint32_t g_evt_end = 0;
volatile uint32_t g_crc_ok  = 0;
volatile uint32_t g_crc_bad = 0;
volatile uint8_t  g_last_len = 0;
volatile int8_t   g_last_rssi = 0;


/* --- minimal memcpy to avoid pulling in newlib <string.h> (size_t clash) --- */
static void *memcpy_local(void *dst, const void *src, SZ n)
{
    unsigned char *d = (unsigned char*)dst;
    const unsigned char *s = (const unsigned char*)src;
    while (n--) *d++ = *s++;
    return dst;
}
#define memcpy memcpy_local

#ifndef __INLINE
#define __INLINE static inline
#endif

/* ------- Configurable limits (DAL typical MTU ~32 bytes) ------- */
#define RADIO_MAX_PAYLOAD   32
#define RX_QUEUE_DEPTH      4

/* ------- μT-Kernel sync primitives ------- */
static ID rx_sem = 0;  /* counts queued RX packets */

/* ------- Simple RX ring ------- */
typedef struct {
    uint8_t len;
    uint8_t data[RADIO_MAX_PAYLOAD];
    int8_t  rssi;
} rx_pkt_t;

static volatile rx_pkt_t rxq[RX_QUEUE_DEPTH];
static volatile uint8_t  rx_head = 0, rx_tail = 0;

/* Current settings (reflect DAL semantics) */
static volatile uint8_t g_group  = RADIO_DEFAULT_GROUP;
static volatile uint8_t g_band   = RADIO_DEFAULT_BAND;
static volatile uint8_t g_txpwr  = RADIO_DEFAULT_TXPOWER;

/* Forward decls of driver entry points */
static ER radio_open(ID devid, UINT omode, void *exinf);
static ER radio_close(ID devid, UINT option, void *exinf);
static ER radio_exec(T_DEVREQ *req, TMO tmout, void *exinf);
static ER radio_event(ID devid, INT evttyp, void *evtinf);

/* ------------- Tiny helpers ------------- */
static __INLINE void hfclk_start(void)
{
    /* Ensure HFCLK (32 MHz) is running for radio accuracy. */
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART = 1;
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0) { /* wait */ }
}

/* Map DAL band (0..100) -> NRF_RADIO->FREQUENCY (MHz offset from 2400) */
static __INLINE uint32_t freq_from_band(uint8_t band)
{
    if (band > 100) band = 100;
    return band; /* RADIO->FREQUENCY expects 0..100 meaning 2400+X MHz */
}

/* Use group as 8-bit PREFIX, with BASE chosen constant (privacy: same address space). */
static void program_addressing(uint8_t group)
{
    /* All devices share address BASE; group maps to PREFIX[0]. Mirrors DAL "group" idea. */
    NRF_RADIO->BASE0   = 0x75626974UL; /* ASCII 'ubit' (arbitrary but constant). */
    NRF_RADIO->PREFIX0 = group;        /* 8-bit prefix = group id */
    NRF_RADIO->TXADDRESS   = 0;        /* use address 0 for TX */
    NRF_RADIO->RXADDRESSES = 1 << 0;   /* listen on address 0 */
}

/* Configure PHY, packet format (~32B payload), CRC16 */
static void program_packet_config(void)
{
    /* 1 Mbps mode, whitening off by default. */
    NRF_RADIO->MODE = RADIO_MODE_MODE_Nrf_1Mbit;

    /* Length field size = 8 bits, no S0/S1. */
    NRF_RADIO->PCNF0 =
        (0 << RADIO_PCNF0_S0LEN_Pos) |
        (0 << RADIO_PCNF0_S1LEN_Pos) |
        (8 << RADIO_PCNF0_LFLEN_Pos);

    /* Max payload length 32, little endian, base address length 4 bytes, no whitening. */
    NRF_RADIO->PCNF1 =
        (4 << RADIO_PCNF1_BALEN_Pos) |
        (RADIO_MAX_PAYLOAD << RADIO_PCNF1_MAXLEN_Pos) |
        (0 << RADIO_PCNF1_STATLEN_Pos) |
        (0 << RADIO_PCNF1_ENDIAN_Pos) |
        (1 << RADIO_PCNF1_WHITEEN_Pos);

    NRF_RADIO->DATAWHITEIV = 0x18;           // <—— MATCH CODAL

    /* CRC: 16-bit polynomial (0x1021), init 0xFFFF. */
    NRF_RADIO->CRCCNF = RADIO_CRCCNF_LEN_Two;   /* 16-bit */
    NRF_RADIO->CRCINIT = 0xFFFF;
    NRF_RADIO->CRCPOLY = 0x11021;
}

/* TX power per DAL 0..7 ladder (map to closest NRF enum) */
static void program_txpower(uint8_t pwr)
{
    static const int8_t map[8] = {
        RADIO_TXPOWER_TXPOWER_Neg30dBm,
        RADIO_TXPOWER_TXPOWER_Neg20dBm,
        RADIO_TXPOWER_TXPOWER_Neg16dBm,
        RADIO_TXPOWER_TXPOWER_Neg12dBm,
        RADIO_TXPOWER_TXPOWER_Neg8dBm,
        RADIO_TXPOWER_TXPOWER_Neg4dBm,
        RADIO_TXPOWER_TXPOWER_0dBm,
        RADIO_TXPOWER_TXPOWER_Pos4dBm
    };
    if (pwr > 7) pwr = 7;
    NRF_RADIO->TXPOWER = map[pwr];
}

/* Start RX: program PACKETPTR and kick state machine */
static uint8_t rx_buf[RADIO_MAX_PAYLOAD + 1]; /* byte 0 = length, rest = payload */

static void radio_kick_rx(void)
{
    /* Prepare RX buffer: first byte is length (hardware places length there). */
    NRF_RADIO->PACKETPTR = (uint32_t)rx_buf;

    NRF_RADIO->SHORTS = RADIO_SHORTS_READY_START_Msk
                      | RADIO_SHORTS_END_START_Msk; /* auto-continue RX */
    NRF_RADIO->EVENTS_READY = 0;
    NRF_RADIO->EVENTS_END   = 0;
    NRF_RADIO->TASKS_RXEN   = 1;
}

/* Push one received packet into ring and post rx_sem */
static void rxq_push(uint8_t *frame, uint8_t len, int8_t rssi)
{
    uint8_t next = (rx_head + 1) % RX_QUEUE_DEPTH;
    if (next == rx_tail) {
        /* overflow -> drop oldest */
        rx_tail = (rx_tail + 1) % RX_QUEUE_DEPTH;
    }
    rxq[rx_head].len = (len > RADIO_MAX_PAYLOAD) ? RADIO_MAX_PAYLOAD : len;
    memcpy((void*)rxq[rx_head].data, frame, rxq[rx_head].len);
    rxq[rx_head].rssi = rssi;
    rx_head = next;

    /* This port doesn’t provide isig_sem; tk_sig_sem is allowed from handler here. */
    (void)tk_sig_sem(rx_sem, 1);
}

/* μT-Kernel ISR (registered via tk_def_int/EnableInt) */
// static void radio_isr(UINT intno)
// {
//     (void)intno;

//     if (NRF_RADIO->EVENTS_END) {
//         NRF_RADIO->EVENTS_END = 0;

//         /* Good CRC? If so, queue. */
//         if ((NRF_RADIO->CRCSTATUS & RADIO_CRCSTATUS_CRCSTATUS_Msk) ==
//              RADIO_CRCSTATUS_CRCSTATUS_CRCOk) {
//             int8_t  rssi = (int8_t)NRF_RADIO->RSSISAMPLE;
//             uint8_t len  = rx_buf[0];
//             if (len > 0 && len <= RADIO_MAX_PAYLOAD) {
//                 rxq_push(&rx_buf[1], len, rssi);
//             }
//         }
//         /* Ready for the next frame due to SHORTS (END->START). */
//     }
// }


static void radio_isr(UINT intno)
{
    (void)intno;

    if (NRF_RADIO->EVENTS_END) {
        NRF_RADIO->EVENTS_END = 0;
        g_evt_end++;

        int ok = (NRF_RADIO->CRCSTATUS & RADIO_CRCSTATUS_CRCSTATUS_Msk)
                  == RADIO_CRCSTATUS_CRCSTATUS_CRCOk;

        uint8_t len = rx_buf[0];
        g_last_len = len;
        g_last_rssi = (int8_t)NRF_RADIO->RSSISAMPLE;

        if (ok) {
            g_crc_ok++;
            if (len > 0 && len <= RADIO_MAX_PAYLOAD)
                rxq_push(&rx_buf[1], len, g_last_rssi);
        } else {
            g_crc_bad++;
        }
        /* RX continues via SHORTS END->START */
    }
}


/* ------- μT-Kernel device entry points ------- */

// static ER radio_open(ID devid, UINT omode, void *exinf)
// {
//     (void)devid; (void)omode; (void)exinf;

//     /* Create RX semaphore once */
//     if (rx_sem == 0) {
//         T_CSEM csem = {0};
//         csem.sematr = TA_TPRI;  /* prioritize tasks waiting on sem */
//         csem.isemcnt = 0;
//         csem.maxsem  = RX_QUEUE_DEPTH;
//         rx_sem = tk_cre_sem(&csem);
//     }

//     /* Clock and basic radio config */
//     hfclk_start();
//     NRF_RADIO->EVENTS_DISABLED = 0;
//     NRF_RADIO->TASKS_DISABLE = 1;
//     while (NRF_RADIO->EVENTS_DISABLED == 0) {}

//     program_packet_config();
//     program_txpower(g_txpwr);
//     NRF_RADIO->FREQUENCY = freq_from_band(g_band);
//     program_addressing(g_group);

   
//     /* Enable radio interrupt via μT-Kernel */
//     NRF_RADIO->INTENSET = RADIO_INTENSET_END_Msk;

//     T_DINT dint = (T_DINT){0};
//     dint.inthdr = (FP)radio_isr;
//     tk_def_int(RADIO_INTNO, &dint);
//     EnableInt(RADIO_INTNO, 3);   /* pick a reasonable priority for your port */

//     /* Start RX loop */
//     radio_kick_rx();

//     return E_OK;
// }

static ER radio_open(ID devid, UINT omode, void *exinf)
{
    (void)devid; (void)omode; (void)exinf;

    /* Create RX semaphore once */
    if (rx_sem == 0) {
        T_CSEM csem = {0};
        csem.sematr = TA_TPRI;
        csem.isemcnt = 0;
        csem.maxsem  = RX_QUEUE_DEPTH;
        rx_sem = tk_cre_sem(&csem);
    }

    /* Clock and basic radio config */
    hfclk_start();

    /* Ensure radio is fully disabled before (re)config */
    NRF_RADIO->EVENTS_DISABLED = 0;
    NRF_RADIO->TASKS_DISABLE   = 1;
    while (NRF_RADIO->EVENTS_DISABLED == 0) {}
    NRF_RADIO->EVENTS_DISABLED = 0;

    program_packet_config();
    program_txpower(g_txpwr);
    NRF_RADIO->FREQUENCY = freq_from_band(g_band);
    program_addressing(g_group);

    /* Debug: dump config */
    tm_printf("[cfg] MODE=%08lx PCNF0=%08lx PCNF1=%08lx\n",
        NRF_RADIO->MODE, NRF_RADIO->PCNF0, NRF_RADIO->PCNF1);
    tm_printf("[cfg] CRC: CNF=%08lx INIT=%08lx POLY=%08lx\n",
        NRF_RADIO->CRCCNF, NRF_RADIO->CRCINIT, NRF_RADIO->CRCPOLY);
    tm_printf("[cfg] ADDR: BASE0=%08lx PREFIX0=%08lx TXADDR=%lu RXADDR=%08lx BALEN=%lu\n",
        NRF_RADIO->BASE0, NRF_RADIO->PREFIX0, (unsigned long)NRF_RADIO->TXADDRESS,
        NRF_RADIO->RXADDRESSES,
        (unsigned long)((NRF_RADIO->PCNF1 >> RADIO_PCNF1_BALEN_Pos) & 7));
    tm_printf("[cfg] FREQ=%lu WHITEEN=%lu IV=%lu SHORTS=%08lx\n",
        (unsigned long)NRF_RADIO->FREQUENCY,
        (unsigned long)((NRF_RADIO->PCNF1 >> RADIO_PCNF1_WHITEEN_Pos) & 1),
        (unsigned long)NRF_RADIO->DATAWHITEIV,
        NRF_RADIO->SHORTS);
    tm_printf("[cfg] RADIO_IRQn=%d RADIO_INTNO=%d\n", RADIO_IRQn, RADIO_INTNO);

    /* Enable radio interrupt via μT-Kernel */
    NRF_RADIO->INTENCLR = 0xFFFFFFFF;
    NRF_RADIO->INTENSET = RADIO_INTENSET_END_Msk;

    T_DINT dint = {0};
    dint.inthdr = (FP)radio_isr;
    tk_def_int(RADIO_INTNO, &dint);
    EnableInt(RADIO_INTNO, 2);   /* choose a sensible priority for your port */

    /* ---- Start RX loop (explicit first START) ---- */
    NRF_RADIO->PACKETPTR = (uint32_t)rx_buf; /* byte 0 = length */
    NRF_RADIO->SHORTS    = RADIO_SHORTS_READY_START_Msk
                         | RADIO_SHORTS_END_START_Msk; /* auto-continue RX */

    NRF_RADIO->EVENTS_READY = 0;
    NRF_RADIO->EVENTS_END   = 0;

    NRF_RADIO->TASKS_RXEN   = 1;
    while (NRF_RADIO->EVENTS_READY == 0) { /* wait once on boot */ }
    NRF_RADIO->EVENTS_READY = 0;

    NRF_RADIO->TASKS_START  = 1;  /* explicit first START */

    return E_OK;
}


static ER radio_close(ID devid, UINT option, void *exinf)
{
    (void)devid; (void)option; (void)exinf;

    DisableInt(RADIO_INTNO);
    NRF_RADIO->INTENCLR = 0xFFFFFFFF;
    NRF_RADIO->SHORTS = 0;
    NRF_RADIO->EVENTS_DISABLED = 0;
    NRF_RADIO->TASKS_DISABLE = 1;
    while (NRF_RADIO->EVENTS_DISABLED == 0) {}
    return E_OK;
}

/* Handle READ/WRITE requests:
   - READ: waits on rx_sem, then copies out one queued frame.
   - WRITE: transmits the buffer and waits for END, then returns to RX.
*/
static ER radio_exec(T_DEVREQ *req, TMO tmout, void *exinf)
{
    (void)exinf;

    switch (req->cmd) {
    case TDC_READ: {
        /* Wait for a packet */
        ER er = tk_wai_sem(rx_sem, 1, tmout);
        if (er < E_OK) { req->asize = 0; return er; }

        /* Pop from ring */
        if (rx_tail == rx_head) { req->asize = 0; return E_OK; }
        uint8_t idx = rx_tail;
        rx_tail = (rx_tail + 1) % RX_QUEUE_DEPTH;

        /* Copy out, truncating to caller buffer size */
        SZ tocpy = (rxq[idx].len <= req->size) ? rxq[idx].len : req->size;
        memcpy(req->buf, (const void*)rxq[idx].data, tocpy);
        req->asize = tocpy;
        return E_OK;
    }

    case TDC_WRITE: {
        /* Copy into a TX frame: DAL length-prefixed format */
        uint8_t local[RADIO_MAX_PAYLOAD + 1];
        SZ len = (req->size > RADIO_MAX_PAYLOAD) ? RADIO_MAX_PAYLOAD : req->size;
        local[0] = (uint8_t)len;
        memcpy(&local[1], req->buf, len);

        /* Disable RX, set PACKETPTR, TX, wait END, then re-enter RX. */
        NRF_RADIO->SHORTS = 0;             /* stop auto RX */
        NRF_RADIO->EVENTS_END = 0;
        NRF_RADIO->PACKETPTR = (uint32_t)local;

        NRF_RADIO->TASKS_DISABLE = 1;      /* ensure clean state */
        while (NRF_RADIO->EVENTS_DISABLED == 0) {}
        NRF_RADIO->EVENTS_DISABLED = 0;

        NRF_RADIO->TASKS_TXEN = 1;
        while (NRF_RADIO->EVENTS_READY == 0) {}
        NRF_RADIO->EVENTS_READY = 0;

        NRF_RADIO->TASKS_START = 1;
        while (NRF_RADIO->EVENTS_END == 0) {}
        NRF_RADIO->EVENTS_END = 0;

        /* Back to RX */
        NRF_RADIO->EVENTS_DISABLED = 0;
        NRF_RADIO->TASKS_DISABLE = 1;
        while (NRF_RADIO->EVENTS_DISABLED == 0) {}
        radio_kick_rx();

        req->asize = len;
        return E_OK;
    }

    default:
        req->asize = 0;
        return E_PAR;
    }
}

/* Not used in this simple driver, but provided to satisfy μT-Kernel device API. */
static ER radio_event(ID devid, INT evttyp, void *evtinf)
{
    (void)devid; (void)evttyp; (void)evtinf;
    return E_OK;
}

/* Public init: register the device name from radio_driver.h */
void radio_driver_init(void)
{
    T_DDEV ddev = (T_DDEV){0};

    ddev.exinf   = 0;
    ddev.drvatr  = 0;
    ddev.openfn  = (FP)radio_open;
    ddev.closefn = (FP)radio_close;
    ddev.execfn  = (FP)radio_exec;
    ddev.waitfn  = NULL;
    ddev.abortfn = NULL;
    ddev.eventfn = (FP)radio_event;

    tk_def_dev((UB*)RADIO_DEVNAME, &ddev, NULL);
    tm_printf("[radio] registered '%s'\n", RADIO_DEVNAME);
}

/* ---- Runtime setters (mirror DAL) ---- */

int radio_set_group(uint8_t group)
{
    g_group = group;
    program_addressing(group);
    return E_OK;
}

int radio_set_band(uint8_t band)
{
    g_band = (band > 100) ? 100 : band;
    NRF_RADIO->FREQUENCY = freq_from_band(g_band);
    return E_OK;
}

int radio_set_txpower(uint8_t power)
{
    g_txpwr = (power > 7) ? 7 : power;
    program_txpower(g_txpwr);
    return E_OK;
}
