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
 * @brief Hardware presentation/abstraction for Digital I/O Port (PORT\#/PORT\#_R)
 *
 * A Digital I/O port capability exists on every MSP430 MCU.
 * Differences between families are reflected in the HPL structures
 * below.
 *
 * Conventional peripheral handles are #BSP430_PERIPH_PORT1.  The
 * handles are available only when the corresponding @HPL is
 * requested.  The peripheral handle can be used with
 * xBSP430hplLookupPORT() and xBSP430hplLookupPORTIE() to obtain the
 * HPL handles, and with hBSP430halPORT() to obtain the HAL handle.
 *
 * @section h_periph_port_opt Module Configuration Options
 *
 * @li #configBSP430_HPL_PORT1 to enable the HPL handle declarations
 * @li #configBSP430_HAL_PORT1 to enable the HAL infrastructure
 * @li #configBSP430_HAL_PORT1_ISR to control inclusion of the HAL ISR
 * for the port
 *
 * Substitute other 8-bit port identifiers (e.g., @b PORT7) as necessary.
 *
 * @section h_periph_port_hpl Hardware Presentation Layer
 *
 * The module defines five distinct @HPL structures:
 *
 * @li #sBSP430hplPORT_8 is an 8-bit port lacking interrupt support on a
 * pre-5xx family device
 *
 * @li #sBSP430hplPORT_IE_8 is an 8-bit port supporting interrupts on
 * a pre-5xx family device
 *
 * @li #sBSP430hplPORT_16 is a 16-bit port lacking interrupt support
 * on a pre-5xx family device
 *
 * @li #sBSP430hplPORT_5XX_8 is an 8-bit port supporting interrupts on
 * a 5xx/6xx/FR5xx family device
 *
 * @li #sBSP430hplPORT_5XX_16 is a 16-bit port lacking interrupt
 * support on a 5xx/6xx/FR5xx family device
 *
 * The 16 bit port structures are provided for reference only.  All
 * HPL and HAL access in BSP430 is done using the 8-bit structures.
 *
 * @note On pre-5xx targets not all register capabilities may be
 * accessible through the HPL structures.  Specifically, the ability
 * to control pullup/pulldown resistors and to select secondary or
 * tertiary peripheral functions may require registers that are not
 * contiguous with the remaining registers.  That functionality must
 * be accessed through the HAL structure.
 *
 * As a convenience, the following two typedefs are provided and
 * resolve to the appropriate underlying structure supported by the
 * target MCU:
 *
 * @li #sBSP430hplPORT denotes the @HPL for an 8-bit port that lacks
 * interrupt support (generally ports 3 and higher)
 *
 * @li #sBSP430hplPORTIE denotes the @HPL for an 8-bit port that does
 * support interrupts (generally ports 1 and 2, but higher ports on
 * some MCUs)
 *
 * @section h_periph_port_hal Hardware Adaptation Layer
 *
 * The port @HAL uses the sBSP430halPORT structure.  All variant HPL
 * structures are accessible through this structure.
 *
 * When enabled by #configBSP430_HAL_PORT1_ISR the
 * sBSP430halPORT.pin_cbchain_ni array is used by a provided HAL ISR is
 * enabled to notify application and library code of events that occur
 * on a particular port pin.  It is the responsibility of that code to
 * @link callbacks register a callback @endlink and to configure the
 * corresponding interrupt enable and edge select bits (e.g. using
 * #BSP430_PORT_HAL_GET_HPL_PORTIE to get the #sBSP430hplPORTIE
 * handle).
 *
 * @homepage http://github.com/pabigot/bsp430
 * @copyright Copyright 2012-2014, Peter A. Bigot.  Licensed under <a href="http://www.opensource.org/licenses/BSD-3-Clause">BSD-3-Clause</a>
 */

/* !BSP430! periph=port */
/* !BSP430! instance=PORT1,PORT2,PORT3,PORT4,PORT5,PORT6,PORT7,PORT8,PORT9,PORT10,PORT11 */

#ifndef BSP430_PERIPH_PORT_H
#define BSP430_PERIPH_PORT_H

#include <bsp430/periph.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Defined on inclusion of <bsp430/periph/port.h>.  The value
 * evaluates to true if the target MCU supports the Digital I/O
 * Module, and false if it does not.
 *
 * @cppflag
 */
#define BSP430_MODULE_PORT (defined(__MSP430_HAS_PORT1__)       \
                            || defined(__MSP430_HAS_PORT1_R__))

#if defined(BSP430_DOXYGEN) || (BSP430_MODULE_PORT - 0)

/* Analysis of port capabilities:
 *
 * 1xx: P1/P2 are uniform contiguous with interrupt capability.  P3-P6
 * are uniform contiguous without interrupt capability.  There is no
 * REN capability.
 *
 * 2xx: P1/P2 are uniform contiguous inclusive of REN but exclusive of
 * SEL2, which is contiguous at a distinct base address, and have
 * interrupt capability.  P3-P6 are uniform 8-bit and P7-P8 are
 * uniform 16-bit contiguous, exclusive of REN and SEL2.  If any port
 * has SEL2, all ports have SEL2.  PA=(P7,P8) in the headers, but is
 * not named as such in the user's guide.
 *
 * 3xx: P1/P2 are uniform contiguous with interrupt capability.  P3-P4
 * are uniform contiguous without interrupt capability.  There is no
 * REN capability.
 *
 * 4xx: P1/P2 are uniform contiguous including REN capability, and
 * have interrupt capability.  P3-P6 are uniform 8-bit and P7-P10 are
 * uniform 16-bit contiguous; these are exclusive of REN, which is
 * contiguous at a distinct base address.  Some MCUs have SEL2 on P1,
 * but no other ports have SEL2.  PA=(P7,P8), PB=(P9,P10).
 *
 * 5xx: All ports uniform 16-bit contiguous.  PA=(P1,P2), PB=(P3,P4),
 * and so forth.  All registers have space reserved for interrupt
 * support.  Only FR5XX has PxSELC.
 *
 * In the above, "8-bit" means that the register fields are
 * immediately adjacent with byte addresses.  "16-bit" means that
 * register pairs are interleaved within function; e.g. P1IN would be
 * at 0x00 while P2IN is at 0x01 and P1OUT at 0x02.
 *
 * 16-bit ports were introduced in MSP430X and are available on
 * several chips in the 2xx and 4xx family.  In 2xx/4xx MCUs, PORTA
 * overlays P7/P8; in 5xx MCUs it overlays P1/P2.
 *
 * In pre-5xx families, registers supporting interrupts have a
 * different layout than registers that do not support interrupts.
 * However, the layouts (exclusive of SEL2 and REN) are compatible
 * within those types.
 *
 * Thus there are several distinct layouts that need to be supported:
 *
 * + [PORT_IE_8] Pre-5xx interrupt-capable, 8-bit interface
 * + [PORT_8] Pre-5xx non-interrupt, 8-bit interface
 * + [PORT_16] Pre-5xx non-interrupt, 16-bit interface
 * + [PORT_5XX] 5xx registers, 8-bit interface
 * + [PORT_5XX_16] 5xx registers, 16-bit interface
 *
 * 5xx chips are identified by MSP430XV2_CPU.  For these, a uniform
 * HPL based on the 16-bit alphabetic register names is presented.
 */

/** A macro that is defined to a true value if the port peripheral
 * supports a resistor-enable register, and to a false value if it
 * does not.
 *
 * This can be used in application and library code to use or avoid
 * reference to the @a ren field of HPL structures, either directly or
 * though the #BSP430_PORT_HAL_HPL_REN() macro.
 *
 * Support for the PxREN register is determined by the @c
 * __MSP430_HAS_PORT1_R__ functional presence indicator for
 * <msp430.h>; in all MCUs, if P1 has the feature, so do any other
 * ports.
 */
#if defined(__MSP430_HAS_PORT1_R__)
#define BSP430_PORT_SUPPORTS_REN 1
#else /* P1REN */
#define BSP430_PORT_SUPPORTS_REN 0
#endif /* P1REN */

/** Value for #BSP430_PORT_HAL_SET_REN() for high impedance input.
 *
 * This disables any port resistor enabled capability.  It corresponds
 * to REN reset, OUT dontcare. */
#define BSP430_PORT_REN_HIGH_Z -1

/** Value for #BSP430_PORT_HAL_SET_REN() for pulldown resistor.
 *
 * This corresponds to REN set, OUT reset. */
#define BSP430_PORT_REN_PULL_DOWN 0

/** Value for #BSP430_PORT_HAL_SET_REN() for pullup resistor.
 *
 * This corresponds to REN set, OUT set. */
#define BSP430_PORT_REN_PULL_UP 1

/** Set port resistor-enabled configuration via @HAL handle.
 *
 * On MCUs that support internal pullup and pulldown resistors this
 * function macro can be used to configure between those choices and
 * floating inputs.  This version is suited for all families, and is
 * required for 2xx/4xx devices where resistor-enable capability is
 * not present within the @HPL structure.
 *
 * @see #BSP430_PORT_HPL_SET_REN()
 *
 * @note This does not reconfigure the pin for input.  It only affects
 * internal pullup configuration.
 *
 * @note If #BSP430_PORT_SUPPORTS_REN is false, this macro does
 * nothing.
 *
 * @param hal_ the handle for the port @HAL
 *
 * @param bit_ the bit for which resistor/impedance is to be configured
 *
 * @param rv_ the selected configuration, one of
 * #BSP430_PORT_REN_HIGH_Z, #BSP430_PORT_REN_PULL_DOWN,
 * #BSP430_PORT_REN_PULL_UP.
 */
#if defined(BSP430_DOXYGEN) || (BSP430_PORT_SUPPORTS_REN - 0)
#define BSP430_PORT_HAL_SET_REN(hal_, bit_, rv_) do {    \
    if ((BSP430_PORT_REN_PULL_UP == (rv_))               \
        || (BSP430_PORT_REN_PULL_DOWN == (rv_))) {       \
      if (BSP430_PORT_REN_PULL_UP == (rv_)) {            \
        BSP430_PORT_HAL_HPL_OUT(hal_) |= (bit_);         \
      } else {                                           \
        BSP430_PORT_HAL_HPL_OUT(hal_) &= ~(bit_);        \
      }                                                  \
      BSP430_PORT_HAL_HPL_REN(hal_) |= (bit_);           \
    } else {                                             \
      BSP430_PORT_HAL_HPL_REN(hal_) &= ~(bit_);          \
    }                                                    \
  } while (0)
#else /* BSP430_PORT_SUPPORTS_REN */
#define BSP430_PORT_HAL_SET_REN(hal_, bit_, rv_) do { } while (0)
#endif /* BSP430_PORT_SUPPORTS_REN */

/** Set port resistor-enabled configuration via @HPL handle.
 *
 * On MCUs that support internal pullup and pulldown resistors this
 * function macro can be used to configure between those choices and
 * floating inputs.  This version is suited for 5xx-family devices for
 * which the resistor-enable functionality is within the port @HPL.
 *
 * @note This does not reconfigure the pin for input.  It only affects
 * internal pullup configuration.
 *
 * @note If #BSP430_PORT_SUPPORTS_REN is false, this macro does
 * nothing.
 *
 * @see #BSP430_PORT_HAL_SET_REN()
 *
 * @param hpl_ the handle for the port @HPL
 *
 * @param bit_ the bit for which resistor/impedance is to be configured
 *
 * @param rv_ the selected configuration, one of
 * #BSP430_PORT_REN_HIGH_Z, #BSP430_PORT_REN_PULL_DOWN,
 * #BSP430_PORT_REN_PULL_UP.
 *
 * @dependency BSP430_CORE_FAMILY_IS_5XX
 */
#if defined(BSP430_DOXYGEN) || (BSP430_PORT_SUPPORTS_REN - 0)
#define BSP430_PORT_HPL_SET_REN(hpl_, bit_, rv_) do {    \
    if ((BSP430_PORT_REN_PULL_UP == (rv_))               \
        || (BSP430_PORT_REN_PULL_DOWN == (rv_))) {       \
      if (BSP430_PORT_REN_PULL_UP == (rv_)) {            \
        (hpl_)->out |= (bit_);                           \
      } else {                                           \
        (hpl_)->out &= ~(bit_);                          \
      }                                                  \
      (hpl_)->ren |= (bit_);                             \
    } else {                                             \
      (hpl_)->ren &= ~(bit_);                            \
    }                                                    \
  } while (0)
#else /* BSP430_PORT_SUPPORTS_REN */
#define BSP430_PORT_HPL_SET_REN(hal_, bit_, rv_) do { } while (0)
#endif /* BSP430_PORT_SUPPORTS_REN */

/** Defined to true value if MCU supports multiple peripheral function
 * selections with 2xx API (P1SEL, P1SEL2, no complement, all
 * ports).
 *
 * The value may be overridden in platform-specific @c bsp430_config.h
 * files when the toolchain does not allow inferring the correct
 * setting, as with Code Composer Studio for some families.
 *
 * @defaulted */
#if defined(BSP430_DOXYGEN) || ! defined(BSP430_PORT_SUPPORTS_SEL2_2XX)
#define BSP430_PORT_SUPPORTS_SEL2_2XX ((! (BSP430_CORE_FAMILY_IS_5XX - 0)) \
                                       && (defined(P1SEL2_) || defined(P1SEL)) \
                                       && (defined(P2SEL2_) || defined(P2SEL2)))
#endif /* BSP430_PORT_SUPPORTS_SEL2_2XX */

/** Defined to true value if MCU supports multiple peripheral function
 * selections with 4xx API (P1SEL, P1SEL2, no complement, only
 * PORT1).
 *
 * The value may be overridden in platform-specific @c bsp430_config.h
 * files when the toolchain does not allow inferring the correct
 * setting, as with Code Composer Studio for some families.
 *
 * @defaulted */
#if defined(BSP430_DOXYGEN) || ! defined(BSP430_PORT_SUPPORTS_SEL2_47XX)
#define BSP430_PORT_SUPPORTS_SEL2_47XX ((! (BSP430_CORE_FAMILY_IS_5XX - 0)) \
                                        && (defined(P1SEL2_) || defined(P1SEL2)) \
                                        && ! (defined(P2SEL2_) || defined(P2SEL2)))
#endif /* BSP430_PORT_SUPPORTS_SEL2_47XX */

/** Defined to true value if MCU supports multiple peripheral function
 * selections with the base 5xx API (PxSEL0, PxSEL1, no complement
 * function).
 *
 * The value may be overridden in platform-specific @c bsp430_config.h
 * files when the toolchain does not allow inferring the correct
 * setting, as with Code Composer Studio for some families.
 *
 * @defaulted */
#if defined(BSP430_DOXYGEN) || ! defined(BSP430_PORT_SUPPORTS_SEL2_5XX)
#define BSP430_PORT_SUPPORTS_SEL2_5XX ((BSP430_CORE_FAMILY_IS_5XX - 0)  \
                                       && (defined(PASEL0_) || defined(PASEL0)) \
                                       && ! (defined(PASELC_) || defined(PASELC)))
#endif /* BSP430_PORT_SUPPORTS_SEL2_5XX */

/** Defined to true value if MCU supports multiple peripheral function
 * selections with the FR5xx API (PxSEL0, PxSEL1, PxSELC).
 *
 * The value may be overridden in platform-specific @c bsp430_config.h
 * files when the toolchain does not allow inferring the correct
 * setting, as with Code Composer Studio for some families.
 *
 * @defaulted */
