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

#include <bsp430/platform.h>
#include <bsp430/clock.h>
#include <bsp430/serial.h>
#include <bsp430/periph/eusci.h>

/* eUSCI on FR4xx/2xx devices uses UCSSEL_1 to identify MODCLK at 5
 * MHz instead of ACLK at whatever. */
#define UCSSEL_1_IS_MODCLK (defined(UCSSEL__MODCLK) && !defined(UCSSEL__ACLK) && (UCSSEL__MODCLK == UCSSEL_1))

#define SERIAL_HAL_HPL_A(hal_) (hal_)->hpl.euscia
#define SERIAL_HAL_HPL_B(hal_) (hal_)->hpl.euscib
#define HAL_HPL_FIELD(hal_,fld_) (*(BSP430_SERIAL_HAL_HPL_VARIANT_IS_EUSCIB(hal_) ? &(hal_)->hpl.euscib->fld_ : &(hal_)->hpl.euscia->fld_))

#define MODE_IS_I2C(hal_) ((UCSYNC | UCMODE_3) == ((UCSYNC | UCMODE_3) & HAL_HPL_FIELD(hal_,ctlw0)))

#define SERIAL_HAL_WAKEUP_TRANSMIT_RH(hal_) do {                        \
    HAL_HPL_FIELD(hal_,ie) |= UCTXIE;                                   \
  } while (0)

#define UART_RAW_TRANSMIT_RH(hal_, _c) do {             \
    while (! (SERIAL_HAL_HPL_A(hal_)->ifg & UCTXIFG)) { \
      ;                                                 \
    }                                                   \
    ++(hal_)->num_tx;                                   \
    SERIAL_HAL_HPL_A(hal_)->txbuf = _c;                 \
  } while (0)

#define SERIAL_HAL_FLUSH_NI(hal_) do {                  \
    while (HAL_HPL_FIELD(hal_,statw) & UCBUSY) {        \
      ;                                                 \
    }                                                   \
  } while (0)

/** Inspect bits in CTL0 to determine the appropriate peripheral
 * configuration. */
static BSP430_CORE_INLINE
int
peripheralConfigFlag (unsigned int ctlw0)
{
  if (0 == (ctlw0 & UCSYNC)) {
    return BSP430_PERIPHCFG_SERIAL_UART;
  }
  switch (ctlw0 & (UCMODE1 | UCMODE0)) {
    case 0:
      return BSP430_PERIPHCFG_SERIAL_SPI3;
    case UCMODE0:
    case UCMODE1:
      return BSP430_PERIPHCFG_SERIAL_SPI4;
    case UCMODE0 | UCMODE1:
      return BSP430_PERIPHCFG_SERIAL_I2C;
  }
  return 0;
}

static
unsigned long
ulBSP430eusciRate (hBSP430halSERIAL hal)
{
  unsigned long clock_Hz;
  switch (UCSSEL_3 & HAL_HPL_FIELD(hal,ctlw0)) {
    case UCSSEL_0:
      clock_Hz = 0;
      break;
    case UCSSEL_1:
#if (UCSSEL_1_IS_MODCLK - 0)
      clock_Hz = BSP430_NOMINAL_MODCLK_HZ;
#else /* UCSSEL_1_IS_MODCLK */
      clock_Hz = ulBSP430clockACLK_Hz();
#endif /* UCSSEL_1_IS_MODCLK */
      break;
    default:
      clock_Hz = ulBSP430clockSMCLK_Hz();
      break;
  }
  return clock_Hz / HAL_HPL_FIELD(hal,brw);
}

static
hBSP430halSERIAL
eusciConfigure (hBSP430halSERIAL hal,
                unsigned int ctlw0,
                unsigned int ctlw1,
                unsigned int brw,
                unsigned int mctlw,
                int set_mctl)
{
  BSP430_CORE_SAVED_INTERRUPT_STATE(istate);

  BSP430_CORE_DISABLE_INTERRUPT();
  HAL_HPL_FIELD(hal,ctlw0) = UCSWRST;
  do {
    int periph_config = peripheralConfigFlag(ctlw0);

    /* Reject if the pins can't be configured */
    if (0 != iBSP430platformConfigurePeripheralPins_ni(xBSP430periphFromHPL(hal->hpl.any), periph_config, 1)) {
      hal = NULL;
      break;
    }
    HAL_HPL_FIELD(hal, ctlw0) |= ctlw0;
    HAL_HPL_FIELD(hal, ctlw1) = ctlw1;
    HAL_HPL_FIELD(hal, brw) = brw;
    if (set_mctl) {
      SERIAL_HAL_HPL_A(hal)->mctlw = mctlw;
    }

    /* Reset device statistics */
    hal->num_rx = hal->num_tx = 0;

    /* Release the device for use */
    iBSP430eusciSetReset_rh(hal, 0);
  } while (0);
  BSP430_CORE_RESTORE_INTERRUPT_STATE(istate);

  return hal;
}

