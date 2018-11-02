#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "frame.h"
#include "prioqueue.h"

// GLOBAL
#define ackLength 6

typedef struct {
  frame* frames;
  int length;
} Buffer;

void initBuffer(Buffer* a, int buffersize) {
    a->frames = (frame*) malloc(buffersize * sizeof(frame));
    a->length = 0;
}

void die(char *s){
    perror(s);
    exit(1);
}

int initSocket(int* udpSocket, struct sockaddr_in *address, char* ip, int port){
    //try to create UDP socket
    *udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(*udpSocket < 0){
        die("Failed to create UDP Socket");
    }

    memset(address, 0, sizeof(*address));
    address->sin_family = AF_INET;
    address->sin_port = htons(port); 
    inet_aton(ip , &address->sin_addr);

    printf("Create UDP Socket on address : %s port : %d SUCCESS\n", ip, port);
	fflush(stdout);
    return 1;
}

int main(int argc, char *argv[]){

    if (argc < 6){
        die("<filename> <windowsize> <buffersize> <destination_ip> <destination_port>");
    }

    //accept arguments
    char* filename = argv[1];
    int SWS = atoi(argv[2]);    //SWS = Send Window Size
    int buffersize = atoi(argv[3]);
    char* destinationip = argv[4];

    int udpSocket, len;
    struct sockaddr_in clientAddress;

    //create socket
    initSocket(&udpSocket, &clientAddress, destinationip, atoi(argv[5]));

    //try to open file
    FILE *fp;
    fp = fopen(filename, "r");
    if(fp == NULL){
        die("Could not open file");
    }

    //initial buffer
    Buffer send_buffer;
    initBuffer(&send_buffer,buffersize);

    int LAR = -1;	//LAR = Sequence number of last acknowledgement received 
    char* frame_buff;
    char* raw;
    int LFS = -1;	//LFS = last frame sent

	int c;
	while (1) {
		free(send_buffer.frames);
		initBuffer(&send_buffer,buffersize);
		printf("Input %d frame to buffer\n", buffersize); fflush(stdout);
		
		//input file to buffer
		int n = 0;
		while (n < buffersize && (c = fgetc(fp)) != EOF){
			ungetc(c,fp);                
			int length = 0;
			int framesize = 0;
			char* temp = (char*) malloc(sizeof(char)*1024);

			while (length < 1024 && (c = fgetc(fp)) != EOF) {
				temp[length] = (char) c;
				length = length + 1;
			}
			
			framesize = length + 10;
			frame frm = create_frame(n,length,temp); 
			char* rawFrame = (char*) malloc(sizeof(char)*framesize);
			frame_to_raw(frm, rawFrame);
			frm.checksum = count_checksum(rawFrame, framesize);
			free(rawFrame);
			send_buffer.frames[n] = frm;
			n++;
		}
		
		PrioQueue packets;
		CreateEmpty(&packets);
		
		int isibuffer = n;
		int maxLAR = 0;
		int seqNum = 0;	
		int windowsize = seqNum + SWS;
		LFS = seqNum;
		  
		while (seqNum < buffersize && isibuffer > 0) {
			//send a frame
			if (LFS < n && LFS < windowsize){
				frame_buff = (char*) malloc(sizeof(char)*(send_buffer.frames[LFS].dataLength+10));
				frame_to_raw(send_buffer.frames[LFS],frame_buff);
				sendto(udpSocket,frame_buff,(send_buffer.frames[LFS].dataLength+10),0,(struct sockaddr *)&clientAddress,sizeof(clientAddress));
				printf("Frame %d was sent\n", LFS); fflush(stdout);
				
				free(frame_buff);
				
				infotype x;
				x.sentTime = time(NULL);
				x.seqNum = LFS;
				Add(&packets,x);
				printPrioQueue(packets);
				
				LFS = LFS + 1;
			}
			
			//check timeout
			long int current_time = time(NULL);
			infotype y;
			Del(&packets,&y);
			if ((current_time - y.sentTime) == 10) {
				frame_buff = (char*) malloc(sizeof(char)*(send_buffer.frames[y.seqNum].dataLength+10));
				frame_to_raw(send_buffer.frames[y.seqNum],frame_buff);
				
				sendto(udpSocket,frame_buff,(send_buffer.frames[y.seqNum].dataLength+10),0,(struct sockaddr *)&clientAddress,sizeof(clientAddress));   //kirimnya itu per frame, mgkn pake ukuran frames[pos]
				printf("Frame %d was sent\n", y.seqNum); fflush(stdout);
				free(frame_buff);
				y.sentTime = time(NULL);
			}
			Add(&packets,y);
			
			//set timeout
			fd_set select_fds;
			struct timeval timeout;

			FD_ZERO(&select_fds);
			FD_SET(udpSocket, &select_fds);

			timeout.tv_sec = 1;
			timeout.tv_usec = 0;
			
			//accept ACK
			packet_ack ack;
			if (select(32, &select_fds, NULL, NULL, &timeout) == 0){	//wait for 5 seconds until timeout
				printf("Waiting for ACK...\n"); fflush(stdout);
			} else {
				char* rawAck = (char*) malloc(ackLength*sizeof(char));
				len = recvfrom(udpSocket,rawAck,ackLength,0,NULL,NULL);
				to_ack(&ack, rawAck);
				
				if ((ack.ack == 0x1) /*&& (count_checksum(rawAck,5) == ack.checksum)*/) {
					LAR = ack.nextSeqNumber - 1;
					if (LAR > maxLAR) {
						maxLAR = LAR;
					}

					printf("ACK accepted. LAR : %d\n",LAR); fflush(stdout);
					
					isibuffer = isibuffer-1;
					printf("\nPacket in buffer :\n"); fflush(stdout);
					printPrioQueue(packets);
					
					DelSpecific(&packets,LAR);
					
					if (LAR == seqNum) {
						if (maxLAR == LAR) {
							seqNum = seqNum + 1;
							windowsize = windowsize + 1;
						} else if (maxLAR > LAR) {
							seqNum = seqNum + (maxLAR - LAR);
							windowsize = windowsize + (maxLAR - LAR);
						}
					}
				} else if ((ack.ack == 0x0) /*|| (count_checksum(rawAck,5) != ack.checksum)*/){
					printf("NAK accepted, resending frame...\n");
					int resendSeqNum = ack.nextSeqNumber - 1;
					DelSpecific(&packets,resendSeqNum);
					frame_buff = (char*) malloc(sizeof(char)*(send_buffer.frames[resendSeqNum].dataLength+10));
					frame_to_raw(send_buffer.frames[resendSeqNum],frame_buff);
					
					sendto(udpSocket,frame_buff,(send_buffer.frames[resendSeqNum].dataLength+10),0,(struct sockaddr *)&clientAddress,sizeof(clientAddress));   //kirimnya itu per frame, mgkn pake ukuran frames[pos]
					printf("Frame %d was sent\n", resendSeqNum); fflush(stdout);
					free(frame_buff);

					infotype resendInfo;
					resendInfo.sentTime = time(NULL);
					resendInfo.seqNum = resendSeqNum;
					Add(&packets,resendInfo);
				}
				free(rawAck); 
			}
        }
        
        if (c == EOF) {
			//sign the end of file to receiver
			frame s = create_sentinel();
			frame_buff = (char*) malloc(sizeof(char)*11);
			frame_to_raw(s,frame_buff);
			sendto(udpSocket,frame_buff,11,0,(struct sockaddr *)&clientAddress,sizeof(clientAddress));
			free(frame_buff);
			printf("Reached the end of file.\n"); fflush(stdout);
			break;
		}
    }
    
    free(send_buffer.frames);
    close(udpSocket);

    return 0;
}
