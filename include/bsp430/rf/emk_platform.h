/* Copyright 2013, Peter A. Bigot
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
 * @brief Entrypoint for platform-specific capabilities.
 * @homepage http://github.com/pabigot/bsp430
 * @copyright Copyright 2013, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

#ifndef BSP430_RF_EMKPLATFORM_H
#define BSP430_RF_EMKPLATFORM_H

/* !BSP430! insert=emk_platform emk=cc110xemk tag=cc110x gpio=GDO0,GDO1,GDO2 */
/* BEGIN AUTOMATICALLY GENERATED CODE---DO NOT MODIFY [emk_platform] */
#if (configBSP430_RF_CC110XEMK - 0)
#define BSP430_RF_CC110X 1
#define BSP430_RF_CC110X_SPI_PERIPH_HANDLE BSP430_RFEM_SERIAL_PERIPH_HANDLE
#if defined(BSP430_RFEM_RF1P10_PORT_PERIPH_HANDLE)
#define BSP430_RF_CC110X_GDO0_PORT_PERIPH_HANDLE BSP430_RFEM_RF1P10_PORT_PERIPH_HANDLE
#endif /* BSP430_RFEM_RF1P10_PORT_PERIPH_HANDLE */
#if defined(BSP430_RFEM_RF1P10_PORT_BIT)
#define BSP430_RF_CC110X_GDO0_PORT_BIT BSP430_RFEM_RF1P10_PORT_BIT
#endif /* BSP430_RFEM_RF1P10_PORT_BIT */
#if defined(BSP430_RFEM_RF1P10_TIMER_PERIPH_HANDLE)
#define BSP430_RF_CC110X_GDO0_TIMER_PERIPH_HANDLE BSP430_RFEM_RF1P10_TIMER_PERIPH_HANDLE
#endif /* BSP430_RFEM_RF1P10_TIMER_PERIPH_HANDLE */
#if defined(BSP430_RFEM_RF1P10_TIMER_CCIDX)
#define BSP430_RF_CC110X_GDO0_TIMER_CCIDX BSP430_RFEM_RF1P10_TIMER_CCIDX
#endif /* BSP430_RFEM_RF1P10_TIMER_CCIDX */
#if defined(BSP430_RFEM_RF1P10_TIMER_CCIS)
#define BSP430_RF_CC110X_GDO0_TIMER_CCIS BSP430_RFEM_RF1P10_TIMER_CCIS
#endif /* BSP430_RFEM_RF1P10_TIMER_CCIS */
#if defined(BSP430_RFEM_RF1P20_PORT_PERIPH_HANDLE)
#define BSP430_RF_CC110X_GDO1_PORT_PERIPH_HANDLE BSP430_RFEM_RF1P20_PORT_PERIPH_HANDLE
#endif /* BSP430_RFEM_RF1P20_PORT_PERIPH_HANDLE */
#if defined(BSP430_RFEM_RF1P20_PORT_BIT)
#define BSP430_RF_CC110X_GDO1_PORT_BIT BSP430_RFEM_RF1P20_PORT_BIT
#endif /* BSP430_RFEM_RF1P20_PORT_BIT */
#if defined(BSP430_RFEM_RF1P20_TIMER_PERIPH_HANDLE)
#define BSP430_RF_CC110X_GDO1_TIMER_PERIPH_HANDLE BSP430_RFEM_RF1P20_TIMER_PERIPH_HANDLE
#endif /* BSP430_RFEM_RF1P20_TIMER_PERIPH_HANDLE */
#if defined(BSP430_RFEM_RF1P20_TIMER_CCIDX)
#define BSP430_RF_CC110X_GDO1_TIMER_CCIDX BSP430_RFEM_RF1P20_TIMER_CCIDX
#endif /* BSP430_RFEM_RF1P20_TIMER_CCIDX */
#if defined(BSP430_RFEM_RF1P20_TIMER_CCIS)
#define BSP430_RF_CC110X_GDO1_TIMER_CCIS BSP430_RFEM_RF1P20_TIMER_CCIS
#endif /* BSP430_RFEM_RF1P20_TIMER_CCIS */
#if defined(BSP430_RFEM_RF1P12_PORT_PERIPH_HANDLE)
#define BSP430_RF_CC110X_GDO2_PORT_PERIPH_HANDLE BSP430_RFEM_RF1P12_PORT_PERIPH_HANDLE
#endif /* BSP430_RFEM_RF1P12_PORT_PERIPH_HANDLE */
#if defined(BSP430_RFEM_RF1P12_PORT_BIT)
#define BSP430_RF_CC110X_GDO2_PORT_BIT BSP430_RFEM_RF1P12_PORT_BIT
#endif /* BSP430_RFEM_RF1P12_PORT_BIT */
#if defined(BSP430_RFEM_RF1P12_TIMER_PERIPH_HANDLE)
#define BSP430_RF_CC110X_GDO2_TIMER_PERIPH_HANDLE BSP430_RFEM_RF1P12_TIMER_PERIPH_HANDLE
#endif /* BSP430_RFEM_RF1P12_TIMER_PERIPH_HANDLE */
#if defined(BSP430_RFEM_RF1P12_TIMER_CCIDX)
#define BSP430_RF_CC110X_GDO2_TIMER_CCIDX BSP430_RFEM_RF1P12_TIMER_CCIDX
#endif /* BSP430_RFEM_RF1P12_TIMER_CCIDX */
#if defined(BSP430_RFEM_RF1P12_TIMER_CCIS)
#define BSP430_RF_CC110X_GDO2_TIMER_CCIS BSP430_RFEM_RF1P12_TIMER_CCIS
#endif /* BSP430_RFEM_RF1P12_TIMER_CCIS */
#if defined(BSP430_RFEM_RF1P14_PORT_PERIPH_HANDLE)
#define BSP430_RF_CC110X_CSn_PORT_PERIPH_HANDLE BSP430_RFEM_RF1P14_PORT_PERIPH_HANDLE
#endif /* BSP430_RFEM_RF1P14_PORT_PERIPH_HANDLE */
#if defined(BSP430_RFEM_RF1P14_PORT_BIT)
#define BSP430_RF_CC110X_CSn_PORT_BIT BSP430_RFEM_RF1P14_PORT_BIT
#endif /* BSP430_RFEM_RF1P14_PORT_BIT */
#endif /* configBSP430_RF_CC110XEMK */
/* END AUTOMATICALLY GENERATED CODE [emk_platform] */
/* !BSP430! end=emk_platform */
/* !BSP430! insert=emk_platform emk=cc1125emk tag=cc1125 gpio=GPIO0,GPIO1,GPIO2,GPIO3 */
/* BEGIN AUTOMATICALLY GENERATED CODE---DO NOT MODIFY [emk_platform] */
#if (configBSP430_RF_CC1125EMK - 0)
#define BSP430_RF_CC1125 1
#define BSP430_RF_CC1125_SPI_PERIPH_HANDLE BSP430_RFEM_SERIAL_PERIPH_HANDLE
#if defined(BSP430_RFEM_RF1P10_PORT_PERIPH_HANDLE)
#define BSP430_RF_CC1125_GPIO0_PORT_PERIPH_HANDLE BSP430_RFEM_RF1P10_PORT_PERIPH_HANDLE
#endif /* BSP430_RFEM_RF1P10_PORT_PERIPH_HANDLE */
#if defined(BSP430_RFEM_RF1P10_PORT_BIT)
#define BSP430_RF_CC1125_GPIO0_PORT_BIT BSP430_RFEM_RF1P10_PORT_BIT
#endif /* BSP430_RFEM_RF1P10_PORT_BIT */
#if defined(BSP430_RFEM_RF1P10_TIMER_PERIPH_HANDLE)
#define BSP430_RF_CC1125_GPIO0_TIMER_PERIPH_HANDLE BSP430_RFEM_RF1P10_TIMER_PERIPH_HANDLE
#endif /* BSP430_RFEM_RF1P10_TIMER_PERIPH_HANDLE */
#if defined(BSP430_RFEM_RF1P10_TIMER_CCIDX)
#define BSP430_RF_CC1125_GPIO0_TIMER_CCIDX BSP430_RFEM_RF1P10_TIMER_CCIDX
#endif /* BSP430_RFEM_RF1P10_TIMER_CCIDX */
#if defined(BSP430_RFEM_RF1P10_TIMER_CCIS)
#define BSP430_RF_CC1125_GPIO0_TIMER_CCIS BSP430_RFEM_RF1P10_TIMER_CCIS
#endif /* BSP430_RFEM_RF1P10_TIMER_CCIS */
#if defined(BSP430_RFEM_RF1P20_PORT_PERIPH_HANDLE)
#define BSP430_RF_CC1125_GPIO1_PORT_PERIPH_HANDLE BSP430_RFEM_RF1P20_PORT_PERIPH_HANDLE
#endif /* BSP430_RFEM_RF1P20_PORT_PERIPH_HANDLE */
#if defined(BSP430_RFEM_RF1P20_PORT_BIT)
#define BSP430_RF_CC1125_GPIO1_PORT_BIT BSP430_RFEM_RF1P20_PORT_BIT
#endif /* BSP430_RFEM_RF1P20_PORT_BIT */
#if defined(BSP430_RFEM_RF1P20_TIMER_PERIPH_HANDLE)
#define BSP430_RF_CC1125_GPIO1_TIMER_PERIPH_HANDLE BSP430_RFEM_RF1P20_TIMER_PERIPH_HANDLE
#endif /* BSP430_RFEM_RF1P20_TIMER_PERIPH_HANDLE */
#if defined(BSP430_RFEM_RF1P20_TIMER_CCIDX)
#define BSP430_RF_CC1125_GPIO1_TIMER_CCIDX BSP430_RFEM_RF1P20_TIMER_CCIDX
#endif /* BSP430_RFEM_RF1P20_TIMER_CCIDX */
#if defined(BSP430_RFEM_RF1P20_TIMER_CCIS)
#define BSP430_RF_CC1125_GPIO1_TIMER_CCIS BSP430_RFEM_RF1P20_TIMER_CCIS
#endif /* BSP430_RFEM_RF1P20_TIMER_CCIS */
#if defined(BSP430_RFEM_RF1P12_PORT_PERIPH_HANDLE)
#define BSP430_RF_CC1125_GPIO2_PORT_PERIPH_HANDLE BSP430_RFEM_RF1P12_PORT_PERIPH_HANDLE
#endif /* BSP430_RFEM_RF1P12_PORT_PERIPH_HANDLE */
#if defined(BSP430_RFEM_RF1P12_PORT_BIT)
#define BSP430_RF_CC1125_GPIO2_PORT_BIT BSP430_RFEM_RF1P12_PORT_BIT
#endif /* BSP430_RFEM_RF1P12_PORT_BIT */
#if defined(BSP430_RFEM_RF1P12_TIMER_PERIPH_HANDLE)
#define BSP430_RF_CC1125_GPIO2_TIMER_PERIPH_HANDLE BSP430_RFEM_RF1P12_TIMER_PERIPH_HANDLE
#endif /* BSP430_RFEM_RF1P12_TIMER_PERIPH_HANDLE */
#if defined(BSP430_RFEM_RF1P12_TIMER_CCIDX)
#define BSP430_RF_CC1125_GPIO2_TIMER_CCIDX BSP430_RFEM_RF1P12_TIMER_CCIDX
#endif /* BSP430_RFEM_RF1P12_TIMER_CCIDX */
#if defined(BSP430_RFEM_RF1P12_TIMER_CCIS)
#define BSP430_RF_CC1125_GPIO2_TIMER_CCIS BSP430_RFEM_RF1P12_TIMER_CCIS
#endif /* BSP430_RFEM_RF1P12_TIMER_CCIS */
#if defined(BSP430_RFEM_RF1P14_PORT_PERIPH_HANDLE)
#define BSP430_RF_CC1125_CSn_PORT_PERIPH_HANDLE BSP430_RFEM_RF1P14_PORT_PERIPH_HANDLE
#endif /* BSP430_RFEM_RF1P14_PORT_PERIPH_HANDLE */
#if defined(BSP430_RFEM_RF1P14_PORT_BIT)
#define BSP430_RF_CC1125_CSn_PORT_BIT BSP430_RFEM_RF1P14_PORT_BIT
#endif /* BSP430_RFEM_RF1P14_PORT_BIT */
#if defined(BSP430_RFEM_RF2P15_PORT_PERIPH_HANDLE)
#define BSP430_RF_CC1125_RESETn_PORT_PERIPH_HANDLE BSP430_RFEM_RF2P15_PORT_PERIPH_HANDLE
#endif /* BSP430_RFEM_RF2P15_PORT_PERIPH_HANDLE */
#if defined(BSP430_RFEM_RF2P15_PORT_BIT)
#define BSP430_RF_CC1125_RESETn_PORT_BIT BSP430_RFEM_RF2P15_PORT_BIT
#endif /* BSP430_RFEM_RF2P15_PORT_BIT */
#if defined(BSP430_RFEM_RF2P18_PORT_PERIPH_HANDLE)
#define BSP430_RF_CC1125_GPIO3_PORT_PERIPH_HANDLE BSP430_RFEM_RF2P18_PORT_PERIPH_HANDLE
#endif /* BSP430_RFEM_RF2P18_PORT_PERIPH_HANDLE */
#if defined(BSP430_RFEM_RF2P18_PORT_BIT)
#define BSP430_RF_CC1125_GPIO3_PORT_BIT BSP430_RFEM_RF2P18_PORT_BIT
#endif /* BSP430_RFEM_RF2P18_PORT_BIT */
#if defined(BSP430_RFEM_RF2P18_TIMER_PERIPH_HANDLE)
#define BSP430_RF_CC1125_GPIO3_TIMER_PERIPH_HANDLE BSP430_RFEM_RF2P18_TIMER_PERIPH_HANDLE
#endif /* BSP430_RFEM_RF2P18_TIMER_PERIPH_HANDLE */
#if defined(BSP430_RFEM_RF2P18_TIMER_CCIDX)
#define BSP430_RF_CC1125_GPIO3_TIMER_CCIDX BSP430_RFEM_RF2P18_TIMER_CCIDX
#endif /* BSP430_RFEM_RF2P18_TIMER_CCIDX */
#if defined(BSP430_RFEM_RF2P18_TIMER_CCIS)
#define BSP430_RF_CC1125_GPIO3_TIMER_CCIS BSP430_RFEM_RF2P18_TIMER_CCIS
#endif /* BSP430_RFEM_RF2P18_TIMER_CCIS */
#endif /* configBSP430_RF_CC1125EMK */
/* END AUTOMATICALLY GENERATED CODE [emk_platform] */
/* !BSP430! end=emk_platform */
/* !BSP430! insert=emk_platform emk=cc2520emk tag=cc2520 gpio=GPIO0,GPIO1,GPIO2,GPIO3,GPIO4,GPIO5 */
/* BEGIN AUTOMATICALLY GENERATED CODE---DO NOT MODIFY [emk_platform] */
#if (configBSP430_RF_CC2520EMK - 0)
#define BSP430_RF_CC2520 1
#define BSP430_RF_CC2520_SPI_PERIPH_HANDLE BSP430_RFEM_SERIAL_PERIPH_HANDLE
#if defined(BSP430_RFEM_RF1P3_PORT_PERIPH_HANDLE)
#define BSP430_RF_CC2520_GPIO0_PORT_PERIPH_HANDLE BSP430_RFEM_RF1P3_PORT_PERIPH_HANDLE
#endif /* BSP430_RFEM_RF1P3_PORT_PERIPH_HANDLE */
#if defined(BSP430_RFEM_RF1P3_PORT_BIT)
#define BSP430_RF_CC2520_GPIO0_PORT_BIT BSP430_RFEM_RF1P3_PORT_BIT
#endif /* BSP430_RFEM_RF1P3_PORT_BIT */
#if defined(BSP430_RFEM_RF1P3_TIMER_PERIPH_HANDLE)
#define BSP430_RF_CC2520_GPIO0_TIMER_PERIPH_HANDLE BSP430_RFEM_RF1P3_TIMER_PERIPH_HANDLE
#endif /* BSP430_RFEM_RF1P3_TIMER_PERIPH_HANDLE */
#if defined(BSP430_RFEM_RF1P3_TIMER_CCIDX)
#define BSP430_RF_CC2520_GPIO0_TIMER_CCIDX BSP430_RFEM_RF1P3_TIMER_CCIDX
#endif /* BSP430_RFEM_RF1P3_TIMER_CCIDX */
#if defined(BSP430_RFEM_RF1P3_TIMER_CCIS)
#define BSP430_RF_CC2520_GPIO0_TIMER_CCIS BSP430_RFEM_RF1P3_TIMER_CCIS
#endif /* BSP430_RFEM_RF1P3_TIMER_CCIS */
#if defined(BSP430_RFEM_RF1P7_PORT_PERIPH_HANDLE)
#define BSP430_RF_CC2520_GPIO1_PORT_PERIPH_HANDLE BSP430_RFEM_RF1P7_PORT_PERIPH_HANDLE
#endif /* BSP430_RFEM_RF1P7_PORT_PERIPH_HANDLE */
#if defined(BSP430_RFEM_RF1P7_PORT_BIT)
#define BSP430_RF_CC2520_GPIO1_PORT_BIT BSP430_RFEM_RF1P7_PORT_BIT
#endif /* BSP430_RFEM_RF1P7_PORT_BIT */
#if defined(BSP430_RFEM_RF1P7_TIMER_PERIPH_HANDLE)
#define BSP430_RF_CC2520_GPIO1_TIMER_PERIPH_HANDLE BSP430_RFEM_RF1P7_TIMER_PERIPH_HANDLE
#endif /* BSP430_RFEM_RF1P7_TIMER_PERIPH_HANDLE */
#if defined(BSP430_RFEM_RF1P7_TIMER_CCIDX)
#define BSP430_RF_CC2520_GPIO1_TIMER_CCIDX BSP430_RFEM_RF1P7_TIMER_CCIDX
#endif /* BSP430_RFEM_RF1P7_TIMER_CCIDX */
#if defined(BSP430_RFEM_RF1P7_TIMER_CCIS)
#define BSP430_RF_CC2520_GPIO1_TIMER_CCIS BSP430_RFEM_RF1P7_TIMER_CCIS
#endif /* BSP430_RFEM_RF1P7_TIMER_CCIS */
#if defined(BSP430_RFEM_RF1P9_PORT_PERIPH_HANDLE)
#define BSP430_RF_CC2520_GPIO2_PORT_PERIPH_HANDLE BSP430_RFEM_RF1P9_PORT_PERIPH_HANDLE
#endif /* BSP430_RFEM_RF1P9_PORT_PERIPH_HANDLE */
#if defined(BSP430_RFEM_RF1P9_PORT_BIT)
#define BSP430_RF_CC2520_GPIO2_PORT_BIT BSP430_RFEM_RF1P9_PORT_BIT
#endif /* BSP430_RFEM_RF1P9_PORT_BIT */
#if defined(BSP430_RFEM_RF1P9_TIMER_PERIPH_HANDLE)
#define BSP430_RF_CC2520_GPIO2_TIMER_PERIPH_HANDLE BSP430_RFEM_RF1P9_TIMER_PERIPH_HANDLE
#endif /* BSP430_RFEM_RF1P9_TIMER_PERIPH_HANDLE */
#if defined(BSP430_RFEM_RF1P9_TIMER_CCIDX)
#define BSP430_RF_CC2520_GPIO2_TIMER_CCIDX BSP430_RFEM_RF1P9_TIMER_CCIDX
#endif /* BSP430_RFEM_RF1P9_TIMER_CCIDX */
#if defined(BSP430_RFEM_RF1P9_TIMER_CCIS)
#define BSP430_RF_CC2520_GPIO2_TIMER_CCIS BSP430_RFEM_RF1P9_TIMER_CCIS
#endif /* BSP430_RFEM_RF1P9_TIMER_CCIS */
#if defined(BSP430_RFEM_RF1P12_PORT_PERIPH_HANDLE)
#define BSP430_RF_CC2520_GPIO3_PORT_PERIPH_HANDLE BSP430_RFEM_RF1P12_PORT_PERIPH_HANDLE
#endif /* BSP430_RFEM_RF1P12_PORT_PERIPH_HANDLE */
#if defined(BSP430_RFEM_RF1P12_PORT_BIT)
#define BSP430_RF_CC2520_GPIO3_PORT_BIT BSP430_RFEM_RF1P12_PORT_BIT
#endif /* BSP430_RFEM_RF1P12_PORT_BIT */
#if defined(BSP430_RFEM_RF1P12_TIMER_PERIPH_HANDLE)
#define BSP430_RF_CC2520_GPIO3_TIMER_PERIPH_HANDLE BSP430_RFEM_RF1P12_TIMER_PERIPH_HANDLE
#endif /* BSP430_RFEM_RF1P12_TIMER_PERIPH_HANDLE */
#if defined(BSP430_RFEM_RF1P12_TIMER_CCIDX)
#define BSP430_RF_CC2520_GPIO3_TIMER_CCIDX BSP430_RFEM_RF1P12_TIMER_CCIDX
#endif /* BSP430_RFEM_RF1P12_TIMER_CCIDX */
#if defined(BSP430_RFEM_RF1P12_TIMER_CCIS)
#define BSP430_RF_CC2520_GPIO3_TIMER_CCIS BSP430_RFEM_RF1P12_TIMER_CCIS
#endif /* BSP430_RFEM_RF1P12_TIMER_CCIS */
#if defined(BSP430_RFEM_RF1P14_PORT_PERIPH_HANDLE)
#define BSP430_RF_CC2520_CSn_PORT_PERIPH_HANDLE BSP430_RFEM_RF1P14_PORT_PERIPH_HANDLE
#endif /* BSP430_RFEM_RF1P14_PORT_PERIPH_HANDLE */
#if defined(BSP430_RFEM_RF1P14_PORT_BIT)
#define BSP430_RF_CC2520_CSn_PORT_BIT BSP430_RFEM_RF1P14_PORT_BIT
#endif /* BSP430_RFEM_RF1P14_PORT_BIT */
#if defined(BSP430_RFEM_RF1P20_PORT_PERIPH_HANDLE)
#define BSP430_RF_CC2520_SO_PORT_PERIPH_HANDLE BSP430_RFEM_RF1P20_PORT_PERIPH_HANDLE
#endif /* BSP430_RFEM_RF1P20_PORT_PERIPH_HANDLE */
#if defined(BSP430_RFEM_RF1P20_PORT_BIT)
#define BSP430_RF_CC2520_SO_PORT_BIT BSP430_RFEM_RF1P20_PORT_BIT
#endif /* BSP430_RFEM_RF1P20_PORT_BIT */
#if defined(BSP430_RFEM_RF2P15_PORT_PERIPH_HANDLE)
#define BSP430_RF_CC2520_RESETn_PORT_PERIPH_HANDLE BSP430_RFEM_RF2P15_PORT_PERIPH_HANDLE
#endif /* BSP430_RFEM_RF2P15_PORT_PERIPH_HANDLE */
#if defined(BSP430_RFEM_RF2P15_PORT_BIT)
#define BSP430_RF_CC2520_RESETn_PORT_BIT BSP430_RFEM_RF2P15_PORT_BIT
#endif /* BSP430_RFEM_RF2P15_PORT_BIT */
#if defined(BSP430_RFEM_RF2P18_PORT_PERIPH_HANDLE)
#define BSP430_RF_CC2520_GPIO4_PORT_PERIPH_HANDLE BSP430_RFEM_RF2P18_PORT_PERIPH_HANDLE
#endif /* BSP430_RFEM_RF2P18_PORT_PERIPH_HANDLE */
#if defined(BSP430_RFEM_RF2P18_PORT_BIT)
#define BSP430_RF_CC2520_GPIO4_PORT_BIT BSP430_RFEM_RF2P18_PORT_BIT
#endif /* BSP430_RFEM_RF2P18_PORT_BIT */
#if defined(BSP430_RFEM_RF2P18_TIMER_PERIPH_HANDLE)
#define BSP430_RF_CC2520_GPIO4_TIMER_PERIPH_HANDLE BSP430_RFEM_RF2P18_TIMER_PERIPH_HANDLE
#endif /* BSP430_RFEM_RF2P18_TIMER_PERIPH_HANDLE */
#if defined(BSP430_RFEM_RF2P18_TIMER_CCIDX)
#define BSP430_RF_CC2520_GPIO4_TIMER_CCIDX BSP430_RFEM_RF2P18_TIMER_CCIDX
#endif /* BSP430_RFEM_RF2P18_TIMER_CCIDX */
#if defined(BSP430_RFEM_RF2P18_TIMER_CCIS)
#define BSP430_RF_CC2520_GPIO4_TIMER_CCIS BSP430_RFEM_RF2P18_TIMER_CCIS
#endif /* BSP430_RFEM_RF2P18_TIMER_CCIS */
#if defined(BSP430_RFEM_RF2P19_PORT_PERIPH_HANDLE)
#define BSP430_RF_CC2520_GPIO5_PORT_PERIPH_HANDLE BSP430_RFEM_RF2P19_PORT_PERIPH_HANDLE
#endif /* BSP430_RFEM_RF2P19_PORT_PERIPH_HANDLE */
#if defined(BSP430_RFEM_RF2P19_PORT_BIT)
#define BSP430_RF_CC2520_GPIO5_PORT_BIT BSP430_RFEM_RF2P19_PORT_BIT
#endif /* BSP430_RFEM_RF2P19_PORT_BIT */
#if defined(BSP430_RFEM_RF2P19_TIMER_PERIPH_HANDLE)
#define BSP430_RF_CC2520_GPIO5_TIMER_PERIPH_HANDLE BSP430_RFEM_RF2P19_TIMER_PERIPH_HANDLE
#endif /* BSP430_RFEM_RF2P19_TIMER_PERIPH_HANDLE */
#if defined(BSP430_RFEM_RF2P19_TIMER_CCIDX)
#define BSP430_RF_CC2520_GPIO5_TIMER_CCIDX BSP430_RFEM_RF2P19_TIMER_CCIDX
#endif /* BSP430_RFEM_RF2P19_TIMER_CCIDX */
#if defined(BSP430_RFEM_RF2P19_TIMER_CCIS)
#define BSP430_RF_CC2520_GPIO5_TIMER_CCIS BSP430_RFEM_RF2P19_TIMER_CCIS
#endif /* BSP430_RFEM_RF2P19_TIMER_CCIS */
#endif /* configBSP430_RF_CC2520EMK */
/* END AUTOMATICALLY GENERATED CODE [emk_platform] */
/* !BSP430! end=emk_platform */
/* !BSP430! insert=emk_platform emk=cc3000em tag=cc3000 gpio=IRQn */
/* BEGIN AUTOMATICALLY GENERATED CODE---DO NOT MODIFY [emk_platform] */
#if (configBSP430_RF_CC3000EM - 0)
#define BSP430_RF_CC3000 1
#define BSP430_RF_CC3000_SPI_PERIPH_HANDLE BSP430_RFEM_SERIAL_PERIPH_HANDLE
#if defined(BSP430_RFEM_RF1P10_PORT_PERIPH_HANDLE)
#define BSP430_RF_CC3000_PWR_EN_PORT_PERIPH_HANDLE BSP430_RFEM_RF1P10_PORT_PERIPH_HANDLE
#endif /* BSP430_RFEM_RF1P10_PORT_PERIPH_HANDLE */
#if defined(BSP430_RFEM_RF1P10_PORT_BIT)
#define BSP430_RF_CC3000_PWR_EN_PORT_BIT BSP430_RFEM_RF1P10_PORT_BIT
#endif /* BSP430_RFEM_RF1P10_PORT_BIT */
#if defined(BSP430_RFEM_RF1P12_PORT_PERIPH_HANDLE)
#define BSP430_RF_CC3000_IRQn_PORT_PERIPH_HANDLE BSP430_RFEM_RF1P12_PORT_PERIPH_HANDLE
#endif /* BSP430_RFEM_RF1P12_PORT_PERIPH_HANDLE */
#if defined(BSP430_RFEM_RF1P12_PORT_BIT)
#define BSP430_RF_CC3000_IRQn_PORT_BIT BSP430_RFEM_RF1P12_PORT_BIT
#endif /* BSP430_RFEM_RF1P12_PORT_BIT */
#if defined(BSP430_RFEM_RF1P12_TIMER_PERIPH_HANDLE)
#define BSP430_RF_CC3000_IRQn_TIMER_PERIPH_HANDLE BSP430_RFEM_RF1P12_TIMER_PERIPH_HANDLE
#endif /* BSP430_RFEM_RF1P12_TIMER_PERIPH_HANDLE */
#if defined(BSP430_RFEM_RF1P12_TIMER_CCIDX)
#define BSP430_RF_CC3000_IRQn_TIMER_CCIDX BSP430_RFEM_RF1P12_TIMER_CCIDX
#endif /* BSP430_RFEM_RF1P12_TIMER_CCIDX */
#if defined(BSP430_RFEM_RF1P12_TIMER_CCIS)
#define BSP430_RF_CC3000_IRQn_TIMER_CCIS BSP430_RFEM_RF1P12_TIMER_CCIS
#endif /* BSP430_RFEM_RF1P12_TIMER_CCIS */
#if defined(BSP430_RFEM_RF1P14_PORT_PERIPH_HANDLE)
#define BSP430_RF_CC3000_CSn_PORT_PERIPH_HANDLE BSP430_RFEM_RF1P14_PORT_PERIPH_HANDLE
#endif /* BSP430_RFEM_RF1P14_PORT_PERIPH_HANDLE */
#if defined(BSP430_RFEM_RF1P14_PORT_BIT)
#define BSP430_RF_CC3000_CSn_PORT_BIT BSP430_RFEM_RF1P14_PORT_BIT
#endif /* BSP430_RFEM_RF1P14_PORT_BIT */
#endif /* configBSP430_RF_CC3000EM */
/* END AUTOMATICALLY GENERATED CODE [emk_platform] */
/* !BSP430! end=emk_platform */

#endif /* BSP430_RF_EMKPLATFORM_H */
