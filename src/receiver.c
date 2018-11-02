#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "frame.h"

typedef struct {
  frame* frames;
  int length;
} Buffer;

void initBuffer(Buffer* a, int buffersize) {
    // free(a->frames);
    a->frames = (frame*) malloc(buffersize * sizeof(frame));
    a->length = 0;
}

void die(char *s){
    perror(s);
    exit(1);
}

void initSocket(int* udpSocket, int port){
    /*Create UDP socket*/
    *udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(*udpSocket < 0){
        die("Failed to create UDP Socket");
    }

    /*Configure settings in address struct*/
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));  
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // setting timeout
    struct timeval tv;
    tv.tv_sec = 0.5;
    tv.tv_usec = 0;  // Not init'ing this can cause strange errors
    setsockopt(*udpSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval));

    /*Bind socket with address struct*/
    if(bind(*udpSocket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0){
        die("Couldn't bind socket");
    }

    printf("[%d] socket success on port %d\n", (int) time(NULL), port);
    fflush(stdout);
}

void writeToFile(char* filename, char* message, int n) {
    /*printf("hah\n");
    for(int i=0; i<n; i++){
        printf("WRITE : %c\n", message[i]);
    }*/
    FILE *fp;
    fp = fopen(filename, "a+");
    fwrite(message, sizeof(message[0]), n, fp);
    fclose(fp);
}

void emptyBufferContent(Buffer* a, char* filename, int buffersize) { //buat nulis ke file
    printf("length buffer : %d \n", a->length); 
    for (int i = 0; i < a->length; i++) {
        frame aframe = *(a->frames+i);
        printf("data length : %d\n", aframe.dataLength);
        writeToFile(filename, aframe.data, aframe.dataLength);
    }
    // printf("\n");
    free(a->frames);
    initBuffer(a,buffersize);
}

void insertToBuffer(Buffer *a, frame aframe, int buffersize) {
    printf("masuk insert buffer array\n");
    int curr = a->length;
    int last_mem = curr + 1;

    if (last_mem >= buffersize){
        printf("berarti masuk sini???\n");
    } else {
        *(a->frames + curr) = aframe;
        a->length = a->length + 1;
    }
    printf("seqNumber : %d\n",aframe.seqNumber);
}

void openFile(char* filename){
    FILE *fp;
    fp=fopen(filename, "w");
    fclose(fp);
}

int main(int argc, char *argv[]){
    
    if(argc < 5){
        die("<filename> <windowsize> <buffersize> <port>");
    }
    printf("ini file apa\n"); fflush(stdout);
    // read from argument
    char* filename = argv[1];
    int RWS = atoi(argv[2]);
    int buffersize = atoi(argv[3]);
    int port = atoi(argv[4]);

    int udpSocket, len;
    // open connection
    initSocket(&udpSocket, port);

    // initial buffer
    Buffer recv_buffer;
    initBuffer(&recv_buffer,buffersize);

    // initial file
    openFile(filename);

    
    //LAF equivalent sama LFS
    int LFR = -1;
    int seqNum = 0;
    int windowsize = seqNum + RWS;
    int LAF = seqNum;
    int maxLFR = 0;
    int finished = 0;

    while (1) {
        char* frame_buff;
        struct sockaddr_in clientAddress;
        int clientSize = sizeof(clientAddress);

        frame frm;
        int emptySpace = buffersize;
        
        while (seqNum < buffersize && emptySpace > 0) {
             // timeout setting
            fd_set select_fds;
            struct timeval timeout;

            FD_ZERO(&select_fds);
            FD_SET(udpSocket, &select_fds);

            timeout.tv_sec = 1;
            timeout.tv_usec = 0;

            if (select(32, &select_fds, NULL, NULL, &timeout) == 0) {  
                printf("Waiting for packet...\n");
            } else {
                // receive from client
                printf("LFR: %d  seqNum: %d  windowsize: %d\n", LFR, seqNum, windowsize); //n gantinya apa
                frame_buff = (char*) malloc(sizeof(char)*1034);
                len = recvfrom(udpSocket,frame_buff,1034,0,(struct sockaddr*) &clientAddress, &clientSize);
                to_frame(&frm,frame_buff);
                free(frame_buff);

                printf("[%ld] frame %d caught | have a data\n", time(NULL), frm.seqNumber); fflush(stdout);

                if (frm.seqNumber == -1) {
                    printf("kan\n");    fflush(stdout);
                    emptyBufferContent(&recv_buffer, filename, buffersize);    //pindahin dari buffer ke file
                    printf("FINISH ALL MSG\n");
                    finished = 1;
                    break;
                } else {
                    //nanti hrs dicek salah ga, kalau salah kirim NAK.
                    // insert to buffer & accept frame
                    insertToBuffer(&recv_buffer,frm,buffersize);
                    emptySpace = emptySpace - 1;

                    LFR = frm.seqNumber;
                    if (LFR > maxLFR) { //LFR equivalent sama LAR
                        maxLFR = LFR;
                    }    

                    if (LFR == seqNum) { //antara geser 1 atau geser banyak
                        printf("\n"); fflush(stdout);
                        if (maxLFR == LFR) {    //geser 1
                            printf("    geser 1\n"); fflush(stdout);
                            seqNum = seqNum + 1;    //geser sisi kiri sliding window
                            windowsize = windowsize + 1;    //geser sisi kanan sliding window
                        } else if (maxLFR > LFR) { //geser banyak
                            printf("    geser banyak\n"); fflush(stdout);
                            seqNum = seqNum + (maxLFR - LFR);
                            windowsize = windowsize + (maxLFR - LFR);
                        }
                    }

                    printf("---------\n");
                    printf(" Frame SeqNum : %d \n", frm.seqNumber);
                    printf(" LFR : %d \n", LFR);
                    printf(" RWS : %d \n", RWS);
                    printf(" LAF : %d \n", LAF);
                    printf(" MAX : %d\n", buffersize);
                    printf(" --------\n");
                    printf(" Next frame number %d \n", frm.seqNumber + 1);
                    printf("---------\n\n");
                    
                    packet_ack send_ack;

                    //char* rawFrame = (char*) malloc(sizeof(char)*(frm.dataLength+10));
                    //frame_to_raw(frm, rawFrame);
                    //if (count_checksum(rawFrame,1033) == frm.checksum){
                        send_ack.ack = 0x1;

                    //} else {    // NAK
                        //send_ack.ack = 0x0;
                    //}

                    /*printf("%d,%d",count_checksum(rawFrame,1033),frm.checksum);*/
                    //free(rawFrame);
                    
                    send_ack.nextSeqNumber = frm.seqNumber + 1;

                    char* rawAck = (char*) malloc(sizeof(char)*6);
                    ack_to_raw(send_ack, rawAck);
                    send_ack.checksum = count_checksum(rawAck,5);
                    rawAck[5] = send_ack.checksum;

                    sendto(udpSocket,rawAck,6,0,(struct sockaddr*) &clientAddress, clientSize);
                    printf("BERHASIL SEND\n");
                    free(rawAck);
                }
            }
        }

        if (finished) {
            break;
        }

        // if next frame hit maximum allowed buffer
        if (emptySpace == 0) {
            emptyBufferContent(&recv_buffer,filename,buffersize);
            LFR = -1;
            seqNum = 0;
            windowsize = seqNum + RWS;
            LAF = seqNum;
            maxLFR = 0;
        }            
    }

    free(recv_buffer.frames);
    close(udpSocket);

    return 0;
}