#if defined(BSP430_DOXYGEN) || ! defined(BSP430_PORT_SUPPORTS_SEL2C_5XX)
#define BSP430_PORT_SUPPORTS_SEL2C_5XX ((BSP430_CORE_FAMILY_IS_5XX - 0) \
                                        && (defined(PASEL0_) || defined(PASEL0)) \
                                        && (defined(PASELC_) || defined(PASELC)))
#endif /* BSP430_PORT_SUPPORTS_SEL2C_5XX */

/** Defined to true value if MCU supports multiple peripheral function
 * selections. */
#define BSP430_PORT_SUPPORTS_SEL2 ((BSP430_PORT_SUPPORTS_SEL2_2XX - 0) \
                                   || (BSP430_PORT_SUPPORTS_SEL2_47XX - 0) \
                                   || (BSP430_PORT_SUPPORTS_SEL2_5XX - 0) \
                                   || (BSP430_PORT_SUPPORTS_SEL2C_5XX - 0))

/** @def BSP430_PORT_HAL_SET_SEL
 *
 * Set port function selection value via @HAL handle.
 *
 * MSP430 ports support peripheral functions that are enabled through
 * port-specific function selector registers.  In some MCUs a second
 * and third function may be selected with a second port-specific
 * function selector register.  This function simplifies selecting the
 * peripheral function.  It exists to allow a macro to identify the
 * desired function as a single value, and to resolve inconsistencies
 * in selection register naming across the MSP430 product line.
 *
 * @see #BSP430_PORT_SUPPORTS_SEL2_2XX, #BSP430_PORT_SUPPORTS_SEL2_47XX,
 * #BSP430_PORT_SUPPORTS_SEL2_5XX, #BSP430_PORT_SUPPORTS_SEL2C_5XX
 *
 * The @p sel_ parameter value 0 clears all sel registers for the
 * specified port and bit, leaving the pin in its digital i/o role.
 *
 * Non-zero values are treated as binary numbers, where the least
 * significant bit corresponds to the setting for @c PxSEL (@c PxSEL0
 * on FRAM devices), the next most significant bit to the setting for
 * @c PxSEL2 (normally, @c PxSEL1 on FRAM devices)
 *
 * @note Setting a new selection that requires changing bit values in
 * two registers is safe only on #BSP430_PORT_SUPPORTS_SEL2C_5XX
 * systems.  For all other systems the register will be configured to
 * an intermediate selection before the final bit is set correctly.
 *
 * @warning This function macro currently does not support @p sel_
 * values above 1 under #BSP430_PORT_SUPPORTS_SEL2_47XX.  Request
 * support if you need this.
 *
 * @param hal_ the handle for the port @HAL
 *
 * @param bit_ the bit identifing the pin for which the function is
 * desired
 *
 * @param sel_ the selection function index, with 0 selecting digital
 * input/output (no peripheral function)
 *
 * @platformdep
 */

/** @def BSP430_PORT_HPL_SET_SEL
 *
 * Set port function selection value via @HPL handle.
 *
 * As with #BSP430_PORT_HAL_SET_SEL(), but not fully supported for
 * families where the necessary registers cannot be reached from the
 * @HPL handle.
 *
 * @warning For #BSP430_PORT_SUPPORTS_SEL2_2XX and
 * #BSP430_PORT_SUPPORTS_SEL2_47XX this function macro does not
 * correctly process @p sel_ values above 1.
 *
 * @param hpl_ the handle for the port @HPL
 *
 * @param bit_ the bit identifing the pin for which the function is
 * desired
 *
 * @param sel_ the selection function index, with 0 selecting digital
 * input/output (no peripheral function)
 *
 * @platformdep
 */
#if (BSP430_PORT_SUPPORTS_SEL2_2XX - 0)
#define BSP430_PORT_HPL_SET_SEL(hpl_, bit_, sel_) do {  \
    if (0x01 & (sel_)) {                                \
      (hpl_)->sel |= (bit_);                            \
    } else {                                            \
      (hpl_)->sel &= ~(bit_);                           \
    }                                                   \
  } while (0)
#define BSP430_PORT_HAL_SET_SEL(hal_, bit_, sel_) do {  \
    if (0x01 & (sel_)) {                                \
      BSP430_PORT_HAL_HPL_SEL(hal_) |= (bit_);          \
    } else {                                            \
      BSP430_PORT_HAL_HPL_SEL(hal_) &= ~(bit_);         \
    }                                                   \
    if (0x02 & (sel_)) {                                \
      *(hal_)->sel2p |= (bit_);                         \
    } else {                                            \
      *(hal_)->sel2p &= ~(bit_);                        \
    }                                                   \
  } while (0)
/* #elif (BSP430_PORT_SUPPORTS_SEL2_47XX - 0) : NOT SUPPORTED AT THIS TIME */
#elif (BSP430_PORT_SUPPORTS_SEL2_5XX - 0) || defined(BSP430_DOXYGEN)
#define BSP430_PORT_HPL_SET_SEL(hpl_, bit_, sel_) do {  \
    if (0x01 & (sel_)) {                                \
      (hpl_)->sel |= (bit_);                            \
    } else {                                            \
      (hpl_)->sel &= ~(bit_);                           \
    }                                                   \
    if (0x02 & (sel_)) {                                \
      (hpl_)->sel1 |= (bit_);                           \
    } else {                                            \
      (hpl_)->sel1 &= ~(bit_);                          \
    }                                                   \
  } while (0)
#define BSP430_PORT_HAL_SET_SEL(hal_, bit_, sel_) BSP430_PORT_HPL_SET_SEL((hal_)->hpl.port_5xx_8, bit_, sel_)
#elif (BSP430_PORT_SUPPORTS_SEL2C_5XX - 0)
/* Ugliness: if bits in both registers are to change, then use SELC.
 * Otherwise only one actually changes, so it's ok to write both. */
#define BSP430_PORT_HPL_SET_SEL(hpl_, bit_, sel_) do {                  \
    unsigned char cs = (((!!((bit_) & (hpl_)->sel1) != !!(0x02 & (sel_))) << 1) \
                        | (!!((bit_) & (hpl_)->sel0) != !!(0x01 & (sel_)))); \
    if (0x03 == cs) {                                                   \
      (hpl_)->selc |= (bit_);                                           \
    } else {                                                            \
      if (0x01 & (sel_)) {                                              \
        (hpl_)->sel0 |= (bit_);                                         \
      } else {                                                          \
        (hpl_)->sel0 &= ~(bit_);                                        \
      }                                                                 \
      if (0x02 & (sel_)) {                                              \
        (hpl_)->sel1 |= (bit_);                                         \
      } else {                                                          \
        (hpl_)->sel1 &= ~(bit_);                                        \
      }                                                                 \
    }                                                                   \
  } while (0)
#define BSP430_PORT_HAL_SET_SEL(hal_, bit_, sel_) BSP430_PORT_HPL_SET_SEL((hal_)->hpl.port_5xx_8, bit_, sel_)
#else /* SEL2 options */
#define BSP430_PORT_HPL_SET_SEL(hpl_, bit_, sel_) do {  \
    if (0x01 & (sel_)) {                                \
      (hpl_)->sel |= (bit_);                            \
    } else {                                            \
      (hpl_)->sel &= ~(bit_);                           \
    }                                                   \
  } while (0)
#define BSP430_PORT_HAL_SET_SEL(hal_, bit_, sel_) do {  \
    if (0x01 & (sel_)) {                                \
      BSP430_PORT_HAL_HPL_SEL(hal_) |= (bit_);          \
    } else {                                            \
      BSP430_PORT_HAL_HPL_SEL(hal_) &= ~(bit_);         \
    }                                                   \
  } while (0)
#endif /* SEL2 options */

/** Layout for pre-5xx--family ports supporting interrupts.
 *
 * Access to SEL2 capability for these ports is not available in the
 * hardware-presentation layer. */
typedef struct sBSP430hplPORT_IE_8 {
  unsigned char in;             /* 0x00 */
  unsigned char out;            /* 0x01 */
  unsigned char dir;            /* 0x02 */
  unsigned char ifg;            /* 0x03 */
  unsigned char ies;            /* 0x04 */
  unsigned char ie;             /* 0x05 */
  unsigned char sel;            /* 0x06 */
  unsigned char ren;            /* 0x07 */
} sBSP430hplPORT_IE_8;

/** Layout for pre-5xx--family 8-bit ports not supporting interrupts.
 *
 * Access to SEL2 and REN capability for these ports is not available
 * in the hardware-presentation layer. */
typedef struct sBSP430hplPORT_8 {
  unsigned char in;             /* 0x00 */
  unsigned char out;            /* 0x01 */
  unsigned char dir;            /* 0x02 */
  unsigned char sel;            /* 0x03 */
} sBSP430hplPORT_8;

/** Helper for accessing 8-bit registers within 16-bit registers */
typedef union uBSP430hplPORT_16 {
  unsigned int w;               /**< Word access (e.g., PAIN) */
  unsigned char b[2];           /**< Byte access, indexed */
  struct {
    unsigned char l;            /**< Low byte access for odd members of pair (e.g., P1IN) */
    unsigned char h;            /**< High byte access for even members of pair (e.g., P2IN) */
  };
} uBSP430hplPORT_16;

/** Layout for 2xx/4xx family 16-bit ports not supporting interrupts.
 *
 * Access to SEL2 and REN capability for these ports is not available
 * in the hardware-presentation layer. */
typedef struct sBSP430hplPORT_16 {
  uBSP430hplPORT_16 in;         /* 0x00 */
  uBSP430hplPORT_16 out;        /* 0x02 */
  uBSP430hplPORT_16 dir;        /* 0x04 */
  uBSP430hplPORT_16 sel;        /* 0x08 */
} sBSP430hplPORT_16;

/** Layout for 5xx family ports, 16-bit access
 */
typedef struct sBSP430hplPORT_5XX_16 {
  uBSP430hplPORT_16 in;         /**< PxIN */ /* 0x00 */
  uBSP430hplPORT_16 out;        /**< PxOUT */ /* 0x02 */
  uBSP430hplPORT_16 dir;        /**< PxDIR (set for output) */ /* 0x04 */
  uBSP430hplPORT_16 ren;        /**< PxREN (set to enable) */ /* 0x06 */
  uBSP430hplPORT_16 ds;         /**< PxDS (drive select, some devices) */ /* 0x08 */
  union {
    uBSP430hplPORT_16 sel;      /**< PxSEL (non-FR5xx devices) */ /* 0x0A */
    uBSP430hplPORT_16 sel0;     /**< PxSEL0 (FR5xx devices) */ /* 0x0A */
  };
  uBSP430hplPORT_16 sel1;       /**< PxSEL1 (secondary/tertiary function, FR5xx devices only) */ /* 0x0C */
  unsigned int _reserved_x0E;
  unsigned int _reserved_x10;
  unsigned int _reserved_x12;
  unsigned int _reserved_x14;
  uBSP430hplPORT_16 selc;       /**< PxSELC (support atomic transition to tertiary function, FR5xx devices only) */ /* 0x16 */
  uBSP430hplPORT_16 ies;        /**< PxIES */ /* 0x18 */
  uBSP430hplPORT_16 ie;         /**< PxIE */ /* 0x1A */
  uBSP430hplPORT_16 ifg;        /**< PxIFG */ /* 0x1C */
} sBSP430hplPORT_5XX_16;

/** Layout for 5xx family ports, 8-bit access
 *
 * This structure includes the necessary padding to maintain alignment
 * when referencing the second 8-bit register in a 16-bit bank.
 */
typedef struct sBSP430hplPORT_5XX_8 {
  unsigned char in;              /**< PxIN */ /* 0x00 */
  unsigned char _reserved_x01;
  unsigned char out;             /**< PxOUT */ /* 0x02 */
  unsigned char _reserved_x03;
  unsigned char dir;             /**< PxDIR (set for output) */ /* 0x04 */
  unsigned char _reserved_x05;
  unsigned char ren;             /**< PxREN (set to enable) */ /* 0x06 */
  unsigned char _reserved_x07;
  unsigned char ds;              /**< PxDS (drive select, some devices) */ /* 0x08 */
  unsigned char _reserved_x09;
  union __attribute__((__packed__))
  {
    unsigned char sel;           /**< PxSEL (non-FR5xx devices) */ /* 0x0A */
    unsigned char sel0;          /**< PxSEL0 (FR5xx devices) */ /* 0x0A */
  };
  unsigned char _reserved_x0B;
  unsigned char sel1;            /**< PxSEL1 (secondary/tertiary function, FR5xx devices only) */ /* 0x0C */
  unsigned char _reserved_x0D;
  unsigned char _reserved_x0E;
  unsigned char _reserved_x0F;
  unsigned char _reserved_x10;
  unsigned char _reserved_x11;
  unsigned char _reserved_x12;
  unsigned char _reserved_x13;
  unsigned char _reserved_x14;
  unsigned char _reserved_x15;
  unsigned char selc;            /**< PxSELC (support atomic transition to tertiary function, FR5xx devices only) *//* 0x16 */
  unsigned char _reserved_x17;
  unsigned char ies;             /**< PxIES */ /* 0x18 */
  unsigned char _reserved_x19;
  unsigned char ie;              /**< PxIE */ /* 0x1A */
  unsigned char _reserved_x1B;
  unsigned char ifg;             /**< PxIFG */ /* 0x1C */
} sBSP430hplPORT_5XX_8;

/** Structure used to access ports with interrupt capability.
 *
 * The underlying structure selected depends on the MCU family. */
#if (BSP430_CORE_FAMILY_IS_5XX - 0)
typedef sBSP430hplPORT_5XX_8 sBSP430hplPORTIE;
#else /* 5XX */
typedef sBSP430hplPORT_IE_8 sBSP430hplPORTIE;
#endif /* 5XX */

/** Structure used to access ports without interrupt capability.
 *
 * The underlying structure selected depends on the MCU family. */
#if (BSP430_CORE_FAMILY_IS_5XX - 0)
typedef sBSP430hplPORT_5XX_8 sBSP430hplPORT;
#else /* 5XX */
typedef sBSP430hplPORT_8 sBSP430hplPORT;
#endif /* 5XX */

/** Structure used to access 16-bit ports.
 *
 * The underlying structure selected depends on the MCU family. */
#if (BSP430_CORE_FAMILY_IS_5XX - 0)
typedef sBSP430hplPORT_5XX_16 sBSP430hplPORTW;
#else /* 5XX */
typedef sBSP430hplPORT_16 sBSP430hplPORTW;
#endif /* 5XX */

/** @cond DOXYGEN_INTERNAL */

/* Port address constants are not defined for use outside the BSP430
 * implementation, but are used outside the port HPL and HAL feature.
 * In particular these constants are used to simplify the
 * implementation of iBSP430platformConfigurePeripheralPins_ni without
 * requiring access to the "real" HPL handles for the peripheral
 * ports. */

#if (BSP430_CORE_FAMILY_IS_5XX - 0)
/* All 5xx ports are resistor enabled */
#define BSP430_PERIPH_PORTA_BASEADDRESS_ __MSP430_BASEADDRESS_PORTA_R__
#define BSP430_PERIPH_PORTB_BASEADDRESS_ __MSP430_BASEADDRESS_PORTB_R__
#define BSP430_PERIPH_PORTC_BASEADDRESS_ __MSP430_BASEADDRESS_PORTC_R__
#define BSP430_PERIPH_PORTD_BASEADDRESS_ __MSP430_BASEADDRESS_PORTD_R__
#define BSP430_PERIPH_PORTE_BASEADDRESS_ __MSP430_BASEADDRESS_PORTE_R__
#define BSP430_PERIPH_PORTF_BASEADDRESS_ __MSP430_BASEADDRESS_PORTF_R__

