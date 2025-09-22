// // sender_main.c

// #include <tk/tkernel.h>
// #include <tm/tmonitor.h>
// // #include <string.h>
// // #include <stdint.h>
// #include "radio_driver.h"


// // --- Radio API declarations ---
// extern void radio_open(void);
// extern INT  radio_recv(UB* buf, INT maxlen);

// EXPORT void usermain(void)
// {
//     UB buffer[64];
//     INT len;

//     // Initialize the radio hardware
//     radio_open();
//     tm_printf("Receiver started. Listening for messages...\n");

//     while (1) {
//         // Try to receive a message into buffer
//         len = radio_recv(buffer, sizeof(buffer) - 1);  // Leave space for null terminator
//         if (len > 0) {
//             buffer[len] = '\0';  // Null-terminate the message for safe printing
//             tm_printf("Received message: %s\n", buffer);
//         }

//         // Wait briefly before checking again (non-blocking mode assumed)
//         tk_dly_tsk(100);  // 100 ms
//     }
// }


// #include <tk/tkernel.h>
// #include <tm/tmonitor.h>
// #include "radio_driver.h"

// EXPORT void usermain(void) {

//     tm_printf("=== Sender USERMAIN STARTED ===\n");
    
//     UB buf[RADIO_MAX_PAYLOAD_LEN];
//     radio_init();

//     tk_dly_tsk(100);  // Let radio settle

//     UB msg[] = "hi";

//     // Select role at compile time
//     // Uncomment ONE of the two roles below

//     // === SENDER ===
//     int i = 0;

//     while (1) {
//         tm_printf("Loop #%d: Sending...\n", i++);
//         radio_send(msg, 2);
//         tm_printf("Loop #%d: Sent: hi\n", i);
//         tk_dly_tsk(800);  // Delay between transmissions
//     }


//     // // === RECEIVER ===
//     // while (1) {
//     //     INT len = radio_receive(buf);
//     //     if (len > 0) {
//     //         tm_printf("Received: ");
//     //         for (int i = 0; i < len; i++) {
//     //             tm_putchar(buf[i]);
//     //         }
//     //         tm_putchar('\n');
//     //     }
//     // }
// }




// sender code 
// #include <tk/tkernel.h>
// #include <tk/syslib.h>   // for tk_def_int
// #include <tm/tmonitor.h>
// #include <stdint.h>

// IMPORT void radio_init(void);
// IMPORT void radio_send(unsigned char *data, unsigned char length);
// extern void RADIO_IRQHandler(void);  // Declare your radio ISR

// #define NVIC_ISER0 (*(volatile uint32_t *)0xE000E100)  // IRQ enable register

// EXPORT void usermain(void) {
//     tm_printf("=== Sendering code  ===\n");


//     // Register the RADIO interrupt handler with the kernel
//     T_DINT dint;
//     dint.intatr = TA_HLNG;               // High-level language interrupt
//     dint.inthdr = (FP)RADIO_IRQHandler;  // Your defined ISR function
//     tk_def_int(1, &dint);                // IRQ number 1 = RADIO IRQ

//     radio_init();
//     tk_dly_tsk(100); // Allow radio to stabilize

//     unsigned char msg[] = "hi";
//     int count = 0;

//     while (1) {
//         tm_printf("Loop #%d: Sending...\n", count);
//         radio_send(msg, 2);
//         tm_printf("Loop #%d: Sent: %s\n", count, msg);
//         count++;
//         tk_dly_tsk(1000); // Delay 1 second
//     }
// }

// working momentary state reading based off 5th document example 
// #include <tk/tkernel.h>
// #include <tm/tmonitor.h>
// #include <sys/sysdepend/cpu/nrf5/sysdef.h>

// LOCAL void btn_init(void) {
//     out_w(GPIO(P0, PIN_CNF(14)), 0); // Button A input
//     out_w(GPIO(P0, PIN_CNF(23)), 0); // Button B input
// }

