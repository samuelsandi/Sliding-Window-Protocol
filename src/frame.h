#ifndef FRAME_H
#define FRAME_H

#include <stdio.h>
#include <stdlib.h>

typedef struct {
	char soh;
	int seqNumber;
	int dataLength;
	char* data;
	unsigned char checksum;
} frame;

typedef struct {
	char ack;
	int nextSeqNumber;
	unsigned char checksum;
} packet_ack;

void frame_to_raw(frame frm, char* raw);

void to_frame(frame* frm, char* raw);

void ack_to_raw(packet_ack ack_frm, char* raw);

void to_ack(packet_ack* ack_frm, char* raw);

unsigned char count_checksum_packet(frame frm);

unsigned char count_checksum_ACK(packet_ack paket);

frame create_frame(int n, int dl, char* c);

frame create_eof();

#endif