/* There are __MSP430_BASEADDRESS_PORT1_R__ and
 * __MSP430_BASEADDRESS_PORT2_R__, but they have the same value. */
#define BSP430_PERIPH_PORT1_BASEADDRESS_ __MSP430_BASEADDRESS_PORTA_R__
#define BSP430_PERIPH_PORT2_BASEADDRESS_ (1+__MSP430_BASEADDRESS_PORTA_R__)
#define BSP430_PERIPH_PORT3_BASEADDRESS_ __MSP430_BASEADDRESS_PORTB_R__
#define BSP430_PERIPH_PORT4_BASEADDRESS_ (1+__MSP430_BASEADDRESS_PORTB_R__)
#define BSP430_PERIPH_PORT5_BASEADDRESS_ __MSP430_BASEADDRESS_PORTC_R__
#define BSP430_PERIPH_PORT6_BASEADDRESS_ (1+__MSP430_BASEADDRESS_PORTC_R__)
#define BSP430_PERIPH_PORT7_BASEADDRESS_ __MSP430_BASEADDRESS_PORTD_R__
#define BSP430_PERIPH_PORT8_BASEADDRESS_ (1+__MSP430_BASEADDRESS_PORTD_R__)
#define BSP430_PERIPH_PORT9_BASEADDRESS_ __MSP430_BASEADDRESS_PORTE_R__
#define BSP430_PERIPH_PORT10_BASEADDRESS_ (1+__MSP430_BASEADDRESS_PORTE_R__)
#define BSP430_PERIPH_PORT11_BASEADDRESS_ __MSP430_BASEADDRESS_PORTF_R__

#define BSP430_PERIPH_PORTJ_BASEADDRESS_ __MSP430_BASEADDRESS_PORTJ_R__

#else /* 5xx */
/* Port addresses are fixed for all pre-5xx families. */
#define BSP430_PERIPH_PORT1_BASEADDRESS_ 0x0020
#define BSP430_PERIPH_PORT2_BASEADDRESS_ 0x0028
#define BSP430_PERIPH_PORT3_BASEADDRESS_ 0x0018
#define BSP430_PERIPH_PORT4_BASEADDRESS_ 0x001C
#define BSP430_PERIPH_PORT5_BASEADDRESS_ 0x0030
#define BSP430_PERIPH_PORT6_BASEADDRESS_ 0x0034

#define BSP430_PERIPH_PORTA_BASEADDRESS_ 0x0038
#define BSP430_PERIPH_PORT7_BASEADDRESS_ 0x0038
#define BSP430_PERIPH_PORT8_BASEADDRESS_ 0x0039

#define BSP430_PERIPH_PORTB_BASEADDRESS_ 0x0008
#define BSP430_PERIPH_PORT9_BASEADDRESS_ 0x0008
#define BSP430_PERIPH_PORT10_BASEADDRESS_ 0x0009

#endif /* 5xx */

/** @endcond */ /* DOXYGEN_INTERNAL */

/** Field value for variant stored in sBSP430halPORT.hal_state.cflags
 * when HPL reference is to an #sBSP430hplPORT_IE_8. */
#define BSP430_PORT_HAL_HPL_VARIANT_PORT_IE_8 1

/** Field value for variant stored in sBSP430halPORT.hal_state.cflags
 * when HPL reference is to an #sBSP430hplPORT_8. */
#define BSP430_PORT_HAL_HPL_VARIANT_PORT_8 2

/** Field value for variant stored in sBSP430halPORT.hal_state.cflags
 * when HPL reference is to an #sBSP430hplPORT_16. */
#define BSP430_PORT_HAL_HPL_VARIANT_PORT_16 3

/** Field value for variant stored in sBSP430halPORT.hal_state.cflags
 * when HPL reference is to an #sBSP430hplPORT_5XX_8. */
#define BSP430_PORT_HAL_HPL_VARIANT_PORT_5XX_8 4

/** Field value for variant stored in sBSP430halPORT.hal_state.cflags
 * when HPL reference is to an #sBSP430hplPORT_5XX_16. */
#define BSP430_PORT_HAL_HPL_VARIANT_PORT_5XX_16 5

/** Field value for variant stored in sBSP430halPORT.hal_state.cflags
 * when HPL reference is to a byte-accessed port without interrupt
 * capabilities.
 *
 * This identifies the appropriate structure-specific variant value
 * based on the MCU capabilities. */
#if (BSP430_CORE_FAMILY_IS_5XX - 0)
#define BSP430_PORT_HAL_HPL_VARIANT_PORTIE BSP430_PORT_HAL_HPL_VARIANT_PORT_5XX_8
#else /* 5XX */
#define BSP430_PORT_HAL_HPL_VARIANT_PORTIE BSP430_PORT_HAL_HPL_VARIANT_PORT_IE_8
#endif /* 5XX */

/** Field value for variant stored in sBSP430halPORT.hal_state.cflags
 * when HPL reference is to a byte-accessed port without interrupt
 * capabilities.
 *
 * This identifies the appropriate structure-specific variant value
 * based on the MCU capabilities. */
#if (BSP430_CORE_FAMILY_IS_5XX - 0)
#define BSP430_PORT_HAL_HPL_VARIANT_PORT BSP430_PORT_HAL_HPL_VARIANT_PORT_5XX_8
#else /* 5XX */
#define BSP430_PORT_HAL_HPL_VARIANT_PORT BSP430_PORT_HAL_HPL_VARIANT_PORT_8
#endif /* 5XX */

/** Field value for variant stored in sBSP430halPORT.hal_state.cflags
 * when HPL reference is to a word-accessed port.
 *
 * This identifies the appropriate structure-specific variant value
 * based on the MCU capabilities. */
#if (BSP430_CORE_FAMILY_IS_5XX - 0)
#define BSP430_PORT_HAL_HPL_VARIANT_PORTW BSP430_PORT_HAL_HPL_VARIANT_PORT_5XX_16
#else /* 5XX */
#define BSP430_PORT_HAL_HPL_VARIANT_PORTW BSP430_PORT_HAL_HPL_VARIANT_PORT_16
#endif /* 5XX */

/* !BSP430! instance=port_ie_8,port_8,port_16,port_5xx_8,port_5xx_16,port,portie,portw */
/* !BSP430! insert=hal_variant_hpl_macro */
/* BEGIN AUTOMATICALLY GENERATED CODE---DO NOT MODIFY [hal_variant_hpl_macro] */
/** True iff the HPL pointer of the port HAL is a PORT_IE_8 variant. */
#define BSP430_PORT_HAL_HPL_VARIANT_IS_PORT_IE_8(hal_) (BSP430_PORT_HAL_HPL_VARIANT_PORT_IE_8 == BSP430_PERIPH_HAL_STATE_CFLAGS_VARIANT(hal_))

/** Get the port HPL pointer if the HAL is a PORT_IE_8 variant.
 *
 * Value is a null pointer if the HAL references a different HPL type. */
#define BSP430_PORT_HAL_GET_HPL_PORT_IE_8(hal_) ((BSP430_PORT_HAL_HPL_VARIANT_IS_PORT_IE_8(hal_)) ? (hal_)->hpl.port_ie_8 : (void *)0)

/** True iff the HPL pointer of the port HAL is a PORT_8 variant. */
#define BSP430_PORT_HAL_HPL_VARIANT_IS_PORT_8(hal_) (BSP430_PORT_HAL_HPL_VARIANT_PORT_8 == BSP430_PERIPH_HAL_STATE_CFLAGS_VARIANT(hal_))

/** Get the port HPL pointer if the HAL is a PORT_8 variant.
 *
 * Value is a null pointer if the HAL references a different HPL type. */
#define BSP430_PORT_HAL_GET_HPL_PORT_8(hal_) ((BSP430_PORT_HAL_HPL_VARIANT_IS_PORT_8(hal_)) ? (hal_)->hpl.port_8 : (void *)0)

/** True iff the HPL pointer of the port HAL is a PORT_16 variant. */
#define BSP430_PORT_HAL_HPL_VARIANT_IS_PORT_16(hal_) (BSP430_PORT_HAL_HPL_VARIANT_PORT_16 == BSP430_PERIPH_HAL_STATE_CFLAGS_VARIANT(hal_))

/** Get the port HPL pointer if the HAL is a PORT_16 variant.
 *
 * Value is a null pointer if the HAL references a different HPL type. */
#define BSP430_PORT_HAL_GET_HPL_PORT_16(hal_) ((BSP430_PORT_HAL_HPL_VARIANT_IS_PORT_16(hal_)) ? (hal_)->hpl.port_16 : (void *)0)

/** True iff the HPL pointer of the port HAL is a PORT_5XX_8 variant. */
#define BSP430_PORT_HAL_HPL_VARIANT_IS_PORT_5XX_8(hal_) (BSP430_PORT_HAL_HPL_VARIANT_PORT_5XX_8 == BSP430_PERIPH_HAL_STATE_CFLAGS_VARIANT(hal_))

/** Get the port HPL pointer if the HAL is a PORT_5XX_8 variant.
 *
 * Value is a null pointer if the HAL references a different HPL type. */
#define BSP430_PORT_HAL_GET_HPL_PORT_5XX_8(hal_) ((BSP430_PORT_HAL_HPL_VARIANT_IS_PORT_5XX_8(hal_)) ? (hal_)->hpl.port_5xx_8 : (void *)0)

/** True iff the HPL pointer of the port HAL is a PORT_5XX_16 variant. */
#define BSP430_PORT_HAL_HPL_VARIANT_IS_PORT_5XX_16(hal_) (BSP430_PORT_HAL_HPL_VARIANT_PORT_5XX_16 == BSP430_PERIPH_HAL_STATE_CFLAGS_VARIANT(hal_))

/** Get the port HPL pointer if the HAL is a PORT_5XX_16 variant.
 *
 * Value is a null pointer if the HAL references a different HPL type. */
#define BSP430_PORT_HAL_GET_HPL_PORT_5XX_16(hal_) ((BSP430_PORT_HAL_HPL_VARIANT_IS_PORT_5XX_16(hal_)) ? (hal_)->hpl.port_5xx_16 : (void *)0)

/** True iff the HPL pointer of the port HAL is a PORT variant. */
#define BSP430_PORT_HAL_HPL_VARIANT_IS_PORT(hal_) (BSP430_PORT_HAL_HPL_VARIANT_PORT == BSP430_PERIPH_HAL_STATE_CFLAGS_VARIANT(hal_))

/** Get the port HPL pointer if the HAL is a PORT variant.
 *
 * Value is a null pointer if the HAL references a different HPL type. */
#define BSP430_PORT_HAL_GET_HPL_PORT(hal_) ((BSP430_PORT_HAL_HPL_VARIANT_IS_PORT(hal_)) ? (hal_)->hpl.port : (void *)0)

/** True iff the HPL pointer of the port HAL is a PORTIE variant. */
#define BSP430_PORT_HAL_HPL_VARIANT_IS_PORTIE(hal_) (BSP430_PORT_HAL_HPL_VARIANT_PORTIE == BSP430_PERIPH_HAL_STATE_CFLAGS_VARIANT(hal_))

/** Get the port HPL pointer if the HAL is a PORTIE variant.
 *
 * Value is a null pointer if the HAL references a different HPL type. */
#define BSP430_PORT_HAL_GET_HPL_PORTIE(hal_) ((BSP430_PORT_HAL_HPL_VARIANT_IS_PORTIE(hal_)) ? (hal_)->hpl.portie : (void *)0)

/** True iff the HPL pointer of the port HAL is a PORTW variant. */
#define BSP430_PORT_HAL_HPL_VARIANT_IS_PORTW(hal_) (BSP430_PORT_HAL_HPL_VARIANT_PORTW == BSP430_PERIPH_HAL_STATE_CFLAGS_VARIANT(hal_))

/** Get the port HPL pointer if the HAL is a PORTW variant.
 *
 * Value is a null pointer if the HAL references a different HPL type. */
#define BSP430_PORT_HAL_GET_HPL_PORTW(hal_) ((BSP430_PORT_HAL_HPL_VARIANT_IS_PORTW(hal_)) ? (hal_)->hpl.portw : (void *)0)

/* END AUTOMATICALLY GENERATED CODE [hal_variant_hpl_macro] */
/* !BSP430! end=hal_variant_hpl_macro */
/* !BSP430! instance=PORT1,PORT2,PORT3,PORT4,PORT5,PORT6,PORT7,PORT8,PORT9,PORT10,PORT11 */

/** Get the port HPL peripheral handle.
 *
 * This bypasses any structural interpretation of the HPL pointer,
 * converting it to the underlying #tBSP430periphHandle value
 * directly. */
#define BSP430_PORT_HAL_GET_PERIPH_HANDLE(hal_) xBSP430periphFromHPL((hal_)->hpl.any)

/** Structure holding hardware abstraction layer state for digital I/O
 * ports. */
typedef struct sBSP430halPORT {
  /** Common header used to extract the correct HPL pointer type from
   * the hpl union. */
  sBSP430hplHALStatePrefix hal_state;
  /** Allow the HAL state to be independent of the underlying HPL layout.
   *
   * In practice, a valid pointer here will be compatible with either
   * @a port or @a portie, both in the case of the 5xx port peripheral
   * where a single layout is used.  Use
   * #BSP430_PERIPH_HAL_STATE_CFLAGS_VARIANT() to introspect a HAL
   * instance to determine which pointer is appropriate. */
  union {
    /** Access to the HPL pointer ignoring its underlying type */
    volatile void * any;
    /** Access to the HPL pointer as a pre-5xx 8-bit interrupt-enabled port */
    volatile sBSP430hplPORT_IE_8 * port_ie_8;
    /** Access to the HPL pointer as a pre-5xx 8-bit port not supporting interrupts */
    volatile sBSP430hplPORT_8 * port_8;
    /** Access to the HPL pointer as a pre-5xx 16-bit port not supporting interrupts */
    volatile sBSP430hplPORT_16 * port_16;
    /** Access to the HPL pointer as a 5xx 8-bit port */
    volatile sBSP430hplPORT_5XX_8 * port_5xx_8;
    /** Access to the HPL pointer as a 5xx 16-bit port */
    volatile sBSP430hplPORT_5XX_16 * port_5xx_16;
    /** Access to the HPL pointer as a MCU-dependent interrupt-enabled port */
    volatile sBSP430hplPORTIE * portie;
    /** Access to the HPL pointer as a MCU-dependent port not supporting interrupts */
    volatile sBSP430hplPORT * port;
    /** Access to the HPL pointer as a MCU-dependent word-access port */
    volatile sBSP430hplPORTW * portw;
  } const hpl;
  /** Callbacks invoked when the HAL ISR handler receives an event for
   * the corresponding bit of the port. */
  const struct sBSP430halISRIndexedChainNode * volatile pin_cbchain_ni[8];
#if defined(BSP430_DOXYGEN) || ((BSP430_PORT_SUPPORTS_REN - 0) && ! (BSP430_CORE_FAMILY_IS_5XX - 0))
  /** Pointer to the resistor enable register for this peripheral.
   *
   * On 2xx/4xx MCUs where this register exists, it is outside the
   * normal register area.  In 5xx ports this feature is available in
   * the HPL structure.
   *
   * @dependency Only on non-5xx MCUs that support REN */
  volatile unsigned char * const renp;
#endif /* Pre-5xx REN */
#if defined(BSP430_DOXYGEN) || (BSP430_PORT_SUPPORTS_SEL2_2XX - 0)
  /** Pointer to the secondary selection register for this peripheral.
   *
   * On 2xx/4xx MCUs where this register exists, it is outside the
   * normal register area.  In 5xx ports this feature is available in
   * the HPL structure.
   *
   * @dependency Only on non-5xx MCUs that support SEL2 */
  volatile unsigned char * const sel2p;
#endif /* Pre-5xx SEL2 */
} sBSP430halPORT;

