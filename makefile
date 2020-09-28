#Sample Makefile for Malloc
CC=g++
CFLAGS= --std=c++11 -g -O0 -Wall


test1: 
	$(CC) $(CFLAGS) test1.cc -include malloc.cc malloc.h -pthread -o test1


.PHONY:
clean:
	rm -rf  malloc.o test1 test1.o