hBSP430halSERIAL
hBSP430eusciOpenUART (hBSP430halSERIAL hal,
                      unsigned char ctl0_byte,
                      unsigned char ctl1_byte,
                      unsigned long baud)
{
  unsigned long brclk_Hz;
  unsigned long n;
  unsigned int ctlw0;
  unsigned int brw;
  unsigned int mctlw;
  unsigned int os16 = 0;
  unsigned int brf = 0;
  unsigned int brs;

  /* Reject unsupported HALs */
  if ((NULL == hal)
      || (! BSP430_SERIAL_HAL_HPL_VARIANT_IS_EUSCIA(hal))) {
    return NULL;
  }

  /* Reject invalid baud rates */
  if ((0 == baud) || (BSP430_EUSCI_UART_MAX_BAUD < baud)) {
    return NULL;
  }

  /* Force to UART async and wipe out the clock select fields */
  ctlw0 = (ctl0_byte << 8) | ctl1_byte;
  ctlw0 &= ~(UCMODE1 | UCMODE0 | UCSYNC | UCSSEL1 | UCSSEL0);

#if (UCSSEL_1_IS_MODCLK - 0)
  /* When ACLK is not available, use SMCLK: MODCLK is only available
   * in active mode and LPM0 so there's no real win over SMCLK, and
   * MODCLK is pretty unstable (+/- 20%). */
  ctlw0 |= UCSSEL__SMCLK;
  brclk_Hz = ulBSP430clockSMCLK_Hz_ni();
#else /* BSP430_PERIPH_EUSCI_IS_FR4 */
  /* Assume ACLK <= 20 kHz is VLOCLK and cannot be trusted.  Prefer
   * 32 KiHz ACLK for rates that are low enough.  Use SMCLK for
   * anything larger.  */
  brclk_Hz = ulBSP430clockACLK_Hz_ni();
  if ((brclk_Hz > 20000) && (brclk_Hz >= (3 * baud))) {
    ctlw0 |= UCSSEL__ACLK;
  } else {
    ctlw0 |= UCSSEL__SMCLK;
    brclk_Hz = ulBSP430clockSMCLK_Hz_ni();
  }
#endif /* BSP430_PERIPH_EUSCI_IS_FR4 */

#define BR_FRACTION_SHIFT 6
  /* The value for BRS is supposed to be a table lookup based on the
   * fractional part of f_brclk / baud.  Rather than replicate the
   * table, we simply preserve BR_FRACTION_SHIFT bits of the
   * fraction, then use that as the upper bits of the value of
   * BRS.  Seems to work, at least for 9600 baud. */
  n = (brclk_Hz << BR_FRACTION_SHIFT) / baud;
  brs = n & ((1 << BR_FRACTION_SHIFT) - 1);
  n >>= BR_FRACTION_SHIFT;
  brs <<= 8 - BR_FRACTION_SHIFT;
#undef BR_FRACTION_SHIFT
  brw = n;
  if (16 <= brw) {
    brw = brw / 16;
    os16 = UCOS16;
    brf = n - 16 * brw;
  }
  mctlw = (brf * UCBRF0) | (brs * UCBRS0) | os16;

  return eusciConfigure(hal, ctlw0, 0, brw, mctlw, 1);
}

hBSP430halSERIAL
hBSP430eusciOpenSPI (hBSP430halSERIAL hal,
                     unsigned char ctl0_byte,
                     unsigned char ctl1_byte,
                     unsigned int prescaler)
{
  unsigned int ctlw0;

  /* Reject unsupported HALs */
  if (NULL == hal) {
    return NULL;
  }

  /* Reject invalid mode */
  ctlw0 = (ctl0_byte << 8) | ctl1_byte;
  if (UCMODE_3 == (ctlw0 & (UCMODE0 | UCMODE1))) {
    return NULL;
  }

  /* Calculate default prescaler */
  if (0 == prescaler) {
    prescaler = uiBSP430serialSMCLKPrescaler(BSP430_SERIAL_SPI_BUS_SPEED_HZ);
    ctlw0 |= UCSSEL1;
  }

  /* SPI is synchronous */
  ctlw0 |= UCSYNC;

  return eusciConfigure(hal, ctlw0, 0, prescaler, 0, 0);
}

static void
i2cSetAutoStop_ni (hBSP430halSERIAL hal,
                   int enablep)
{
  /* Only reconfigure if not already in the desired mode */
  if (!!enablep != !!(UCASTP_2 == (UCASTP_3 & HAL_HPL_FIELD(hal, ctlw1)))) {
    unsigned int ctlw1;

    iBSP430eusciSetReset_rh(hal, 1);
    ctlw1 = HAL_HPL_FIELD(hal, ctlw1);
    ctlw1 &= ~UCASTP_3;
    ctlw1 |= (enablep ? UCASTP_2 : UCASTP_0);
    HAL_HPL_FIELD(hal, ctlw1) = ctlw1;
    iBSP430eusciSetReset_rh(hal, 0);
  }
}

