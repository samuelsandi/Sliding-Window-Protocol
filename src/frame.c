#include "frame.h"

void frame_to_raw(frame frm, char *raw) {
	/* soh */
	raw[0] = frm.soh;
	
	/* sequence number */
	char* x = (char*) malloc(sizeof(char)*4);
	sprintf(x,"%d",frm.seqNumber);
	raw[1] = *x;
	raw[2] = *(x+1);
	raw[3] = *(x+2);
	raw[4] = *(x+3);
	
	/* data length */
	char* y = (char*) malloc(sizeof(char)*4);
	sprintf(y,"%d",frm.dataLength);
	raw[5] = *y;
	raw[6] = *(y+1);
	raw[7] = *(y+2);
	raw[8] = *(y+3);
	
	/* data */
	for(int i=0;i<frm.dataLength;i++){
		raw[i+9] = frm.data[i];
	}
	
	/* checksum */
	raw[frm.dataLength+9] = frm.checksum;
	free(x);
	free(y);
}

void to_frame(frame* frm, char* raw) {
	/* start of header */
	frm->soh = *raw;
	
	/* sequence number */
	char* x = (char*) malloc(sizeof(char)*4);
	for(int i=0;i<4;i++) {
		x[i] = *(raw+i+1);	
	}
	frm->seqNumber = atoi(x);
	
	/* data length */
	char* y = (char*) malloc(sizeof(char)*4);
	for(int i=0;i<4;i++) {
		y[i] = *(raw+i+5);
	}
	frm->dataLength = atoi(y);
	
	/* data */
	char* z = (char*) malloc(sizeof(char)*frm->dataLength);
	for(int i=0;i<frm->dataLength;i++){
		z[i] = *(raw+i+9);
	}
	frm->data = z;
	
	/* checksum */
	frm->checksum = *(raw+frm->dataLength+9);
	free(x);
	free(y);
}

void ack_to_raw(packet_ack ack_frm, char* raw) {
	raw[0] = ack_frm.ack;
	char* x = (char*) malloc(sizeof(char)*4);
	sprintf(x,"%d",ack_frm.nextSeqNumber);
	raw[1] = *x;
	raw[2] = *(x+1);
	raw[3] = *(x+2);
	raw[4] = *(x+3);
	raw[5] = ack_frm.checksum;
	free(x);
}

void to_ack(packet_ack* ack_frm, char* raw) {
	ack_frm->ack = *raw;
	char* x = (char*) malloc(sizeof(char)*4);
	for(int i=0;i<4;i++) {
		x[i] = *(raw+i+1);
	}
	ack_frm->nextSeqNumber = atoi(x);
	ack_frm->checksum = *(raw+5);
	free(x);
}

unsigned char count_checksum_packet(frame frm) {
	unsigned char result = 0;
	result += frm.soh;
	result += frm.seqNumber & 0xFF;
	result += frm.dataLength & 0xFF;
	for (int i=0;i<frm.dataLength;i++){
		result += *(frm.data+i);
	}
	return result;
}

unsigned char count_checksum_ACK(packet_ack packet) {
	unsigned char result = 0;
	result += packet.ack;
	result += packet.nextSeqNumber & 0xFF;
	return result;
}

frame create_frame(int n, int dl, char* c){
	frame frm;
	frm.soh = 0x1;
	frm.seqNumber = n;
	frm.dataLength = dl;
	frm.data = c;
	frm.checksum = 0x0;
	return frm;
}

frame create_eof(){
	frame frm;
	frm.soh = 0xFF;
	frm.seqNumber = -1;
	frm.dataLength = 0;
	frm.data = NULL;
	frm.checksum = 0xFF;
	return frm;
}
