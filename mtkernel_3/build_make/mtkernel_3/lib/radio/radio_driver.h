#ifndef RADIO_DRIVER_H
#define RADIO_DRIVER_H

#include <tk/tkernel.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RADIO_MAX_PAYLOAD_LEN 32

// Initializes the radio hardware with basic 1Mbps config
void radio_init(void);

// Sends a raw radio packet (max 32 bytes)
void radio_send(const UB *data, INT len);

// Blocks until a packet is received, returns length (0 = none)
INT radio_receive(UB *out_buf);

#ifdef __cplusplus
}
#endif

#endif // RADIO_DRIVER_H