/** Handle for a port HAL instance */
typedef struct sBSP430halPORT * hBSP430halPORT;

/** Macro to reference a port IN register regardless of HPL layout. */
#if (BSP430_CORE_FAMILY_IS_5XX - 0)
#define BSP430_PORT_HAL_HPL_IN(hal_) ((hal_)->hpl.portie->in)
#else /* 5XX */
#define BSP430_PORT_HAL_HPL_IN(hal_) (*(BSP430_PORT_HAL_HPL_VARIANT_IS_PORTIE(hal_) ? &BSP430_PORT_HAL_GET_HPL_PORTIE(hal_)->in : &BSP430_PORT_HAL_GET_HPL_PORT(hal_)->in))
#endif /* 5XX */
/** Macro to reference a port OUT register regardless of HPL layout. */
#if (BSP430_CORE_FAMILY_IS_5XX - 0)
#define BSP430_PORT_HAL_HPL_OUT(hal_) ((hal_)->hpl.portie->out)
#else /* 5XX */
#define BSP430_PORT_HAL_HPL_OUT(hal_) (*(BSP430_PORT_HAL_HPL_VARIANT_IS_PORTIE(hal_) ? &BSP430_PORT_HAL_GET_HPL_PORTIE(hal_)->out : &BSP430_PORT_HAL_GET_HPL_PORT(hal_)->out))
#endif /* 5XX */
/** Macro to reference a port DIR register regardless of HPL layout. */
#if (BSP430_CORE_FAMILY_IS_5XX - 0)
#define BSP430_PORT_HAL_HPL_DIR(hal_) ((hal_)->hpl.portie->dir)
#else /* 5XX */
#define BSP430_PORT_HAL_HPL_DIR(hal_) (*(BSP430_PORT_HAL_HPL_VARIANT_IS_PORTIE(hal_) ? &BSP430_PORT_HAL_GET_HPL_PORTIE(hal_)->dir : &BSP430_PORT_HAL_GET_HPL_PORT(hal_)->dir))
#endif /* 5XX */
/** Macro to reference a port SEL register regardless of HPL layout. */
#if (BSP430_CORE_FAMILY_IS_5XX - 0)
#define BSP430_PORT_HAL_HPL_SEL(hal_) ((hal_)->hpl.portie->sel)
#else /* 5XX */
#define BSP430_PORT_HAL_HPL_SEL(hal_) (*(BSP430_PORT_HAL_HPL_VARIANT_IS_PORTIE(hal_) ? &BSP430_PORT_HAL_GET_HPL_PORTIE(hal_)->sel : &BSP430_PORT_HAL_GET_HPL_PORT(hal_)->sel))
#endif /* 5XX */
/** Macro to reference a port REN register regardless of HPL layout.
 *
 * @dependency Selected MCU must support REN function */
#if defined(BSP430_DOXYGEN) || (BSP430_PORT_SUPPORTS_REN - 0)
#if (BSP430_CORE_FAMILY_IS_5XX - 0)
#define BSP430_PORT_HAL_HPL_REN(hal_) ((hal_)->hpl.portie->ren)
#else /* 5XX */
#define BSP430_PORT_HAL_HPL_REN(hal_) (*(hal_)->renp)
#endif /* 5XX */
#endif /* REN support */

/* !BSP430! insert=hal_decl */
/* BEGIN AUTOMATICALLY GENERATED CODE---DO NOT MODIFY [hal_decl] */
/** Control inclusion of the @HAL interface to #BSP430_PERIPH_PORT1
 *
 * This must be defined to 1 in @c bsp430_config.h to request the
 * interface be included, and 0 to request it be excluded.  By default
 * the interface is excluded.
 *
 * When enabled, the sBSP430halPORT structure reference is
 * available as #BSP430_HAL_PORT1.
 *
 * It may also be obtained using
 * #hBSP430portLookup(#BSP430_PERIPH_PORT1).
 *
 * @cppflag
 * @affects #configBSP430_HPL_PORT1 is default-enabled
 * @ingroup grp_config_core
 * @defaulted */
#ifndef configBSP430_HAL_PORT1
#define configBSP430_HAL_PORT1 0
#endif /* configBSP430_HAL_PORT1 */

/** @cond DOXYGEN_EXCLUDE */
#if (configBSP430_HAL_PORT1 - 0)
/* You do not need to know about this */
extern sBSP430halPORT xBSP430hal_PORT1_;
#endif /* configBSP430_HAL_PORT1 */
/** @endcond */

/** sBSP430halPORT HAL handle for #BSP430_PERIPH_PORT1.
 *
 * This pointer may be used only if #configBSP430_HAL_PORT1
 * is defined to a true value.
 *
 * @dependency #configBSP430_HAL_PORT1 */
#if defined(BSP430_DOXYGEN) || (configBSP430_HAL_PORT1 - 0)
#define BSP430_HAL_PORT1 (&xBSP430hal_PORT1_)
#endif /* configBSP430_HAL_PORT1 */

/** Control inclusion of the @HAL interface to #BSP430_PERIPH_PORT2
 *
 * This must be defined to 1 in @c bsp430_config.h to request the
 * interface be included, and 0 to request it be excluded.  By default
 * the interface is excluded.
 *
 * When enabled, the sBSP430halPORT structure reference is
 * available as #BSP430_HAL_PORT2.
 *
 * It may also be obtained using
 * #hBSP430portLookup(#BSP430_PERIPH_PORT2).
 *
 * @cppflag
 * @affects #configBSP430_HPL_PORT2 is default-enabled
 * @ingroup grp_config_core
 * @defaulted */
#ifndef configBSP430_HAL_PORT2
#define configBSP430_HAL_PORT2 0
#endif /* configBSP430_HAL_PORT2 */

/** @cond DOXYGEN_EXCLUDE */
#if (configBSP430_HAL_PORT2 - 0)
/* You do not need to know about this */
extern sBSP430halPORT xBSP430hal_PORT2_;
#endif /* configBSP430_HAL_PORT2 */
/** @endcond */

/** sBSP430halPORT HAL handle for #BSP430_PERIPH_PORT2.
 *
 * This pointer may be used only if #configBSP430_HAL_PORT2
 * is defined to a true value.
 *
 * @dependency #configBSP430_HAL_PORT2 */
#if defined(BSP430_DOXYGEN) || (configBSP430_HAL_PORT2 - 0)
#define BSP430_HAL_PORT2 (&xBSP430hal_PORT2_)
#endif /* configBSP430_HAL_PORT2 */

/** Control inclusion of the @HAL interface to #BSP430_PERIPH_PORT3
 *
 * This must be defined to 1 in @c bsp430_config.h to request the
 * interface be included, and 0 to request it be excluded.  By default
 * the interface is excluded.
 *
 * When enabled, the sBSP430halPORT structure reference is
 * available as #BSP430_HAL_PORT3.
 *
 * It may also be obtained using
 * #hBSP430portLookup(#BSP430_PERIPH_PORT3).
 *
 * @cppflag
 * @affects #configBSP430_HPL_PORT3 is default-enabled
 * @ingroup grp_config_core
 * @defaulted */
#ifndef configBSP430_HAL_PORT3
#define configBSP430_HAL_PORT3 0
#endif /* configBSP430_HAL_PORT3 */

/** @cond DOXYGEN_EXCLUDE */
#if (configBSP430_HAL_PORT3 - 0)
/* You do not need to know about this */
extern sBSP430halPORT xBSP430hal_PORT3_;
#endif /* configBSP430_HAL_PORT3 */
/** @endcond */

/** sBSP430halPORT HAL handle for #BSP430_PERIPH_PORT3.
 *
 * This pointer may be used only if #configBSP430_HAL_PORT3
 * is defined to a true value.
 *
 * @dependency #configBSP430_HAL_PORT3 */
#if defined(BSP430_DOXYGEN) || (configBSP430_HAL_PORT3 - 0)
#define BSP430_HAL_PORT3 (&xBSP430hal_PORT3_)
#endif /* configBSP430_HAL_PORT3 */

/** Control inclusion of the @HAL interface to #BSP430_PERIPH_PORT4
 *
 * This must be defined to 1 in @c bsp430_config.h to request the
 * interface be included, and 0 to request it be excluded.  By default
 * the interface is excluded.
 *
 * When enabled, the sBSP430halPORT structure reference is
 * available as #BSP430_HAL_PORT4.
 *
 * It may also be obtained using
 * #hBSP430portLookup(#BSP430_PERIPH_PORT4).
 *
 * @cppflag
 * @affects #configBSP430_HPL_PORT4 is default-enabled
 * @ingroup grp_config_core
 * @defaulted */
#ifndef configBSP430_HAL_PORT4
#define configBSP430_HAL_PORT4 0
#endif /* configBSP430_HAL_PORT4 */

/** @cond DOXYGEN_EXCLUDE */
#if (configBSP430_HAL_PORT4 - 0)
/* You do not need to know about this */
extern sBSP430halPORT xBSP430hal_PORT4_;
#endif /* configBSP430_HAL_PORT4 */
/** @endcond */

/** sBSP430halPORT HAL handle for #BSP430_PERIPH_PORT4.
 *
 * This pointer may be used only if #configBSP430_HAL_PORT4
 * is defined to a true value.
 *
 * @dependency #configBSP430_HAL_PORT4 */
#if defined(BSP430_DOXYGEN) || (configBSP430_HAL_PORT4 - 0)
#define BSP430_HAL_PORT4 (&xBSP430hal_PORT4_)
#endif /* configBSP430_HAL_PORT4 */

/** Control inclusion of the @HAL interface to #BSP430_PERIPH_PORT5
 *
 * This must be defined to 1 in @c bsp430_config.h to request the
 * interface be included, and 0 to request it be excluded.  By default
 * the interface is excluded.
 *
 * When enabled, the sBSP430halPORT structure reference is
 * available as #BSP430_HAL_PORT5.
 *
 * It may also be obtained using
 * #hBSP430portLookup(#BSP430_PERIPH_PORT5).
 *
 * @cppflag
 * @affects #configBSP430_HPL_PORT5 is default-enabled
 * @ingroup grp_config_core
 * @defaulted */
#ifndef configBSP430_HAL_PORT5
#define configBSP430_HAL_PORT5 0
#endif /* configBSP430_HAL_PORT5 */

/** @cond DOXYGEN_EXCLUDE */
#if (configBSP430_HAL_PORT5 - 0)
/* You do not need to know about this */
extern sBSP430halPORT xBSP430hal_PORT5_;
#endif /* configBSP430_HAL_PORT5 */
/** @endcond */

/** sBSP430halPORT HAL handle for #BSP430_PERIPH_PORT5.
 *
 * This pointer may be used only if #configBSP430_HAL_PORT5
 * is defined to a true value.
 *
 * @dependency #configBSP430_HAL_PORT5 */
#if defined(BSP430_DOXYGEN) || (configBSP430_HAL_PORT5 - 0)
#define BSP430_HAL_PORT5 (&xBSP430hal_PORT5_)
#endif /* configBSP430_HAL_PORT5 */

/** Control inclusion of the @HAL interface to #BSP430_PERIPH_PORT6
 *
 * This must be defined to 1 in @c bsp430_config.h to request the
 * interface be included, and 0 to request it be excluded.  By default
 * the interface is excluded.
 *
 * When enabled, the sBSP430halPORT structure reference is
 * available as #BSP430_HAL_PORT6.
 *
 * It may also be obtained using
 * #hBSP430portLookup(#BSP430_PERIPH_PORT6).
 *
 * @cppflag
 * @affects #configBSP430_HPL_PORT6 is default-enabled
 * @ingroup grp_config_core
 * @defaulted */
#ifndef configBSP430_HAL_PORT6
#define configBSP430_HAL_PORT6 0
#endif /* configBSP430_HAL_PORT6 */

/** @cond DOXYGEN_EXCLUDE */
#if (configBSP430_HAL_PORT6 - 0)
/* You do not need to know about this */
extern sBSP430halPORT xBSP430hal_PORT6_;
#endif /* configBSP430_HAL_PORT6 */
/** @endcond */

/** sBSP430halPORT HAL handle for #BSP430_PERIPH_PORT6.
 *
 * This pointer may be used only if #configBSP430_HAL_PORT6
 * is defined to a true value.
 *
 * @dependency #configBSP430_HAL_PORT6 */
#if defined(BSP430_DOXYGEN) || (configBSP430_HAL_PORT6 - 0)
#define BSP430_HAL_PORT6 (&xBSP430hal_PORT6_)
#endif /* configBSP430_HAL_PORT6 */

/** Control inclusion of the @HAL interface to #BSP430_PERIPH_PORT7
 *
 * This must be defined to 1 in @c bsp430_config.h to request the
 * interface be included, and 0 to request it be excluded.  By default
 * the interface is excluded.
 *
 * When enabled, the sBSP430halPORT structure reference is
 * available as #BSP430_HAL_PORT7.
 *
 * It may also be obtained using
 * #hBSP430portLookup(#BSP430_PERIPH_PORT7).
 *
 * @cppflag
 * @affects #configBSP430_HPL_PORT7 is default-enabled
 * @ingroup grp_config_core
 * @defaulted */
#ifndef configBSP430_HAL_PORT7
#define configBSP430_HAL_PORT7 0
#endif /* configBSP430_HAL_PORT7 */

/** @cond DOXYGEN_EXCLUDE */
#if (configBSP430_HAL_PORT7 - 0)
/* You do not need to know about this */
extern sBSP430halPORT xBSP430hal_PORT7_;
#endif /* configBSP430_HAL_PORT7 */
/** @endcond */

/** sBSP430halPORT HAL handle for #BSP430_PERIPH_PORT7.
 *
 * This pointer may be used only if #configBSP430_HAL_PORT7
 * is defined to a true value.
 *
 * @dependency #configBSP430_HAL_PORT7 */
#if defined(BSP430_DOXYGEN) || (configBSP430_HAL_PORT7 - 0)
#define BSP430_HAL_PORT7 (&xBSP430hal_PORT7_)
#endif /* configBSP430_HAL_PORT7 */

/** Control inclusion of the @HAL interface to #BSP430_PERIPH_PORT8
 *
 * This must be defined to 1 in @c bsp430_config.h to request the
 * interface be included, and 0 to request it be excluded.  By default
 * the interface is excluded.
 *
 * When enabled, the sBSP430halPORT structure reference is
 * available as #BSP430_HAL_PORT8.
 *
 * It may also be obtained using
 * #hBSP430portLookup(#BSP430_PERIPH_PORT8).
 *
 * @cppflag
 * @affects #configBSP430_HPL_PORT8 is default-enabled
 * @ingroup grp_config_core
 * @defaulted */
