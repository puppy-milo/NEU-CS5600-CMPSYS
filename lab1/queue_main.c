#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "queue.h"

void usage() {
    printf("Usage: ./queue <file>\n");
}


/**
 * Exercise 5:
 * based on the content of "lineptr",
 * you should either call "enqueue" or "dequeue".
 * Read "lab1-shell-APIs.txt" for examples.
 *
 * Hint: check out what string functions are provided by C library.
 * Also, here are some possibly useful functions: strtok, strcmp
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
 
void process_line(queue_t *q, char *lineptr) {
    // TODO: your code here
    const char delim[2] = " ";
    const int MAX_NBR_ARGS = 2;

    char *argv[MAX_NBR_ARGS];
    memset(argv,'\0', sizeof(char*)*MAX_NBR_ARGS);
    int nbr_args;
    nbr_args = process_cmd(lineptr, delim, argv, MAX_NBR_ARGS);

    if(strcmp(argv[0], "enqueue")==0){
        if(!argv[1]){
            printf("%s\n","ERROR");
            if(lineptr!=NULL){
                free(lineptr);
            }
            free_queue(q);
            free(q);
            for(int i = 0; i < nbr_args; i++){
                free(argv[i]);
            }
            exit(0);
        }
        char *data = argv[1];
        enqueue(q, data);
    }else if(strcmp(argv[0], "dequeue")==0){
        node_t *pnode = NULL;
        if((pnode=dequeue(q))!=NULL){
            printf("%s\n",pnode->data);
            if(pnode->data!=NULL){
                free(pnode->data);
            }
            free(pnode);
        }else{
            printf("%s\n","NULL");
        }
    }
    for(int i = 0; i < nbr_args; i++){
        free(argv[i]);
    }
    // you should remove this line
    //printf("cmd: %s", lineptr);
}

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

    while (getline(&lineptr, &n, fp) != -1) {
        process_line(q, lineptr);
    }
    final_print(q);

    fclose(fp);
    if (lineptr != NULL) {
        free(lineptr);
    }
    free_queue(q);
    free(q);

    return 0;
}