hBSP430halSERIAL
hBSP430eusciOpenI2C (hBSP430halSERIAL hal,
                     unsigned char ctl0_byte,
                     unsigned char ctl1_byte,
                     unsigned int prescaler)
{
  unsigned int ctlw0;

  /* Reject unsupported HALs */
  if ((NULL == hal)
      || (! BSP430_SERIAL_HAL_HPL_VARIANT_IS_EUSCIB(hal))) {
    return NULL;
  }

  /* I2C is synchronous mode 3 */
  ctlw0 = (ctl0_byte << 8) | ctl1_byte;
  ctlw0 |= UCMODE_3 | UCSYNC;

  /* Calculate default prescaler */
  if (0 == prescaler) {
    prescaler = uiBSP430serialSMCLKPrescaler(BSP430_SERIAL_I2C_BUS_SPEED_HZ);
    ctlw0 |= UCSSEL1;
  }

  /* Using UCASTP_2 is recommended for single-byte writes, but since
   * it can't be reconfigured without putting the device in reset,
   * we'll use it for everything.  Which means the spinning receive
   * and transmit are limited to 255-byte transactions. */
  return eusciConfigure(hal, ctlw0, UCASTP_2, prescaler, 0, 0);
}

int
iBSP430eusciSetReset_rh (hBSP430halSERIAL hal,
                         int resetp)
{
  BSP430_CORE_SAVED_INTERRUPT_STATE(istate);
  int rc;

  BSP430_CORE_DISABLE_INTERRUPT();
  do {
    rc = !!(HAL_HPL_FIELD(hal,ctlw0) & UCSWRST);
    if (resetp) {
      if (0 > resetp) {
        SERIAL_HAL_FLUSH_NI(hal);
      }
      HAL_HPL_FIELD(hal,ctlw0) |= UCSWRST;
    } else {
      if (MODE_IS_I2C(hal)) {
        HAL_HPL_FIELD(hal,ctlw0) &= ~(UCTXACK | UCTXNACK | UCTXSTP | UCTXSTT);
      }
      HAL_HPL_FIELD(hal,ctlw0) &= ~UCSWRST;
      if (hal->rx_cbchain_ni) {
        HAL_HPL_FIELD(hal,ie) |= UCRXIE;
      }
    }
  } while (0);
  BSP430_CORE_RESTORE_INTERRUPT_STATE(istate);
  return rc;
}

int
iBSP430eusciSetHold_rh (hBSP430halSERIAL hal,
                        int holdp)
{
  BSP430_CORE_SAVED_INTERRUPT_STATE(istate);
  int rc;
  int periph_config = peripheralConfigFlag(HAL_HPL_FIELD(hal,ctlw0));

  BSP430_CORE_DISABLE_INTERRUPT();
  do {
    if (holdp) {
      rc = iBSP430eusciSetReset_rh(hal, -1);
      (void)iBSP430platformConfigurePeripheralPins_ni(xBSP430periphFromHPL(hal->hpl.any), periph_config, 0);
    } else {
      (void)iBSP430platformConfigurePeripheralPins_ni(xBSP430periphFromHPL(hal->hpl.any), periph_config, 1);
      rc = iBSP430eusciSetReset_rh(hal, 0);
    }
  } while (0);
  BSP430_CORE_RESTORE_INTERRUPT_STATE(istate);
  return rc;
}

int
iBSP430eusciClose (hBSP430halSERIAL hal)
{
  BSP430_CORE_SAVED_INTERRUPT_STATE(istate);
  int rc;

  BSP430_CORE_DISABLE_INTERRUPT();
  do {
    HAL_HPL_FIELD(hal,ctlw0) = UCSWRST;
    rc = iBSP430platformConfigurePeripheralPins_ni(xBSP430periphFromHPL(hal->hpl.any),
                                                   peripheralConfigFlag(HAL_HPL_FIELD(hal,ctlw0)),
                                                   0);
  } while (0);
  BSP430_CORE_RESTORE_INTERRUPT_STATE(istate);

  return rc;
}

void
vBSP430eusciFlush_ni (hBSP430halSERIAL hal)
{
  SERIAL_HAL_FLUSH_NI(hal);
}

void
vBSP430eusciWakeupTransmit_rh (hBSP430halSERIAL hal)
{
  SERIAL_HAL_WAKEUP_TRANSMIT_RH(hal);
}

int
iBSP430eusciUARTrxByte_rh (hBSP430halSERIAL hal)
{
  if (hal->rx_cbchain_ni) {
    return -1;
  }
  if (SERIAL_HAL_HPL_A(hal)->ifg & UCRXIFG) {
    ++hal->num_rx;
    return SERIAL_HAL_HPL_A(hal)->rxbuf;
  }
  return -1;
}

int
iBSP430eusciUARTtxByte_rh (hBSP430halSERIAL hal, uint8_t c)
{
  if (hal->tx_cbchain_ni) {
    return -1;
  }
  UART_RAW_TRANSMIT_RH(hal, c);
  return c;
}

int
iBSP430eusciUARTtxData_rh (hBSP430halSERIAL hal,
                           const uint8_t * data,
                           size_t len)
{
  const uint8_t * p = data;
  const uint8_t * edata = data + len;
  if (hal->tx_cbchain_ni) {
    return -1;
  }
  while (p < edata) {
    UART_RAW_TRANSMIT_RH(hal, *p++);
  }
  return p - data;
}

int
iBSP430eusciUARTtxASCIIZ_rh (hBSP430halSERIAL hal, const char * str)
{
  const char * in_string = str;

  if (hal->tx_cbchain_ni) {
    return -1;
  }
  while (*str) {
    UART_RAW_TRANSMIT_RH(hal, *str);
    ++str;
  }
  return str - in_string;
}

