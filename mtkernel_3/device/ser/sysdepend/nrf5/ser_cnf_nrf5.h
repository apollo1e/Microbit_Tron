/*
 *----------------------------------------------------------------------
 *    Device Driver for micro T-Kernel for μT-Kernel 3.00.05
 *
 *    Copyright (C) 2020-2022 by Ken Sakamura.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 *
 *    Released by TRON Forum(http://www.tron.org) at 2022/05.
 *
 *----------------------------------------------------------------------
 */

/*
 *	ser_cnf_nrf5.h
 *	Serial Device configuration file
 *		for nRF5
 */
#ifndef	__DEV_SER_CNF_NRF5_H__
#define	__DEV_SER_CNF_NRF5_H__

/* Device control data */
#define	DEVCNF_SER_INTPRI	5		// Interrupt priority

/* Debug option
 *	Specify the device used by T-Monitor.
 *	  0: "sera" - UART0
 *	  other : T-Monitor does not use serial devices
 */
#if USE_TMONITOR
#define	DEVCNF_SER_DBGUN	0		// Used by T-Monitor
#else
#define	DEVCNF_SER_DBGUN	(-1)		// T-Monitor not executed
#endif

#endif /* __DEV_SER_CNF_NRF5_H__ */