// void button_task(INT stacd, void *exinf) {
//     tm_putstring("[button_task] Starting.\n");
//     btn_init();

//     while (1) {
//         UW gpio_p0in = in_w(GPIO(P0, IN));
//         BOOL btn_a = ((gpio_p0in & (1 << 14)) == 0);
//         BOOL btn_b = ((gpio_p0in & (1 << 23)) == 0);

//         tm_printf("Button_SW_A: %s, Button_SW_B: %s\n",
//                   (btn_a ? "on" : "off"), (btn_b ? "on" : "off"));

//         tk_dly_tsk(500); // 500ms delay
//     }
// }

// EXPORT INT usermain(void) {
//     T_CTSK ctsk = {
//         .tskatr  = TA_HLNG | TA_RNG0,
//         .stksz   = 1024,
//         .itskpri = 10,
//         .task    = button_task
//     };
//     ID tskid = tk_cre_tsk(&ctsk);
//     tk_sta_tsk(tskid, 0);

//     tk_slp_tsk(TMO_FEVR); // never return
//     return 0;
// }



// working code with toggle state change on button press
// #include <tk/tkernel.h>
// #include <tm/tmonitor.h>
// #include <sys/sysdepend/cpu/nrf5/sysdef.h>

// LOCAL void btn_init(void) {
//     out_w(GPIO(P0, PIN_CNF(14)), 0); // Configure Button A (P0.14) as input
// }

// void button_task(INT stacd, void *exinf) {
//     tm_putstring("[button_task] Starting.\n");
//     btn_init();

//     BOOL led_state = FALSE;     // toggled state
//     BOOL prev_btn = FALSE;      // last button reading

//     while (1) {
//         UW gpio_p0in = in_w(GPIO(P0, IN));
//         BOOL btn_now = ((gpio_p0in & (1 << 14)) == 0); // 1 if pressed

//         // Detect rising edge (not pressed -> pressed)
//         if (btn_now && !prev_btn) {
//             led_state = !led_state;  // flip state
//             tm_printf("[button_task] Toggled state -> %s\n",
//                       led_state ? "ON" : "OFF");
//         }

//         prev_btn = btn_now;   // remember for next loop
//         tk_dly_tsk(50);       // small polling delay (50 ms)
//     }
// }

// EXPORT INT usermain(void) {
//     T_CTSK ctsk = {
//         .tskatr  = TA_HLNG | TA_RNG0,
//         .stksz   = 1024,
//         .itskpri = 10,
//         .task    = button_task
//     };
//     ID tskid = tk_cre_tsk(&ctsk);
//     tk_sta_tsk(tskid, 0);

//     tk_slp_tsk(TMO_FEVR);
//     return 0;
// }


// sender testing code 

// sender task version just after first time manage to pritn hello from sender but then fails 
// #include <tk/tkernel.h>
// #include <tm/tmonitor.h>
// // #include <string.h>          // <-- for strlen
// #include "radio_driver.h"
// #include <tk/device.h>


// /* local strlen to avoid pulling in newlib <string.h> (size_t conflicts) */
// static SZ cstrlen(const char *s) {
//     SZ n = 0;
//     while (s[n]) n++;
//     return n;
// }


// static void sender_task(INT stacd, void *exinf)
// {
//     ID dd = tk_opn_dev((UB*)RADIO_DEVNAME, TD_UPDATE);
//     if (dd < E_OK) { tm_printf("radio open err=%d\n", dd); return; }

//     radio_set_group(10);
//     radio_set_band(7);
//     radio_set_txpower(6);

//     static const char msg[] = "Hello from sender!";   // includes '\0'
//     while (1) {
//         SZ wrote = tk_wri_dev(dd, 0, msg, (SZ)sizeof(msg), TMO_FEVR);
//         tm_printf("TX(%dB): %s\n", (int)wrote, msg);
//         tk_dly_tsk(2000);
//     }
// }

