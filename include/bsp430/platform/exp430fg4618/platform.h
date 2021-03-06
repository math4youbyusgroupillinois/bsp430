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

#ifndef BSP430_PLATFORM_EXP430FG4618_PLATFORM_H
#define BSP430_PLATFORM_EXP430FG4618_PLATFORM_H

/** @file
 *
 * @brief Platform-specific include for <a href="http://www.ti.com/tool/msp-exp430fg4618">MSP-EXP430FG4618</a>
 *
 * The following platform-specific features are supported:
 * <ul>
 *
 * <li> #vBSP430platformSpinForJumper_ni is not implemented on this
 * platform.
 *
 * <li> #BSP430_PERIPH_EXPOSED_CLOCKS Clocks are exposed on header H2,
 * just below the FG4618 JTAG header.  MCLK is available on H2.2
 * (P1.1), SMCLK on H2.5 (P1.4), and ACLK on H2.6 (P1.5)
 *
 * </ul>
 */

/** Unconditionally define this, so as to produce errors if there is a
 * conflict in definition. */
#define BSP430_PLATFORM_EXP430FG4618 1

/** @cond DOXYGEN_EXCLUDE */

/* Available button definitions */
#define BSP430_PLATFORM_BUTTON0_PORT_PERIPH_HANDLE BSP430_PERIPH_PORT1
#define BSP430_PLATFORM_BUTTON0_PORT_BIT BIT0
#define BSP430_PLATFORM_BUTTON1_PORT_PERIPH_HANDLE BSP430_PERIPH_PORT1
#define BSP430_PLATFORM_BUTTON1_PORT_BIT BIT2

/* Standard LED colors */
#define BSP430_LED_GREEN 0
#define BSP430_LED_YELLOW 1
#define BSP430_LED_RED 3

/* How to use ACLK as a capture/compare input source */
/* Settings for TB0: T0B6 ccis=1 ; clk P1.4 ; cc0 P2.1 ; cc1 P2.2 */
#ifndef BSP430_TIMER_CCACLK_ACLK_CCIDX
/* NB: Check against BSP430_TIMER_CCACLK_PERIPH_CPPID in bsp430_config.h */
#define BSP430_TIMER_CCACLK_ACLK_CCIDX 6
#endif /* BSP430_TIMER_CCACLK_ACLK_CCIDX */
#ifndef BSP430_TIMER_CCACLK_ACLK_CCIS
/* NB: Check against BSP430_TIMER_CCACLK_PERIPH_CPPID in bsp430_config.h */
#define BSP430_TIMER_CCACLK_ACLK_CCIS CCIS_1
#endif /* BSP430_TIMER_CCACLK_ACLK_CCIS */
#ifndef BSP430_TIMER_CCACLK_CLK_PORT_BIT
/* NB: Check against BSP430_TIMER_CCACLK_CLK_PORT_PERIPH_CPPID in bsp430_config.h */
#define BSP430_TIMER_CCACLK_CLK_PORT_BIT BIT4
#endif /* BSP430_TIMER_CCACLK_CLK_PORT_BIT */
#ifndef BSP430_TIMER_CCACLK_CC0_PORT_BIT
/* NB: Check against BSP430_TIMER_CCACLK_CC0_PORT_PERIPH_CPPID in bsp430_config.h */
#define BSP430_TIMER_CCACLK_CC0_PORT_BIT BIT1
#endif /* BSP430_TIMER_CCACLK_CC0_PORT_BIT */
#ifndef BSP430_TIMER_CCACLK_CC1_PORT_BIT
/* NB: Check against BSP430_TIMER_CCACLK_CC1_PORT_PERIPH_CPPID in bsp430_config.h */
#define BSP430_TIMER_CCACLK_CC1_PORT_BIT BIT2
#endif /* BSP430_TIMER_CCACLK_CC1_PORT_BIT */

/* !BSP430! insert=rfem_platform platform=exp430fg4618 mcu=msp430fg4618 */
/* BEGIN AUTOMATICALLY GENERATED CODE---DO NOT MODIFY [rfem_platform] */
#if (configBSP430_RFEM - 0)
#define BSP430_RFEM 1
#endif /* configBSP430_RFEM */
#if (BSP430_RFEM - 0)
#define BSP430_RFEM_RF1P3_PORT_PERIPH_HANDLE BSP430_PERIPH_PORT1
#define BSP430_RFEM_RF1P3_PORT_BIT BIT5
#define BSP430_RFEM_RF1P5_PORT_PERIPH_HANDLE BSP430_PERIPH_PORT1
#define BSP430_RFEM_RF1P5_PORT_BIT BIT4
#define BSP430_RFEM_RF1P6_PORT_PERIPH_HANDLE BSP430_PERIPH_PORT1
#define BSP430_RFEM_RF1P6_PORT_BIT BIT6
#define BSP430_RFEM_RF1P8_PORT_PERIPH_HANDLE BSP430_PERIPH_PORT1
#define BSP430_RFEM_RF1P8_PORT_BIT BIT7
#define BSP430_RFEM_RF1P10_PORT_PERIPH_HANDLE BSP430_PERIPH_PORT1
#define BSP430_RFEM_RF1P10_PORT_BIT BIT2
#define BSP430_RFEM_RF1P10_TIMER_PERIPH_HANDLE BSP430_PERIPH_TA0
#define BSP430_RFEM_RF1P10_TIMER_CCIDX 1
#define BSP430_RFEM_RF1P10_TIMER_CCIS CCIS_0
#define BSP430_RFEM_RF1P12_PORT_PERIPH_HANDLE BSP430_PERIPH_PORT1
#define BSP430_RFEM_RF1P12_PORT_BIT BIT3
#define BSP430_RFEM_RF1P14_PORT_PERIPH_HANDLE BSP430_PERIPH_PORT4
#define BSP430_RFEM_RF1P14_PORT_BIT BIT2
#define BSP430_RFEM_RF1P16_PORT_PERIPH_HANDLE BSP430_PERIPH_PORT4
#define BSP430_RFEM_RF1P16_PORT_BIT BIT5
#define BSP430_RFEM_RF1P18_PORT_PERIPH_HANDLE BSP430_PERIPH_PORT4
#define BSP430_RFEM_RF1P18_PORT_BIT BIT3
#define BSP430_RFEM_RF1P20_PORT_PERIPH_HANDLE BSP430_PERIPH_PORT4
#define BSP430_RFEM_RF1P20_PORT_BIT BIT4
#endif /* BSP430_RFEM */
/* END AUTOMATICALLY GENERATED CODE [rfem_platform] */
/* !BSP430! end=rfem_platform */

/** @endcond */

/* Include generic file, in case this is being included directly */
#include <bsp430/platform.h>

#endif /* BSP430_PLATFORM_EXP430FG4618_H */
