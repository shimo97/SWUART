#ifndef MSGLIST
#define MSGLIST
/* This file defines a very crude generic bidirectional list structure, as well as some
 * basic functions that can help manage the list, those functions may help to do very common
 * tasks.
 *
 * The list is composed of list elements list_e structs, they
 * possess a previous and a next pointer, as well as a void* pointer
 * to attach some data.
 * Both list_e and data will reside in some memory that must be externally declared
 * the list is then initialized by linking each list_e inside the list_e array to the previous/next
 * element, as well as to the corresponding data inside the data array, SO LIST ELEMENTS AND DATA
 * ARRAYS MUST HAVE THE SAME DIMENSION.
 *
 * After that, the list can be accessed by using an head pointer, this is
 * basically a list_e * pointer to the first element of the list, but the type list_head can be used for
 * better readability, only a single list_head must be used as head, because some functions will behave
 * according to the informations that reside into this specific pointer, so using a different element pointer
 * as head will result in undefined behavior.
 */

#ifndef NULL
#define NULL 0
#endif

// -------------------- DATA STRUCTURES --------------------

//list element
//A list is only identified by an head pointer to the first list_e
typedef struct list_e{
  struct list_e *prev; //previous element in the list
  struct list_e *next; //next element in the list

  void *data; //pointer to attach data
} list_e;

//list head definition
typedef list_e* list_head; //list head type (pointer to the first element)
#define GETHEADP(ptr_to_list_head) (*ptr_to_list_head) //macro to getting the pointer to the first element of the list from a pointer to list_head
#define GETHEAD(ptr_to_list_head) (*(*ptr_to_list_head)) //macro to getting the first element of the list from a pointer to list_head

// -------------------- FUNCTIONS --------------------
/* function to init a list by assigning memory locations to it, this can be used to quickly init a list
 * from data inside arrays, its usage is mandatory also in case the list is initially empty
 * to init the list head pointer
 *
 * arguments:
 * head - pointer to the list head
 * listBuff - memory buffer that contains the list element structures, can be NULL if bufflen=0
 * dataBuff - memory buffer that contains data objects, can be NULL if bufflen=0
 * bufflen - length of the two memory buffers (number of objects on listBuff and dataBuff, must be the same),
 *          if this argument is =0, the list will be initialized as an empty list
 * dataSize - size of the object type inside dataBuff (bytes)
 */
void initList(list_head *head,list_e *listBuff,void *dataBuff,int bufflen, int dataSize);

/* function to append an element to the list tail
 * the element must already exist inside a proper memory
 * location and its pointer passed to the function
 */
void appendTail(list_head *head, list_e *el);

/* function to append an element to the list head
 * the element must already exist inside a proper memory
 * location and its pointer passed to the function
 */
void appendHead(list_head *head, list_e *el);


/* function to remove one element from a list
 * WARNING: this function doesn't check if the element is
 * really part of the list, so if an element outside the list is
 * passed the behavior is undefined
 * (use removeElement_safe() to avoid this problem)
 *
 */
void removeElement(list_head *head, list_e *toRem);

/* function to remove one element from a list
 * diffrently from removeElement(), this function
 * will check that the element is actually part of
 * the list (but this requires scanning the list up
 * to the head so is slower)
 *
 * return:
 * 0 - success
 * 1 - failure, element is not part of the list
 *
 */
int removeElement_safe(list_head *head, list_e *toRem);

/* function to remove an element from the list tail
 *
 * return:
 * -pointer to the removed element
 * -NULL if list is empty
 */
list_e * removeTail(list_head *head);

/* function to remove an element from the list head
 *
 * return:
 * -pointer to the removed element
 * -NULL if list is empty
 */
list_e * removeHead(list_head *head);

/* function to merge two lists, by moving all the elements
 * of the list "fromHead" to the tail of list "toHead"
 * this function is extremey faster than removing/appending all the single
 * elements from the lists because will only redirect the heads/tails pointers
 *
 */
void mergeList(list_head *fromHead,list_head *toHead);

#endif
