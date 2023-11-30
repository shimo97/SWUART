#include "myList.h"

/* function to init a a list by assigning memory locations to it
 * arguments:
 * head - the list head
 * listBuff - memory buffer that contains the list element structures, can be NULL if bufflen=0
 * dataBuff - memory buffer that contains data objects, can be NULL if bufflen=0
 * bufflen - length of the two memory buffers (number of objects on listBuff and dataBuff, must be the same),
 *          if this argument is =0, the list will be initialized as an empty list
 * dataSize - size of the object type inside dataBuff (bytes)
 * WARNING: this function must be called at least one time before using the list
 */
void initList(list_head *head,list_e *listBuff,void *dataBuff,int bufflen, int dataSize){
    GETHEADP(head)=NULL; //first resetting head pointer

    if(bufflen!=0){

    GETHEADP(head)=&listBuff[0];

        char* tmpptr=(char *)dataBuff; //char pointer used to move inside dataBuff
        for(int o=0;o<bufflen;o++){
            listBuff[o].data=(void*)tmpptr; //casting again pointer to void and assigning value

            //assigning prev pointers
            if(o==0){
                listBuff[o].prev=&listBuff[bufflen-1];
            }else{
                listBuff[o].prev=&listBuff[o-1];
            }
            //assigning next pointers
            if(o==(bufflen-1)){
                listBuff[o].next=&listBuff[0];
            }else{
                listBuff[o].next=&listBuff[o+1];
            }

            tmpptr+=dataSize; //inrementing pointer of dataSize
        }
    }
	
	return;
}


/* function to append an element to the list tail
 * the element must already exist inside a proper memory
 * location and its pointer passed to the function
 */
void appendTail(list_head *head, list_e *el){
  if(GETHEADP(head)==NULL){ //if list is empty
    GETHEADP(head)=el; //new element is head
    (*el).prev=el; //creating circular self pointing
    (*el).next=el; //creating circular self pointing
  }else{
    list_e *currHead, *currTail; //current head and tail pointers
    currHead=GETHEADP(head); //saving current head pointer
    currTail=GETHEAD(head).prev; //saving current tail

    (*el).next=currHead; //pointer to old head
    (*el).prev=currTail; //pointer to tail
    (*currHead).prev=el; //pointer from old head
    (*currTail).next=el; //pointer from old tail
  }
}

/* function to append an element to the list head
 * the element must already exist inside a proper memory
 * location and its pointer passed to the function
 */
void appendHead(list_head *head, list_e *el){
  appendTail(head,el); //inserting element into list
  GETHEADP(head)=el; //new element is head
}

/* function to remove one element from a list
 * WARNING: this function doesn't check if the element is
 * really part of the list, so if an element outside the list is
 * passed the behavior is undefined
 * (use removeElement_safe() to avoid this problem)
 *
 */
void removeElement(list_head *head, list_e *toRem){
  list_e * tmpPrev, *tmpNext;
  tmpPrev=(*toRem).prev; //saving previous and next pointers
  tmpNext=(*toRem).next;

  if(GETHEADP(head)!=NULL){ //if list is not already empty (this should never happen because also means that element is not part of the list)
    if(tmpNext==toRem){ //if it was the only element in the list
      GETHEADP(head)=NULL;
    }else{
      //adjusting pointers
      (*tmpNext).prev=tmpPrev; //next element prev-> previous element
      (*tmpPrev).next=tmpNext; //previous element next -> next element

      if(GETHEADP(head)==toRem){ //if the element removed is Head
        GETHEADP(head)=tmpNext; //head points now to next element
      }
    }
  }

  (*toRem).prev=NULL;
  (*toRem).next=NULL;
}

/* function to remove one element from a list
 * diffrently from removeElement(), this function
 * will check that the element is actually part of
 * the list (but this requires scanning the list so
 * is way slower)
 *
 * return:
 * 0 - success
 * 1 - failure, element is not part of the list
 *
 */
int removeElement_safe(list_head *head, list_e *toRem){
  int ret=1;
  if(GETHEADP(head)!=NULL){ //if list is not empty
    char found=0;
    list_e *tmp=GETHEADP(head);

    do{ //searching for the element inside list
      if(tmp==toRem){
        found=1;
      }
      tmp=(*tmp).next; //going to next element
    }while(tmp!=GETHEADP(head) && found==0); //search goes until element found or list tail

    if(found==1){ //if element found inside list
      removeElement(head,toRem);
      ret=0;
    }
  }
  return ret;
}

/* function to remove an element from the list head
 *
 * return:
 * -pointer to the removed element
 * -NULL if list is empty
 */
list_e * removeHead(list_head *head){
  list_e *ret=NULL;
  if(GETHEADP(head)!=NULL){ //if the list is not empty
    ret=GETHEADP(head); //taking head
    removeElement(head,ret);
  }
  return ret;
}

/* function to remove an element from the list tail
 *
 * return:
 * -pointer to the removed element
 * -NULL if list is empty
 */
list_e * removeTail(list_head *head){
  list_e *ret=NULL;
  if(GETHEADP(head)!=NULL){ //if the list is not empty
    ret=GETHEAD(head).prev; //taking tail
    removeElement(head,ret);
  }
  return ret;
}

/* this function will merge two lists, by moving all the elements
 * of the list "fromHead" to the tail of list "toHead"
 * this function is extremey faster than removing/appending all the single
 * elements from the lists because will only redirect the proper pointers
 *
 */
void mergeList(list_head *fromHead,list_head *toHead){
    if(GETHEADP(fromHead) == NULL){ //if from list is empty
      return;
    }

    if(GETHEADP(toHead) == NULL){ //if to list is empty
      GETHEADP(toHead)=GETHEADP(fromHead); //passing the head
      GETHEADP(fromHead)=NULL; //from list becomes empty
      return;
    }

    //adjusting pointers
    list_e *tmp1=GETHEAD(toHead).prev; //saving to head tail
    list_e *tmp2=GETHEAD(fromHead).prev; //saving from head tail
    GETHEAD(toHead).prev=GETHEAD(fromHead).prev; //to head previous -> from tail
    GETHEAD(fromHead).prev=tmp1; //from head previous -> to tail

    (*tmp2).next=GETHEADP(toHead); //from tail next -> to head
    (*tmp1).next=GETHEADP(fromHead); //to tail next -> to head

    GETHEADP(fromHead)=NULL; //from list becomes empty
}
