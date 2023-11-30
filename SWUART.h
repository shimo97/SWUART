#ifndef SWUART_DS
#define SWUART_DS
/* This is a SWUART, so an UART emulated by software that can be used in contexts
 * where HW UART is not available or a large number of slow UARTs is needed, this
 * library will use basic GPIO read/write to emulate the UART I/O)
 *
 * With this SWUART the user can implement an ideally infinite number of UARTs that
 * can transmit/receive all at the same time and only need an ISR to be called at regular
 * time intervals (can be done with a single timer + interrupts, beware that this way it will be
 * quite slower than an hardware UART)
 *
 * This SWUART works in sampling mode, meaning that the GPIO line is sampled at regular intervals
 * called "quanti", no voting policy is applied so the bit value is determined by a single sample.
 * The number of quanti corresponding to each bit can be defined by the BITQUANTI macro, while the
 * sample number corresponding to the bit center can be defined by SMPQUANTUM macro. The SWUART
 * baud rate is then defined by the user, which has to call the SWUART_isr() function on a regular
 * basis inside an ISR, each call to the isr corresponds to a quantum and so the SWUART baud is 
 * defined by baud=1/(isr_frequency * BITQUANTI)
 * 
 *
 * The SWUART is platform independent, and the user must only implement the functions of SWUART_drivers.h
 * to port it to another platform, this also allows testing using a simulated environment on a laptop
 */

#include "myList.h"
#include "SWUART_drivers.h"

// -------------------- CONFIG MACROS --------------------
#define MAXUARTS 5 //maximum number of software UARTS
#define RXBUFFLEN 10 //maximum number of messages on each RX buffer
#define TXBUFFLEN 10  //maximum number of messages on each TX buffer

#define BITQUANTI 5 //number of quanti on a UART bit
#define SMPQUANTUM 2 //quantum in wich sample is taken (must be < BITQUANTI)

// --------------------ERROR CODES --------------------
#define NOINIT 1 //SWUART not initialized
#define NAMEERR 2 //UART name invalid (eg. already in use/doesn't exist)
#define TXUSED 3 //TX pin already used
#define FULLBUFF 4 //buffer is full
#define EMPTYBUFF 5 //buffer is empty
#define PARERR 6 //parity bit wrong
#define STOPERR 7 //stop violation

// -------------------- FUNCTIONS --------------------

/* Initialize software UART, must be called one time at the
 * beginning of program
 */
void SWUARTinit();

/* Add a new instance of UART
 *
 * TXpin and RXpin are the UART pins (different UARTs can share RXpin, but not
 * TXpin), this function does not impose that a TXpin of an UART cannot be used
 * as RXpin of the same/another one, but this will cause a call to setPinMode() on the same pin
 * to set it as both input and output and also call writePin()/readPin() on that same pin, so this
 * must be taken into account while creating SWUART_drivers.c
 *
 * UARTname is an 8 bit namecode for the UART
 *
 * return:
 * 0 - UART correctly created
 * NOINIT - UART NOT created, SWUART not initialized
 * NAMEERR - UART NOT created, UARTname already used
 * TXUSED - UART NOT created, TXpin already used
 * FULLBUFF - UART NOT created, maximum number reached
 */
int SWUARTadd(int TXpin,int RXpin, char UARTname);

/* Send the msg message with the UART UARTname
 * If clearRX is set (1), the RX buffer of UART UARTname
 * is cleared at the beginning of the message START bit
 * If blank is set (1) the msg is ignored and a blank
 * message (idle line for the period of a message) will be sent, this is useful for
 * resynchronization of the bus
 *
 *
 * return:
 * 0 - message sent
 * NOINIT - message not sent, SWUART not initialized
 * NAMEERR - message not sent, UARTname doesn't exist
 * FULLBUFF - message not sent, TX buffer full
 */
int SWUARTsend(char msg, char UARTname, char clearRX, char blank);

/* Try to read a message into msg from UART UARTname buffer (NON blocking)
 *
 * return:
 * 0 - message correctly read
 * NOINIT - message NOT read, SWUART not initialized
 * PARERR - message read with error, parity bit wrong
 * STOPERR - message read with error, STOP bit violated
 * EMPTYBUFF - message NOT read, no messages for UARTname inside buffer
 * NAMEERR - message NOT read, UARTname doesn't exist
 */
int SWUARTreceive(char *msg, char UARTname);

/* Try to read a message into msg from UART UARTname buffer (blocking)
 *
 * return:
 * 0 - message correctly read
 * NOINIT - message NOT read, SWUART not initialized
 * PARERR - message read with error, parity bit wrong
 * STOPERR - message read with error, STOP bit violated
 * NAMEERR - message NOT read, UARTname doesn't exist
 */
int SWUARTreceive_blocking(char *msg, char UARTname);

/* Clear the TX/RX buffer of UART UARTname
 * TXn_RX is a flag to indicate wich buffer to clear (0=TX , 1=RX)
 * If a message is currently being TX, the transmission will be completed anyway
 *
 * return:
 * 0 - success
 * NOINIT - buffer not cleared, SWUART not initialized
 * NAMEERR - buffer not cleared, UARTname doesn't exist
 */
int SWUARTclearBuffer(char TXn_RX, char UARTname);

/* ISR of the SWUART, this is the code that must be executed at each sampling quantum
 * Inside this function, all the operations to send/receive messages on all UARTs will be implemented
 * Must be set as the ISR of the timer within setupTimer() function of SWUART_drivers.c
 * Alternatively, this function can an also be called inside main program at regular intervals (if
 * e.g. a timer is not available)
 */
void SWUART_isr();

#endif
