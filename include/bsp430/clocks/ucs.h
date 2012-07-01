/* Copyright (c) 2012, Peter A. Bigot <bigotp@acm.org>
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
 * This module supports the Unified Clock System (UCS) and UCS_RF
 * peripherals, which are present in 5xx/6xx-family devices.
 *
 * UCS_RF differs from UCS by adding support for XT2.  This difference
 * is currently ignored.
 *
 * Based on experimentation, the following is assumed or enforced for
 * all supported clock configurations:
 * 
 * @li SELREF is XT1CLK running at 32768 Hz.  The configuration
 *     routine expects the XIN/XOUT pins to have been configured for
 *     their peripheral function prior to being invoked, but will do
 *     the XT1 enabling and fault checking for you.
 *
 * @li FLLD is consistently set to /2, which is the PUC value and
 *     which is adequate to support speeds up to 32 MiHz with a 32768
 *     Hz REFCLK.
 *
 * @li FLLREFDIV is consistently set to /1, which is the PUC value.
 *
 * @li MCLK and SMCLK are set to DCOCLKDIV, and ACLK to XT1CLK.
 *
 * @li Due to UCS10 and UCS7, normal practice on UCS-based MCUs is to
 *     leave the FLL disabled, and adjust it periodically when the
 *     clocks are otherwise unused.  Thus SCG0 is expected to be set
 *     at all times except when trimming.
 *
 * Other refinements in this module:
 *
 * @li The return value of #ulBSP430clockMCLK_Hz is the most recent
 * measured trimmed frequency.
 * 
 * @li The return value of #ulBSP430clockSMCLK_Hz is the most recent
 * measured trimmed frequency shifted right by
 * #configBSP430_CLOCK_SMCLK_DIVIDING_SHIFT.

 * @author Peter A. Bigot <bigotp@acm.org>
 * @homepage http://github.com/pabigot/freertos-mspgcc
 * @date 2012
 * @copyright <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#ifndef BSP430_CLOCKS_UCS_H
#define BSP430_CLOCKS_UCS_H

#include "FreeRTOS.h"
#include <bsp430/common/clocks.h>

/** Call this to initially configure the UCS peripheral.
 *
 * @param ulFrequency_Hz The target frequency for DCOCLKDIV=MCLK.
 * SMCLK will be set to DCOCLKDIV divided in accordance with
 * #configBSP430_CLOCK_SMCLK_DIVIDING_SHIFT.
 *
 * @param sRSEL The DCO frequency range selection.  The appropriate
 * value is frequency-dependent, and specified in the "DCO Frequency"
 * table in the device-specific data sheet.  The same target frequency
 * may be reachable with different RSELs but with different
 * accuracies.  If a negative value is given, the function may be able
 * to select a default from an internal table if that table has been
 * updated to include information on the relevant device.
 * 
 * @return an estimate of the actual running frequency.
 *
 * @note This function expects a valid clock source on XT1.  Port pin
 * configuration for XIN and XOUT is device specific and should be
 * done prior to invoking this function. */

unsigned long ulBSP430ucsConfigure ( unsigned long ulFrequency_Hz,
									 short sRSEL );

/** Call this periodically to trim the FLL.
 *
 * The function uses TB0 to determine the speed of SMCLK, and if it is
 * "too far" from the value specified in the last call to
 * ulBSP430ucsConfigure() enables the FLL for a short period to see if
 * accuracy can be improved.
 *
 * @warning MCLK, SMCLK, and any clocks derived from them are unstable
 * while this routine is being run, so UART, SPI, and other
 * peripherals may need to be shut down first.
 * 
 * @note This function is named in accordance with the FreeRTOS
 * standards that indicate it should be called with interrupts
 * disabled and will not block.  It will, however, delay for as much
 * as 32 milliseconds while waiting for the FLL to settle.  The common
 * case of delay is much less, but it would still be wise not to
 * invoke this from within an interrupt handler.
 *
 * @return an estimate of the actual running frequency.
 */
unsigned long ulBSP430ucsTrimFLLFromISR ();

#endif /* BSP430_CLOCKS_UCS_H */