// this version send the hello from sender but eventually becomes -34b which gpt say is wrong 
// #include <tk/tkernel.h>
// #include <tm/tmonitor.h>
// // #include <string.h>          // <-- for strlen
// #include "radio_driver.h"
// #include <tk/device.h>

// static void sender_task(INT stacd, void *exinf)
// {
//     ID dd = tk_opn_dev((UB*)"radio", TD_UPDATE);
//     if (dd < E_OK) {
//         tm_printf("radio open err=%d\n", dd);
//         return;
//     }

//     radio_set_group(10);
//     radio_set_band(7);
//     radio_set_txpower(6);

//     const char *msg = "Hello from sender!";
//     while (1) {
//         SZ wrote = tk_wri_dev(dd, 0, msg, (SZ)strlen(msg), TMO_FEVR); // no +1
//         tm_printf("TX(%dB): %s\n", (int)wrote, msg);
//         tk_dly_tsk(1000);
//     }
// }


// EXPORT INT usermain(void)
// {
//     radio_driver_init();    // register /dev/radio0 early

//     T_CTSK ctsk = {0};
//     ctsk.tskatr  = TA_HLNG | TA_RNG0;
//     ctsk.task    = (FP)sender_task;
//     ctsk.itskpri = 10;
//     ctsk.stksz   = 1024;
//     ID tskid = tk_cre_tsk(&ctsk);
//     tk_sta_tsk(tskid, 0);

//     tk_slp_tsk(TMO_FEVR);
//     return 0;
// }

// before sleep we try this transmitter 
// #include <tk/tkernel.h>
// #include <tm/tmonitor.h>
// // #include <string.h>
// #include "radio_driver.h"

// static void tx_task(INT stacd, void *exinf)
// {
//     ID dd = tk_opn_dev((UB*)RADIO_DEVNAME, TD_UPDATE);
//     if (dd < E_OK) {
//         tm_printf("[tx] radio open err=%d\n", dd);
//         tk_slp_tsk(TMO_FEVR);
//     }

//     /* Match CODAL defaults */
//     radio_set_group(1);    // PREFIX0
//     radio_set_band(7);     // FREQUENCY = 2407 MHz
//     radio_set_txpower(6);  // ~0 dBm (driver maps 0..7 ladder)

//     tm_printf("[tx] started. band=7, group=1, IV=0x18, whitening=ON\n");

//     uint32_t cnt = 0;
//     char msg[32];

//     while (1) {
//         int n = tm_sprintf(msg, "PING %lu", (unsigned long)cnt++);
//         SZ wrote = tk_wri_dev(dd, 0, (UB*)msg, (SZ)n, 1000);
//         tm_printf("[tx] sent %ldB: %s\n", (long)wrote, msg);
//         tk_dly_tsk(100); // 10 Hz
//     }
// }

// EXPORT INT usermain(void)
// {
//     radio_driver_init();

//     T_CTSK ctsk = {0};
//     ctsk.tskatr  = TA_HLNG | TA_RNG0;
//     ctsk.task    = (FP)tx_task;
//     ctsk.itskpri = 10;
//     ctsk.stksz   = 1024;
//     ID tskid = tk_cre_tsk(&ctsk);
//     tk_sta_tsk(tskid, 0);

//     tk_slp_tsk(TMO_FEVR);
//     return 0;
// }







// // receiver testing code 
// #include <tk/tkernel.h>
// #include <tm/tmonitor.h>
// #include "radio_driver.h"

// extern int strlen(const char *s);   // avoid including <string.h>

// static void receiver_task(INT stacd, void *exinf)
// {
//     ID dd = tk_opn_dev((UB*)RADIO_DEVNAME, TD_UPDATE);
//     if (dd < E_OK) {
//         tm_printf("radio open err=%d\n", dd);
//         tk_slp_tsk(TMO_FEVR);
//     }

//     radio_set_group(1);
//     radio_set_band(7);

//     uint8_t buf[64];
//     SYSTIM last_tick;
//     tk_get_tim(&last_tick);

