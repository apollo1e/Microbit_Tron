#pragma once
#include <tk/tkernel.h>
#include <stdint.h>


/* Public device name */
#define RADIO_DEVNAME   "radio"

/* Defaults (mirrors micro:bit runtime behavior: 1 Mbps, ~32B MTU, band 7 ≈ 2407 MHz) */
#define RADIO_DEFAULT_BAND      7    /* 0..100 -> 2400 + band MHz (DAL uses 2407 MHz typical). */
#define RADIO_DEFAULT_GROUP     1    /* micro:bit default group is 0. */
#define RADIO_DEFAULT_TXPOWER   0    /* 0..7 per docs; 0 ≈ lowest, 7 ≈ highest. */

/* Driver init: registers the device and sets sane defaults. Call once from usermain(). */
#ifdef __cplusplus
extern "C" {
#endif

void radio_driver_init(void);

/* Optional runtime controls (match DAL API semantics) */
int  radio_set_group(uint8_t group);      /* DAL setGroup() – filters address/prefix by group.  */
int  radio_set_band(uint8_t band);        /* DAL setFrequencyBand(0..100) (2400+band MHz).      */
int  radio_set_txpower(uint8_t power);    /* DAL setTransmitPower(0..7).                        */

#ifdef __cplusplus
}
#endif
