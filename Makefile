CC = gcc
BIN = bin/

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
	[ -d $(BIN) ] || mkdir -p $(BIN) 
	$(CC) $(SENDSOURCES) -o bin/sender
	$(CC) $(RECEIVESOURCES) -o bin/receiver

sender:
	[ -d $(BIN) ] || mkdir -p $(BIN)
	$(CC) $(SENDSOURCES) -o bin/sender

receiver:
	[ -d $(BIN) ] || mkdir -p $(BIN)
	$(CC) $(RECEIVESOURCES) -o bin/receiver






