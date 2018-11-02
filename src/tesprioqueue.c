#include <stdio.h>
#include <time.h>
#include "prioqueue.h"

int main () {
	PrioQueue pq;
	CreateEmpty(&pq);
	if (IsEmpty(pq)) {
		printf("kosong\n"); fflush(stdout);
	} else {
		printf("kok ga kosong\n"); fflush(stdout);
	}
	 
	infotype x;
	x.sentTime = 0;
	x.seqNum = 1;
	printf("time : %ld\n",x.sentTime); fflush(stdout);
	Add(&pq,x);
	
	infotype y;
	y.sentTime = 3;
	y.seqNum = 2;
	printf("time : %ld\n",y.sentTime); fflush(stdout);
	Add(&pq,y);

	printf("Jumlah elemen : %d\n",NbElmt(pq));
	printPrioQueue(pq);
	
	infotype z;
	z.sentTime = 2;
	z.seqNum = 3;
	printf("time : %ld",z.sentTime); fflush(stdout);
	Add(&pq,z);
	
	infotype a;
	a.sentTime = 1;
	a.seqNum = 4;
	printf("time : %ld",a.sentTime); fflush(stdout);
	Add(&pq,a);
	
	infotype b;
	b.sentTime = 5;
	b.seqNum = 5;
	printf("time : %ld",b.sentTime); fflush(stdout);
	Add(&pq,b);
	
	printf("Jumlah elemen : %d\n\n",NbElmt(pq));
	printPrioQueue(pq);
	
	Del(&pq,&x);
	printPrioQueue(pq);
	printf("%ld\n\n",x.sentTime);
	
	DelSpecific(&pq,5);
	printf("berhasil delete 5\n");
	printPrioQueue(pq);
	DelSpecific(&pq,4);
	printf("berhasil delete 4\n");
	printPrioQueue(pq);
	DelSpecific(&pq,1);
	printf("berhasil delete 1\n");
	printPrioQueue(pq);
	return 0;
}
