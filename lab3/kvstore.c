#include "kvstore.h"


// Exercise 4: finish implementing kvstore
// TODO: define your synchronization variables here
// (hint: don't forget to initialize them)
int activeUsers = 0;
pthread_cond_t nonuser = PTHREAD_COND_INITIALIZER;
pthread_mutex_t kvm = PTHREAD_MUTEX_INITIALIZER;

// source: http://www.cse.yorku.ca/~oz/hash.html
unsigned int hash33(char *k) {
    unsigned int hash = 5381;
    while (*k)
        hash += (hash << 5) + (*k++);
    return (hash & 0x7FFFFFFF) % HB_MAX_SIZE;
}

kvstore_t *kv_init() {
    kvstore_t *kv = (kvstore_t *)malloc(sizeof(kvstore_t));
    kv->size = 0;
    for (int i = 0; i < HB_MAX_SIZE; i++)
        kv->buckets[i] = NULL;
    return kv;
}

key_entry_t *kv_entry_init(char *k, char *v) {
    key_entry_t *n = (key_entry_t *)malloc(sizeof(key_entry_t));
    memset(n->key, 0, sizeof(n->key));
    n->next = NULL;
    n->value = (char *)malloc(sizeof(char) * (strlen(v)+1));

    strcpy(n->key, k);
    strcpy(n->value, v);
    return n;
}

void kv_destroy(kvstore_t *kv) {
    pthread_mutex_lock(&kvm);
    while (activeUsers)
        pthread_cond_wait(&nonuser, &kvm);
    activeUsers = -1;
    pthread_mutex_unlock(&kvm);

    key_entry_t *h, *t;
    for (int i = 0; i < HB_MAX_SIZE; i++) {
        h = kv->buckets[i];
        while (h) {
            t = h;
            h = h->next;
            free(t->value);
            free(t);
        }
    }
    free(kv);
    kv = NULL;

    pthread_mutex_lock(&kvm);
    activeUsers = 0;
    pthread_cond_broadcast(&nonuser);
    pthread_mutex_unlock(&kvm);
}
/* read a key from the key-value store.
 *
 * if key doesn't exist, return NULL.
 *
 * NOTE: kv-store must be implemented as a monitor.
 */
char* kv_read(kvstore_t *kv, char *key) {
    /* TODO: your code here */
    pthread_mutex_lock(&kvm);
    while (activeUsers < 0)
        pthread_cond_wait(&nonuser, &kvm);
    activeUsers++;
    pthread_mutex_unlock(&kvm);

    unsigned int index = hash33(key);
    key_entry_t *h = NULL;
    if (kv->buckets[index]) {
        h = kv->buckets[index];
        while (h && 0 != strcmp(h->key, key))
            h = h->next;
    }
    char *val = NULL;
    if (h)
        val = h->value;

    pthread_mutex_lock(&kvm);
    if (!--activeUsers)
        pthread_cond_signal(&nonuser);
    pthread_mutex_unlock(&kvm);
    // delete this later cuz printing is slow
    // printf("[INFO] read key[%s]\n", key);
    return val;
}


/* write a key-value pair into the kv-store.
 *
 * - if the key exists, overwrite the old value.
 * - if key doesn't exit,
 *     -- insert one if the number of keys is smaller than TABLE_MAX
 *     -- return failure if the number of keys equals TABLE_MAX
 * - return 0 for success; return 1 for failures.
 *
 * notes:
 * - the input "val" locates on stack, you must copy the string to
 *   kv-store's own memory. (hint: use malloc)
 * - the "val" is a null-terminated string. Pay attention to how many bytes you
 *   need to allocate. (hint: you need an extra to store '\0').
 * - Read "man strlen" and "man strcpy" to see how they handle string length.
 *
 * NOTE: kv-store must be implemented as a monitor.
 */

int kv_write(kvstore_t *kv, char *key, char *val) {
    /* TODO: your code here */
    pthread_mutex_lock(&kvm);
    while (activeUsers)
        pthread_cond_wait(&nonuser, &kvm);
    activeUsers = -1;
    pthread_mutex_unlock(&kvm);

    unsigned int index = hash33(key);
    key_entry_t *h = NULL;
    if (kv->buckets[index]) {
        h = kv->buckets[index];
        while (h && 0 != strcmp(h->key, key))
            h = h->next;
    }

    int ret = 0;
    if (h) {
        free(h->value);
        h->value = (char *) malloc(sizeof(char)*(strlen(val)+1));
        strcpy(h->value, val);
    } else {
        if (kv->size == TABLE_MAX) {
            ret = 1;
        } else {
            key_entry_t * node = kv_entry_init(key, val);
            node->next = kv->buckets[index];
            kv->buckets[index] = node;
            kv->size++;
        }
    }

    pthread_mutex_lock(&kvm);
    activeUsers = 0;
    pthread_cond_broadcast(&nonuser);
    pthread_mutex_unlock(&kvm);
    // delete this later cuz printing is slow
    // printf("[INFO] write key[%s]=val[%s]\n", key, val);
    return ret;
}


/* delete a key-value pair from the kv-store.
 *
 * - if the key exists, delete it.
 * - if the key doesn't exits, do nothing.
 *
 * NOTE: kv-store must be implemented as a monitor.
 */
void kv_delete(kvstore_t *kv, char *key) {
    /* TODO: your code here */
    pthread_mutex_lock(&kvm);
    while (activeUsers)
        pthread_cond_wait(&nonuser, &kvm);
    activeUsers = -1;
    pthread_mutex_unlock(&kvm);

    unsigned int index = hash33(key);
    key_entry_t *h = kv->buckets[index];
    key_entry_t *prev = h;
    if (h) {
        while (h && 0 != strcmp(h->key, key)) {
            prev = h;
            h = h->next;
        }
        if (h) {
            if (prev == h) {
                free(prev->value);
                free(prev);
                kv->buckets[index] = NULL;
                kv->size--;
            } else {
                prev->next = h->next;
                free(h->value);
                free(h);
                kv->size--;
            }
        }
    }

    pthread_mutex_lock(&kvm);
    activeUsers = 0;
    pthread_cond_broadcast(&nonuser);
    pthread_mutex_unlock(&kvm);
    // delete this later cuz printing is slow
    // printf("[INFO] delete key[%s]\n", key);
}


// print kv-store's contents to stdout
// note: use any format that you like; this is mostly for debugging purpose
void kv_dump(kvstore_t *kv) {
    /* TODO: your code here */
    pthread_mutex_lock(&kvm);
    while (activeUsers < 0)
        pthread_cond_wait(&nonuser, &kvm);
    activeUsers++;
    pthread_mutex_unlock(&kvm);

    key_entry_t *h = NULL;
    for (int i = 0; i < HB_MAX_SIZE; i++) {
        h = kv->buckets[i];
        while (h) {
            printf("[Key] ->%s,   [Value] -> %s\n", h->key, h->value);
            h = h->next;
        }
    }

    pthread_mutex_lock(&kvm);
    if (!--activeUsers)
        pthread_cond_signal(&nonuser);
    pthread_mutex_unlock(&kvm);
    // printf("TODO: dump key-value store\n");
}
