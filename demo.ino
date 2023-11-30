//SWUART demo which implements a redundant communication between two Arduinos
//the two microcontrollers communicate through two SWUART channels and continuously
//strobe each other to check if the other one is dead, if no messages are received on
//neither SWUART for a certain amount of time, the arduino will consider the other as dead
//and turn on an LED to signal the condition.

#include "SWUART.h"
#define WAKE 101 //wake code
#define OTHERLED 9 //other led pin
#define WAITTIME 500 //timeout to consider the other dead

int phaseA=0; //veriable to store the current strobe phase of bus A (0=send WAKE, 1:9=send BLANK)
int phaseB=0; //veriable to store the current strobe phase of bus B (0=send WAKE, 1:9=send BLANK)

char Adead=0; //bus state
char Bdead=0;

long timeA=0;
long timeB=0;

char otherLed=0; //flag with the state of other led (0=of the other, 1=mine)

char msg;

void setup() {
  SWUARTinit();
  SWUARTadd(2,3,'A');
  SWUARTadd(4,5,'B');
  timeA=millis();
  timeB=millis();

  pinMode(OTHERLED,OUTPUT);
}

void loop() {
  //bus strobe
  if(phaseA==0){
    if(SWUARTsend(WAKE,'A',0,0)==0){ //WAKE
      phaseA++;
    }  
  }else{
    if(SWUARTsend(WAKE,'A',0,1)==0){ //BLANK
      phaseA++;
      if(phaseA==10) phaseA=0;
    }
  }

  if(phaseB==0){
    if(SWUARTsend(WAKE,'B',0,0)==0){ //WAKE
      phaseB++;
    }  
  }else{
    if(SWUARTsend(WAKE,'B',0,1)==0){ //BLANK
      phaseB++;
      if(phaseB==10) phaseB=0;
    }
  }

  if(SWUARTreceive(&msg,'A')==0){
    if(msg==WAKE){
      timeA=millis();
    }  
  }

  if(SWUARTreceive(&msg,'B')==0){
    if(msg==WAKE){
      timeB=millis();
    }  
  }

  if((millis()-timeA)>WAITTIME){ //if busA dead
    Adead=1;
  }else{
    Adead=0;
  }

  if((millis()-timeB)>WAITTIME){ //if busB dead
    Bdead=1;
  }else{
    Bdead=0;
  }

  if(Adead && Bdead){
    otherLed=1;  
  }else{
    otherLed=0;  
  }

  if (otherLed) digitalWrite(OTHERLED,HIGH);
  else digitalWrite(OTHERLED,LOW);
}
