/** This file is in the public domain.
 *
 * This program initializes the platform then spins toggling the first
 * LED and printing a monotonically increasing counter to the serial
 * port designated as a console.
 *
 * Its role is primarily as a sanity check when bootstrapping a new
 * platform: the third step is to support printf.  (Though printf is
 * heavy-weight, so we're using the console print routines.)
 *
 * @author Peter A. Bigot <bigotp@acm.org>
 * @homepage http://github.com/pabigot/freertos-mspgcc
 * @date 2012
 *
 */

/* Include the generic platform header.  This assumes that
 * #BSP430_PLATFORM_EXP430FR5739 (or another marker that identifies a
 * platform BSP430 supports) has been defined for the preprocessor.
 * If not, you may include the platform-specific version,
 * e.g. <bsp430/platform/exp430g2.h>. */
#include <bsp430/platform.h>

/* We're going to use LEDs.  Enable them. */
#include <bsp430/utility/led.h>

/* We want to know the nominal clock speed so we can delay. */
#include <bsp430/clock.h>

/* We require console support as verified by <bsp430/platform.h> */
#ifndef BSP430_CONSOLE
#error No console UART PERIPH handle has been defined
#endif /* BSP430_CONSOLE */
#include <bsp430/utility/console.h>

void main ()
{
  int rv;

  /* First thing you do in main is configure the platform. */
  vBSP430platformInitialize_ni();

  /* Now initialize the LEDs */
  vBSP430ledInitialize_ni();

  /* Turn the first one on */
  vBSP430ledSet(0, 1);

  /* Configure the console to use the default UART handle */
  rv = iBSP430consoleInitialize();

  /* Indicate we made it this far. */
  vBSP430ledSet(1, 1);

  if (0 > rv) {
    /* Failed to configure the UART HAL */
    vBSP430ledSet(0, 0);
  } else {
    unsigned int counter = 0;

    /* Now blink and display a counter value every nominal half
     * second.  The blink proves that the loop is running even if
     * nothing's coming out the serial port.  On each iteration,
     * attempt to read a character and display it if one is
     * present. */
    while (1) {
      int rc;
      BSP430_CORE_WATCHDOG_CLEAR();
      BSP430_CORE_DELAY_CYCLES(BSP430_CLOCK_NOMINAL_MCLK_HZ / 2);
      rc = iBSP430uartRxByte_ni(hBSP430console());
      if (0 <= rc) {
        cputtext_ni(" rx char ");
        cputu_ni(rc, 10);
        cputtext_ni(" '");
        cputchar_ni(rc);
        cputtext_ni("'");
      }
      cputtext_ni("\nCounter ");
      cputu_ni(counter, 10);
      ++counter;
      vBSP430ledSet(0, -1);
    }
  }
}
