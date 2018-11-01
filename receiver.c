#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include "frame.h"

typedef struct {
  frame* frames;
  int length;
} BufferArray;

void die(char *s){
    perror(s);
    exit(1);
}

void init_socket(int* udpSocket, int port){
    /*Create UDP socket*/
    *udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(*udpSocket < 0){
        die("Failed create UDP Socket");
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

    printf("[%d] socket success on port %d\n", (int) time(0), port);
    fflush(stdout);
}

void initBufferArray(BufferArray* a, int BUFLEN) {
    // free(a->frames);
    a->frames = (frame*) malloc(BUFLEN * sizeof(frame));
    a->length = 0;
}

void writeToFile(char* filename, char* message, int n) {
    for(int i=0; i<n; i++){
        printf("WRITE : %c\n", message[i]);
    }
    FILE *fp;
    fp=fopen(filename, "a+");
    fwrite(message, sizeof(message[0]), n, fp);
    fclose(fp);
}

void drainBufferArray(BufferArray* a, char* filename, int BUFLEN) {
    char temp[a->length];
    for (int i = 0; i < a->length; i++) {
        frame aframe = *(a->frames + i * 1034);
        // printf(" %c",(char) aframe.data);
        temp[i] = (char) aframe.data;
    }
    // printf("\n");
    writeToFile(filename, temp, a->length);
    free(a->frames);
    initBufferArray(a,BUFLEN);
}

void insertBufferArray(BufferArray *a, frame aframe, int buffersize) {
    int curr = a->length * 1034;
    int last_mem = curr + 1034;
    // int remainingMemoryAfterInsertion = buffersize - memoryNeeded;

    // printf("CURR %d MEMNEED %d REMAINING %d\n", curr, memoryNeeded, remainingMemoryAfterInsertion);

    if (last_mem >= buffersize){
    } else {
        *(a->frames + curr) = aframe;
        a->length = a->length + 1;
    }
}

void initFile(char* filename){
    FILE *fp;
    fp=fopen(filename, "w");
    fclose(fp);
}

int main(int argc, char *argv[]){
    
    if(argc < 5){
        die("<filename> <windowsize> <buffersize> <port>");
    }

    // read from argument
    char* FILE_NAME = argv[1];
    int RWS = atoi(argv[2]);
    int BUFLEN = atoi(argv[3]);
    int PORT = atoi(argv[4]);
    // char buf[BUFLEN];

    int udpSocket, len;
    // open connection
    init_socket(&udpSocket, PORT);

    // initial buffer
    BufferArray recv_buffer;
    initBufferArray(&recv_buffer,BUFLEN);

    // initial file
    initFile(FILE_NAME);

    int LFR = -1;
    int LAF = LFR + RWS;
    int seqValid = 1;
    int has_ack[BUFLEN];
    for(int i=0; i<BUFLEN; i++) has_ack[i]=0;
    int next_frm = 0;
    int pos = 0;

    while(1){
        char* frame_buff;
        struct sockaddr_in client_addr;
        int client_size = sizeof(client_addr);

        // timeout setting
        fd_set select_fds;
        struct timeval timeout;

        FD_ZERO(&select_fds);
        FD_SET(udpSocket, &select_fds);

        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        frame frm;
        int reload = 0;
        if ( select(32, &select_fds, NULL, NULL, &timeout) == 0 ){  
            printf("Select has timed out...\n");
        } else{
            // receive from client
            frame_buff = (char*) malloc(sizeof(char)*1034);
            len = recvfrom(udpSocket,frame_buff,1034,0,(struct sockaddr*) &client_addr, &client_size);
            to_frame(&frm,frame_buff);
            free(frame_buff);

            printf("[%d] frame %d caught | have a data\n %s\n", (int) time(0), frm.seqNumber, frm.data);
            fflush(stdout);
            reload = 1;
        }
        // write on file
       /* char* c_in = (char *) malloc(sizeof(char));
        c_in[0] = frm.data;
        writeToFile(FILE_NAME,c_in,1);
        free(c_in);*/

        if(reload){
        
            if(frm.seqNumber == -1){
                drainBufferArray(&recv_buffer,FILE_NAME, BUFLEN);
                printf("FINISH ALL MSG\n");
                break;
            }
            // insert to buffer & accept frame
            insertBufferArray(&recv_buffer,frm,BUFLEN);
            pos = LFR+1;
            printf("%d\n", frm.seqNumber);
            for(int i=0; i<BUFLEN; i++){
                printf("%d", has_ack[i]);
            }
            printf("\n");
            has_ack[frm.seqNumber] = 1;
            int all_full = 1;
            while(pos <= LAF && pos < BUFLEN) {
                if (!has_ack[pos]) {
                    next_frm = pos;
                    LFR=pos-1;
                    LAF=(LFR+RWS < BUFLEN)?LFR+RWS:BUFLEN-1;
                    all_full = 0;
                    break;
                }
                pos++;
            }

            // if next frame hit maximum allowed buffer
            if(all_full){
                drainBufferArray(&recv_buffer,FILE_NAME, BUFLEN);
                for(int i=0; i<BUFLEN; i++) has_ack[i]=0;
                LFR = -1;
                pos = 0;
                LAF = (LFR+RWS < BUFLEN)?LFR+RWS:BUFLEN-1;
                next_frm = LFR+1;
            }            

            printf("---------\n");
            printf(" LFR : %d \n", LFR);
            printf(" RWS : %d \n", RWS);
            printf(" LAF : %d \n", LAF);
            printf(" MAX : %d\n", BUFLEN);
            printf(" --------\n");
            printf(" Next frame Number %d \n", next_frm);
            printf("---------\n\n");

            packet_ack send_ack;
            send_ack.ack = 0x6;
            send_ack.nextSeqNumber = next_frm;
            send_ack.checksum = 0x0;

            char* raw = (char*) malloc(7*sizeof(char));

            ack_to_raw(send_ack,raw);
            send_ack.checksum = checksum_str(raw,5);
	    printf(checksum_str(raw,5));
            raw[5] = send_ack.checksum;

            sendto(udpSocket,raw,7,0,(struct sockaddr*) &client_addr, client_size);
            printf("BERHASIL SEND\n");
            free(raw);
        }
    }
    free(recv_buffer.frames);
    close(udpSocket);

    return 0;
}