//     while (1) {
//         SZ n = tk_rea_dev(dd, 0, buf, (SZ)(sizeof(buf)-1), 1000); // wait 1s
//         if (n > 0) {
//             buf[n] = '\0';
//             tm_printf("RX(%dB): %s\n", (int)n, buf);
//             tk_get_tim(&last_tick);  // reset heartbeat
//         } else {
//             SYSTIM now;
//             tk_get_tim(&now);
//             if ((now.lo - last_tick.lo) >= 30000) {   // compare low 32 bits
//                 tm_printf("[receiver] alive, no packets seen yet...\n");
//                 tm_printf("[dbg] END=%lu OK=%lu BAD=%lu lastLen=%u RSSI=%d\n",
//                 (unsigned long)g_evt_end,
//                 (unsigned long)g_crc_ok,
//                 (unsigned long)g_crc_bad,
//                 g_last_len, (int)g_last_rssi);

//                 last_tick = now;
//             }
//         }
//     }
// }


// EXPORT INT usermain(void)
// {
//     radio_driver_init();

//     T_CTSK ctsk = {0};
//     ctsk.tskatr  = TA_HLNG | TA_RNG0;
//     ctsk.task    = (FP)receiver_task;
//     ctsk.itskpri = 10;
//     ctsk.stksz   = 1024;
//     ID tskid = tk_cre_tsk(&ctsk);
//     tk_sta_tsk(tskid, 0);

//     tk_slp_tsk(TMO_FEVR);
//     return 0;
// }



// before sleep we try this resceiver 
// receiver testing code (μT-Kernel)
#include <tk/tkernel.h>
#include <tm/tmonitor.h>
#include "radio_driver.h"

// Do NOT include <string.h> / newlib to avoid size_t conflict.
// If you need strlen:
extern int strlen(const char *s);   // ok to declare manually

/* Match the types from radio_driver.h exactly */
extern volatile unsigned long g_evt_end, g_crc_ok, g_crc_bad;
extern volatile uint8_t       g_last_len;
extern volatile int8_t        g_last_rssi;

static void receiver_task(INT stacd, void *exinf)
{
    ID dd = tk_opn_dev((UB*)RADIO_DEVNAME, TD_UPDATE);
    if (dd < E_OK) {
        tm_printf("[rx] radio open err=%d\n", dd);
        tk_slp_tsk(TMO_FEVR);
    }

    /* Make sure these match your TX and/or CODAL defaults */
    radio_set_group(1);   // PREFIX0 = 0x01
    radio_set_band(7);    // 2407 MHz

    tm_printf("[rx] listening. band=7, group=1, IV=0x18, whitening=ON\n");

    UB buf[64];
    SYSTIM last_tick = {0};
    (void)tk_get_tim(&last_tick);

    for (;;) {
        SZ n = tk_rea_dev(dd, 0, buf, (SZ)(sizeof(buf) - 1), 1000); // 1s timeout
        if (n > 0) {
            buf[n] = '\0';
            tm_printf("[rx] %ldB: %s\n", (long)n, buf);
            (void)tk_get_tim(&last_tick);
        } else {
            SYSTIM now = {0};
            (void)tk_get_tim(&now);

            // Heartbeat every ~2s
            if ((now.lo - last_tick.lo) >= 2000) {
                tm_printf("[rx] alive. END=%lu OK=%lu BAD=%lu lastLen=%u RSSI=%d\n",
                          g_evt_end, g_crc_ok, g_crc_bad, g_last_len, (int)g_last_rssi);
                last_tick = now;
            }
        }
    }
}

EXPORT INT usermain(void)
{
    radio_driver_init();

    T_CTSK ctsk = {0};
    ctsk.tskatr  = TA_HLNG | TA_RNG0;
    ctsk.task    = (FP)receiver_task;
    ctsk.itskpri = 10;
    ctsk.stksz   = 1024;
    ID tskid = tk_cre_tsk(&ctsk);
    tk_sta_tsk(tskid, 0);

    tk_slp_tsk(TMO_FEVR);
    return 0;
}
