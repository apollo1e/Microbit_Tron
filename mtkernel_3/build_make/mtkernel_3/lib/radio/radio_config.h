#ifndef RADIO_CONFIG_H
#define RADIO_CONFIG_H

// Frequency channel 7 corresponds to 2407 MHz (base: 2400 MHz + channel)
#define RADIO_FREQUENCY_MHZ 7

// Default base address and prefix used for both TX/RX
#define RADIO_BASE_ADDRESS  0xE7E7E7E7
#define RADIO_PREFIX_BYTE   0xE7
#define MICROBIT_RADIO_MAX_PACKET 32

#endif // RADIO_CONFIG_H
