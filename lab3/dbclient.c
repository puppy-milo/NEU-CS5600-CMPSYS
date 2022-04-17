/*
 * db client for CS5600 Lab3
 *
 * this is a really ugly piece of code, need to clean it up before the
 * next time I assign it.
 *
 * Peter Desnoyers, 2020
 *
 * 2021 fall: modified by CS5600 staff
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <zlib.h>
#include <pthread.h>
#include <argp.h>
#include <assert.h>

#include "lab3.h"

/* --------- argument parsing ---------- */

static struct argp_option options[] = {
    {"set",          'S', "KEY",  0, "set KEY to VALUE"},
    {"get",          'G', "KEY",  0, "get value for KEY"},
    {"delete",       'D', "KEY",  0, "delete KEY"},
    {"quit",         'q',  0,     0, "send QUIT command"},
    {"test",         'T', "#threads",  0, "run your concurrency test with given number of threads"},
    {0}
};

enum {OP_SET = 1, OP_GET = 2, OP_DELETE = 3, OP_QUIT = 4, OP_TEST = 5};

struct args {
    int nthreads;
    int count;
    int port;
    int max;
    int op;
    char *key;
    char *val;
    char *logfile;
    FILE *logfp;
    struct sockaddr_in addr;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    struct args *a = state->input;
    switch (key) {
    case ARGP_KEY_INIT:
        a->nthreads = 1;
        a->count = 1000;
        a->port = 5600;
        a->max = 200;
        a->logfp = NULL;
        break;

    case 'l':
        a->logfile = arg;
        if ((a->logfp = fopen(arg, "w")) == NULL)
            fprintf(stderr, "Error opening logfile : %s : %s\n", arg,
                    strerror(errno)), exit(1);
        break;

    case 'q':
        a->op = OP_QUIT;
        break;

    case 'G':
        a->op = OP_GET;
        if (strlen(arg) > 30)
            printf("key must be <= 30 chars\n"), argp_usage(state);
        a->key = arg;
        break;

    case 'S':
        a->op = OP_SET;
        if (strlen(arg) > 30)
            printf("key must be <= 30 chars\n"), argp_usage(state);
        a->key = arg;
        break;

    case 'T':
        a->op = OP_TEST;
        a->nthreads = atoi(arg);
        if (a->nthreads == 0) {
            printf("#threads must be a number\n"), argp_usage(state);
        }
        break;

    case 'D':
        a->op = OP_DELETE;
        if (strlen(arg) > 30)
            printf("key must be <= 30 chars\n"), argp_usage(state);
        a->key = arg;
        break;

    case 'n':
        a->count = atoi(arg); break;

    case 'p':
        a->port = atoi(arg);
        break;

    case ARGP_KEY_ARG:
        if (state->arg_num == 0 && a->op == OP_SET) {
            a->val = arg;
        printf("val set to %s\n", arg);
    }
        else
            argp_usage(state);
        break;
    }
    return 0;
}

static struct argp argp = { options, parse_opt, NULL, NULL};

/* --------- everything else ---------- */

int do_connect(struct sockaddr_in *addr)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0 || connect(sock, (struct sockaddr*)addr, sizeof(*addr)) < 0)
        fprintf(stderr, "can't connect: %s\n", strerror(errno)), exit(0);
    return sock;
}


void do_del(struct args *args, char *name, char *result, int quiet)
{
    int sock = do_connect(&args->addr);

    struct request rq;
    snprintf(rq.name, sizeof(rq.name), "%s", name);

    rq.op_status = 'D';
    int val = write(sock, &rq, sizeof(rq));
    if ((val = read(sock, &rq, sizeof(rq))) < 0)
        printf("DEL: REPLY: READ ERROR: %s\n", strerror(errno));
    else if (val < sizeof(rq))
        printf("DEL: REPLY: SHORT READ: %d\n", val);
    else if (rq.op_status != 'K' && !quiet)
        printf("DEL: FAILED (%c)\n", rq.op_status);
    else if (!quiet)
        printf("ok\n");

    if (result != NULL)
        *result = rq.op_status;

    close(sock);
}

