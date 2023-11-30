#include "SWUART.h"

#define GETFLAG(var,bit) (((var)>>(bit)) & 1) //macro to get the "bit" flag value from variable "var"
#define SETFLAG(var,bit) var|=(1<<(bit)) //macro to set "bit" flag to 1 in variable "var"
#define RESETFLAG(var,bit) var&=(~(1<<(bit))) //macro to set "bit" flag to 0 in variable "var"

// -------------------- DATA STRUCTURES --------------------
//message list element
typedef struct message{
  char data;  //the 8 bit data
  char par;   //parity bit value
  char flags;   //flags (flag mapping defined with the macros below)
} message;
//message flags position mapping
#define BLANK_F 0 //blank message
#define CLEARRX_F 1 //clear RX list at the end of this message TX/RX
#define STOPERR_F 2 //stop violation (sampling 0 as stop bit)
//macro to get message* from void*
#define GETMSGP(vptr) ((message*)vptr)


//UART data structure
//it contains UART informations
//and the RX/TX buffers
typedef struct UARTDS{
  char UARTname; //name of the UART (char integer value)
  int TXpin; //TX pin
  int RXpin; //RX pin

  // ---------- TX ----------
  message TXbuff[TXBUFFLEN];  //TX buffer
  list_e TXlist[TXBUFFLEN]; //TX list
  list_head TXhead; //TX list head
  list_head TXfreehead; //TX free list head

  message currTX; //currently TX message

  char TXflags; //flags of the UART TX (flag mapping defined with the macros below)

  char TXbit; //currently TX bit
    /* TXbit values:
     * 0 - IDLE
     * 1 - START
     * 2:9 - DATA BIT
     * 10 - PARITY
     * 11 - STOP
     */
  char TXquantum; //currently TX bit quantum

  // ---------- RX ----------
  message RXbuff[RXBUFFLEN];  //RX buffer
  list_e RXlist[RXBUFFLEN]; //RX list
  list_head RXhead; //RX list head
  list_head RXfreehead; //RX free list head

  message currRX; //currently RX message

  char RXflags; //flags of the UART RX (flag mapping defined with the macros below)

  char RXbit; //currently RX bit
    /* RXbit values:
     * 0 - IDLE
     * 1 - START
     * 2:9 - DATA BIT
     * 10 - PARITY
     * 11 - STOP
     */
  char RXquantum; //currently RX bit quantum

  char oldVal; //old value of the line (used to detect start falling edge)

} UARTDS;
//TX/RX fags mapping
#define ENABLE_F 0 //channel is enabled
//macro to get UARTDS* from void*
#define GETUDSP(vptr) ((UARTDS*)vptr)

// -------------------- GLOBAL VARIABLES --------------------
// ---------- UART LIST MEMORY SECTIONS ----------
volatile UARTDS UARTbuffer[MAXUARTS]; //UARTDS objects memory
volatile list_e UARTlist[MAXUARTS]; //list elements memory
volatile list_head UARThead; //list head
volatile list_head UARTfreehead; //free list head

char initCalled=0; //flag to set if init has been called


//---------- STATIC (MODULE PRIVATE) FUNCTIONS ----------

/* function to reset all the fields of a UARTDS
 * by also clearing all the buffers
 * WARNING: to be called after initUARTDS()
 */
static void resetUARTDS(UARTDS *uart){
    //resetting name and pins
    uart->UARTname=0;
    uart->RXpin=0;
    uart->TXpin=0;
    //resetting flags
    uart->TXflags=0;
    uart->RXflags=0;
    //resetting current bits
    uart->TXbit=0;
    uart->RXbit=0;
    //resetting current bit quantum
    uart->TXquantum=0;
    uart->RXquantum=0;
    //resetting old line value
    uart->oldVal=0;

    mergeList(&(uart->TXhead),&(uart->TXfreehead)); //moving all the TX messages from list to free list
    mergeList(&(uart->RXhead),&(uart->RXfreehead)); //moving all the RX messages from list to free list
}


/* this function will init an UARTDS struct to its reset values
 * by also initializing the message lists
 *
 */
static void initUARTDS(UARTDS *uart){
    //initializing main lists
    initList(&(uart->TXhead),NULL,NULL,0,sizeof(message));
    initList(&(uart->RXhead),NULL,NULL,0,sizeof(message));

    //initializing free lists
    initList(&(uart->TXhead),uart->TXlist,uart->TXbuff,TXBUFFLEN,sizeof(message));
    initList(&(uart->RXhead),uart->RXlist,uart->RXbuff,RXBUFFLEN,sizeof(message));

    resetUARTDS(uart); //resetting UARTDS
}

