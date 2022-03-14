#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "queue.h"
#include "caesar.h"

void usage() {
    printf("Usage: ./ciphered-queue <file> \n");
}

/**
 * Exercise 6:
 * define and implement two functions: "encode_enqueue" and "dequeue_decode"
 *   -- "encode_enqueue": encode the string and then enqueue
 *   -- "dequeue_decode": dequeue an element and then decode
 * Read "lab1-shell-APIs.txt" for examples.
 **/

// TODO: implement "encode_enqueue" and "dequeue_decode"
void encode_enqueue(queue_t *q, char*plaintext, int key){
    char *ciphertext = encode(plaintext, key);
    enqueue(q,ciphertext);
    return;
}

void *dequeue_decode(queue_t*q, int key){
    node_t* pnode = NULL;
    if((pnode=dequeue(q))==NULL){
        return NULL;
    }
    if(strcmp(pnode->data,"ILLCHAR")==0){
        return pnode;
    }
    decode(pnode->data,key);
    return pnode;
}
/**
 * Exercise 6:
 * based on the content of "lineptr", you should call one of the four functions:
 *  "enqueue", "dequeue", "encode_enqueue", and "dequeue_decode"
 * Read "lab1-shell-APIs.txt" for examples.
 *
 * Hint: (possibly) useful functions: strtok, strcmp, atoi
 **/
 int process_cmd(char *lineptr, const char *delim, char **argv, int MAX_NBR_ARGS){
     int nbr_args = 0;
     char *token = strtok(lineptr, delim);
     while(nbr_args<MAX_NBR_ARGS && token){
        argv[nbr_args] = (char*)malloc(sizeof(char)*(strlen(token)+1));
        strcpy(argv[nbr_args++], token);
        token = strtok(NULL, delim);
    }
    if(argv[nbr_args-1][strlen(argv[nbr_args-1])-1]=='\n'){
        argv[nbr_args-1][strlen(argv[nbr_args-1])-1]='\0';
    }
    return nbr_args;
 }

int check_cmd(char **argv, int nbr_args){
    if(strcmp(argv[0], "encode_enqueue")==0){
        if(nbr_args<3){
            return 0;
        }
    }
    if(strcmp(argv[0], "dequeue_decode")==0){
        if(nbr_args<2){
            return 0;
        }
    }
    if(strcmp(argv[0], "enqueue")==0){
        if(nbr_args<2){
            return 0;
        }
    }
    return 1;
}
queue_t* copy_queue(queue_t *ori_queue){
    queue_t *des_queue = (queue_t *) malloc(sizeof(queue_t));
    des_queue->head = NULL;
    des_queue->tail = NULL;
    node_t *pnode = ori_queue->head;
    while(pnode){
        enqueue(des_queue, pnode->data);
        pnode = pnode->next;
    }
    return des_queue;
}

int process_line2(queue_t *q, char *lineptr, queue_t **gq, int *pnbr_cmds) {
    // TODO: your code here
    const char delim[2] = " ";
    const int MAX_NBR_ARGS = 3;
    int nbr_args = 0;
    char *argv[MAX_NBR_ARGS];
    memset(argv,'\0', sizeof(char*)*MAX_NBR_ARGS);
    nbr_args = process_cmd(lineptr, delim, argv, MAX_NBR_ARGS);
    if(!check_cmd(argv, nbr_args)){
        printf("%s\n","ERROR");
        for(int i = 0; i < nbr_args; i++){
            free(argv[i]);
        }
        return 1;
    }
    *pnbr_cmds = (*pnbr_cmds)+1;
    gq[*pnbr_cmds] = copy_queue(q);
    if(strcmp(argv[0], "encode_enqueue")==0){
        int key = atoi(argv[1]);
        char *data = argv[2];
        encode_enqueue(gq[*pnbr_cmds], data, key);
    }else if(strcmp(argv[0], "dequeue_decode")==0){
        node_t *pnode = NULL;
        int key = atoi(argv[1]);
        if((pnode=dequeue_decode(gq[*pnbr_cmds],key))!=NULL){
            printf("%s\n",pnode->data);
            if(pnode->data!=NULL){
                free(pnode->data);
            }
            free(pnode);
        }else{
            printf("%s\n","NULL");
        }
    }else if(strcmp(argv[0], "enqueue")==0){
        char *data = argv[1];
        enqueue(gq[*pnbr_cmds], data);
    }else if(strcmp(argv[0], "dequeue")==0){
        node_t *pnode = NULL;
        if((pnode=dequeue(gq[*pnbr_cmds]))!=NULL){
            printf("%s\n",pnode->data);
            if(pnode->data!=NULL){
                free(pnode->data);
            }
            free(pnode);
        }else{
            printf("%s\n","NULL");
        }
    } else if(strcmp(argv[0], "undo")==0){
        free_queue(gq[*pnbr_cmds]);
	free(gq[*pnbr_cmds]);
        *pnbr_cmds = (*pnbr_cmds)-1;
        if(*pnbr_cmds>0){
            free_queue(gq[*pnbr_cmds]);
	    free(gq[*pnbr_cmds]);
            *pnbr_cmds = (*pnbr_cmds)-1;
        }
    }
    for(int i = 0; i < nbr_args; i++){
        free(argv[i]);
    }
    
    return 0;
    // you should remove this line
    //printf("cmd: %s", lineptr);
}

// Notice that the following two functions are borrowed from "queue_main.c".
// (which is bad!) You should not copy-paste code. But in order to isolate you
// possible modifications to "queue_main.c", we clone a copy here.

void final_print(queue_t *q) {
    print_queue(q);
}


int main(int argc, const char *argv[]) {

    if (argc != 2) {
        usage();
        return -1;
    }

    // create a new queue
    queue_t *q = (queue_t *) malloc(sizeof(queue_t));
    q->head = NULL;
    q->tail = NULL;

    // read from file line by line
    const char *path = argv[1];
    FILE *fp;
    char *lineptr = NULL;
    size_t n = 0;

    fp = fopen(path, "r");
    assert(fp != NULL);

    queue_t *gq[50000];
    memset(gq,'\0', sizeof(queue_t*)*50000);
    int nbr_cmds = 0;
    gq[nbr_cmds]=q;

    int ret = 0;
    while (getline(&lineptr, &n, fp) != -1){
        ret = process_line2(gq[nbr_cmds], lineptr, gq, &nbr_cmds);
        if(ret){
            fclose(fp);
            if (lineptr != NULL) {
                free(lineptr);
            }
            for(int i = 0; i<=nbr_cmds; i++){
                free_queue(gq[i]);
		free(gq[i]);
            }

            return 0;
        }
    }

    final_print(gq[nbr_cmds]);

    fclose(fp);
    if (lineptr != NULL) {
        free(lineptr);
    }
    for(int i = 0; i<=nbr_cmds; i++){
        free_queue(gq[i]);
	free(gq[i]);
    }

    return 0;
}