#ifndef configBSP430_HAL_PORT8
#define configBSP430_HAL_PORT8 0
#endif /* configBSP430_HAL_PORT8 */

/** @cond DOXYGEN_EXCLUDE */
#if (configBSP430_HAL_PORT8 - 0)
/* You do not need to know about this */
extern sBSP430halPORT xBSP430hal_PORT8_;
#endif /* configBSP430_HAL_PORT8 */
/** @endcond */

/** sBSP430halPORT HAL handle for #BSP430_PERIPH_PORT8.
 *
 * This pointer may be used only if #configBSP430_HAL_PORT8
 * is defined to a true value.
 *
 * @dependency #configBSP430_HAL_PORT8 */
#if defined(BSP430_DOXYGEN) || (configBSP430_HAL_PORT8 - 0)
#define BSP430_HAL_PORT8 (&xBSP430hal_PORT8_)
#endif /* configBSP430_HAL_PORT8 */

/** Control inclusion of the @HAL interface to #BSP430_PERIPH_PORT9
 *
 * This must be defined to 1 in @c bsp430_config.h to request the
 * interface be included, and 0 to request it be excluded.  By default
 * the interface is excluded.
 *
 * When enabled, the sBSP430halPORT structure reference is
 * available as #BSP430_HAL_PORT9.
 *
 * It may also be obtained using
 * #hBSP430portLookup(#BSP430_PERIPH_PORT9).
 *
 * @cppflag
 * @affects #configBSP430_HPL_PORT9 is default-enabled
 * @ingroup grp_config_core
 * @defaulted */
#ifndef configBSP430_HAL_PORT9
#define configBSP430_HAL_PORT9 0
#endif /* configBSP430_HAL_PORT9 */

/** @cond DOXYGEN_EXCLUDE */
#if (configBSP430_HAL_PORT9 - 0)
/* You do not need to know about this */
extern sBSP430halPORT xBSP430hal_PORT9_;
#endif /* configBSP430_HAL_PORT9 */
/** @endcond */

/** sBSP430halPORT HAL handle for #BSP430_PERIPH_PORT9.
 *
 * This pointer may be used only if #configBSP430_HAL_PORT9
 * is defined to a true value.
 *
 * @dependency #configBSP430_HAL_PORT9 */
#if defined(BSP430_DOXYGEN) || (configBSP430_HAL_PORT9 - 0)
#define BSP430_HAL_PORT9 (&xBSP430hal_PORT9_)
#endif /* configBSP430_HAL_PORT9 */

/** Control inclusion of the @HAL interface to #BSP430_PERIPH_PORT10
 *
 * This must be defined to 1 in @c bsp430_config.h to request the
 * interface be included, and 0 to request it be excluded.  By default
 * the interface is excluded.
 *
 * When enabled, the sBSP430halPORT structure reference is
 * available as #BSP430_HAL_PORT10.
 *
 * It may also be obtained using
 * #hBSP430portLookup(#BSP430_PERIPH_PORT10).
 *
 * @cppflag
 * @affects #configBSP430_HPL_PORT10 is default-enabled
 * @ingroup grp_config_core
 * @defaulted */
#ifndef configBSP430_HAL_PORT10
#define configBSP430_HAL_PORT10 0
#endif /* configBSP430_HAL_PORT10 */

/** @cond DOXYGEN_EXCLUDE */
#if (configBSP430_HAL_PORT10 - 0)
/* You do not need to know about this */
extern sBSP430halPORT xBSP430hal_PORT10_;
#endif /* configBSP430_HAL_PORT10 */
/** @endcond */

/** sBSP430halPORT HAL handle for #BSP430_PERIPH_PORT10.
 *
 * This pointer may be used only if #configBSP430_HAL_PORT10
 * is defined to a true value.
 *
 * @dependency #configBSP430_HAL_PORT10 */
#if defined(BSP430_DOXYGEN) || (configBSP430_HAL_PORT10 - 0)
#define BSP430_HAL_PORT10 (&xBSP430hal_PORT10_)
#endif /* configBSP430_HAL_PORT10 */

/** Control inclusion of the @HAL interface to #BSP430_PERIPH_PORT11
 *
 * This must be defined to 1 in @c bsp430_config.h to request the
 * interface be included, and 0 to request it be excluded.  By default
 * the interface is excluded.
 *
 * When enabled, the sBSP430halPORT structure reference is
 * available as #BSP430_HAL_PORT11.
 *
 * It may also be obtained using
 * #hBSP430portLookup(#BSP430_PERIPH_PORT11).
 *
 * @cppflag
 * @affects #configBSP430_HPL_PORT11 is default-enabled
 * @ingroup grp_config_core
 * @defaulted */
#ifndef configBSP430_HAL_PORT11
#define configBSP430_HAL_PORT11 0
#endif /* configBSP430_HAL_PORT11 */

/** @cond DOXYGEN_EXCLUDE */
#if (configBSP430_HAL_PORT11 - 0)
/* You do not need to know about this */
extern sBSP430halPORT xBSP430hal_PORT11_;
#endif /* configBSP430_HAL_PORT11 */
/** @endcond */

/** sBSP430halPORT HAL handle for #BSP430_PERIPH_PORT11.
 *
 * This pointer may be used only if #configBSP430_HAL_PORT11
 * is defined to a true value.
 *
 * @dependency #configBSP430_HAL_PORT11 */
#if defined(BSP430_DOXYGEN) || (configBSP430_HAL_PORT11 - 0)
#define BSP430_HAL_PORT11 (&xBSP430hal_PORT11_)
#endif /* configBSP430_HAL_PORT11 */

/* END AUTOMATICALLY GENERATED CODE [hal_decl] */
/* !BSP430! end=hal_decl */

/* !BSP430! insert=periph_decl */
/* BEGIN AUTOMATICALLY GENERATED CODE---DO NOT MODIFY [periph_decl] */
/** Control inclusion of the @HPL interface to #BSP430_PERIPH_PORT1
 *
 * This must be defined to 1 in @c bsp430_config.h to request the
 * interface be included, and 0 to request it be excluded.  By default
 * the interface is excluded.
 *
 * When enabled, the sBSP430hplPORT structure reference is
 * available as #BSP430_HPL_PORT1.
 *
 * It may also be obtained using
 * #xBSP430hplLookupPORT(#BSP430_PERIPH_PORT1).
 *
 * @cppflag
 * @ingroup grp_config_core
 * @defaulted */
#ifndef configBSP430_HPL_PORT1
#define configBSP430_HPL_PORT1 (configBSP430_HAL_PORT1 - 0)
#endif /* configBSP430_HPL_PORT1 */

#if (configBSP430_HAL_PORT1 - 0) && ! (configBSP430_HPL_PORT1 - 0)
#warning configBSP430_HAL_PORT1 requested without configBSP430_HPL_PORT1
#endif /* HAL and not HPL */

/** Handle for the raw PORT1 device.
 *
 * The handle may be used only if #configBSP430_HPL_PORT1
 * is defined to a true value.
 *
 * @dependency #configBSP430_HPL_PORT1 */
#if defined(BSP430_DOXYGEN) || (configBSP430_HPL_PORT1 - 0)
#define BSP430_PERIPH_PORT1 ((tBSP430periphHandle)(BSP430_PERIPH_PORT1_BASEADDRESS_))
#endif /* configBSP430_HPL_PORT1 */

/** Control inclusion of the @HPL interface to #BSP430_PERIPH_PORT2
 *
 * This must be defined to 1 in @c bsp430_config.h to request the
 * interface be included, and 0 to request it be excluded.  By default
 * the interface is excluded.
 *
 * When enabled, the sBSP430hplPORT structure reference is
 * available as #BSP430_HPL_PORT2.
 *
 * It may also be obtained using
 * #xBSP430hplLookupPORT(#BSP430_PERIPH_PORT2).
 *
 * @cppflag
 * @ingroup grp_config_core
 * @defaulted */
#ifndef configBSP430_HPL_PORT2
#define configBSP430_HPL_PORT2 (configBSP430_HAL_PORT2 - 0)
#endif /* configBSP430_HPL_PORT2 */

#if (configBSP430_HAL_PORT2 - 0) && ! (configBSP430_HPL_PORT2 - 0)
#warning configBSP430_HAL_PORT2 requested without configBSP430_HPL_PORT2
#endif /* HAL and not HPL */

/** Handle for the raw PORT2 device.
 *
 * The handle may be used only if #configBSP430_HPL_PORT2
 * is defined to a true value.
 *
 * @dependency #configBSP430_HPL_PORT2 */
#if defined(BSP430_DOXYGEN) || (configBSP430_HPL_PORT2 - 0)
#define BSP430_PERIPH_PORT2 ((tBSP430periphHandle)(BSP430_PERIPH_PORT2_BASEADDRESS_))
#endif /* configBSP430_HPL_PORT2 */

/** Control inclusion of the @HPL interface to #BSP430_PERIPH_PORT3
 *
 * This must be defined to 1 in @c bsp430_config.h to request the
 * interface be included, and 0 to request it be excluded.  By default
 * the interface is excluded.
 *
 * When enabled, the sBSP430hplPORT structure reference is
 * available as #BSP430_HPL_PORT3.
 *
 * It may also be obtained using
 * #xBSP430hplLookupPORT(#BSP430_PERIPH_PORT3).
 *
 * @cppflag
 * @ingroup grp_config_core
 * @defaulted */
#ifndef configBSP430_HPL_PORT3
#define configBSP430_HPL_PORT3 (configBSP430_HAL_PORT3 - 0)
#endif /* configBSP430_HPL_PORT3 */

#if (configBSP430_HAL_PORT3 - 0) && ! (configBSP430_HPL_PORT3 - 0)
#warning configBSP430_HAL_PORT3 requested without configBSP430_HPL_PORT3
#endif /* HAL and not HPL */

/** Handle for the raw PORT3 device.
 *
 * The handle may be used only if #configBSP430_HPL_PORT3
 * is defined to a true value.
 *
 * @dependency #configBSP430_HPL_PORT3 */
#if defined(BSP430_DOXYGEN) || (configBSP430_HPL_PORT3 - 0)
#define BSP430_PERIPH_PORT3 ((tBSP430periphHandle)(BSP430_PERIPH_PORT3_BASEADDRESS_))
#endif /* configBSP430_HPL_PORT3 */

/** Control inclusion of the @HPL interface to #BSP430_PERIPH_PORT4
 *
 * This must be defined to 1 in @c bsp430_config.h to request the
 * interface be included, and 0 to request it be excluded.  By default
 * the interface is excluded.
 *
 * When enabled, the sBSP430hplPORT structure reference is
 * available as #BSP430_HPL_PORT4.
 *
 * It may also be obtained using
 * #xBSP430hplLookupPORT(#BSP430_PERIPH_PORT4).
 *
 * @cppflag
 * @ingroup grp_config_core
 * @defaulted */
#ifndef configBSP430_HPL_PORT4
#define configBSP430_HPL_PORT4 (configBSP430_HAL_PORT4 - 0)
#endif /* configBSP430_HPL_PORT4 */

#if (configBSP430_HAL_PORT4 - 0) && ! (configBSP430_HPL_PORT4 - 0)
#warning configBSP430_HAL_PORT4 requested without configBSP430_HPL_PORT4
#endif /* HAL and not HPL */

/** Handle for the raw PORT4 device.
 *
 * The handle may be used only if #configBSP430_HPL_PORT4
 * is defined to a true value.
 *
 * @dependency #configBSP430_HPL_PORT4 */
#if defined(BSP430_DOXYGEN) || (configBSP430_HPL_PORT4 - 0)
#define BSP430_PERIPH_PORT4 ((tBSP430periphHandle)(BSP430_PERIPH_PORT4_BASEADDRESS_))
#endif /* configBSP430_HPL_PORT4 */

/** Control inclusion of the @HPL interface to #BSP430_PERIPH_PORT5
 *
 * This must be defined to 1 in @c bsp430_config.h to request the
 * interface be included, and 0 to request it be excluded.  By default
 * the interface is excluded.
 *
 * When enabled, the sBSP430hplPORT structure reference is
 * available as #BSP430_HPL_PORT5.
 *
 * It may also be obtained using
 * #xBSP430hplLookupPORT(#BSP430_PERIPH_PORT5).
 *
 * @cppflag
 * @ingroup grp_config_core
 * @defaulted */
#ifndef configBSP430_HPL_PORT5
#define configBSP430_HPL_PORT5 (configBSP430_HAL_PORT5 - 0)
#endif /* configBSP430_HPL_PORT5 */

#if (configBSP430_HAL_PORT5 - 0) && ! (configBSP430_HPL_PORT5 - 0)
#warning configBSP430_HAL_PORT5 requested without configBSP430_HPL_PORT5
#endif /* HAL and not HPL */

/** Handle for the raw PORT5 device.
 *
 * The handle may be used only if #configBSP430_HPL_PORT5
 * is defined to a true value.
 *
 * @dependency #configBSP430_HPL_PORT5 */
#if defined(BSP430_DOXYGEN) || (configBSP430_HPL_PORT5 - 0)
#define BSP430_PERIPH_PORT5 ((tBSP430periphHandle)(BSP430_PERIPH_PORT5_BASEADDRESS_))
#endif /* configBSP430_HPL_PORT5 */

/** Control inclusion of the @HPL interface to #BSP430_PERIPH_PORT6
 *
 * This must be defined to 1 in @c bsp430_config.h to request the
 * interface be included, and 0 to request it be excluded.  By default
 * the interface is excluded.
 *
 * When enabled, the sBSP430hplPORT structure reference is
 * available as #BSP430_HPL_PORT6.
 *
 * It may also be obtained using
 * #xBSP430hplLookupPORT(#BSP430_PERIPH_PORT6).
 *
 * @cppflag
 * @ingroup grp_config_core
 * @defaulted */
#ifndef configBSP430_HPL_PORT6
#define configBSP430_HPL_PORT6 (configBSP430_HAL_PORT6 - 0)
#endif /* configBSP430_HPL_PORT6 */

#if (configBSP430_HAL_PORT6 - 0) && ! (configBSP430_HPL_PORT6 - 0)
#warning configBSP430_HAL_PORT6 requested without configBSP430_HPL_PORT6
#endif /* HAL and not HPL */

/** Handle for the raw PORT6 device.
 *
 * The handle may be used only if #configBSP430_HPL_PORT6
 * is defined to a true value.
 *
 * @dependency #configBSP430_HPL_PORT6 */
#if defined(BSP430_DOXYGEN) || (configBSP430_HPL_PORT6 - 0)
#define BSP430_PERIPH_PORT6 ((tBSP430periphHandle)(BSP430_PERIPH_PORT6_BASEADDRESS_))
#endif /* configBSP430_HPL_PORT6 */

/** Control inclusion of the @HPL interface to #BSP430_PERIPH_PORT7
 *
 * This must be defined to 1 in @c bsp430_config.h to request the
 * interface be included, and 0 to request it be excluded.  By default
 * the interface is excluded.
 *
 * When enabled, the sBSP430hplPORT structure reference is
 * available as #BSP430_HPL_PORT7.
 *
 * It may also be obtained using
 * #xBSP430hplLookupPORT(#BSP430_PERIPH_PORT7).
 *
 * @cppflag
 * @ingroup grp_config_core
 * @defaulted */
