# SOFTWARE UART LIBRARY

This is a SWUART, so an UART emulated by software that can be used in contexts
where HW UART is not available or a large number of slow UARTs is needed, this
library will use basic GPIO read/write to emulate the UART I/O

With this SWUART the user can implement an ideally infinite number of UARTs that
can transmit/receive all at the same time and only need an ISR to be called at regular
time intervals (can be done with a single timer + interrupts, beware that this way it will be
quite slower than an hardware UART)

This SWUART works in sampling mode, meaning that the GPIO line is sampled at regular intervals
called "quanti", no voting policy is applied so the bit value is determined by a single sample.
The number of quanti corresponding to each bit can be defined by the BITQUANTI macro, while the
sample number corresponding to the bit center can be defined by SMPQUANTUM macro. The SWUART
baud rate is then defined by the user, which has to call the SWUART_isr() function on a regular
basis inside an ISR, each call to the isr corresponds to a quantum and so the SWUART baud is 
defined by baud=1/(isr_frequency * BITQUANTI)

> [!TIP]
> The SWUART is platform independent, and the user must only implement the functions of SWUART_drivers.h to port it to another platform, this also allows testing using a simulated environment on a laptop.

LAPTOP TEST

To use the laptop test bench the user has to:
- Change the extension of .cpp files in "src" to .c
- Replace SWUART_drivers.c in "src" with the file present in "laptop_test" folder
- Compile the src files with the main.c in "laptop_test" folder

The detailed design documentation can be read here: https://drive.google.com/file/d/17QffW90TGDeSP_6F9FyWKWPq4Yl3Kk1T/view?usp=sharing