int
iBSP430eusciSPITxRx_rh (hBSP430halSERIAL hal,
                        const uint8_t * tx_data,
                        size_t tx_len,
                        size_t rx_len,
                        uint8_t * rx_data)
{
  size_t transaction_length = tx_len + rx_len;
  size_t i = 0;

  volatile unsigned int * ifgp = &HAL_HPL_FIELD(hal, ifg);
  volatile unsigned int * txbp = &HAL_HPL_FIELD(hal, txbuf);
  volatile unsigned int * rxbp = &HAL_HPL_FIELD(hal, rxbuf);

  if (hal->tx_cbchain_ni) {
    return -1;
  }
  while (i < transaction_length) {
    uint8_t rx_dummy;

    while (! (UCTXIFG & *ifgp)) {
      ;
    }
    ++hal->num_tx;
    *txbp = (i < tx_len) ? tx_data[i] : BSP430_SERIAL_SPI_READ_TX_BYTE(i-tx_len);
    while (! (UCRXIFG & *ifgp)) {
      ;
    }
    ++hal->num_rx;
    rx_dummy = *rxbp;
    if (rx_data) {
      *rx_data++ = rx_dummy;
    }
    ++i;
  }
  return i;
}

int
iBSP430eusciI2CsetAddresses_rh (hBSP430halSERIAL hal,
                                int own_address,
                                int slave_address)
{
  if (0 <= own_address) {
    SERIAL_HAL_HPL_B(hal)->i2coa0 = own_address;
  }
  if (0 <= slave_address) {
    SERIAL_HAL_HPL_B(hal)->i2csa = slave_address;
  }
  return 0;
}

/** Check for standard I2C transaction-aborting errors; if present,
 * return from the containing function with a negative error code. */
#define I2C_ERRCHECK_RETURN() do {                              \
    unsigned int ifg = hpl->ifg;                                \
    if (ifg & (UCNACKIFG | UCALIFG)) {                          \
      return -(BSP430_I2C_ERRFLAG_PROTOCOL | (0x0FF & ifg));    \
    }                                                           \
  } while (0)

/** Loop repeatedly (at least once) until @a c_ evaluates to false.
 *
 * Within the loop, an I2C transaction-aborting error will result in
 * the containing function returning a negative error code
 * <tt>-(#BSP430_I2C_ERRFLAG_PROTOCOL | code)</tt> for some code.
 *
 * If #BSP430_I2C_SPIN_LIMIT is positive and @a c_ remains true while
 * the loop executes #BSP430_I2C_SPIN_LIMIT times, the containing
 * function will return the negative error code
 * #BSP430_I2C_ERRFLAG_SPINLIMIT.
 *
 * If #BSP430_I2C_SPIN_LIMIT is zero or negative, the loop will
 * execute an unbounded number of times until @a c_ becomes false.
 */
#if defined(BSP430_DOXYGEN) || (0 < BSP430_I2C_SPIN_LIMIT)
#define I2C_ERRCHECK_SPIN_WHILE_COND(c_) do {                   \
    unsigned int limit = (unsigned int)BSP430_I2C_SPIN_LIMIT;   \
    while (1) {                                                 \
      I2C_ERRCHECK_RETURN();                                    \
      if (! (c_)) {                                             \
        break;                                                  \
      }                                                         \
      if (0 == --limit) {                                       \
        return -BSP430_I2C_ERRFLAG_SPINLIMIT;                   \
      }                                                         \
    }                                                           \
  } while (0)
#else
#define I2C_ERRCHECK_SPIN_WHILE_COND(c_) do {                   \
    I2C_ERRCHECK_RETURN();                                      \
  } while (c_);
#endif

int
iBSP430eusciI2CrxData_rh (hBSP430halSERIAL hal,
                          uint8_t * data,
                          size_t len)
{
  volatile struct sBSP430hplEUSCIB * hpl = SERIAL_HAL_HPL_B(hal);
  uint8_t * dp = data;
  int use_auto_stop;
  const uint8_t * const dpe = data + len;

  /* Check for errors while waiting for previous activity to
   * complete */
  I2C_ERRCHECK_SPIN_WHILE_COND(hpl->statw & UCBBUSY);

  /* Set for receive */
  hpl->ctlw0 &= ~UCTR;

  /* Prefer to use UCASTP_2 via UCBxTBCNT because this is recommended
   * for single-byte transfers.  Since this would limit transmissions
   * to positive 8-bit lengths fall back to the manual mode in other
   * cases. */
  use_auto_stop = (0 < len) && (255 >= len);
  i2cSetAutoStop_ni(hal, use_auto_stop);

  /* Store the receive length.  This is ignored if not using auto-stop. */
  hpl->tbcnt = len;

  /* Issue a start */
  hpl->ctlw0 |= UCTXSTT;

  /* Read it in as soon as it arrives. */
  while (dp < dpe) {
    if ((dpe == (dp+1)) && (! use_auto_stop)) {
      /* This will be last character, and we're not using auto-stop.
       * Wait for any in-progress start to complete then issue
       * stop. */
      if (hpl->ctlw0 & UCTXSTT) {
        I2C_ERRCHECK_SPIN_WHILE_COND(hpl->ctlw0 & UCTXSTT);
      }
      hpl->ctlw0 |= UCTXSTP;
    }
    I2C_ERRCHECK_SPIN_WHILE_COND(! (hpl->ifg & UCRXIFG));
    ++hal->num_rx;
    *dp++ = hpl->rxbuf;
  }

  if (! use_auto_stop) {
    /* Wait for STP transmission to complete */
    I2C_ERRCHECK_SPIN_WHILE_COND(hpl->ctlw0 & UCTXSTP);
  }

  return dp - data;
}

