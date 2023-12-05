#include "SWUART_drivers.h"
#include <stdio.h>
#include <stdlib.h>

#define GPIONUM 2 //number of gpio pins
char gpio[GPIONUM];//emulated gpio

int getGPIONUM(){
    return GPIONUM;
}

void setupTimer(){}

void stopTimer(){}

void startTimer(){}

void setPinMode(int pin,char mode){
    if(pin<GPIONUM)
        gpio[pin]=1;
}

void writePin(int pin,char val){
    if(pin<GPIONUM)
        gpio[pin]=val;
}

char readPin(int pin){
    if(pin<GPIONUM)
        return gpio[pin];

    return 'U';
}

void disableInt(){}

void enableInt(){}
