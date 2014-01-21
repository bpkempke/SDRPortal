/* Generate the inline BUTTERFLY macro calls for viterbi.c */

/* The two generator polynomials for the NASA Standard K=7 code */
#define	POLYA	0x4f
#define	POLYB	0x6d

extern char Partab[];

main()
{
	int e,i;

	for(i=0;i<32;i++){
		e = Partab[2*i & POLYA] << 1;
		e |= 1-Partab[2*i & POLYB];
		printf("BUTTERFLY(%d,%d)\n",i,e);
	}
}