int
iBSP430eusciI2CtxData_rh (hBSP430halSERIAL hal,
                          const uint8_t * data,
                          size_t len)
{
  volatile struct sBSP430hplEUSCIB * hpl = SERIAL_HAL_HPL_B(hal);
  int use_auto_stop;
  int i;

  /* Check for errors while waiting for previous activity to
   * complete */
  I2C_ERRCHECK_SPIN_WHILE_COND(hpl->statw & UCBBUSY);

  /* Prefer to use UCASTP_2 via UCBxTBCNT because this is recommended
   * for single-byte transfers.  Since this would limit transmissions
   * to positive 8-bit lengths fall back to the manual mode in other
   * cases. */
  use_auto_stop = (0 < len) && (255 >= len);
  i2cSetAutoStop_ni(hal, use_auto_stop);

  /* Set the transaction length.  If we're not using auto-stop this
   * won't hurt. */
  hpl->tbcnt = len;

  /* Issue a start for transmit */
  hpl->ctlw0 |= UCTR | UCTXSTT;

  /* Spit it all out as soon as there's space */
  i = 0;
  while (i < len) {
    I2C_ERRCHECK_SPIN_WHILE_COND(! (hpl->ifg & UCTXIFG));
    ++hal->num_tx;
    hpl->txbuf = data[i];
    ++i;
  }
  if (! use_auto_stop) {
    /* Wait for any queued data to be transmitted before we stop, lest
     * it get dropped. */
    I2C_ERRCHECK_SPIN_WHILE_COND(! (hpl->ifg & UCTXIFG));

    /* Send the stop and wait for it to complete. */
    hpl->ctlw0 |= UCTXSTP;
    I2C_ERRCHECK_SPIN_WHILE_COND(hpl->ctlw0 & UCTXSTP);
  }

  /* eUSCI module handles stop */
  return i;
}

/* Since the interrupt code is the same for all peripherals, on MCUs
 * with multiple USCI devices it is more space efficient to share it.
 * This does add an extra call/return for some minor cost in stack
 * space.
 *
 * Making the implementation function __c16__ ensures it's legitimate
 * to use portYIELD_FROM_ISR().
 *
 * Adding __always_inline__ supports maintainability by having a
 * single implementation and speed by forcing the implementation into
 * each handler.  It's a lot cleaner than defining the body as a
 * macro.  GCC will normally inline the code if there's only one call
 * point; there should be a configPORT_foo option to do so in other
 * cases. */
#if ((configBSP430_HAL_EUSCI_A0_ISR - 0)        \
     || (configBSP430_HAL_EUSCI_A1_ISR - 0)     \
     || (configBSP430_HAL_EUSCI_A2_ISR - 0)     \
     || (configBSP430_HAL_EUSCI_A3_ISR - 0))
static int
#if (20120406 < __MSPGCC__) && (__MSP430X__ - 0)
__attribute__ ( ( __c16__ ) )
#endif /* CPUX */
/* __attribute__((__always_inline__)) */
euscia_isr (hBSP430halSERIAL hal)
{
  int did_tx;
  int rv = 0;

  switch (SERIAL_HAL_HPL_A(hal)->iv) {
    default:
    case USCI_NONE:
      break;
    case USCI_UART_UCTXIFG: /* == USCI_SPI_UCTXIFG */
      rv = iBSP430callbackInvokeISRVoid_ni(&hal->tx_cbchain_ni, hal, 0);
      did_tx = 0;
      if (rv & BSP430_HAL_ISR_CALLBACK_BREAK_CHAIN) {
        /* Found some data; send it out */
        ++hal->num_tx;
        did_tx = 1;
        SERIAL_HAL_HPL_A(hal)->txbuf = hal->tx_byte;
      } else {
        /* No data; mark transmission disabled */
        rv |= BSP430_HAL_ISR_CALLBACK_DISABLE_INTERRUPT;
      }
      /* If no more is expected, clear the interrupt so we don't wake
       * again.  Further, if we didn't transmit anything mark that the
       * function is ready so when the interrupt is next set it will
       * fire. */
      if (rv & BSP430_HAL_ISR_CALLBACK_DISABLE_INTERRUPT) {
        SERIAL_HAL_HPL_A(hal)->ie &= ~UCTXIE;
        if (! did_tx) {
          SERIAL_HAL_HPL_A(hal)->ifg |= UCTXIFG;
        }
      }
      break;
    case USCI_UART_UCRXIFG: /* == USCI_SPI_UCRXIFG */
      hal->rx_byte = SERIAL_HAL_HPL_A(hal)->rxbuf;
      ++hal->num_rx;
      rv = iBSP430callbackInvokeISRVoid_ni(&hal->rx_cbchain_ni, hal, 0);
      break;
  }
  return rv;
}
#endif /* EUSCIB ISR */

