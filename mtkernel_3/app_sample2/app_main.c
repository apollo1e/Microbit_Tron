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
#include <tk/tkernel.h>
#include <tk/syslib.h>   // for tk_def_int
#include <tm/tmonitor.h>
#include <stdint.h>

IMPORT void radio_init(void);
IMPORT void radio_send(unsigned char *data, unsigned char length);
extern void RADIO_IRQHandler(void);  // Declare your radio ISR

#define NVIC_ISER0 (*(volatile uint32_t *)0xE000E100)  // IRQ enable register

EXPORT void usermain(void) {
    tm_printf("=== Sendering code  ===\n");


    // Register the RADIO interrupt handler with the kernel
    T_DINT dint;
    dint.intatr = TA_HLNG;               // High-level language interrupt
    dint.inthdr = (FP)RADIO_IRQHandler;  // Your defined ISR function
    tk_def_int(1, &dint);                // IRQ number 1 = RADIO IRQ

    radio_init();
    tk_dly_tsk(100); // Allow radio to stabilize

    unsigned char msg[] = "hi";
    int count = 0;

    while (1) {
        tm_printf("Loop #%d: Sending...\n", count);
        radio_send(msg, 2);
        tm_printf("Loop #%d: Sent: %s\n", count, msg);
        count++;
        tk_dly_tsk(1000); // Delay 1 second
    }
}
