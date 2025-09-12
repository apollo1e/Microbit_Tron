/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.05
 *
 *    Copyright (C) 2006-2022 by Ken Sakamura.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 *
 *    Released by TRON Forum(http://www.tron.org) at 2022/05.
 *
 *----------------------------------------------------------------------
 */

/*
 *	profile.h
 *
 *	Service Profile (micro:bit depended)
 */

#ifndef __SYS_DEPEND_PROFILE_H__
#define __SYS_DEPEND_PROFILE_H__

/*
 **** CPU-depeneded profile (nRF5)
 */
#include "../cpu/nrf5/profile.h"

/*
 **** Target-depeneded profile (micro:bit)
 */

/*
 * Power management
 */
#define TK_SUPPORT_LOWPOWER	FALSE	/* Support of power management */



#endif /* __SYS_DEPEND_PROFILE_H__ */