#ifndef configBSP430_HPL_PORT7
#define configBSP430_HPL_PORT7 (configBSP430_HAL_PORT7 - 0)
#endif /* configBSP430_HPL_PORT7 */

#if (configBSP430_HAL_PORT7 - 0) && ! (configBSP430_HPL_PORT7 - 0)
#warning configBSP430_HAL_PORT7 requested without configBSP430_HPL_PORT7
#endif /* HAL and not HPL */

/** Handle for the raw PORT7 device.
 *
 * The handle may be used only if #configBSP430_HPL_PORT7
 * is defined to a true value.
 *
 * @dependency #configBSP430_HPL_PORT7 */
#if defined(BSP430_DOXYGEN) || (configBSP430_HPL_PORT7 - 0)
#define BSP430_PERIPH_PORT7 ((tBSP430periphHandle)(BSP430_PERIPH_PORT7_BASEADDRESS_))
#endif /* configBSP430_HPL_PORT7 */

/** Control inclusion of the @HPL interface to #BSP430_PERIPH_PORT8
 *
 * This must be defined to 1 in @c bsp430_config.h to request the
 * interface be included, and 0 to request it be excluded.  By default
 * the interface is excluded.
 *
 * When enabled, the sBSP430hplPORT structure reference is
 * available as #BSP430_HPL_PORT8.
 *
 * It may also be obtained using
 * #xBSP430hplLookupPORT(#BSP430_PERIPH_PORT8).
 *
 * @cppflag
 * @ingroup grp_config_core
 * @defaulted */
#ifndef configBSP430_HPL_PORT8
#define configBSP430_HPL_PORT8 (configBSP430_HAL_PORT8 - 0)
#endif /* configBSP430_HPL_PORT8 */

#if (configBSP430_HAL_PORT8 - 0) && ! (configBSP430_HPL_PORT8 - 0)
#warning configBSP430_HAL_PORT8 requested without configBSP430_HPL_PORT8
#endif /* HAL and not HPL */

/** Handle for the raw PORT8 device.
 *
 * The handle may be used only if #configBSP430_HPL_PORT8
 * is defined to a true value.
 *
 * @dependency #configBSP430_HPL_PORT8 */
#if defined(BSP430_DOXYGEN) || (configBSP430_HPL_PORT8 - 0)
#define BSP430_PERIPH_PORT8 ((tBSP430periphHandle)(BSP430_PERIPH_PORT8_BASEADDRESS_))
#endif /* configBSP430_HPL_PORT8 */

/** Control inclusion of the @HPL interface to #BSP430_PERIPH_PORT9
 *
 * This must be defined to 1 in @c bsp430_config.h to request the
 * interface be included, and 0 to request it be excluded.  By default
 * the interface is excluded.
 *
 * When enabled, the sBSP430hplPORT structure reference is
 * available as #BSP430_HPL_PORT9.
 *
 * It may also be obtained using
 * #xBSP430hplLookupPORT(#BSP430_PERIPH_PORT9).
 *
 * @cppflag
 * @ingroup grp_config_core
 * @defaulted */
#ifndef configBSP430_HPL_PORT9
#define configBSP430_HPL_PORT9 (configBSP430_HAL_PORT9 - 0)
#endif /* configBSP430_HPL_PORT9 */

#if (configBSP430_HAL_PORT9 - 0) && ! (configBSP430_HPL_PORT9 - 0)
#warning configBSP430_HAL_PORT9 requested without configBSP430_HPL_PORT9
#endif /* HAL and not HPL */

/** Handle for the raw PORT9 device.
 *
 * The handle may be used only if #configBSP430_HPL_PORT9
 * is defined to a true value.
 *
 * @dependency #configBSP430_HPL_PORT9 */
#if defined(BSP430_DOXYGEN) || (configBSP430_HPL_PORT9 - 0)
#define BSP430_PERIPH_PORT9 ((tBSP430periphHandle)(BSP430_PERIPH_PORT9_BASEADDRESS_))
#endif /* configBSP430_HPL_PORT9 */

/** Control inclusion of the @HPL interface to #BSP430_PERIPH_PORT10
 *
 * This must be defined to 1 in @c bsp430_config.h to request the
 * interface be included, and 0 to request it be excluded.  By default
 * the interface is excluded.
 *
 * When enabled, the sBSP430hplPORT structure reference is
 * available as #BSP430_HPL_PORT10.
 *
 * It may also be obtained using
 * #xBSP430hplLookupPORT(#BSP430_PERIPH_PORT10).
 *
 * @cppflag
 * @ingroup grp_config_core
 * @defaulted */
#ifndef configBSP430_HPL_PORT10
#define configBSP430_HPL_PORT10 (configBSP430_HAL_PORT10 - 0)
#endif /* configBSP430_HPL_PORT10 */

#if (configBSP430_HAL_PORT10 - 0) && ! (configBSP430_HPL_PORT10 - 0)
#warning configBSP430_HAL_PORT10 requested without configBSP430_HPL_PORT10
#endif /* HAL and not HPL */

/** Handle for the raw PORT10 device.
 *
 * The handle may be used only if #configBSP430_HPL_PORT10
 * is defined to a true value.
 *
 * @dependency #configBSP430_HPL_PORT10 */
#if defined(BSP430_DOXYGEN) || (configBSP430_HPL_PORT10 - 0)
#define BSP430_PERIPH_PORT10 ((tBSP430periphHandle)(BSP430_PERIPH_PORT10_BASEADDRESS_))
#endif /* configBSP430_HPL_PORT10 */

/** Control inclusion of the @HPL interface to #BSP430_PERIPH_PORT11
 *
 * This must be defined to 1 in @c bsp430_config.h to request the
 * interface be included, and 0 to request it be excluded.  By default
 * the interface is excluded.
 *
 * When enabled, the sBSP430hplPORT structure reference is
 * available as #BSP430_HPL_PORT11.
 *
 * It may also be obtained using
 * #xBSP430hplLookupPORT(#BSP430_PERIPH_PORT11).
 *
 * @cppflag
 * @ingroup grp_config_core
 * @defaulted */
#ifndef configBSP430_HPL_PORT11
#define configBSP430_HPL_PORT11 (configBSP430_HAL_PORT11 - 0)
#endif /* configBSP430_HPL_PORT11 */

#if (configBSP430_HAL_PORT11 - 0) && ! (configBSP430_HPL_PORT11 - 0)
#warning configBSP430_HAL_PORT11 requested without configBSP430_HPL_PORT11
#endif /* HAL and not HPL */

/** Handle for the raw PORT11 device.
 *
 * The handle may be used only if #configBSP430_HPL_PORT11
 * is defined to a true value.
 *
 * @dependency #configBSP430_HPL_PORT11 */
#if defined(BSP430_DOXYGEN) || (configBSP430_HPL_PORT11 - 0)
#define BSP430_PERIPH_PORT11 ((tBSP430periphHandle)(BSP430_PERIPH_PORT11_BASEADDRESS_))
#endif /* configBSP430_HPL_PORT11 */

/* END AUTOMATICALLY GENERATED CODE [periph_decl] */
/* !BSP430! end=periph_decl */

/* !BSP430! insert=port_hpl_decl */
/* BEGIN AUTOMATICALLY GENERATED CODE---DO NOT MODIFY [port_hpl_decl] */
/** HPL pointer for PORT1.
 *
 * Typed pointer to a volatile structure overlaying the PORT1
 * peripheral register map.
 *
 * The pointer may be used only if #configBSP430_HPL_PORT1
 * is defined to a true value.
 *
 * @note The underlying type is #sBSP430hplPORTIE for ports 1 and 2
 * and ports on 5xx/6xx MCUs, and is #sBSP430hplPORT for ports 3
 * and higher on pre-5xx MCUs.  The generated documentation may not
 * reflect the correct structure.
 *
 * @dependency #configBSP430_HPL_PORT1 */
#if defined(BSP430_DOXYGEN) || (configBSP430_HPL_PORT1 - 0)
#if (BSP430_CORE_FAMILY_IS_5XX - 0) || (1 <= 2)
#define BSP430_HPL_PORT1 ((volatile sBSP430hplPORTIE *)BSP430_PERIPH_PORT1)
#else /* IE */
#define BSP430_HPL_PORT1 ((volatile sBSP430hplPORT *)BSP430_PERIPH_PORT1)
#endif /* IE */
#endif /* configBSP430_HPL_PORT1 */

/** HPL pointer for PORT2.
 *
 * Typed pointer to a volatile structure overlaying the PORT2
 * peripheral register map.
 *
 * The pointer may be used only if #configBSP430_HPL_PORT2
 * is defined to a true value.
 *
 * @note The underlying type is #sBSP430hplPORTIE for ports 1 and 2
 * and ports on 5xx/6xx MCUs, and is #sBSP430hplPORT for ports 3
 * and higher on pre-5xx MCUs.  The generated documentation may not
 * reflect the correct structure.
 *
 * @dependency #configBSP430_HPL_PORT2 */
#if defined(BSP430_DOXYGEN) || (configBSP430_HPL_PORT2 - 0)
#if (BSP430_CORE_FAMILY_IS_5XX - 0) || (2 <= 2)
#define BSP430_HPL_PORT2 ((volatile sBSP430hplPORTIE *)BSP430_PERIPH_PORT2)
#else /* IE */
#define BSP430_HPL_PORT2 ((volatile sBSP430hplPORT *)BSP430_PERIPH_PORT2)
#endif /* IE */
#endif /* configBSP430_HPL_PORT2 */

/** HPL pointer for PORT3.
 *
 * Typed pointer to a volatile structure overlaying the PORT3
 * peripheral register map.
 *
 * The pointer may be used only if #configBSP430_HPL_PORT3
 * is defined to a true value.
 *
 * @note The underlying type is #sBSP430hplPORTIE for ports 1 and 2
 * and ports on 5xx/6xx MCUs, and is #sBSP430hplPORT for ports 3
 * and higher on pre-5xx MCUs.  The generated documentation may not
 * reflect the correct structure.
 *
 * @dependency #configBSP430_HPL_PORT3 */
#if defined(BSP430_DOXYGEN) || (configBSP430_HPL_PORT3 - 0)
#if (BSP430_CORE_FAMILY_IS_5XX - 0) || (3 <= 2)
#define BSP430_HPL_PORT3 ((volatile sBSP430hplPORTIE *)BSP430_PERIPH_PORT3)
#else /* IE */
#define BSP430_HPL_PORT3 ((volatile sBSP430hplPORT *)BSP430_PERIPH_PORT3)
#endif /* IE */
#endif /* configBSP430_HPL_PORT3 */

/** HPL pointer for PORT4.
 *
 * Typed pointer to a volatile structure overlaying the PORT4
 * peripheral register map.
 *
 * The pointer may be used only if #configBSP430_HPL_PORT4
 * is defined to a true value.
 *
 * @note The underlying type is #sBSP430hplPORTIE for ports 1 and 2
 * and ports on 5xx/6xx MCUs, and is #sBSP430hplPORT for ports 3
 * and higher on pre-5xx MCUs.  The generated documentation may not
 * reflect the correct structure.
 *
 * @dependency #configBSP430_HPL_PORT4 */
#if defined(BSP430_DOXYGEN) || (configBSP430_HPL_PORT4 - 0)
#if (BSP430_CORE_FAMILY_IS_5XX - 0) || (4 <= 2)
#define BSP430_HPL_PORT4 ((volatile sBSP430hplPORTIE *)BSP430_PERIPH_PORT4)
#else /* IE */
#define BSP430_HPL_PORT4 ((volatile sBSP430hplPORT *)BSP430_PERIPH_PORT4)
#endif /* IE */
#endif /* configBSP430_HPL_PORT4 */

/** HPL pointer for PORT5.
 *
 * Typed pointer to a volatile structure overlaying the PORT5
 * peripheral register map.
 *
 * The pointer may be used only if #configBSP430_HPL_PORT5
 * is defined to a true value.
 *
 * @note The underlying type is #sBSP430hplPORTIE for ports 1 and 2
 * and ports on 5xx/6xx MCUs, and is #sBSP430hplPORT for ports 3
 * and higher on pre-5xx MCUs.  The generated documentation may not
 * reflect the correct structure.
 *
 * @dependency #configBSP430_HPL_PORT5 */
#if defined(BSP430_DOXYGEN) || (configBSP430_HPL_PORT5 - 0)
#if (BSP430_CORE_FAMILY_IS_5XX - 0) || (5 <= 2)
#define BSP430_HPL_PORT5 ((volatile sBSP430hplPORTIE *)BSP430_PERIPH_PORT5)
#else /* IE */
#define BSP430_HPL_PORT5 ((volatile sBSP430hplPORT *)BSP430_PERIPH_PORT5)
#endif /* IE */
#endif /* configBSP430_HPL_PORT5 */

/** HPL pointer for PORT6.
 *
 * Typed pointer to a volatile structure overlaying the PORT6
 * peripheral register map.
 *
 * The pointer may be used only if #configBSP430_HPL_PORT6
 * is defined to a true value.
 *
 * @note The underlying type is #sBSP430hplPORTIE for ports 1 and 2
 * and ports on 5xx/6xx MCUs, and is #sBSP430hplPORT for ports 3
 * and higher on pre-5xx MCUs.  The generated documentation may not
 * reflect the correct structure.
 *
 * @dependency #configBSP430_HPL_PORT6 */
#if defined(BSP430_DOXYGEN) || (configBSP430_HPL_PORT6 - 0)
#if (BSP430_CORE_FAMILY_IS_5XX - 0) || (6 <= 2)
#define BSP430_HPL_PORT6 ((volatile sBSP430hplPORTIE *)BSP430_PERIPH_PORT6)
#else /* IE */
#define BSP430_HPL_PORT6 ((volatile sBSP430hplPORT *)BSP430_PERIPH_PORT6)
#endif /* IE */
#endif /* configBSP430_HPL_PORT6 */

/** HPL pointer for PORT7.
 *
 * Typed pointer to a volatile structure overlaying the PORT7
 * peripheral register map.
 *
 * The pointer may be used only if #configBSP430_HPL_PORT7
 * is defined to a true value.
 *
 * @note The underlying type is #sBSP430hplPORTIE for ports 1 and 2
 * and ports on 5xx/6xx MCUs, and is #sBSP430hplPORT for ports 3
 * and higher on pre-5xx MCUs.  The generated documentation may not
 * reflect the correct structure.
 *
 * @dependency #configBSP430_HPL_PORT7 */
#if defined(BSP430_DOXYGEN) || (configBSP430_HPL_PORT7 - 0)
#if (BSP430_CORE_FAMILY_IS_5XX - 0) || (7 <= 2)
#define BSP430_HPL_PORT7 ((volatile sBSP430hplPORTIE *)BSP430_PERIPH_PORT7)
#else /* IE */
#define BSP430_HPL_PORT7 ((volatile sBSP430hplPORT *)BSP430_PERIPH_PORT7)
#endif /* IE */
#endif /* configBSP430_HPL_PORT7 */

/** HPL pointer for PORT8.
 *
 * Typed pointer to a volatile structure overlaying the PORT8
 * peripheral register map.
 *
 * The pointer may be used only if #configBSP430_HPL_PORT8
 * is defined to a true value.
 *
 * @note The underlying type is #sBSP430hplPORTIE for ports 1 and 2
 * and ports on 5xx/6xx MCUs, and is #sBSP430hplPORT for ports 3
 * and higher on pre-5xx MCUs.  The generated documentation may not
 * reflect the correct structure.
 *
 * @dependency #configBSP430_HPL_PORT8 */
