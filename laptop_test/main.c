#include <stdio.h>
#include <stdlib.h>

#include "myList.h"
#include "SWUART.h"
#include "SWUART_drivers.h"
#include <string.h>

int printIndex=0; //time index to be printed with output
char guiText[400];

void printGUI(){
    int num=getGPIONUM();
    if(printIndex==0){
        printf("\t");
        for(int c=0;c<num;c++){
            if(c<10) printf("|  PIN %d   ",c);
            else printf("|  PIN %d  ",c);
        }
        printf("|\n");

    }

    printf("%d:\t",printIndex++);
    for(int c=0;c<num;c++){
        if(readPin(c)==0){
                printf("|  0       ");
        }else{
                printf("|       1  ");
        }
    }
    printf("|%s\n",guiText);
    guiText[0]='\0';
}

void translateErr(int code, char*str){ //function to translate the error codes of SWUART.h
/*
 * NOINIT 1 //SWUART not initialized
 * NAMEERR 2 //UART name invalid (eg. already in use/doesn't exist)
 * TXUSED 3 //TX pin already used
 * FULLBUFF 4 //buffer is full
 * EMPTYBUFF 5 //buffer is empty
 * PARERR 6 //parity bit wrong
 * STOPERR 7 //stop violation
 */
 str[0]='\0';
 switch(code){
    case 0: strcat(str,"0"); break;
    case 1: strcat(str,"NOINIT"); break;
    case 2: strcat(str,"NAMEERR"); break;
    case 3: strcat(str,"TXUSED"); break;
    case 4: strcat(str,"FULLBUFF"); break;
    case 5: strcat(str,"EMPTYBUFF"); break;
    case 6: strcat(str,"PARERR"); break;
    case 7: strcat(str,"STOPERR"); break;
}
}

void addGui1(char *str,int ret){ //adds a message to gui with only string and return value
    char num[20];
    strcat(guiText," -\"");
    strcat(guiText,str);
    strcat(guiText,"\"=");
    translateErr(ret,num);
    strcat(guiText,num);
    strcat(guiText," ");
}

void addGui2(char *str,int val,int ret){ //same as addGui1() but it also concatenates a number (dec and bin) to the string
    char num[20];
    char tmpstr[50];
    tmpstr[0]='\0';
    strcat(tmpstr,str);
    itoa(val,num,10);
    strcat(tmpstr,num);
    strcat(tmpstr,"(");
    itoa(val,num,2);
    for(int z=0;z<(8-strlen(num));z++) strcat(tmpstr,"0");
    strcat(tmpstr,num);
    strcat(tmpstr,")");
    addGui1(tmpstr,ret);
}


void step(int num){ //simulate num steps (quantum) and print the output
    for(int s=0;s<num;s++){
        SWUART_isr();
        printGUI();
    }
}

sendWrongPar(int pin){ //sends a message with wrong parity bit
    writePin(pin,0);
    step(2);
    for(int c=0;c<4;c++){
        writePin(pin,1);
        step(2);
    }
    for(int c=0;c<4;c++){
        writePin(pin,0);
        step(2);
    }
    writePin(pin,1);
    step(2);
    writePin(pin,1);
    step(2);

}

sendWrongStop(int pin){ //sends a message with wrong stop bit
    writePin(pin,0);
    step(2);
    for(int c=0;c<4;c++){
        writePin(pin,1);
        step(2);
    }
    for(int c=0;c<4;c++){
        writePin(pin,0);
        step(2);
    }
    writePin(pin,1);
    step(2);
    writePin(pin,0);
    step(2);

}

int main()
{
    //PRINTING TITLE
    printf("\n");
    printf("  / ___/ |     / / / / /   |  / __ \\/_  __/\n");
    printf("  \\__ \\| | /| / / / / / /| | / /_/ / / /\n");
    printf(" ___/ /| |/ |/ / /_/ / ___ |/ _, _/ / /\n");
    printf("/____/ |__/|__/\\____/_/  |_/_/ |_| /_/ by Shimo97\n");
    printf("\n");
    //simulation variables reset
    guiText[0]='\0';
    //useful variables
    char msg;

    SWUARTinit();
    addGui1("Add UART", SWUARTadd(0,1,'A'));
    addGui1("Add UART", SWUARTadd(1,0,'B'));
    addGui1("Send", SWUARTsend(23,'B',1,0));
    step(5);
    addGui1("Send", SWUARTsend(45,'A',0,0));
    step(23);
    //addGui1("Clear", SWUARTclearBuffer(1,'B'));
    //step(20);
    //addGui1("Send", SWUARTsend(23,'B',1,0));
    step(3);
    addGui2("B reads ",msg,SWUARTreceive(&msg,'B'));
    step(15);
    //addGui1("Clear", SWUARTclearBuffer(1,'B'));
    addGui2("B reads ",msg,SWUARTreceive(&msg,'B'));
    step(1);

    return 0;













}
