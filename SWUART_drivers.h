#ifndef SWUART_DRIVERS
#define SWUART_DRIVERS
/* PLATFORM DEPENDENT FUNCTIONS (API)
 * this header defines the platform dependent function to be
 * implemented in order to port the SWUART to another uC or to test it on a laptop
 */

//extern declaration of the ISR function (defined into SWUART.c)
//this function must be set as ISR of the timer inside setupTimer() function
extern void SWUART_isr();

//timer (and relative ISR) setup
//inside this function, SWUART_isr() must be set as timer ISR
void setupTimer();

//timer (and relative ISR) stop
void stopTimer();

//timer (and relative ISR) start
void startTimer();

/* function to set a pin as input/output
 * mode values:
 * 'I':input
 * 'O':output
 * any other value: disabled
 */
void setPinMode(int pin,char mode);

// function to write on a pin
// val=0 : LOW
// val=1 : HIGH
void writePin(int pin,char val);

// function to read from a pin
// returns the value as char(0=LOW,1=HIGH)
char readPin(int pin);

//interrupt disable function
void disableInt();

//interrupt enable function
void enableInt();


#endif