void do_set(struct args *args, char *name, void *data, int len, char *result, int quiet)
{
    int sock = do_connect(&args->addr);

    struct request rq;
    snprintf(rq.name, sizeof(rq.name), "%s", name);
    int val;

    rq.op_status = 'W';
    sprintf(rq.len, "%d", len);
    write(sock, &rq, sizeof(rq));
    write(sock, data, len);
    if ((val = read(sock, &rq, sizeof(rq))) < 0)
        printf("WRITE: REPLY: READ ERROR: %s\n", strerror(errno));
    else if (val < sizeof(rq))
        printf("WRITE: REPLY: SHORT READ: %d\n", val);
    else if (rq.op_status != 'K' && !quiet)
        printf("WRITE: FAILED (%c)\n", rq.op_status);
    else if (!quiet)
        printf("ok\n");

    if (result != NULL)
        *result = rq.op_status;
    close(sock);
}

void do_quit(struct args *args)
{
    int sock = do_connect(&args->addr);
    struct request rq;
    rq.op_status = 'Q';
    write(sock, &rq, sizeof(rq));
    /* if both sides close-on-exit you won't get TIME_WAIT */
}

void do_get(struct args *args, char *name, void *data, int *len_p, char *result)
{
    int val, sock = do_connect(&args->addr);
    struct request rq;
    snprintf(rq.name, sizeof(rq.name), "%s", name);

    rq.op_status = 'R';
    sprintf(rq.len, "%d", 0);
    write(sock, &rq, sizeof(rq));
    if ((val = read(sock, &rq, sizeof(rq))) < 0)
        printf("READ: REPLY: READ ERROR: %s\n", strerror(errno));
    else if (val < sizeof(rq))
        printf("READ: REPLY: SHORT READ: %d\n", val);
    else if (rq.op_status != 'K')
        printf("READ: FAILED (%c)\n", rq.op_status);
    else {
        int len = atoi(rq.len);
        char buf[len];

        for (void *ptr = buf, *max = ptr+len; ptr < max; ) {
            int n = read(sock, ptr, max-ptr);
            if (n < 0) {
                printf("READ DATA: READ ERROR: %s\n", strerror(errno));
                break;
            }
            ptr += n;
        }
        if (data != NULL) {
            memcpy(data, buf, len);
            *len_p = len;
        }
        else
            printf("=\"%.*s\"\n", len, buf);
    }

    if (result != NULL)
        *result = rq.op_status;

    close(sock);
}


// === concurrency test ===
// helper functions
// (feel free to modify helper functions)

// helper function: generate random string
void randstr(char *buf, int len)
{
    for (int i = 0; i < len; i++)
        buf[i] = 'A' + (random() % 25);
}

// helper function: generate random keys
void gen_randkey(char *buf) {
    int len = random() % 30 + 1;
    randstr(buf, len);
}

// helper function: generate random values
void gen_randval(char *buf) {
    int len = random() % 4095 + 1;
    randstr(buf, len);
}

/*
 * Exercise 5:
 * Write your own concurrency tests.
 *
 * You should (these are *suggestions* not requirements):
 *
 *  - create multiple threads and send concurrent operations to dbserver
 *
 *  - try to produce unnatural schedules and interleaving to increase chances
 *  to find bugs
 *
 *  - you don't have to follow the six rules or always use monitor here
 *    because this is for debugging purpose. For example,
 *
 *    -- you can use sleep,
 *
 *    -- and feel free to use synchronization primitives (e.g., mutexes)
 *    whatever way you like
 *
 *  - to check correctness, we suggest you to compare your writes and check
 *  the return values from reads. How? remember your write values and compare
 *  the values when you read from the same keys.
 *
 *  - hints:
 *    -- read "do_get/do_set/do_del" to see how to issue these operations
 *    -- you can use the above helper functions to generate keys/values
 *    -- if you want to share data between multiple threads, protect the shared data with mutex!
 *       (note: you will not lose points due to coding style in Exercise 5.)
 */

struct entry_bench_t {
    char key[31];
    char value[4096];
    int used;
};

struct param_test_t {
    struct entry_bench_t *benchmark;
    pthread_mutex_t *benchM;
    struct args *a;
};

