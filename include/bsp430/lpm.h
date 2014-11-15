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
 * @brief Support for low power mode execution.
 *
 * #SCG1 : Status register bit that, when set, turns off the DCO generator if
 * DCOCLK is not used for MCLK or SMCLK.  It is set in LPM modes 2, 3,
 * and 4.
 *
 * #SCG0 : Status register bit that, when set, turns off FLL loop
 * control.  It is set in LPM modes 1, 3, and 4.
 *
 * #OSCOFF : Status register bit that, when set, turns off the LFXT1
 * crystal oscillator when LFXT1CLK is not used for MCLK or SMCLK.  It
 * is set in LPM mode 4 and in LPMx.5.
 *
 * #CPUOFF : Status register bit that, when set, turns off the CPU.  It
 * is set in all LPM modes.
 *
 * #GIE : Status register bit that, when set, enables maskable
 * interrrupts.  It should be set in any LPM mode for which exit is
 * signalled by an interrupt (which is most of them).  It may be left
 * set on wakeup if interrupts are normally disabled in the main
 * application.
 *
 * @homepage http://github.com/pabigot/bsp430
 * @copyright Copyright 2012-2014, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#ifndef BSP430_LPM_H
#define BSP430_LPM_H

#include <bsp430/core.h>
#include <msp430.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Reset all port pins for low power applications.
 *
 * This function should be invoked at application power-up so that
 * only peripheral pins required by the application are in a state
 * that consumes energy.  Without doing this, connected pins tend to
 * float and keep the MCU power consumption much higher than it should
 * be.
 *
 * All digital I/O ports on the MCU (PORTA through PORTJ or their
 * 8-bit equivalents, PORT1 through PORT12) are configured to their
 * i/o function, output direction, and outputs set to zero.
 */
void vBSP430lpmConfigurePortsForLPM_ni (void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BSP430_LPM_H */
