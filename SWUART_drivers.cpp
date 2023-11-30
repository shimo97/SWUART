#include "SWUART_drivers.h"
#include "Arduino.h"

volatile int isrflag=0;

ISR(TIMER2_COMPA_vect){
  if(isrflag)
    SWUART_isr();
}

//timer (and relative ISR) setup
//inside this function, SWUART_isr() must be set as timer ISR
void setupTimer(){
  noInterrupts();
  TCCR2A=2; //setting CTC mode
  OCR2A=166; //setting compare value
  TIMSK2=2; //enable output compare A interupt
  isrflag=1; //enable ISR
  interrupts();
}

//timer (and relative ISR) stop
void stopTimer(){
    TCCR2B=0;  
}

//timer (and relative ISR) start
void startTimer(){
    TCCR2B=4; //setting prescaler of 64  
}

/* function to set a pin as input/output
 * mod values:
 * 'I':input
 * 'O':output
 * any other value: disabled
 */
void setPinMode(int pin,char mode){
  if(mode=='O')
    pinMode(pin,OUTPUT);
  else
    pinMode(pin,INPUT_PULLUP);
}

// function to write on a pin
// val=0 : LOW
// val=1 : HIGH
void writePin(int pin,char val){
  digitalWrite(pin,val);
}

// function to read from a pin
// returns the value as char(0=LOW,1=HIGH)
char readPin(int pin){
  return digitalRead(pin);
}

//interrupt disable function
void disableInt(){
  noInterrupts();  
}

//interrupt enable function
void enableInt(){
  interrupts();
}
