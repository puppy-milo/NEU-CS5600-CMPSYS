#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

// this function is supposed to return a value that doubles "i"
int double_num(int i) {
    return 2*i;
    // Error 2: 2i is not recognized, it should be 2*i
}

char *msg_from_internet() {
    return "hello world!";  // a fake msg
}

// get a message from internet, copy it to a newly allocated memory,
// and return the memory
char* get_msg() {
    // the original message
    char *msg = msg_from_internet();

    // allocate memory
    int msg_len = strlen(msg);
    char *mem = malloc(msg_len+1);
    /* set enough memory to store string including terminating NULL character*/

    // copy message to memory
    for (int i=0; i<=msg_len; i++) {
	// copy entire string including terminating NULL character    
        mem[i] = msg[i];
    }

    return mem;
    /* msg refers to stack that will be destroyed at the end of function*/
}

void process_msg(char *mem) {
    int len = strlen(mem);
    for (int i=0; i<len; i++) {
        mem[i] = toupper(mem[i]);  // capitalize letters
    }
}


int main(void)
{
    // Exercise 2: find two errors and fix them
    int num = 10;
    // Error 1: semicolon is required in the end of line
    int doub_num = double_num(num);
    printf("double a number (%d) is (%d)\n", num, doub_num);
    // check if the returned value is what we want
    assert(num + num == doub_num);

    // Exercise 3: fix segfault
    char *mem = get_msg();
    process_msg(mem);
    printf("processed msg: %s\n", mem);
    free(mem);   // free memory

    return 0;
}

