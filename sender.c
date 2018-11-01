#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "frame.h"

// GLOBAL
#define ACKSIZE 6

typedef struct {
  frame* frames;
  int length;
} BufferArray;

void initBufferArray(BufferArray* a, int buffersize) {
    a->frames = (frame*) malloc(buffersize * sizeof(frame));
    a->length = 0;
}

void die(char *s){
    perror(s);
    exit(1);
}

int init_socket_address(int* udpSocket, struct sockaddr_in *address, char* ip, int port){
    /*Create UDP socket*/
    *udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(*udpSocket < 0){
        die("Failed create UDP Socket");
    }

    /*Configure settings in address struct*/
    memset(address, 0, sizeof(*address));
    address->sin_family = AF_INET;
    address->sin_port = htons(port); 
    inet_aton(ip , &address->sin_addr);

    printf("[%d] success on address %s port %d\n", (int) time(0), ip, port);
    fflush(stdout);
    return 1;
}

int main(int argc, char *argv[]){
    if (argc < 6){
        die("<filename> <windowsize> <buffersize> <destination_ip> <destination_port>");
    }

    // read from argument
    char* filename = argv[1];
    int SWS = atoi(argv[2]);    //SWS = Send Window Size
    int buffersize = atoi(argv[3]); //buffer size ganti dalam frame
    char* destinationip = argv[4];

    int udpSocket, len;
    struct sockaddr_in clientAddress;

    // open connection
    init_socket_address(&udpSocket, &clientAddress, destinationip, atoi(argv[5]));

    // open file to read
    FILE *fp;
    fp = fopen(filename, "r");
    if(fp == NULL){
        die("Couldn't open file");
    }

    // initial buffer
    BufferArray send_buffer;
    initBufferArray(&send_buffer,buffersize);

    int LAR = -1;//, LFS = LAR + SWS; LAR = Sequence number of last acknowledgement received, LFS = last frame sent
    char* frame_buff;
    char* raw;
    int LFS = -1;

	int c;
	while(1){
		free(send_buffer.frames);
		initBufferArray(&send_buffer,buffersize);
		printf("[%d] prepare %d frame to buffer\n", time(0), buffersize); fflush(stdout);
		
		int n = 0;
		while (n < buffersize && (c = fgetc(fp)) != EOF){ //intinya ini itu buat bikin frame sejumlah buffersize/ sampe eof
			ungetc(c,fp);                
			int length = 0;
			int framesize = 0;
			char* temp = (char*) malloc(sizeof(char)*1024);

			while (length < 1024 && (c = fgetc(fp)) != EOF) {
				temp[length] = (char) c;
				length = length + 1;
			}

			if (c == EOF) {
				temp[length-1] = '\0';
				length = length - 1;
			}
			
			framesize = length + 10;
			frame frm = create_frame(n,length,temp); 
			char* raw = (char*) malloc(sizeof(char)*framesize); //ini dia bikin jadi raw itu cuma buat checksum ya wkkw menarik tp mmg bener klo ga mau kaya gimana checksumnya
			frame_to_raw(frm, raw);
			frm.checksum = checksum_str(raw, 8);	//checksum blm dicek
			send_buffer.frames[n] = frm;    //trs nanti frame yg udh jadi (ada datanya+checksum) dimasukin ke send_buffer.frames[n]
			n++;
		}
		
		int isibuffer = n;
		int maxLAR = 0;
		int seqNum = 0;	//seqnum itu jd sisi kiri sliding window
		int windowsize = seqNum + SWS;	//windowsize itu jadi sisi kanan sliding windows
		LFS = seqNum;
		 
		//dia kirim  
		while (seqNum < buffersize && isibuffer > 0) {	//mungkin gini ga sih, bikin mode kirim sama mode terima
			//kirim 1 frame
			printf("oh kesini juga\n"); fflush(stdout);
			printf("LFS: %d	windowsize: %d",LFS,windowsize);
			if (LFS < n && LFS < windowsize){ //sw itu gunanya kalau ada ack yang belum sampe, dia gabisa kirim yg lain, tunggu dlu semua yg di window kekirim dan dapet ack
				frame_buff = (char*) malloc(sizeof(char)*(send_buffer.frames[LFS].dataLength+10));
				frame_to_raw(send_buffer.frames[LFS],frame_buff);
				sendto(udpSocket,frame_buff,(send_buffer.frames[LFS].dataLength+10),0,(struct sockaddr *)&clientAddress,sizeof(clientAddress));
				printf("[%d] frame %c%c%c%c with data %c was sent\n", (int) time(0), frame_buff[1],frame_buff[2],frame_buff[3],frame_buff[4],frame_buff[9]); fflush(stdout);
				/*infotype x;
				x.sentTime = time(0);
				x.seqNum = LFS;
				Add(&packets,x);*/
				printf("dimanaaa\n"); fflush(stdout);
				free(frame_buff);
				
				printf("%d\n",LFS);
				printf("%s\n\n",send_buffer.frames[LFS].data);
				
				LFS = LFS + 1;
			}
			
			// timeout setting
			fd_set select_fds;
			struct timeval timeout;

			FD_ZERO(&select_fds);
			FD_SET(udpSocket, &select_fds);

			timeout.tv_sec = 1;
			timeout.tv_usec = 0;
			//kalau udh terima ack, delete dari queue
			//terima ack
			
			/*int current_time = time(0);
			if ((current_time - waktu kirim suatu packet) == brp gitu) {
				kirim ulang
			}*/
			packet_ack ack;
			if (select(32, &select_fds, NULL, NULL, &timeout) == 0){	//menunggu 5 detik sampe timeout abis
				printf("lewat timeout\n"); fflush(stdout);
				/*frame_buff = (char*) malloc(sizeof(char)*(send_buffer.frames[LFS-1].dataLength+10));
				frame_to_raw(send_buffer.frames[LFS-1],frame_buff);
				
				sendto(udpSocket,frame_buff,(send_buffer.frames[LFS-1].dataLength+10),0,(struct sockaddr *)&clientAddress,sizeof(clientAddress));   //kirimnya itu per frame, mgkn pake ukuran frames[pos]
				printf("[%d] frame %c%c%c%c with data %c was sent\n", (int) time(0), frame_buff[1],frame_buff[2],frame_buff[3],frame_buff[4],frame_buff[9]); fflush(stdout);
				free(frame_buff5);*/
			} else {
				// prepare raw to receive ACK
				raw = (char*) malloc(ACKSIZE*sizeof(char));
				len = recvfrom(udpSocket,raw,ACKSIZE,0,NULL,NULL);
				to_ack(&ack, raw);
				free(raw);
				LAR = ack.nextSeqNumber - 1;
				if (LAR > maxLAR) {
					maxLAR = LAR;
				}

				printf("wah dapet ack\n"); fflush(stdout);
				printf("Ini ACK : %d",LAR);
				isibuffer = isibuffer-1;
				printf("sekarang isi buffer jadi %d",isibuffer); fflush(stdout);
				/*if any of the frame in sliding window timeout dan belum dapet ack|| any of the frame in sliding window nak{ //ambil sequence numbernya, trs
					frame_buff = (char*) malloc(sizeof(char)*(send_buffer.frames[i].dataLength+10));
					frame_to_raw(send_buffer.frames[i],frame_buff);
					
					sendto(udpSocket,frame_buff,(send_buffer.frames[i].dataLength+10),0,(struct sockaddr *)&clientAddress,sizeof(clientAddress));   //kirimnya itu per frame, mgkn pake ukuran frames[pos]
					printf("[%d] frame %c%c%c%c with data %c was sent\n", (int) time(0), frame_buff[1],frame_buff[2],frame_buff[3],frame_buff[4],frame_buff[9]); fflush(stdout);
					free(frame_buff);
				}*/
				
				/*if ack benar bkn nak {
					Del()
				}*/
				
				if (LAR == seqNum) { //antara geser 1 atau geser banyak
					printf("masuk ke sini ga?\n"); fflush(stdout);
					if (maxLAR == LAR) {	//geser 1
						printf("	oh geser 1 ya\n"); fflush(stdout);
						seqNum = seqNum + 1;	//geser sisi kiri sliding window
						windowsize = windowsize + 1;	//geser sisi kanan sliding window
					} else if (maxLAR > LAR) { //geser banyak
						printf("	oh geser banyak ya\n"); fflush(stdout);
						seqNum = seqNum + (maxLAR - LAR);
						windowsize = windowsize + (maxLAR - LAR);
					}
				}
					
			}
			printf("haloooo\n"); fflush(stdout);
        }
        if(c==EOF){
			// send last sentinel
			frame s = create_sentinel();
			frame_buff = (char*) malloc(sizeof(char)*11);
			frame_to_raw(s,frame_buff);
			sendto(udpSocket,frame_buff,11,0,(struct sockaddr *)&clientAddress,sizeof(clientAddress));
			free(frame_buff);
			printf("FINISH\n"); fflush(stdout);
			break;
		}
    }
    
    free(send_buffer.frames);
    close(udpSocket);

    return 0;
}