#if ((configBSP430_HAL_EUSCI_B0_ISR - 0)        \
     || (configBSP430_HAL_EUSCI_B1_ISR - 0))
static int
#if (20120406 < __MSPGCC__) && (__MSP430X__ - 0)
__attribute__ ( ( __c16__ ) )
#endif /* CPUX */
/* __attribute__((__always_inline__)) */
euscib_isr (hBSP430halSERIAL hal)
{
  int did_tx;
  int rv = 0;

  switch (SERIAL_HAL_HPL_B(hal)->iv) {
    default:
    case USCI_NONE:
      break;
    case USCI_I2C_UCALIFG: /* == USCI_SPI_UCRXIFG */
      hal->rx_byte = SERIAL_HAL_HPL_B(hal)->rxbuf;
      ++hal->num_rx;
      rv = iBSP430callbackInvokeISRVoid_ni(&hal->rx_cbchain_ni, hal, 0);
      break;
    case USCI_I2C_UCNACKIFG: /* == USCI_SPI_UCTXIFG */
      rv = iBSP430callbackInvokeISRVoid_ni(&hal->tx_cbchain_ni, hal, 0);
      did_tx = 0;
      if (rv & BSP430_HAL_ISR_CALLBACK_BREAK_CHAIN) {
        /* Found some data; send it out */
        ++hal->num_tx;
        did_tx = 1;
        SERIAL_HAL_HPL_B(hal)->txbuf = hal->tx_byte;
      } else {
        /* No data; mark transmission disabled */
        rv |= BSP430_HAL_ISR_CALLBACK_DISABLE_INTERRUPT;
      }
      /* If no more is expected, clear the interrupt so we don't wake
       * again.  Further, if we didn't transmit anything mark that the
       * function is ready so when the interrupt is next set it will
       * fire. */
      if (rv & BSP430_HAL_ISR_CALLBACK_DISABLE_INTERRUPT) {
        SERIAL_HAL_HPL_B(hal)->ie &= ~UCTXIE;
        if (! did_tx) {
          SERIAL_HAL_HPL_B(hal)->ifg |= UCTXIFG;
        }
      }
      break;
    case USCI_I2C_UCSTTIFG:
    case USCI_I2C_UCSTPIFG:
    case USCI_I2C_UCRXIFG3:
    case USCI_I2C_UCTXIFG3:
    case USCI_I2C_UCRXIFG2:
    case USCI_I2C_UCTXIFG2:
    case USCI_I2C_UCRXIFG1:
    case USCI_I2C_UCTXIFG1:
    case USCI_I2C_UCRXIFG0:
    case USCI_I2C_UCTXIFG0:
    case USCI_I2C_UCBCNTIFG:
    case USCI_I2C_UCCLTOIFG:
    case USCI_I2C_UCBIT9IFG:
      break;
  }
  return rv;
}
#endif /* EUSCIB ISR */

#if (BSP430_SERIAL - 0)
static struct sBSP430serialDispatch dispatch_ = {
#if (configBSP430_SERIAL_ENABLE_UART - 0)
  .openUART = hBSP430eusciOpenUART,
  .uartRxByte_rh = iBSP430eusciUARTrxByte_rh,
  .uartTxByte_rh = iBSP430eusciUARTtxByte_rh,
  .uartTxData_rh = iBSP430eusciUARTtxData_rh,
  .uartTxASCIIZ_rh = iBSP430eusciUARTtxASCIIZ_rh,
#endif /* configBSP430_SERIAL_ENABLE_UART */
#if (configBSP430_SERIAL_ENABLE_SPI - 0)
  .openSPI = hBSP430eusciOpenSPI,
  .spiTxRx_rh = iBSP430eusciSPITxRx_rh,
#endif /* configBSP430_SERIAL_ENABLE_SPI */
#if (configBSP430_SERIAL_ENABLE_I2C - 0)
  .openI2C = hBSP430eusciOpenI2C,
  .i2cSetAddresses_rh = iBSP430eusciI2CsetAddresses_rh,
  .i2cRxData_rh = iBSP430eusciI2CrxData_rh,
  .i2cTxData_rh = iBSP430eusciI2CtxData_rh,
#endif /* configBSP430_SERIAL_ENABLE_I2C */
  .setReset_rh = iBSP430eusciSetReset_rh,
  .setHold_rh = iBSP430eusciSetHold_rh,
  .close = iBSP430eusciClose,
  .wakeupTransmit_rh = vBSP430eusciWakeupTransmit_rh,
  .flush_ni = vBSP430eusciFlush_ni,
  .rate = ulBSP430eusciRate,
};
#endif /* BSP430_SERIAL */

