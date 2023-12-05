#include "SWUART_drivers.h"
#include "Arduino.h"

volatile int isrflag=0;

ISR(TIMER2_COMPA_vect){
  if(isrflag)
    SWUART_isr();
}

void setupTimer(){
  noInterrupts();
  TCCR2A=2; //setting CTC mode
  OCR2A=166; //setting compare value
  TIMSK2=2; //enable output compare A interupt
  isrflag=1; //enable ISR
  interrupts();
}

void stopTimer(){
    TCCR2B=0;  
}

void startTimer(){
    TCCR2B=4; //setting prescaler of 64  
}

void setPinMode(int pin,char mode){
  if(mode=='O')
    pinMode(pin,OUTPUT);
  else
    pinMode(pin,INPUT_PULLUP);
}

void writePin(int pin,char val){
  digitalWrite(pin,val);
}

char readPin(int pin){
  return digitalRead(pin);
}

void disableInt(){
  noInterrupts();  
}

void enableInt(){
  interrupts();
}
