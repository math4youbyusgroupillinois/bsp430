/* Copyright 2012-2014, Peter A. Bigot
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the software nor the names of its contributors may be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/** @file
 *
 * @brief Platform-specific include for <a href="http://www.ti.com/tool/msp-fet430u48c">MSP430FR5969 in a TS430RGZ48C carrier</a>.
 *
 * The following platform-specific features are supported: None
 *
 * @homepage http://github.com/pabigot/bsp430
 * @copyright Copyright 2012-2014, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */
#ifndef BSP430_PLATFORM_WOLVERINE_PLATFORM_H
#define BSP430_PLATFORM_WOLVERINE_PLATFORM_H

/** Unconditionally define this, so as to produce errors if there is a
 * conflict in definition. */
#define BSP430_PLATFORM_WOLVERINE 1

/** @cond DOXYGEN_EXCLUDE */

/* SW2 on P1.3.  Not necessarily populated.  When not populated, can
 * sometimes be faked by shorting the south pad of R14 to the north
 * pad of SW2, but result may leave button pressed. */
#define BSP430_PLATFORM_BUTTON0_PORT_PERIPH_HANDLE BSP430_PERIPH_PORT1
#define BSP430_PLATFORM_BUTTON0_PORT_BIT BIT3

/* One LED on P1.0.  Unpopulated LEDs on P1.1 and P1.2. */
#define BSP430_LED_GREEN 0
#define BSP430_LED_RED 1
#define BSP430_LED_GREEN1 2

/* How to use ACLK as a capture/compare input source */
/* Settings for TA1: T1A2 ccis=1 ; clk P1.1 ; cc0 P1.7 ; cc1 P1.2 */
#ifndef BSP430_TIMER_CCACLK_ACLK_CCIDX
/* NB: Check against BSP430_TIMER_CCACLK_PERIPH_CPPID in bsp430_config.h */
#define BSP430_TIMER_CCACLK_ACLK_CCIDX 2
#endif /* BSP430_TIMER_CCACLK_ACLK_CCIDX */
#ifndef BSP430_TIMER_CCACLK_ACLK_CCIS
/* NB: Check against BSP430_TIMER_CCACLK_PERIPH_CPPID in bsp430_config.h */
#define BSP430_TIMER_CCACLK_ACLK_CCIS CCIS_1
#endif /* BSP430_TIMER_CCACLK_ACLK_CCIS */
#ifndef BSP430_TIMER_CCACLK_CLK_PORT_BIT
/* NB: Check against BSP430_TIMER_CCACLK_CLK_PORT_PERIPH_CPPID in bsp430_config.h */
#define BSP430_TIMER_CCACLK_CLK_PORT_BIT BIT1
#endif /* BSP430_TIMER_CCACLK_CLK_PORT_BIT */
#ifndef BSP430_TIMER_CCACLK_CC0_PORT_BIT
/* NB: Check against BSP430_TIMER_CCACLK_CC0_PORT_PERIPH_CPPID in bsp430_config.h */
#define BSP430_TIMER_CCACLK_CC0_PORT_BIT BIT7
#endif /* BSP430_TIMER_CCACLK_CC0_PORT_BIT */
#ifndef BSP430_TIMER_CCACLK_CC1_PORT_BIT
/* NB: Check against BSP430_TIMER_CCACLK_CC1_PORT_PERIPH_CPPID in bsp430_config.h */
#define BSP430_TIMER_CCACLK_CC1_PORT_BIT BIT2
#endif /* BSP430_TIMER_CCACLK_CC1_PORT_BIT */

/** @endcond */

/* Include generic file, in case this is being included directly */
#include <bsp430/platform.h>

#endif /* BSP430_PLATFORM_WOLVERINE_H */
