#include "queue.h"
#include "string.h"
#include "assert.h"

/**
 * Exercise 4:
 * this function adds "data" to the end of the queue "q".
 *
 * Hint: you may need "malloc".
 **/
void enqueue(queue_t *q, char *data) {
    /*TODO: add your code here */
    node_t *pnode = (node_t*)malloc(sizeof(node_t));
    pnode->data = (char*)malloc(sizeof(char)*(strlen(data)+1));
    strcpy(pnode->data, data);
    pnode->next = NULL;
    if(!q->tail){
        q->head = pnode;
        q->tail = pnode;
    }else{
        q->tail->next = pnode;
        q->tail = pnode;
    }
    return;
}


/**
 * Exercise 4:
 * this function removes and returns the element
 * at the front of the queue "q".
 *
 * Hint: think of when to free the memory.
 **/
void * dequeue(queue_t *q) {
    /*TODO: add your code here */
    if(!q->head){
        return NULL;
    }
    node_t *pnode = q->head;
    if(!q->head->next){
        q->tail = NULL;
    }
    q->head = q->head->next;
    return pnode;
    //return "UNIMPLEMENTED";
}


/**
 * Exercise 4:
 * this function will be called to destroy an existing queue,
 * for example, in the end of main() function.
 * Use free() to free the memory you allocated.
 **/
void free_queue(queue_t *q) {
    /*TODO: add your code here*/
    node_t *pnode = NULL;
    while((pnode=dequeue(q))!=NULL){
        if(pnode->data!=NULL){
            free(pnode->data);
        }
        free(pnode);
    }
}

/**
 * Exercise 4:
 * this function prints a queue in the following format (quotation marks are
 * not part of the output):
 * """
 * finally: [1st element]->[2nd element]->[3rd element]
 * """
 *
 * Example #1:
 *
 * for a queue with one element with data = "ABC", the output should be:
 * finally: [ABC]
 *
 * Example #2:
 *
 * for a queue with two elements "ABC" and "DEF", the output should be:
 * finally: [ABC]->[DEF]
 *
 * Example #3:
 *
 * for an empty queue, the output should be nothing but "finally: ":
 * finally:
 *
 * Notice: you must strictly follow the format above because the grading tool
 * only recognizes this format. If you print something else, you will not get
 * the scores for this exercise.
 **/
void print_queue(queue_t *q) {
    // the output should start with "finally: "
    printf("finally: ");

    /*TODO: add your code here */
    node_t *pnode = q->head;
    while(pnode){
        printf("[%s]",pnode->data);
        pnode = pnode->next;
        if(pnode){
            printf("->");
        }
    }

    // the output requires a new line in the end
    printf("\n");
}
