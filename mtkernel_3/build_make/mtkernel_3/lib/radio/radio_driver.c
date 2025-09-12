#include <tk/tkernel.h>
#include <tm/tmonitor.h>

typedef unsigned char  uint8_t;
typedef signed char    int8_t;
typedef unsigned int   uint32_t;
typedef int            bool;

#define true 1
#define false 0

// Constants for radio configuration
#define MICROBIT_RADIO_BASE_ADDRESS  0x75626974
#define MICROBIT_RADIO_GROUP         1
#define MICROBIT_RADIO_MAX_PACKET    32

#define NRF_RADIO_BASE   0x40001000
#define NRF_CLOCK_BASE   0x40000000

#define NRF_RADIO        ((NRF_RADIO_Type *)NRF_RADIO_BASE)
#define NRF_CLOCK        ((NRF_CLOCK_Type *)NRF_CLOCK_BASE)

volatile int last_rssi = 0;
volatile bool packet_received = false;

// Minimal type definitions for needed peripherals
typedef struct {
    uint32_t TASKS_HFCLKSTART;
    uint32_t TASKS_HFCLKSTOP;
    uint32_t RESERVED0[62];
    uint32_t EVENTS_HFCLKSTARTED;
} NRF_CLOCK_Type;

typedef struct {
    uint32_t TASKS_TXEN;
    uint32_t TASKS_RXEN;
    uint32_t TASKS_START;
    uint32_t TASKS_STOP;
    uint32_t TASKS_DISABLE;
    uint32_t RESERVED0[59];
    uint32_t EVENTS_READY;
    uint32_t EVENTS_END;
    uint32_t EVENTS_DISABLED;
    uint32_t RESERVED1[125];
    uint32_t SHORTS;
    uint32_t INTENSET;
    uint32_t INTENCLR;
    uint32_t CRCSTATUS;
    uint32_t RXMATCH;
    uint32_t RXCRC;
    uint32_t DAI;
    uint32_t PACKETPTR;
    uint32_t FREQUENCY;
    uint32_t TXPOWER;
    uint32_t MODE;
    uint32_t PCNF0;
    uint32_t PCNF1;
    uint32_t BASE0;
    uint32_t BASE1;
    uint32_t PREFIX0;
    uint32_t PREFIX1;
    uint32_t TXADDRESS;
    uint32_t RXADDRESSES;
    uint32_t CRCINIT;
    uint32_t CRCPOLY;
    uint32_t CRCCNF;
    uint32_t TEST;
    uint32_t TIFS;
    uint32_t RSSISAMPLE;
    // More fields omitted

    uint32_t RESERVED2[40];  // Add this line
    uint32_t STATE;          // Add this line


} NRF_RADIO_Type;

// Packet structure for micro:bit custom radio
typedef struct FrameBuffer {
    uint8_t length;
    uint8_t version;
    uint8_t group;
    uint8_t protocol;
    uint8_t payload[MICROBIT_RADIO_MAX_PACKET];
    int8_t  rssi;
} FrameBuffer;

// Global buffers (aligned for DMA use)
static FrameBuffer rx_buf __attribute__((aligned(4)));
static FrameBuffer tx_buf __attribute__((aligned(4)));
static FrameBuffer received_frame;

// Call once at startup on both sender and receiver
EXPORT void radio_init(void) {
    // Start high frequency clock needed for radio
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART = 1;
    while (!NRF_CLOCK->EVENTS_HFCLKSTARTED);

    // Enable RADIO interrupt (IRQ 1) and enable EVENTS_END interrupt
    NRF_RADIO->INTENSET = (1 << 3);  // Bit 3 = EVENTS_END
    *(volatile uint32_t *)0xE000E100 = (1 << 1); // Enable interrupt 1 in NVIC

    // Configure basic radio settings
    NRF_RADIO->TXPOWER   = 0x00;  // 0 dBm
    NRF_RADIO->FREQUENCY = 7;     // Channel 7 = 2407 MHz
    NRF_RADIO->MODE      = 1;     // 1 Mbps

    // Set base address and prefix
    NRF_RADIO->BASE0 = MICROBIT_RADIO_BASE_ADDRESS;
    NRF_RADIO->PREFIX0 = MICROBIT_RADIO_GROUP;

    // Use logical address 0 for TX and RX
    NRF_RADIO->TXADDRESS = 0;
    NRF_RADIO->RXADDRESSES = 1;

    // Packet configuration
    NRF_RADIO->PCNF0 = (8 << 0); // LFLEN = 8 bits
    NRF_RADIO->PCNF1 = (1 << 25) | (4 << 16) | (MICROBIT_RADIO_MAX_PACKET << 0);

    // CRC configuration
    NRF_RADIO->CRCCNF  = 1;
    NRF_RADIO->CRCINIT = 0xFFFF;
    NRF_RADIO->CRCPOLY = 0x11021;

    // Enable SHORT: READY → START for automatic RX start
    NRF_RADIO->SHORTS = (1 << 0);  // READY -> START

    // Set RX buffer
    NRF_RADIO->PACKETPTR = (uint32_t)&rx_buf;

    // Start receiving
    NRF_RADIO->TASKS_RXEN = 1;
    while (!NRF_RADIO->EVENTS_READY);
    NRF_RADIO->EVENTS_READY = 0;
    NRF_RADIO->TASKS_START = 1;
}