/* this function will search an UARTDS by name inside a list
 * returns NULL if not found, otherwise pointer to UARTDS
 */
static UARTDS* searchUARTDSbyName(char UARTname, list_head *lst){
    list_e *tmp=GETHEADP(lst); //getting first element
    UARTDS* retPtr=NULL;
    if(tmp==NULL) return retPtr; //if list empty
    else{
        do{ //looping into full list
            UARTDS *uptr=GETUDSP(tmp->data); //getting UARTDS* from list element pointer
            if(uptr->UARTname == UARTname){ //if name correspondence found
                retPtr=uptr; //return the UARTDS*
            }
            tmp=tmp->next;
        }while(tmp!=UARThead && retPtr==NULL);

    }
    return retPtr;
 }

//function to compute parity of message msg (ret=1 if odd number of 1s)
static char computeParity(char msg){
    char retVal=0;
    for(int b=0;b<8;b++){
        retVal+=msg>>b; //summing each bit of message to retVal LSB
    }
    retVal &= 1; //mask to extract only LSB
    return retVal;
}

// ---------- PUBLIC FUNCTIONS ----------

/* initialize software UART, must be called one time at the
 * beginning of program
 */
void SWUARTinit(){
    if(initCalled == 1){ //if already init, return 1
        return;
    }

    //initializing data
    for(int u=0;u<MAXUARTS;u++){
        initUARTDS((UARTDS *)&UARTbuffer[u]); //initializing all the UART structs inside memory
    }

    //creating lists
    initList((list_e **)&UARThead,NULL,NULL,0,sizeof(UARTDS)); //initializing empty uart list
    initList((list_e **)&UARTfreehead,(list_e*)UARTlist,(UARTDS *)UARTbuffer,MAXUARTS,sizeof(UARTDS)); //initializing full free list

    initCalled=1;

    setupTimer(); //setup timer (and relative ISR)
    startTimer(); //starting timer

    return;

}

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
int SWUARTadd(int TXpin,int RXpin, char UARTname){
    int retVal=0;
    if(initCalled == 0){ //if swuart not initialized
        return NOINIT;
    }

    disableInt();

    if(UARTfreehead==NULL){ //if list full
        retVal=FULLBUFF;
    }

    if(!retVal){ //if no errors occurred, searching if name or pins already used
        list_e *tmp=UARThead; //pointer to list element
        if(tmp!=NULL){ //if list not empty
            UARTDS *uptr; //pointer to UARTDS

            do{ //looping into full list
                uptr=GETUDSP(tmp->data);
                if(uptr->UARTname == UARTname){
                    retVal=NAMEERR;
                }else if(uptr->TXpin == TXpin){
                    retVal=TXUSED;
                }

                tmp=tmp->next;
            }while(tmp!=UARThead && !retVal);
        }
    }

    if(!retVal){ //if no errors occurred, creating the UART
        list_e *tmp=removeHead((list_head*)&UARTfreehead); //removing element from free list
        //setting the new UART
        UARTDS *uptr=GETUDSP(tmp->data); //getting pointer to UARTDS

        resetUARTDS(uptr); //resetting uart

        uptr->UARTname=UARTname; //setting name
        uptr->TXpin=TXpin; //setting TX pin
        uptr->RXpin=RXpin; //setting RX pin
        //setting pin modes
        setPinMode(TXpin,'O');
        writePin(TXpin,1);
        setPinMode(RXpin,'I');
        //reading old line value for rx
        uptr->oldVal=readPin(RXpin);

        //enabling UART channels
        SETFLAG(uptr->TXflags,ENABLE_F);
        SETFLAG(uptr->RXflags,ENABLE_F);

        appendTail((list_head*)&UARThead,tmp); //adding uart to list
    }

    enableInt();

    if(!retVal) SWUARTsend(0,UARTname,0,1); //sending a blank message to let the receiver synchronize
    
    return retVal;
}

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
int SWUARTsend(char msg, char UARTname, char clearRX, char blank){
    int retVal=0;
    if(initCalled == 0){ //if swuart not initialized
        return NOINIT;
    }

    disableInt();

    UARTDS *uart=searchUARTDSbyName(UARTname,(list_head*)&UARThead); //searching UART by name
    if(uart==NULL) retVal=NAMEERR; //if not found, return error
    else if(uart->TXfreehead == NULL) retVal=FULLBUFF; //if buffer full, return error
    else{ //otherwise add message to queue
        list_e* tmp=removeHead(&(uart->TXfreehead)); //getting a message from TX free list

        message* msgp=GETMSGP(tmp->data); //getting message pointer

        msgp->data=msg; //setting message
        msgp->par=computeParity(msg);
        msgp->flags=0; //clearing flags
        if(clearRX) SETFLAG(msgp->flags,CLEARRX_F); //setting the eventual clear RX flag
        if(blank) SETFLAG(msgp->flags,BLANK_F); //setting the eventual blank flag

        appendTail(&(uart->TXhead),tmp); //attaching message to queue

    }

    enableInt();

    return retVal;

}

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
int SWUARTreceive(char *msg, char UARTname){
    int retVal=0;
    if(initCalled == 0){ //if swuart not initialized
        return NOINIT;
    }

    disableInt();

    UARTDS *uart=searchUARTDSbyName(UARTname,(list_head*)&UARThead); //searching UART by name
    if(uart==NULL) retVal=NAMEERR; //if not found, return error
    else if(uart->RXhead == NULL) retVal=EMPTYBUFF; //if RX buffer empty
    else{
        list_e* tmp=removeHead(&(uart->RXhead)); //removing message from RX list head

        message* msgp=GETMSGP(tmp->data); //getting message pointer

        *msg=msgp->data; //reading message

        if(GETFLAG(msgp->flags,STOPERR_F))retVal=STOPERR; //if stop violation
        else if(computeParity(*msg) != msgp->par) retVal=PARERR; //if parity bit wrong

        appendTail(&(uart->RXfreehead),tmp); //putting message on free list

    }

    enableInt();

    return retVal;
}