/* !BSP430! periph=euscia instance=EUSCI_A0,EUSCI_A1,EUSCI_A2,EUSCI_A3 insert=hal_serial_defn */
/* BEGIN AUTOMATICALLY GENERATED CODE---DO NOT MODIFY [hal_serial_defn] */
#if (configBSP430_HAL_EUSCI_A0 - 0)
struct sBSP430halSERIAL xBSP430hal_EUSCI_A0_ = {
  .hal_state = {
    .cflags = BSP430_SERIAL_HAL_HPL_VARIANT_EUSCIA
#if (configBSP430_HAL_EUSCI_A0_ISR - 0)
    | BSP430_PERIPH_HAL_STATE_CFLAGS_ISR
#endif /* configBSP430_HAL_EUSCI_A0_ISR */
  },
  .hpl = { .euscia = BSP430_HPL_EUSCI_A0 },
#if (BSP430_SERIAL - 0)
  .dispatch = &dispatch_,
#endif /* BSP430_SERIAL */
};
#endif /* configBSP430_HAL_EUSCI_A0 */

#if (configBSP430_HAL_EUSCI_A1 - 0)
struct sBSP430halSERIAL xBSP430hal_EUSCI_A1_ = {
  .hal_state = {
    .cflags = BSP430_SERIAL_HAL_HPL_VARIANT_EUSCIA
#if (configBSP430_HAL_EUSCI_A1_ISR - 0)
    | BSP430_PERIPH_HAL_STATE_CFLAGS_ISR
#endif /* configBSP430_HAL_EUSCI_A1_ISR */
  },
  .hpl = { .euscia = BSP430_HPL_EUSCI_A1 },
#if (BSP430_SERIAL - 0)
  .dispatch = &dispatch_,
#endif /* BSP430_SERIAL */
};
#endif /* configBSP430_HAL_EUSCI_A1 */

#if (configBSP430_HAL_EUSCI_A2 - 0)
struct sBSP430halSERIAL xBSP430hal_EUSCI_A2_ = {
  .hal_state = {
    .cflags = BSP430_SERIAL_HAL_HPL_VARIANT_EUSCIA
#if (configBSP430_HAL_EUSCI_A2_ISR - 0)
    | BSP430_PERIPH_HAL_STATE_CFLAGS_ISR
#endif /* configBSP430_HAL_EUSCI_A2_ISR */
  },
  .hpl = { .euscia = BSP430_HPL_EUSCI_A2 },
#if (BSP430_SERIAL - 0)
  .dispatch = &dispatch_,
#endif /* BSP430_SERIAL */
};
#endif /* configBSP430_HAL_EUSCI_A2 */

#if (configBSP430_HAL_EUSCI_A3 - 0)
struct sBSP430halSERIAL xBSP430hal_EUSCI_A3_ = {
  .hal_state = {
    .cflags = BSP430_SERIAL_HAL_HPL_VARIANT_EUSCIA
#if (configBSP430_HAL_EUSCI_A3_ISR - 0)
    | BSP430_PERIPH_HAL_STATE_CFLAGS_ISR
#endif /* configBSP430_HAL_EUSCI_A3_ISR */
  },
  .hpl = { .euscia = BSP430_HPL_EUSCI_A3 },
#if (BSP430_SERIAL - 0)
  .dispatch = &dispatch_,
#endif /* BSP430_SERIAL */
};
#endif /* configBSP430_HAL_EUSCI_A3 */

/* END AUTOMATICALLY GENERATED CODE [hal_serial_defn] */
/* !BSP430! end=hal_serial_defn */
/* !BSP430! uscifrom=eusci insert=hal_isr_defn */
/* BEGIN AUTOMATICALLY GENERATED CODE---DO NOT MODIFY [hal_isr_defn] */
#if (configBSP430_HAL_EUSCI_A0_ISR - 0)
BSP430_CORE_DECLARE_INTERRUPT(USCI_A0_VECTOR)
isr_EUSCI_A0 (void)
{
  int rv = euscia_isr(BSP430_HAL_EUSCI_A0);
  BSP430_HAL_ISR_CALLBACK_TAIL_NI(rv);
}
#endif /* configBSP430_HAL_EUSCI_A0_ISR */

#if (configBSP430_HAL_EUSCI_A1_ISR - 0)
BSP430_CORE_DECLARE_INTERRUPT(USCI_A1_VECTOR)
isr_EUSCI_A1 (void)
{
  int rv = euscia_isr(BSP430_HAL_EUSCI_A1);
  BSP430_HAL_ISR_CALLBACK_TAIL_NI(rv);
}
#endif /* configBSP430_HAL_EUSCI_A1_ISR */

#if (configBSP430_HAL_EUSCI_A2_ISR - 0)
BSP430_CORE_DECLARE_INTERRUPT(USCI_A2_VECTOR)
isr_EUSCI_A2 (void)
{
  int rv = euscia_isr(BSP430_HAL_EUSCI_A2);
  BSP430_HAL_ISR_CALLBACK_TAIL_NI(rv);
}
#endif /* configBSP430_HAL_EUSCI_A2_ISR */

#if (configBSP430_HAL_EUSCI_A3_ISR - 0)
BSP430_CORE_DECLARE_INTERRUPT(USCI_A3_VECTOR)
isr_EUSCI_A3 (void)
{
  int rv = euscia_isr(BSP430_HAL_EUSCI_A3);
  BSP430_HAL_ISR_CALLBACK_TAIL_NI(rv);
}
#endif /* configBSP430_HAL_EUSCI_A3_ISR */

/* END AUTOMATICALLY GENERATED CODE [hal_isr_defn] */
/* !BSP430! end=hal_isr_defn */

