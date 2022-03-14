#include <string.h>
#include <assert.h>
#include "caesar.h"


/**
 * Exercise 1
 * this function encodes the string "plaintext" using the Caesar cipher
 * by shifting characters by "key" positions.
 * Hint: you can reuse the memory from input as the output has
 * the same length as the input.
 **/
char *encode(char *plaintext, int key) {
    /*TODO: add your code here*/
    const int NBR_BASE = 10;
    const int ALFA_BASE = 26;
    for(int i = 0; i < strlen(plaintext); i++){
        int c = plaintext[i];
        if(c>='0'&&c<='9'){
            plaintext[i] = (c-'0'+key)%NBR_BASE+'0';
            continue;
        }
        if(c>='a'&&c<='z'){
            plaintext[i] = (c-'a'+key)%ALFA_BASE+'a';
            continue;
        }
        if(c>='A'&&c<='Z'){
            plaintext[i] = (c-'A'+key)%ALFA_BASE+'A';
            continue;
        }
        return "ILLCHAR";
    }
    return plaintext;
    //return "UNIMPLEMENTED";
}


/**
 * Exercise 2
 * This function decodes the "ciphertext" string using the Caesar cipher
 * by shifting the characters back by "key" positions.
 **/
char *decode(char *ciphertext, int key) {
    /*TODO: add your code here*/
    const int NBR_BASE = 10;
    const int ALFA_BASE = 26;
    for(int i = 0; i < strlen(ciphertext); i++){
        int c = ciphertext[i];
        if(c>='0'&&c<='9'){
            while(c-'0'-key<0){
                c+=NBR_BASE;
            }
            ciphertext[i] = (c-'0'-key)%NBR_BASE+'0';
            continue;
        }
        if(c>='a'&&c<='z'){
            while(c-'a'-key<0){
                c+=ALFA_BASE;
            }
            ciphertext[i] = (c-'a'-key)%ALFA_BASE+'a';
            continue;
        }
        if(c>='A'&&c<='Z'){
            while(c-'A'-key<0){
                c+=ALFA_BASE;
            }
            ciphertext[i] = (c-'A'-key)%ALFA_BASE+'A';
            continue;
        }
        return "ILLCHAR";
    }
    return ciphertext;
    //return "UNIMPLEMENTED";
}
