#!/usr/bin/make -f

CC := gcc
TARGETS := caesar queue ciphered-queue
CFLAGS := -Wall -g -fstack-protector-all -fsanitize=address

all: $(TARGETS)

caesar: caesar.h caesar.c caesar_main.c
	gcc -o caesar $(CFLAGS) caesar.c caesar_main.c -I.


queue: queue.h queue.c queue_main.c
	gcc -o queue $(CFLAGS) queue.c queue_main.c -I.


ciphered-queue: caesar.c queue.c ciphered_queue_main.c
	gcc -o ciphered_queue $(CFLAGS) caesar.c queue.c ciphered_queue_main.c -I.

test1: all
	python3 ./tests/caesar_run_tests.py ./caesar

test2: all
	python3 ./tests/queue_run_tests.py ./queue

test3: all
	python3 ./tests/cipher_queue_run_tests.py ./ciphered_queue


.PHONY: clean 
clean:
	rm -rf *.o $(TARGETS)
