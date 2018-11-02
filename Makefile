CC = gcc

SENDSOURCES = \
src/sender.c \
src/frame.h \
src/frame.c \
src/prioqueue.h \
src/prioqueue.c 
 
RECEIVESOURCES = \
src/receiver.c \
src/frame.h \
src/frame.c 

default:
	$(CC) $(SENDSOURCES) -o bin/sender
	$(CC) $(RECEIVESOURCES) -o bin/receiver

sender:
	$(CC) $(SENDSOURCES) -o bin/sender

receiver:
	$(CC) $(RECEIVESOURCES) -o bin/receiver






