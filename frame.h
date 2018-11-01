#ifndef FRAME_H
#define FRAME_H

#include <stdio.h>
#include <stdlib.h>

typedef struct {
	char soh;
	int seqNumber;
	int dataLength;
	char* data;
	char checksum;
} frame;

typedef struct {
	char ack;
	int nextSeqNumber;
	char checksum;
} packet_ack;

void frame_to_raw(frame frm, char* raw);

void to_frame(frame* frm, char* raw);

void ack_to_raw(packet_ack ack_frm, char* raw);

void to_ack(packet_ack* ack_frm, char* raw);

char checksum_str(char* x, int len);

frame create_frame(int n, int dl, char* c);

frame create_sentinel();

#endif