#if defined(BSP430_DOXYGEN) || (configBSP430_HPL_PORT8 - 0)
#if (BSP430_CORE_FAMILY_IS_5XX - 0) || (8 <= 2)
#define BSP430_HPL_PORT8 ((volatile sBSP430hplPORTIE *)BSP430_PERIPH_PORT8)
#else /* IE */
#define BSP430_HPL_PORT8 ((volatile sBSP430hplPORT *)BSP430_PERIPH_PORT8)
#endif /* IE */
#endif /* configBSP430_HPL_PORT8 */

/** HPL pointer for PORT9.
 *
 * Typed pointer to a volatile structure overlaying the PORT9
 * peripheral register map.
 *
 * The pointer may be used only if #configBSP430_HPL_PORT9
 * is defined to a true value.
 *
 * @note The underlying type is #sBSP430hplPORTIE for ports 1 and 2
 * and ports on 5xx/6xx MCUs, and is #sBSP430hplPORT for ports 3
 * and higher on pre-5xx MCUs.  The generated documentation may not
 * reflect the correct structure.
 *
 * @dependency #configBSP430_HPL_PORT9 */
#if defined(BSP430_DOXYGEN) || (configBSP430_HPL_PORT9 - 0)
#if (BSP430_CORE_FAMILY_IS_5XX - 0) || (9 <= 2)
#define BSP430_HPL_PORT9 ((volatile sBSP430hplPORTIE *)BSP430_PERIPH_PORT9)
#else /* IE */
#define BSP430_HPL_PORT9 ((volatile sBSP430hplPORT *)BSP430_PERIPH_PORT9)
#endif /* IE */
#endif /* configBSP430_HPL_PORT9 */

/** HPL pointer for PORT10.
 *
 * Typed pointer to a volatile structure overlaying the PORT10
 * peripheral register map.
 *
 * The pointer may be used only if #configBSP430_HPL_PORT10
 * is defined to a true value.
 *
 * @note The underlying type is #sBSP430hplPORTIE for ports 1 and 2
 * and ports on 5xx/6xx MCUs, and is #sBSP430hplPORT for ports 3
 * and higher on pre-5xx MCUs.  The generated documentation may not
 * reflect the correct structure.
 *
 * @dependency #configBSP430_HPL_PORT10 */
#if defined(BSP430_DOXYGEN) || (configBSP430_HPL_PORT10 - 0)
#if (BSP430_CORE_FAMILY_IS_5XX - 0) || (10 <= 2)
#define BSP430_HPL_PORT10 ((volatile sBSP430hplPORTIE *)BSP430_PERIPH_PORT10)
#else /* IE */
#define BSP430_HPL_PORT10 ((volatile sBSP430hplPORT *)BSP430_PERIPH_PORT10)
#endif /* IE */
#endif /* configBSP430_HPL_PORT10 */

/** HPL pointer for PORT11.
 *
 * Typed pointer to a volatile structure overlaying the PORT11
 * peripheral register map.
 *
 * The pointer may be used only if #configBSP430_HPL_PORT11
 * is defined to a true value.
 *
 * @note The underlying type is #sBSP430hplPORTIE for ports 1 and 2
 * and ports on 5xx/6xx MCUs, and is #sBSP430hplPORT for ports 3
 * and higher on pre-5xx MCUs.  The generated documentation may not
 * reflect the correct structure.
 *
 * @dependency #configBSP430_HPL_PORT11 */
#if defined(BSP430_DOXYGEN) || (configBSP430_HPL_PORT11 - 0)
#if (BSP430_CORE_FAMILY_IS_5XX - 0) || (11 <= 2)
#define BSP430_HPL_PORT11 ((volatile sBSP430hplPORTIE *)BSP430_PERIPH_PORT11)
#else /* IE */
#define BSP430_HPL_PORT11 ((volatile sBSP430hplPORT *)BSP430_PERIPH_PORT11)
#endif /* IE */
#endif /* configBSP430_HPL_PORT11 */

/* END AUTOMATICALLY GENERATED CODE [port_hpl_decl] */
/* !BSP430! end=port_hpl_decl */

/* !BSP430! insert=hal_port_isr_decl */
/* BEGIN AUTOMATICALLY GENERATED CODE---DO NOT MODIFY [hal_port_isr_decl] */
/** Control inclusion of the primary @HAL interrupt handler for #BSP430_PERIPH_PORT1
 *
 * This must be defined to 1 in @c bsp430_config.h to request the
 * interrupt handler be included, and 0 to request it be excluded.
 * Use of the interrupt handler requires that the corresponding @HAL
 * be requested.  By default the interface is included when
 * #configBSP430_HAL_PORT1 is set, but it may be explicitly
 * disabled if you intend to provide your own implementation or will
 * not be using the interrupt features.
 *
 * Interact with the handler using the @ref callbacks via #BSP430_HAL_PORT1.
 *
 * @cppflag
 * @ingroup grp_config_core
 * @defaulted */
#ifndef configBSP430_HAL_PORT1_ISR
#define configBSP430_HAL_PORT1_ISR ((configBSP430_HAL_PORT1 - 0) && defined(PORT1_VECTOR))
#endif /* configBSP430_HAL_PORT1_ISR */

#if (configBSP430_HAL_PORT1_ISR - 0) && ! (configBSP430_HAL_PORT1 - 0)
#warning configBSP430_HAL_PORT1_ISR requested without configBSP430_HAL_PORT1
#endif /* HAL_ISR and not HAL */

/** Control inclusion of the primary @HAL interrupt handler for #BSP430_PERIPH_PORT2
 *
 * This must be defined to 1 in @c bsp430_config.h to request the
 * interrupt handler be included, and 0 to request it be excluded.
 * Use of the interrupt handler requires that the corresponding @HAL
 * be requested.  By default the interface is included when
 * #configBSP430_HAL_PORT2 is set, but it may be explicitly
 * disabled if you intend to provide your own implementation or will
 * not be using the interrupt features.
 *
 * Interact with the handler using the @ref callbacks via #BSP430_HAL_PORT2.
 *
 * @cppflag
 * @ingroup grp_config_core
 * @defaulted */
#ifndef configBSP430_HAL_PORT2_ISR
#define configBSP430_HAL_PORT2_ISR ((configBSP430_HAL_PORT2 - 0) && defined(PORT2_VECTOR))
#endif /* configBSP430_HAL_PORT2_ISR */

#if (configBSP430_HAL_PORT2_ISR - 0) && ! (configBSP430_HAL_PORT2 - 0)
#warning configBSP430_HAL_PORT2_ISR requested without configBSP430_HAL_PORT2
#endif /* HAL_ISR and not HAL */

/** Control inclusion of the primary @HAL interrupt handler for #BSP430_PERIPH_PORT3
 *
 * This must be defined to 1 in @c bsp430_config.h to request the
 * interrupt handler be included, and 0 to request it be excluded.
 * Use of the interrupt handler requires that the corresponding @HAL
 * be requested.  By default the interface is included when
 * #configBSP430_HAL_PORT3 is set, but it may be explicitly
 * disabled if you intend to provide your own implementation or will
 * not be using the interrupt features.
 *
 * Interact with the handler using the @ref callbacks via #BSP430_HAL_PORT3.
 *
 * @cppflag
 * @ingroup grp_config_core
 * @defaulted */
#ifndef configBSP430_HAL_PORT3_ISR
#define configBSP430_HAL_PORT3_ISR ((configBSP430_HAL_PORT3 - 0) && defined(PORT3_VECTOR))
#endif /* configBSP430_HAL_PORT3_ISR */

#if (configBSP430_HAL_PORT3_ISR - 0) && ! (configBSP430_HAL_PORT3 - 0)
#warning configBSP430_HAL_PORT3_ISR requested without configBSP430_HAL_PORT3
#endif /* HAL_ISR and not HAL */

/** Control inclusion of the primary @HAL interrupt handler for #BSP430_PERIPH_PORT4
 *
 * This must be defined to 1 in @c bsp430_config.h to request the
 * interrupt handler be included, and 0 to request it be excluded.
 * Use of the interrupt handler requires that the corresponding @HAL
 * be requested.  By default the interface is included when
 * #configBSP430_HAL_PORT4 is set, but it may be explicitly
 * disabled if you intend to provide your own implementation or will
 * not be using the interrupt features.
 *
 * Interact with the handler using the @ref callbacks via #BSP430_HAL_PORT4.
 *
 * @cppflag
 * @ingroup grp_config_core
 * @defaulted */
#ifndef configBSP430_HAL_PORT4_ISR
#define configBSP430_HAL_PORT4_ISR ((configBSP430_HAL_PORT4 - 0) && defined(PORT4_VECTOR))
#endif /* configBSP430_HAL_PORT4_ISR */

#if (configBSP430_HAL_PORT4_ISR - 0) && ! (configBSP430_HAL_PORT4 - 0)
#warning configBSP430_HAL_PORT4_ISR requested without configBSP430_HAL_PORT4
#endif /* HAL_ISR and not HAL */

/** Control inclusion of the primary @HAL interrupt handler for #BSP430_PERIPH_PORT5
 *
 * This must be defined to 1 in @c bsp430_config.h to request the
 * interrupt handler be included, and 0 to request it be excluded.
 * Use of the interrupt handler requires that the corresponding @HAL
 * be requested.  By default the interface is included when
 * #configBSP430_HAL_PORT5 is set, but it may be explicitly
 * disabled if you intend to provide your own implementation or will
 * not be using the interrupt features.
 *
 * Interact with the handler using the @ref callbacks via #BSP430_HAL_PORT5.
 *
 * @cppflag
 * @ingroup grp_config_core
 * @defaulted */
#ifndef configBSP430_HAL_PORT5_ISR
#define configBSP430_HAL_PORT5_ISR ((configBSP430_HAL_PORT5 - 0) && defined(PORT5_VECTOR))
#endif /* configBSP430_HAL_PORT5_ISR */

#if (configBSP430_HAL_PORT5_ISR - 0) && ! (configBSP430_HAL_PORT5 - 0)
#warning configBSP430_HAL_PORT5_ISR requested without configBSP430_HAL_PORT5
#endif /* HAL_ISR and not HAL */

/** Control inclusion of the primary @HAL interrupt handler for #BSP430_PERIPH_PORT6
 *
 * This must be defined to 1 in @c bsp430_config.h to request the
 * interrupt handler be included, and 0 to request it be excluded.
 * Use of the interrupt handler requires that the corresponding @HAL
 * be requested.  By default the interface is included when
 * #configBSP430_HAL_PORT6 is set, but it may be explicitly
 * disabled if you intend to provide your own implementation or will
 * not be using the interrupt features.
 *
 * Interact with the handler using the @ref callbacks via #BSP430_HAL_PORT6.
 *
 * @cppflag
 * @ingroup grp_config_core
 * @defaulted */
#ifndef configBSP430_HAL_PORT6_ISR
#define configBSP430_HAL_PORT6_ISR ((configBSP430_HAL_PORT6 - 0) && defined(PORT6_VECTOR))
#endif /* configBSP430_HAL_PORT6_ISR */

#if (configBSP430_HAL_PORT6_ISR - 0) && ! (configBSP430_HAL_PORT6 - 0)
#warning configBSP430_HAL_PORT6_ISR requested without configBSP430_HAL_PORT6
#endif /* HAL_ISR and not HAL */

/** Control inclusion of the primary @HAL interrupt handler for #BSP430_PERIPH_PORT7
 *
 * This must be defined to 1 in @c bsp430_config.h to request the
 * interrupt handler be included, and 0 to request it be excluded.
 * Use of the interrupt handler requires that the corresponding @HAL
 * be requested.  By default the interface is included when
 * #configBSP430_HAL_PORT7 is set, but it may be explicitly
 * disabled if you intend to provide your own implementation or will
 * not be using the interrupt features.
 *
 * Interact with the handler using the @ref callbacks via #BSP430_HAL_PORT7.
 *
 * @cppflag
 * @ingroup grp_config_core
 * @defaulted */
#ifndef configBSP430_HAL_PORT7_ISR
#define configBSP430_HAL_PORT7_ISR ((configBSP430_HAL_PORT7 - 0) && defined(PORT7_VECTOR))
#endif /* configBSP430_HAL_PORT7_ISR */

#if (configBSP430_HAL_PORT7_ISR - 0) && ! (configBSP430_HAL_PORT7 - 0)
#warning configBSP430_HAL_PORT7_ISR requested without configBSP430_HAL_PORT7
#endif /* HAL_ISR and not HAL */

/** Control inclusion of the primary @HAL interrupt handler for #BSP430_PERIPH_PORT8
 *
 * This must be defined to 1 in @c bsp430_config.h to request the
 * interrupt handler be included, and 0 to request it be excluded.
 * Use of the interrupt handler requires that the corresponding @HAL
 * be requested.  By default the interface is included when
 * #configBSP430_HAL_PORT8 is set, but it may be explicitly
 * disabled if you intend to provide your own implementation or will
 * not be using the interrupt features.
 *
 * Interact with the handler using the @ref callbacks via #BSP430_HAL_PORT8.
 *
 * @cppflag
 * @ingroup grp_config_core
 * @defaulted */
#ifndef configBSP430_HAL_PORT8_ISR
#define configBSP430_HAL_PORT8_ISR ((configBSP430_HAL_PORT8 - 0) && defined(PORT8_VECTOR))
#endif /* configBSP430_HAL_PORT8_ISR */

#if (configBSP430_HAL_PORT8_ISR - 0) && ! (configBSP430_HAL_PORT8 - 0)
#warning configBSP430_HAL_PORT8_ISR requested without configBSP430_HAL_PORT8
#endif /* HAL_ISR and not HAL */

/** Control inclusion of the primary @HAL interrupt handler for #BSP430_PERIPH_PORT9
 *
 * This must be defined to 1 in @c bsp430_config.h to request the
 * interrupt handler be included, and 0 to request it be excluded.
 * Use of the interrupt handler requires that the corresponding @HAL
 * be requested.  By default the interface is included when
 * #configBSP430_HAL_PORT9 is set, but it may be explicitly
 * disabled if you intend to provide your own implementation or will
 * not be using the interrupt features.
 *
 * Interact with the handler using the @ref callbacks via #BSP430_HAL_PORT9.
 *
 * @cppflag
 * @ingroup grp_config_core
 * @defaulted */
#ifndef configBSP430_HAL_PORT9_ISR
#define configBSP430_HAL_PORT9_ISR ((configBSP430_HAL_PORT9 - 0) && defined(PORT9_VECTOR))
#endif /* configBSP430_HAL_PORT9_ISR */

#if (configBSP430_HAL_PORT9_ISR - 0) && ! (configBSP430_HAL_PORT9 - 0)
#warning configBSP430_HAL_PORT9_ISR requested without configBSP430_HAL_PORT9
#endif /* HAL_ISR and not HAL */

/** Control inclusion of the primary @HAL interrupt handler for #BSP430_PERIPH_PORT10
 *
 * This must be defined to 1 in @c bsp430_config.h to request the
 * interrupt handler be included, and 0 to request it be excluded.
 * Use of the interrupt handler requires that the corresponding @HAL
 * be requested.  By default the interface is included when
 * #configBSP430_HAL_PORT10 is set, but it may be explicitly
 * disabled if you intend to provide your own implementation or will
 * not be using the interrupt features.
 *
 * Interact with the handler using the @ref callbacks via #BSP430_HAL_PORT10.
 *
 * @cppflag
 * @ingroup grp_config_core
 * @defaulted */
