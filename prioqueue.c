#include <stdio.h>
#include "prioqueue.h"

/* Prototype manajemen memori */
void Alokasi (address *P, infotype X) {
	// ALGORITMA
	*P = (address) malloc (sizeof (ElmtQueue));
	if (*P != NULL) {
		Info(*P) = X;
		Next(*P) = NULL;
	} else {
		*P = NULL;
	}
}	
/* I.S. Sembarang */
/* F.S. Alamat P dialokasi, jika berhasil maka Info(P)=X dan Next(P)=NULL */
/*      P=NULL jika alokasi gagal */

void Dealokasi (address  P) {
	free(P);
}
/* I.S. P adalah hasil alokasi, P != NULL */
/* F.S. Alamat P didealokasi, dikembalikan ke sistem */

int IsEmpty (PrioQueue Q) {
	return (Head(Q) == NULL);
}
/* Mengirim true jika Q kosong: HEAD(Q)=NULL */

int NbElmt(PrioQueue Q) {
	// KAMUS
	address P;
	int count = 0;
	
	// ALGORITMA
	if (!IsEmpty(Q)) {
		P = Head(Q);
		count = 1;
		while (Next(P) != NULL) {
			count++;
			P = Next(P);
		}
	}
	return count;	
}
/* Mengirimkan banyaknya elemen queue. Mengirimkan 0 jika Q kosong */


/*** Kreator ***/

void CreateEmpty(PrioQueue * Q) {
	Head(*Q) = NULL;
}
/* I.S. sembarang */
/* F.S. Sebuah Q kosong terbentuk */


/*** Primitif Add/Delete ***/

void Add (PrioQueue * Q, infotype X) {
	// KAMUS
	address P, PrevPrec, Prec, Last;
	
	// ALGORITMA
	Alokasi(&P, X);
	if (IsEmpty(*Q)) {
		Head(*Q) = P;
	} else {
		Prec = Head(*Q);
		Last = Next(Head(*Q));

		if (Info(Prec).sentTime <= X.sentTime) {
			while ((Info(Prec).sentTime <= X.sentTime) && (Last != NULL)) {
				PrevPrec = Prec;
				Prec = Last;
				Last = Next(Last);
			}

			if (Info(Prec).sentTime > X.sentTime) {
				Next(P) = Prec;
				Next(PrevPrec) = P;
			} else {
				Next(P) = Next(Prec);
				Next(Prec) = P;
			}
		} else {
			Next(P) = Prec;
			Head(*Q) = P;
		}
	}	
}
/* Proses: Mengalokasi X dan menambahkan X sesuai aturan priority queue
   jika alokasi berhasil; jika alokasi gagal Q tetap */
/* I.S. Q mungkin kosong */

void Del(PrioQueue * Q, infotype * X) {
	// KAMUS
	address P;
	
	// ALGORITMA
	*X = InfoHead(*Q);
	P = Head(*Q);
	if (NbElmt(*Q) == 1) {
		CreateEmpty(Q);
	} else {
		P = Head(*Q);
		Head(*Q) = Next(P);
		Next(P) = NULL;
	}
	Dealokasi(P);
}
/* Proses: Menghapus X pada bagian HEAD dari Q dan mendealokasi elemen HEAD */
/* Pada dasarnya operasi delete first */
/* I.S. Q tidak mungkin kosong */
/* F.S. X = Nilai elemen HEAD pd I.S., HEAD "mundur" */

void DelSpecific(PrioQueue * Q, int seqNumber) {
	// KAMUS
	address P, Prev;
	
	// ALGORITMA
	P = Head(*Q);
	while (P != NULL){
		if (Info(P).seqNum == seqNumber) {
			break;
		}
		Prev = P;
		P = Next(P);
	}
	
	if (P != NULL) {
		if (NbElmt(*Q) == 1) {
			CreateEmpty(Q);
		} else if (P == Head(*Q)){
			Head(*Q) = Next(P);
			Next(P) = NULL;
		} else {
			Next(Prev) = Next(P);
			Next(P) = NULL;
			Dealokasi(P);
		}
	} else {
		printf("Tidak terdapat sequence number yang sama pada PrioQueue\n");
	}
}

void printPrioQueue(PrioQueue Q) {
	address P;
	
	if (IsEmpty(Q)) {
		printf("PrioQueue kosong.\n"); fflush(stdout);
	} else {
		P = Head(Q);
		while (P != NULL) {
			printf("seqNum : %d sentTime : %ld\n",Info(P).seqNum,Info(P).sentTime); fflush(stdout);
			P = Next(P);
		}
	}	
}
