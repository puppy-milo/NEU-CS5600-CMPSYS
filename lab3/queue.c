#include "queue.h"

// Exercise 2: implement a concurrent queue

// TODO: define your synchronization variables here
// (hint: don't forget to initialize them)
pthread_cond_t nonempty = PTHREAD_COND_INITIALIZER;
pthread_mutex_t qm = PTHREAD_MUTEX_INITIALIZER;

// add a new task to the end of the queue
// NOTE: queue must be implemented as a monitor
void enqueue(queue_t *q, task_t *t) {
    /* TODO: your code here */
    pthread_mutex_lock(&qm);

    if (q->count == 0) {
        q->head = t;
        q->tail = t;
        q->count++;
    } else {
        q->tail->next = t;
        q->tail = t;
        q->count++;
    }

    pthread_cond_broadcast(&nonempty);
    pthread_mutex_unlock(&qm);
}

// fetch a task from the head of the queue.
// if the queue is empty, the thread should wait.
// NOTE: queue must be implemented as a monitor
task_t* dequeue(queue_t *q) {
    /* TODO: your code here */
    pthread_mutex_lock(&qm);
    while (q->count == 0)
        pthread_cond_wait(&nonempty, &qm);

    task_t *curHead = q->head;
    q->head = curHead->next;
    q->count--;

    pthread_mutex_unlock(&qm);
    return curHead;
}

// return the number of tasks in the queue.
// NOTE: queue must be implemented as a monitor
int queue_count(queue_t *q) {
    /* TODO: your code here */
    pthread_mutex_lock(&qm);

    int cnt = q->count;

    pthread_mutex_unlock(&qm);
    return cnt;
}