void *test_set(void *T) {
    struct param_test_t *Test = (struct param_test_t *)T;
    int i = random() % 50;
    char result;
    do_set(Test->a, Test->benchmark[i].key, Test->benchmark[i].value,
           strlen(Test->benchmark[i].value), &result, 1);
    if (result == 'K') {
        pthread_mutex_lock(Test->benchM);
        Test->benchmark[i].used = 1;
        pthread_mutex_unlock(Test->benchM);
    }
    printf("*** PASS: test_cases/set\n");
    return 0;
}

void *test_get(void *T) {
    struct param_test_t *Test = (struct param_test_t *)T;
    // choose one record in benchmark table randomly
    int i = random() % 50;
    char data[4096];
    memset(data, 0, sizeof(data));
    int len;
    char result;
    do_get(Test->a, Test->benchmark[i].key, data, &len, &result);
    if (result == 'K') {
        // if execute ok, compare expected and got
        if (0 == strcmp(Test->benchmark[i].value, data))
            printf("*** PASS: test_cases/get\n");
        else
            printf("*** NOT PASS: test_cases/get\n");
    } else if (result == 'X') {
        // if execute wrong, no record in benchmark table
        if (0 == Test->benchmark[i].used)
            printf("*** PASS: test_cases/get\n");
        else
            printf("*** NOT PASS: test_cases/get\n");
    }
    return 0;
}

void *test_del(void *T) {
    struct param_test_t *Test = (struct param_test_t *)T;
    int i = random() % 50;
    char result;
    do_del(Test->a, Test->benchmark[i].key, &result, 1);
    if (result == 'K') {
        pthread_mutex_lock(Test->benchM);
        Test->benchmark[i].used = 0;
        pthread_mutex_unlock(Test->benchM);
        printf("*** PASS: test_cases/delete\n");
    } else
        printf("*** NOT PASS: test_cases/delete\n");
    return 0;
}

void do_test(struct args *a)
{
    // fix random seed; easier to deterministically reproduce bugs
    srandom(5600);

    int num_threads = a->nthreads;

    /* TODO: your code here */
    pthread_t tid[num_threads];

    // a benchmark table; used to compare what is expected and what is got
    struct entry_bench_t benchmarks[50];
    memset(benchmarks, 0, sizeof(benchmarks));
    // helper funciton to generate random key and value
    for (int i = 0; i < 50; i++) {
        gen_randkey(benchmarks[i].key);
        gen_randval(benchmarks[i].value);
    }
    // monitor; make operation on tables exclusively
    pthread_mutex_t benchM = PTHREAD_MUTEX_INITIALIZER;
    struct param_test_t T;
    T.a = a;
    T.benchmark = benchmarks;
    T.benchM = &benchM;

    int op;
    for (int i = 0; i < num_threads; i++) {
        // decide operation randomly, the ratio R/W/D is 8:1.5:0.5, very likely instruction.
        op = random() % 100;
        if (op < 80)
            pthread_create(&tid[i], 0, test_get, &T);
        if (op >= 80 && op < 95)
            pthread_create(&tid[i], 0, test_set, &T);
        if (op >= 95)
            pthread_create(&tid[i], 0, test_del, &T);
    }
    // wait util all threads exit
    for (int i = 0; i < num_threads; i++) {
        pthread_join(tid[i], 0);
    }
    printf("Your test here with %d threads\n", num_threads);

}

int main(int argc, char **argv)
{
    struct args args;
    memset(&args, 0, sizeof(args));

    argp_parse(&argp, argc, argv, 0, 0, &args);

    args.addr = (struct sockaddr_in){
        .sin_family = AF_INET,
        .sin_port = htons(args.port),
        .sin_addr.s_addr = inet_addr("127.0.0.1")}; /* localhost */

    if (args.op == OP_SET)
        do_set(&args, args.key, args.val, strlen(args.val), NULL, 0);
    else if (args.op == OP_GET)
        do_get(&args, args.key, NULL, NULL, NULL);
    else if (args.op == OP_DELETE)
        do_del(&args, args.key, NULL, 0);
    else if (args.op == OP_QUIT)
        do_quit(&args);
    else if (args.op == OP_TEST)
        do_test(&args);
    else
        printf("See usage: \"dbclient --help\"\n");
}

