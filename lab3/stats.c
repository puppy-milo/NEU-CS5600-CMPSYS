#include "stats.h"

// Exercise 3: fix concurrency bugs by Monitor

// FIXME:
// These statistics should be implemented as a Monitor,
// which keeps track of dbserver's status

int n_writes = 0;  // number of writes
int n_reads = 0;   // number of reads
int n_deletes = 0; // number of deletes
int n_fails = 0;   // number of failed operations


// TODO: define your synchronization variables here
// (hint: don't forget to initialize them)

/* your code here */
pthread_mutex_t wm = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t rm = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t dm = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t fm = PTHREAD_MUTEX_INITIALIZER;

// FIXME: implementation below is not thread-safe.
// Fix this by implementing them as a Monitor.

void inc_write() {
    pthread_mutex_lock(&wm);

    n_writes++;

    pthread_mutex_unlock(&wm);
}

void inc_read() {
    pthread_mutex_lock(&rm);

    n_reads++;

    pthread_mutex_unlock(&rm);
}

void inc_delete() {
    pthread_mutex_lock(&dm);

    n_deletes++;

    pthread_mutex_unlock(&dm);
}

void inc_fail() {
    pthread_mutex_lock(&fm);

    n_fails++;

    pthread_mutex_unlock(&fm);
}


int get_writes() {
    return n_writes;
}

int get_reads() {
    return n_reads;
}

int get_deletes() {
    return n_deletes;
}

int get_fails() {
    return n_fails;
}