/* Try to read a message into msg from UART UARTname buffer (blocking)
 *
 * return:
 * 0 - message correctly read
 * NOINIT - message NOT read, SWUART not initialized
 * PARERR - message read with error, parity bit wrong
 * STOPERR - message read with error, STOP bit violated
 * NAMEERR - message NOT read, UARTname doesn't exist
 */
int SWUARTreceive_blocking(char *msg, char UARTname){
    int retVal=SWUARTreceive(msg,UARTname);
    while(retVal == EMPTYBUFF){    //looping if RX buffer empty
        retVal=SWUARTreceive(msg,UARTname);
    }
    return retVal;
}

/* Clear the TX/RX buffer of UART UARTname
 * TXn_RX is a flag to indicate wich buffer to clear (0=TX , 1=RX)
 * If a message is currently being TX, the transmission will be completed anyway
 *
 * return:
 * 0 - success
 * NOINIT - buffer not cleared, SWUART not initialized
 * NAMEERR - buffer not cleared, UARTname doesn't exist
 */
int SWUARTclearBuffer(char TXn_RX, char UARTname){
    int retVal=0;
    if(initCalled == 0){ //if swuart not initialized
        return NOINIT;
    }

    disableInt();

    UARTDS *uart=searchUARTDSbyName(UARTname,(list_head*)&UARThead); //searching UART by name
    if(uart==NULL) retVal=NAMEERR; //if not found, return error
    else{ //otherwise clear the buffer
        if(!TXn_RX){ //TX buffer
            mergeList(&(uart->TXhead),&(uart->TXfreehead)); //moving all the elements of list to free list

        }else{ //RX buffer
            mergeList(&(uart->RXhead),&(uart->RXfreehead)); //moving all the elements of list to free list
            SETFLAG(uart->currRX.flags,CLEARRX_F); //setting currently RX message to not be saved
        }
    }

    enableInt();
    return retVal;
}

/* ISR of the SWUART, this is the code that must be executed at each sampling quantum
 * Inside this function, all the operations to send/receive messages on all UARTs will be implemented
 * Must be set as the ISR of the timer within setupTimer() function of SWUART_drivers.c
 * Alternatively, this function can an also be called inside main program at regular intervals (if
 * e.g. a timer is not available)
 */