#ifndef configBSP430_HAL_PORT10_ISR
#define configBSP430_HAL_PORT10_ISR ((configBSP430_HAL_PORT10 - 0) && defined(PORT10_VECTOR))
#endif /* configBSP430_HAL_PORT10_ISR */

#if (configBSP430_HAL_PORT10_ISR - 0) && ! (configBSP430_HAL_PORT10 - 0)
#warning configBSP430_HAL_PORT10_ISR requested without configBSP430_HAL_PORT10
#endif /* HAL_ISR and not HAL */

/** Control inclusion of the primary @HAL interrupt handler for #BSP430_PERIPH_PORT11
 *
 * This must be defined to 1 in @c bsp430_config.h to request the
 * interrupt handler be included, and 0 to request it be excluded.
 * Use of the interrupt handler requires that the corresponding @HAL
 * be requested.  By default the interface is included when
 * #configBSP430_HAL_PORT11 is set, but it may be explicitly
 * disabled if you intend to provide your own implementation or will
 * not be using the interrupt features.
 *
 * Interact with the handler using the @ref callbacks via #BSP430_HAL_PORT11.
 *
 * @cppflag
 * @ingroup grp_config_core
 * @defaulted */
#ifndef configBSP430_HAL_PORT11_ISR
#define configBSP430_HAL_PORT11_ISR ((configBSP430_HAL_PORT11 - 0) && defined(PORT11_VECTOR))
#endif /* configBSP430_HAL_PORT11_ISR */

#if (configBSP430_HAL_PORT11_ISR - 0) && ! (configBSP430_HAL_PORT11 - 0)
#warning configBSP430_HAL_PORT11_ISR requested without configBSP430_HAL_PORT11
#endif /* HAL_ISR and not HAL */

/* END AUTOMATICALLY GENERATED CODE [hal_port_isr_decl] */
/* !BSP430! end=hal_port_isr_decl */

/** Get the peripheral register pointer for an interrupt-enabled port.
 *
 * @note All ports on 5xx-family devices are interrupt-enabled.
 *
 * @param periph The handle identifier, such as #BSP430_PERIPH_PORT1.
 *
 * @return A typed pointer that can be used to manipulate the port.  A
 * null pointer is returned if the handle does not correspond to a
 * timer which has been enabled (e.g., with
 * #configBSP430_HPL_PORT1), or if the specified port does not
 * support interrupts (see #xBSP430hplLookupPORT)
 */
static BSP430_CORE_INLINE
volatile sBSP430hplPORTIE *
xBSP430hplLookupPORTIE (tBSP430periphHandle periph)
{
  /* !BSP430! ie_test=<= subst=ie_test insert=periph_hpl_port_demux */
  /* BEGIN AUTOMATICALLY GENERATED CODE---DO NOT MODIFY [periph_hpl_port_demux] */
#if (configBSP430_HPL_PORT1 - 0) && ((BSP430_CORE_FAMILY_IS_5XX - 0) || (1 <= 2))
  if (BSP430_PERIPH_PORT1 == periph) {
    return BSP430_HPL_PORT1;
  }
#endif /* configBSP430_HPL_PORT1 */

#if (configBSP430_HPL_PORT2 - 0) && ((BSP430_CORE_FAMILY_IS_5XX - 0) || (2 <= 2))
  if (BSP430_PERIPH_PORT2 == periph) {
    return BSP430_HPL_PORT2;
  }
#endif /* configBSP430_HPL_PORT2 */

#if (configBSP430_HPL_PORT3 - 0) && ((BSP430_CORE_FAMILY_IS_5XX - 0) || (3 <= 2))
  if (BSP430_PERIPH_PORT3 == periph) {
    return BSP430_HPL_PORT3;
  }
#endif /* configBSP430_HPL_PORT3 */

#if (configBSP430_HPL_PORT4 - 0) && ((BSP430_CORE_FAMILY_IS_5XX - 0) || (4 <= 2))
  if (BSP430_PERIPH_PORT4 == periph) {
    return BSP430_HPL_PORT4;
  }
#endif /* configBSP430_HPL_PORT4 */

#if (configBSP430_HPL_PORT5 - 0) && ((BSP430_CORE_FAMILY_IS_5XX - 0) || (5 <= 2))
  if (BSP430_PERIPH_PORT5 == periph) {
    return BSP430_HPL_PORT5;
  }
#endif /* configBSP430_HPL_PORT5 */

#if (configBSP430_HPL_PORT6 - 0) && ((BSP430_CORE_FAMILY_IS_5XX - 0) || (6 <= 2))
  if (BSP430_PERIPH_PORT6 == periph) {
    return BSP430_HPL_PORT6;
  }
#endif /* configBSP430_HPL_PORT6 */

#if (configBSP430_HPL_PORT7 - 0) && ((BSP430_CORE_FAMILY_IS_5XX - 0) || (7 <= 2))
  if (BSP430_PERIPH_PORT7 == periph) {
    return BSP430_HPL_PORT7;
  }
#endif /* configBSP430_HPL_PORT7 */

#if (configBSP430_HPL_PORT8 - 0) && ((BSP430_CORE_FAMILY_IS_5XX - 0) || (8 <= 2))
  if (BSP430_PERIPH_PORT8 == periph) {
    return BSP430_HPL_PORT8;
  }
#endif /* configBSP430_HPL_PORT8 */

#if (configBSP430_HPL_PORT9 - 0) && ((BSP430_CORE_FAMILY_IS_5XX - 0) || (9 <= 2))
  if (BSP430_PERIPH_PORT9 == periph) {
    return BSP430_HPL_PORT9;
  }
#endif /* configBSP430_HPL_PORT9 */

#if (configBSP430_HPL_PORT10 - 0) && ((BSP430_CORE_FAMILY_IS_5XX - 0) || (10 <= 2))
  if (BSP430_PERIPH_PORT10 == periph) {
    return BSP430_HPL_PORT10;
  }
#endif /* configBSP430_HPL_PORT10 */

#if (configBSP430_HPL_PORT11 - 0) && ((BSP430_CORE_FAMILY_IS_5XX - 0) || (11 <= 2))
  if (BSP430_PERIPH_PORT11 == periph) {
    return BSP430_HPL_PORT11;
  }
#endif /* configBSP430_HPL_PORT11 */

  /* END AUTOMATICALLY GENERATED CODE [periph_hpl_port_demux] */
  /* !BSP430! end=periph_hpl_port_demux */
  return NULL;
}

/** Get the peripheral register pointer for a non-interrupt-enabled port.
 *
 * @param periph The handle identifier, such as #BSP430_PERIPH_PORT1.
 *
 * @return A typed pointer that can be used to manipulate the port.  A
 * null pointer is returned if the handle does not correspond to a
 * timer which has been enabled (e.g., with
 * #configBSP430_HPL_PORT1), or if the specified port supports
 * interrupts (see #xBSP430hplLookupPORTIE)
 */
static BSP430_CORE_INLINE
volatile sBSP430hplPORT *
xBSP430hplLookupPORT (tBSP430periphHandle periph)
{
  /* !BSP430! ie_test=> subst=ie_test insert=periph_hpl_port_demux */
  /* BEGIN AUTOMATICALLY GENERATED CODE---DO NOT MODIFY [periph_hpl_port_demux] */
#if (configBSP430_HPL_PORT1 - 0) && ((BSP430_CORE_FAMILY_IS_5XX - 0) || (1 > 2))
  if (BSP430_PERIPH_PORT1 == periph) {
    return BSP430_HPL_PORT1;
  }
#endif /* configBSP430_HPL_PORT1 */

#if (configBSP430_HPL_PORT2 - 0) && ((BSP430_CORE_FAMILY_IS_5XX - 0) || (2 > 2))
  if (BSP430_PERIPH_PORT2 == periph) {
    return BSP430_HPL_PORT2;
  }
#endif /* configBSP430_HPL_PORT2 */

#if (configBSP430_HPL_PORT3 - 0) && ((BSP430_CORE_FAMILY_IS_5XX - 0) || (3 > 2))
  if (BSP430_PERIPH_PORT3 == periph) {
    return BSP430_HPL_PORT3;
  }
#endif /* configBSP430_HPL_PORT3 */

#if (configBSP430_HPL_PORT4 - 0) && ((BSP430_CORE_FAMILY_IS_5XX - 0) || (4 > 2))
  if (BSP430_PERIPH_PORT4 == periph) {
    return BSP430_HPL_PORT4;
  }
#endif /* configBSP430_HPL_PORT4 */

#if (configBSP430_HPL_PORT5 - 0) && ((BSP430_CORE_FAMILY_IS_5XX - 0) || (5 > 2))
  if (BSP430_PERIPH_PORT5 == periph) {
    return BSP430_HPL_PORT5;
  }
#endif /* configBSP430_HPL_PORT5 */

#if (configBSP430_HPL_PORT6 - 0) && ((BSP430_CORE_FAMILY_IS_5XX - 0) || (6 > 2))
  if (BSP430_PERIPH_PORT6 == periph) {
    return BSP430_HPL_PORT6;
  }
#endif /* configBSP430_HPL_PORT6 */

#if (configBSP430_HPL_PORT7 - 0) && ((BSP430_CORE_FAMILY_IS_5XX - 0) || (7 > 2))
  if (BSP430_PERIPH_PORT7 == periph) {
    return BSP430_HPL_PORT7;
  }
#endif /* configBSP430_HPL_PORT7 */

#if (configBSP430_HPL_PORT8 - 0) && ((BSP430_CORE_FAMILY_IS_5XX - 0) || (8 > 2))
  if (BSP430_PERIPH_PORT8 == periph) {
    return BSP430_HPL_PORT8;
  }
#endif /* configBSP430_HPL_PORT8 */

#if (configBSP430_HPL_PORT9 - 0) && ((BSP430_CORE_FAMILY_IS_5XX - 0) || (9 > 2))
  if (BSP430_PERIPH_PORT9 == periph) {
    return BSP430_HPL_PORT9;
  }
#endif /* configBSP430_HPL_PORT9 */

#if (configBSP430_HPL_PORT10 - 0) && ((BSP430_CORE_FAMILY_IS_5XX - 0) || (10 > 2))
  if (BSP430_PERIPH_PORT10 == periph) {
    return BSP430_HPL_PORT10;
  }
#endif /* configBSP430_HPL_PORT10 */

#if (configBSP430_HPL_PORT11 - 0) && ((BSP430_CORE_FAMILY_IS_5XX - 0) || (11 > 2))
  if (BSP430_PERIPH_PORT11 == periph) {
    return BSP430_HPL_PORT11;
  }
#endif /* configBSP430_HPL_PORT11 */

  /* END AUTOMATICALLY GENERATED CODE [periph_hpl_port_demux] */
  /* !BSP430! end=periph_hpl_port_demux */
  return NULL;
}

/** Get the HAL handle for a specific port.
 *
 * @param periph The handle identifier, such as #BSP430_PERIPH_TA0.
 *
 * @return the HAL handle for the port.  A null pointer is returned
 * if the handle does not correspond to a port for which the HAL
 * interface has been enabled (e.g., with #configBSP430_HAL_PORT1).
 */
static BSP430_CORE_INLINE
hBSP430halPORT
hBSP430portLookup (tBSP430periphHandle periph)
{
  /* !BSP430! insert=periph_hal_demux */
  /* BEGIN AUTOMATICALLY GENERATED CODE---DO NOT MODIFY [periph_hal_demux] */
#if (configBSP430_HAL_PORT1 - 0)
  if (BSP430_PERIPH_PORT1 == periph) {
    return BSP430_HAL_PORT1;
  }
#endif /* configBSP430_HAL_PORT1 */

#if (configBSP430_HAL_PORT2 - 0)
  if (BSP430_PERIPH_PORT2 == periph) {
    return BSP430_HAL_PORT2;
  }
#endif /* configBSP430_HAL_PORT2 */

#if (configBSP430_HAL_PORT3 - 0)
  if (BSP430_PERIPH_PORT3 == periph) {
    return BSP430_HAL_PORT3;
  }
#endif /* configBSP430_HAL_PORT3 */

#if (configBSP430_HAL_PORT4 - 0)
  if (BSP430_PERIPH_PORT4 == periph) {
    return BSP430_HAL_PORT4;
  }
#endif /* configBSP430_HAL_PORT4 */

#if (configBSP430_HAL_PORT5 - 0)
  if (BSP430_PERIPH_PORT5 == periph) {
    return BSP430_HAL_PORT5;
  }
#endif /* configBSP430_HAL_PORT5 */

#if (configBSP430_HAL_PORT6 - 0)
  if (BSP430_PERIPH_PORT6 == periph) {
    return BSP430_HAL_PORT6;
  }
#endif /* configBSP430_HAL_PORT6 */

#if (configBSP430_HAL_PORT7 - 0)
  if (BSP430_PERIPH_PORT7 == periph) {
    return BSP430_HAL_PORT7;
  }
#endif /* configBSP430_HAL_PORT7 */

#if (configBSP430_HAL_PORT8 - 0)
  if (BSP430_PERIPH_PORT8 == periph) {
    return BSP430_HAL_PORT8;
  }
#endif /* configBSP430_HAL_PORT8 */

#if (configBSP430_HAL_PORT9 - 0)
  if (BSP430_PERIPH_PORT9 == periph) {
    return BSP430_HAL_PORT9;
  }
#endif /* configBSP430_HAL_PORT9 */

#if (configBSP430_HAL_PORT10 - 0)
  if (BSP430_PERIPH_PORT10 == periph) {
    return BSP430_HAL_PORT10;
  }
#endif /* configBSP430_HAL_PORT10 */

#if (configBSP430_HAL_PORT11 - 0)
  if (BSP430_PERIPH_PORT11 == periph) {
    return BSP430_HAL_PORT11;
  }
#endif /* configBSP430_HAL_PORT11 */

  /* END AUTOMATICALLY GENERATED CODE [periph_hal_demux] */
  /* !BSP430! end=periph_hal_demux */
  return NULL;
}

/** Get a human-readable identifier for the port
 *
 * @param periph The handle identifier, such as #BSP430_PERIPH_PORT1.
 *
 * @return The short name of the port, e.g. "P1".  If the peripheral
 * is not recognized as a port, a null pointer is returned. */
const char * xBSP430portName (tBSP430periphHandle periph);

/** Convert from a bit value to a pin value.
 *
 * Mostly used for informational messages, but also for ISR IFG
 * queries.  The reverse operation is simply a left shift.
 *
 * @param bitx The value of a bit within a byte-addressed port @return
 * the index of the first bit that is set within bitx; 0 for a bitx
 * value of 0x01, 3 for a bitx value of 0x08, etc.  The return value
 * is -1 if no bit is set in the low byte of @p bitx.
 *
 * @warning This function is explicitly intended for support of 8-bit
 * ports, and cannot be used to detect the position of the lowest set
 * bit in a wider integer. */
static BSP430_CORE_INLINE
int
iBSP430portBitPosition (unsigned int bitx)
{
  unsigned char bit = 0x01;
  int rv = 0;
  while (bit && !(bit & bitx)) {
    bit <<= 1;
    ++rv;
  }
  return bit ? rv : -1;
}

#endif /* BSP430_MODULE_PORT */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BSP430_PERIPH_PORT_H */
