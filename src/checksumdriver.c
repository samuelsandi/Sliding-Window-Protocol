#include <stdio.h>
#include "frame.h"

int main () {
	char* apa = (char*) malloc (3*sizeof(char));
	apa[0] = 'a';
	apa[1] = 'b';
	apa[2] = 'c';
	frame cf = create_frame(1,3,apa);
	char* raw = (char*) malloc (13*sizeof(char));
	frame_to_raw(cf,raw);
	printf("%d,%c\n",count_checksum(raw,12),count_checksum(raw,12));
	cf.checksum = count_checksum(raw, 12);
	free(raw);
	char* rawx = (char*) malloc (13*sizeof(char));
	frame_to_raw(cf,rawx);
	printf("%d,%d\n",count_checksum(rawx,12),count_checksum(rawx,12));
	free(rawx);
	return 0;
}