void SWUART_isr(){
    if(UARThead==NULL) return; //return if no active UARTs

    list_e *tmp=UARThead;
    UARTDS * uptr;
    //RX loop
    do{
        uptr=GETUDSP(tmp->data);//getting the UART pointer
        if(GETFLAG(uptr->RXflags,ENABLE_F)){ //if UART RX is enabled
            //incrementing quantum
            uptr->RXquantum++;

            char smp=readPin(uptr->RXpin); //reading RX pin

            if(uptr->RXbit!=0){ //IF NOT IDLE
                if(uptr->RXquantum == BITQUANTI){ //if bit window over
                    uptr->RXbit++; //incrementing bit
                    uptr->RXquantum=0; //resetting quantum

                    if(uptr->RXbit == 12){ //if STOP
                        //saving message
                        if(!GETFLAG(uptr->currRX.flags,CLEARRX_F)){ //if not clearRX
                            if(uptr->RXfreehead != NULL){//if RX buffer not full
                                list_e *tmp=removeHead(&(uptr->RXfreehead)); //taking element from free list
                                message *msg=GETMSGP(tmp->data); //getting message pointer
    
                                msg->data=uptr->currRX.data; //setting data
                                msg->par=uptr->currRX.par; //setting parity
                                msg->flags=uptr->currRX.flags; //setting flags
    
                                appendTail(&(uptr->RXhead),tmp); //putting element into list
                            }
                        }else{
                            mergeList(&(uptr->RXhead),&(uptr->RXfreehead)); //moving all the elements of list to free list
                        }
    
                        uptr->RXbit=0; //return to IDLE
                    }

                }

                if(uptr->RXquantum == SMPQUANTUM){ //if it's time to sample
                    switch (uptr->RXbit){
                        case 1:{ //START
                            if(smp != 0){ //START violation
                                uptr->RXbit=0; //setting bit as IDLE
                            }
                            break;
                        }
                        case 10:{ //PARITY
                            uptr->currRX.par=smp;
                            break;
                        }
                        case 11:{ //STOP
                            if(smp != 1){ //STOP violation
                                SETFLAG(uptr->currRX.flags,STOPERR_F); //setting stop error flag
                            }
                            break;
                        }
                        default:{ //DATA BITS
                            if(smp) //if 1
                                SETFLAG(uptr->currRX.data,uptr->RXbit-2);
                            else
                                RESETFLAG(uptr->currRX.data,uptr->RXbit-2);

                            break;
                        }
                    }
                }
            }

            if(uptr->RXbit==0){ //IF IDLE
                if(uptr->oldVal == 1 && smp==0){ //if START CONDITION
                        uptr->RXbit=1; //setting bit as start
                        uptr->RXquantum=0; //resetting quantum

                        uptr->currRX.flags=0; //resetting current RX message flags
                }
            }

            //updating oldVal
            uptr->oldVal=smp;
        }

        tmp=tmp->next; //going to next UART
    }while(tmp!=UARThead);

    //TX loop

    tmp=UARThead;
    do{
        uptr=GETUDSP(tmp->data);//getting the UART pointer
        if(GETFLAG(uptr->TXflags,ENABLE_F)){ //if UART TX is enabled
            char outVal=-1; //value to be written as output

            //incrementing quantum
            uptr->TXquantum++;

            if(uptr->TXbit!=0){ //IF NOT IDLE
                if(uptr->TXquantum == BITQUANTI){ //if it's time to change output
                    //changing to next bit
                    uptr->TXbit++; //incrementing bit
                    uptr->TXquantum=0; //resetting quantum

                    if(uptr->TXbit==12){ //if stop over
                        uptr->TXbit=0; //return to IDLE
                    }

                    switch (uptr->TXbit){

                        case 10:{ //PARITY
                            outVal=uptr->currTX.par;
                            break;
                        }
                        case 11:{ //STOP
                            outVal=1;
                            break;
                        }
                        default:{ //DATA BITS
                            outVal=GETFLAG(uptr->currTX.data,uptr->TXbit-2);

                            break;
                        }
                    }
                }



            }

            if(uptr->TXbit==0){ //IF IDLE
                if(uptr->TXhead!=NULL){ //if some message to send
                        list_e *tmp=removeHead(&(uptr->TXhead)); //taking element from list
                        message *msg=GETMSGP(tmp->data); //getting message pointer

                        uptr->TXbit=1; //setting bit as start
                        uptr->TXquantum=0; //resetting quantum
                        outVal=0; //setting output as 0

                        //copying message into currently TX message
                        uptr->currTX.data=msg->data;
                        uptr->currTX.par=msg->par;
                        uptr->currTX.flags=msg->flags;

                        appendTail(&(uptr->TXfreehead),tmp); //putting element into free list

                        //if CLEARRX_F, clearing RX buffer
                        if(GETFLAG(uptr->currTX.flags,CLEARRX_F)){
                            SETFLAG(uptr->currRX.flags,CLEARRX_F); //setting current RX message to clear
                            mergeList(&(uptr->RXhead),&(uptr->RXfreehead)); //moving all the elements of RX list to free list
                        }
                }else{
                    outVal=1;
                }
            }
            //writing output
            if(outVal!=-1 && !GETFLAG(uptr->currTX.flags,BLANK_F)) writePin(uptr->TXpin,outVal); //if out value has been changed
        }
        tmp=tmp->next; //going to next UART
    }while(tmp!=UARThead);
}