// Send data (blocking)
EXPORT void radio_send(uint8_t *data, uint8_t length) {
    if (length > MICROBIT_RADIO_MAX_PACKET) return;

    // Fill the transmit buffer with header and payload
    tx_buf.length = length + 3;  // Includes version, group, protocol
    tx_buf.version = 1;
    tx_buf.group = MICROBIT_RADIO_GROUP;
    tx_buf.protocol = 1;
    for (int i = 0; i < length; ++i)
        tx_buf.payload[i] = data[i];

    // --- Step 1: Ensure radio is in DISABLED state ---

    tm_printf("DEBUG: Forcing radio to disable state...\n");

    // Stop any ongoing operation just in case
    NRF_RADIO->TASKS_STOP = 1;
    tk_dly_tsk(1);  // Short wait to ensure STOP takes effect

    // Reset all relevant event flags
    NRF_RADIO->SHORTS = 0;
    NRF_RADIO->EVENTS_READY = 0;
    NRF_RADIO->EVENTS_END = 0;
    NRF_RADIO->EVENTS_DISABLED = 0;

    // Send DISABLE command
    NRF_RADIO->TASKS_DISABLE = 1;

    // Wait until the radio is truly disabled (STATE == 0)
    int timeout = 100000;
    while ((NRF_RADIO->STATE != 0) && --timeout);
    if (timeout <= 0) {
        tm_printf("ERROR: Radio failed to enter DISABLED state!\n");
        return;
    }
    tm_printf("DEBUG: Radio is now DISABLED\n");

    // --- Step 2: Setup TX ---

    NRF_RADIO->PACKETPTR = (uint32_t)&tx_buf;
    NRF_RADIO->EVENTS_READY = 0;
    NRF_RADIO->EVENTS_END = 0;

    // Start TX mode
    tm_printf("DEBUG: Enabling TX mode...\n");
    NRF_RADIO->TASKS_TXEN = 1;

    // Wait until radio is ready
    timeout = 100000;
    while (!NRF_RADIO->EVENTS_READY && --timeout);
    if (timeout <= 0) {
        tm_printf("ERROR: TX READY TIMEOUT!\n");
        return;
    }
    NRF_RADIO->EVENTS_READY = 0;

    // Start transmission
    tm_printf("DEBUG: Starting transmission...\n");
    NRF_RADIO->TASKS_START = 1;

    // Wait for TX to complete
    timeout = 100000;
    while (!NRF_RADIO->EVENTS_END && --timeout);
    if (timeout <= 0) {
        tm_printf("ERROR: TX END TIMEOUT!\n");
        return;
    }
    NRF_RADIO->EVENTS_END = 0;

    tm_printf("DEBUG: Transmission complete\n");

    // --- Step 3: Return to RX mode ---

    // Disable TX mode
    NRF_RADIO->TASKS_DISABLE = 1;
    timeout = 100000;
    while ((NRF_RADIO->STATE != 0) && --timeout);
    if (timeout <= 0) {
        tm_printf("ERROR: Radio failed to DISABLE after TX!\n");
        return;
    }
    NRF_RADIO->EVENTS_DISABLED = 0;

    // Prepare RX mode
    NRF_RADIO->PACKETPTR = (uint32_t)&rx_buf;
    NRF_RADIO->EVENTS_READY = 0;

    NRF_RADIO->TASKS_RXEN = 1;
    timeout = 100000;
    while (!NRF_RADIO->EVENTS_READY && --timeout);
    if (timeout <= 0) {
        tm_printf("ERROR: RXEN READY TIMEOUT!\n");
        return;
    }
    NRF_RADIO->EVENTS_READY = 0;
    NRF_RADIO->TASKS_START = 1;

    tm_printf("DEBUG: Returned to RX mode\n");
}



// Called in main loop to poll for received packet
EXPORT int radio_receive(uint8_t *out_buf) {
    if (!packet_received) return 0;

    packet_received = false;

    int payload_len = received_frame.length - 3;
    if (payload_len > MICROBIT_RADIO_MAX_PACKET) payload_len = MICROBIT_RADIO_MAX_PACKET;

    for (int i = 0; i < payload_len; i++)
        out_buf[i] = received_frame.payload[i];

    return payload_len;
}

// RADIO IRQ handler (triggered on EVENTS_END)
EXPORT void RADIO_IRQHandler(void) {
    if (NRF_RADIO->EVENTS_END) {
        NRF_RADIO->EVENTS_END = 0;
        NRF_RADIO->EVENTS_DISABLED = 0;

        if (NRF_RADIO->CRCSTATUS == 1) {
            last_rssi = -((int)NRF_RADIO->RSSISAMPLE);
            received_frame = rx_buf;
            packet_received = true;
        }

        // Prepare for next reception
        NRF_RADIO->PACKETPTR = (uint32_t)&rx_buf;
        NRF_RADIO->TASKS_START = 1;
    }
}