/* !BSP430! periph=euscib instance=EUSCI_B0,EUSCI_B1 insert=hal_serial_defn */
/* BEGIN AUTOMATICALLY GENERATED CODE---DO NOT MODIFY [hal_serial_defn] */
#if (configBSP430_HAL_EUSCI_B0 - 0)
struct sBSP430halSERIAL xBSP430hal_EUSCI_B0_ = {
  .hal_state = {
    .cflags = BSP430_SERIAL_HAL_HPL_VARIANT_EUSCIB
#if (configBSP430_HAL_EUSCI_B0_ISR - 0)
    | BSP430_PERIPH_HAL_STATE_CFLAGS_ISR
#endif /* configBSP430_HAL_EUSCI_B0_ISR */
  },
  .hpl = { .euscib = BSP430_HPL_EUSCI_B0 },
#if (BSP430_SERIAL - 0)
  .dispatch = &dispatch_,
#endif /* BSP430_SERIAL */
};
#endif /* configBSP430_HAL_EUSCI_B0 */

#if (configBSP430_HAL_EUSCI_B1 - 0)
struct sBSP430halSERIAL xBSP430hal_EUSCI_B1_ = {
  .hal_state = {
    .cflags = BSP430_SERIAL_HAL_HPL_VARIANT_EUSCIB
#if (configBSP430_HAL_EUSCI_B1_ISR - 0)
    | BSP430_PERIPH_HAL_STATE_CFLAGS_ISR
#endif /* configBSP430_HAL_EUSCI_B1_ISR */
  },
  .hpl = { .euscib = BSP430_HPL_EUSCI_B1 },
#if (BSP430_SERIAL - 0)
  .dispatch = &dispatch_,
#endif /* BSP430_SERIAL */
};
#endif /* configBSP430_HAL_EUSCI_B1 */

/* END AUTOMATICALLY GENERATED CODE [hal_serial_defn] */
/* !BSP430! end=hal_serial_defn */
/* !BSP430! uscifrom=eusci insert=hal_isr_defn */
/* BEGIN AUTOMATICALLY GENERATED CODE---DO NOT MODIFY [hal_isr_defn] */
#if (configBSP430_HAL_EUSCI_B0_ISR - 0)
BSP430_CORE_DECLARE_INTERRUPT(USCI_B0_VECTOR)
isr_EUSCI_B0 (void)
{
  int rv = euscib_isr(BSP430_HAL_EUSCI_B0);
  BSP430_HAL_ISR_CALLBACK_TAIL_NI(rv);
}
#endif /* configBSP430_HAL_EUSCI_B0_ISR */

#if (configBSP430_HAL_EUSCI_B1_ISR - 0)
BSP430_CORE_DECLARE_INTERRUPT(USCI_B1_VECTOR)
isr_EUSCI_B1 (void)
{
  int rv = euscib_isr(BSP430_HAL_EUSCI_B1);
  BSP430_HAL_ISR_CALLBACK_TAIL_NI(rv);
}
#endif /* configBSP430_HAL_EUSCI_B1_ISR */

/* END AUTOMATICALLY GENERATED CODE [hal_isr_defn] */
/* !BSP430! end=hal_isr_defn */

/* !BSP430! periph=eusci instance=EUSCI_A0,EUSCI_A1,EUSCI_A2,EUSCI_A3,EUSCI_B0,EUSCI_B1 */

const char *
xBSP430eusciName (tBSP430periphHandle periph)
{
  /* !BSP430! insert=periph_name_demux */
  /* BEGIN AUTOMATICALLY GENERATED CODE---DO NOT MODIFY [periph_name_demux] */
#if (configBSP430_HPL_EUSCI_A0 - 0)
  if (BSP430_PERIPH_EUSCI_A0 == periph) {
    return "EUSCI_A0";
  }
#endif /* configBSP430_HPL_EUSCI_A0 */

#if (configBSP430_HPL_EUSCI_A1 - 0)
  if (BSP430_PERIPH_EUSCI_A1 == periph) {
    return "EUSCI_A1";
  }
#endif /* configBSP430_HPL_EUSCI_A1 */

#if (configBSP430_HPL_EUSCI_A2 - 0)
  if (BSP430_PERIPH_EUSCI_A2 == periph) {
    return "EUSCI_A2";
  }
#endif /* configBSP430_HPL_EUSCI_A2 */

#if (configBSP430_HPL_EUSCI_A3 - 0)
  if (BSP430_PERIPH_EUSCI_A3 == periph) {
    return "EUSCI_A3";
  }
#endif /* configBSP430_HPL_EUSCI_A3 */

#if (configBSP430_HPL_EUSCI_B0 - 0)
  if (BSP430_PERIPH_EUSCI_B0 == periph) {
    return "EUSCI_B0";
  }
#endif /* configBSP430_HPL_EUSCI_B0 */

#if (configBSP430_HPL_EUSCI_B1 - 0)
  if (BSP430_PERIPH_EUSCI_B1 == periph) {
    return "EUSCI_B1";
  }
#endif /* configBSP430_HPL_EUSCI_B1 */

  /* END AUTOMATICALLY GENERATED CODE [periph_name_demux] */
  /* !BSP430! end=periph_name_demux */
  return NULL;
}
