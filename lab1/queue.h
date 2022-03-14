#ifndef CS5600_LAB1_QUEUE_H
#define CS5600_LAB1_QUEUE_H

#include <stdio.h>
#include <stdlib.h>

typedef struct node_t {
    struct node_t *next;
    //char data[128];
    /*Challenge 1: allocate memory dynamically
    */
    char *data;
} node_t;


typedef struct queue_t {
    struct node_t *head;
    struct node_t *tail;
} queue_t;


void enqueue(queue_t *q, char *data);
void *dequeue(queue_t *q);
void print_queue(queue_t *q);
void free_queue(queue_t *q);
void pop_queue(queue_t *q);

#endif
