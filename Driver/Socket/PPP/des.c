/***********************************************************************
 *
 *	Copyright (c) Global PC 1998 -- All Rights Reserved
 *
 *			GLOBAL PC CONFIDENTIAL
 *
 * PROJECT:	  Socket
 * MODULE:	  PPP Driver
 * FILE:	  des.c
 *
 * AUTHOR:  	  Brian Chin: October 6, 1998
 *
 * ROUTINES:
 *	Name	  	    Description
 *	----	  	    -----------
 *	des_set_key
 *	des_ecb_encrypt
 *	des_set_odd_parity
 *
 * REVISION HISTORY:
 *	Date	  Name	    Description
 *	----	  ----	    -----------
 *	10/6/98	  brianc    Initial version
 *
 * DESCRIPTION:
 *	DES algorithms.
 *
 * 	$Id$
 *
 ***********************************************************************/

/* set_key.c */
/* Copyright (C) 1993 Eric Young - see README for more details */
/* set_key.c v 1.4 eay 24/9/91
 * 1.4 Speed up by 400% :-)
 * 1.3 added register declarations.
 * 1.2 unrolled make_key_sched a bit more
 * 1.1 added norm_expand_bits
 * 1.0 First working version
 */
#include "des.h"

/*
 * local defines
 */

/* des_locl.h */
/* Copyright (C) 1993 Eric Young - see README for more details */

#define ITERATIONS 16
#define HALF_ITERATIONS 8

#define c2l(c,l)	(l =((unsigned long)(*((c)++)))    , \
			 l|=((unsigned long)(*((c)++)))<< 8, \
			 l|=((unsigned long)(*((c)++)))<<16, \
			 l|=((unsigned long)(*((c)++)))<<24)

/* NOTE - c is not incremented as per c2l */
#define c2ln(c,l1,l2,n)	{ \
			c+=n; \
			l1=l2=0; \
			switch (n) { \
			case 8: l2 =((unsigned long)(*(--(c))))<<24; \
			case 7: l2|=((unsigned long)(*(--(c))))<<16; \
			case 6: l2|=((unsigned long)(*(--(c))))<< 8; \
			case 5: l2|=((unsigned long)(*(--(c))));     \
			case 4: l1 =((unsigned long)(*(--(c))))<<24; \
			case 3: l1|=((unsigned long)(*(--(c))))<<16; \
			case 2: l1|=((unsigned long)(*(--(c))))<< 8; \
			case 1: l1|=((unsigned long)(*(--(c))));     \
				} \
			}

#define l2c(l,c)	(*((c)++)=(unsigned char)(((l)    )&0xff), \
			 *((c)++)=(unsigned char)(((l)>> 8)&0xff), \
			 *((c)++)=(unsigned char)(((l)>>16)&0xff), \
			 *((c)++)=(unsigned char)(((l)>>24)&0xff))

/* replacements for htonl and ntohl since I have no idea what to do
 * when faced with machines with 8 byte longs. */
#define HDRSIZE 4

#define n2l(c,l)	(l =((unsigned long)(*((c)++)))<<24, \
			 l|=((unsigned long)(*((c)++)))<<16, \
			 l|=((unsigned long)(*((c)++)))<< 8, \
			 l|=((unsigned long)(*((c)++))))

#define l2n(l,c)	(*((c)++)=(unsigned char)(((l)>>24)&0xff), \
			 *((c)++)=(unsigned char)(((l)>>16)&0xff), \
			 *((c)++)=(unsigned char)(((l)>> 8)&0xff), \
			 *((c)++)=(unsigned char)(((l)    )&0xff))

/* NOTE - c is not incremented as per l2c */
#define l2cn(l1,l2,c,n)	{ \
			c+=n; \
			switch (n) { \
			case 8: *(--(c))=(unsigned char)(((l2)>>24)&0xff); \
			case 7: *(--(c))=(unsigned char)(((l2)>>16)&0xff); \
			case 6: *(--(c))=(unsigned char)(((l2)>> 8)&0xff); \
			case 5: *(--(c))=(unsigned char)(((l2)    )&0xff); \
			case 4: *(--(c))=(unsigned char)(((l1)>>24)&0xff); \
			case 3: *(--(c))=(unsigned char)(((l1)>>16)&0xff); \
			case 2: *(--(c))=(unsigned char)(((l1)>> 8)&0xff); \
			case 1: *(--(c))=(unsigned char)(((l1)    )&0xff); \
				} \
			}

/* The changes to this macro may help or hinder, depending on the
 * compiler and the achitecture.  gcc2 always seems to do well :-).
 * Inspired by Dana How <how@isl.stanford.edu>
 * DO NOT use the alternative version on machines with 8 byte longs. */
#ifdef ALT_ECB
#define D_ENCRYPT(L,R,S) \
	u=((R^s[S  ])<<2);	\
	t= R^s[S+1]; \
	t=((t>>2)+(t<<30)); \
	L^= \
	*(unsigned long *)(des_SP+0x0100+((t    )&0xfc))+ \
	*(unsigned long *)(des_SP+0x0300+((t>> 8)&0xfc))+ \
	*(unsigned long *)(des_SP+0x0500+((t>>16)&0xfc))+ \
	*(unsigned long *)(des_SP+0x0700+((t>>24)&0xfc))+ \
	*(unsigned long *)(des_SP+       ((u    )&0xfc))+ \
  	*(unsigned long *)(des_SP+0x0200+((u>> 8)&0xfc))+ \
  	*(unsigned long *)(des_SP+0x0400+((u>>16)&0xfc))+ \
 	*(unsigned long *)(des_SP+0x0600+((u>>24)&0xfc));
#else /* original version */
#define D_ENCRYPT(L,R,S)	\
	U.l=R^s[S+1]; \
	T.s[0]=((U.s[0]>>4)|(U.s[1]<<12))&0x3f3f; \
	T.s[1]=((U.s[1]>>4)|(U.s[0]<<12))&0x3f3f; \
	U.l=(R^s[S  ])&0x3f3f3f3f; \
	L^=	des_SPtrans[1][(T.c[0])]| \
		des_SPtrans[3][(T.c[1])]| \
		des_SPtrans[5][(T.c[2])]| \
		des_SPtrans[7][(T.c[3])]| \
		des_SPtrans[0][(U.c[0])]| \
		des_SPtrans[2][(U.c[1])]| \
		des_SPtrans[4][(U.c[2])]| \
		des_SPtrans[6][(U.c[3])];
#endif

	/* IP and FP
	 * The problem is more of a geometric problem that random bit fiddling.
	 0  1  2  3  4  5  6  7      62 54 46 38 30 22 14  6
	 8  9 10 11 12 13 14 15      60 52 44 36 28 20 12  4
	16 17 18 19 20 21 22 23      58 50 42 34 26 18 10  2
	24 25 26 27 28 29 30 31  to  56 48 40 32 24 16  8  0

	32 33 34 35 36 37 38 39      63 55 47 39 31 23 15  7
	40 41 42 43 44 45 46 47      61 53 45 37 29 21 13  5
	48 49 50 51 52 53 54 55      59 51 43 35 27 19 11  3
	56 57 58 59 60 61 62 63      57 49 41 33 25 17  9  1

	The output has been subject to swaps of the form
	0 1 -> 3 1 but the odd and even bits have been put into
	2 3    2 0
	different words.  The main trick is to remember that
	t=((l>>size)^r)&(mask);
	r^=t;
	l^=(t<<size);
	can be used to swap and move bits between words.

	So l =  0  1  2  3  r = 16 17 18 19
	        4  5  6  7      20 21 22 23
	        8  9 10 11      24 25 26 27
	       12 13 14 15      28 29 30 31
	becomes (for size == 2 and mask == 0x3333)
	   t =   2^16  3^17 -- --   l =  0  1 16 17  r =  2  3 18 19
		 6^20  7^21 -- --        4  5 20 21       6  7 22 23
		10^24 11^25 -- --        8  9 24 25      10 11 24 25
		14^28 15^29 -- --       12 13 28 29      14 15 28 29

	Thanks for hints from Richard Outerbridge - he told me IP&FP
	could be done in 15 xor, 10 shifts and 5 ands.
	When I finally started to think of the problem in 2D
	I first got ~42 operations without xors.  When I remembered
	how to use xors :-) I got it to its final state.
	*/
#define PERM_OP(a,b,t,n,m) ((t)=((((a)>>(n))^(b))&(m)),\
	(b)^=(t),\
	(a)^=((t)<<(n)))

#define IP(l,r,t) \
	PERM_OP(r,l,t, 4,0x0f0f0f0f); \
	PERM_OP(l,r,t,16,0x0000ffff); \
	PERM_OP(r,l,t, 2,0x33333333); \
	PERM_OP(l,r,t, 8,0x00ff00ff); \
	PERM_OP(r,l,t, 1,0x55555555);

#define FP(l,r,t) \
	PERM_OP(l,r,t, 1,0x55555555); \
	PERM_OP(r,l,t, 8,0x00ff00ff); \
	PERM_OP(l,r,t, 2,0x33333333); \
	PERM_OP(r,l,t,16,0x0000ffff); \
	PERM_OP(l,r,t, 4,0x0f0f0f0f);


/* podd.h */
/* Copyright (C) 1993 Eric Young - see README for more details */
static unsigned char odd_parity[256]={
  1,  1,  2,  2,  4,  4,  7,  7,  8,  8, 11, 11, 13, 13, 14, 14,
 16, 16, 19, 19, 21, 21, 22, 22, 25, 25, 26, 26, 28, 28, 31, 31,
 32, 32, 35, 35, 37, 37, 38, 38, 41, 41, 42, 42, 44, 44, 47, 47,
 49, 49, 50, 50, 52, 52, 55, 55, 56, 56, 59, 59, 61, 61, 62, 62,
 64, 64, 67, 67, 69, 69, 70, 70, 73, 73, 74, 74, 76, 76, 79, 79,
 81, 81, 82, 82, 84, 84, 87, 87, 88, 88, 91, 91, 93, 93, 94, 94,
 97, 97, 98, 98,100,100,103,103,104,104,107,107,109,109,110,110,
112,112,115,115,117,117,118,118,121,121,122,122,124,124,127,127,
128,128,131,131,133,133,134,134,137,137,138,138,140,140,143,143,
145,145,146,146,148,148,151,151,152,152,155,155,157,157,158,158,
161,161,162,162,164,164,167,167,168,168,171,171,173,173,174,174,
176,176,179,179,181,181,182,182,185,185,186,186,188,188,191,191,
193,193,194,194,196,196,199,199,200,200,203,203,205,205,206,206,
208,208,211,211,213,213,214,214,217,217,218,218,220,220,223,223,
224,224,227,227,229,229,230,230,233,233,234,234,236,236,239,239,
241,241,242,242,244,244,247,247,248,248,251,251,253,253,254,254};


/* sk.h */
/* Copyright (C) 1993 Eric Young - see README for more details */
static unsigned long des_skb[8][64]={
/* for C bits (numbered as per FIPS 46) 1 2 3 4 5 6 */
0x00000000,0x00000010,0x20000000,0x20000010,
0x00010000,0x00010010,0x20010000,0x20010010,
0x00000800,0x00000810,0x20000800,0x20000810,
0x00010800,0x00010810,0x20010800,0x20010810,
0x00000020,0x00000030,0x20000020,0x20000030,
0x00010020,0x00010030,0x20010020,0x20010030,
0x00000820,0x00000830,0x20000820,0x20000830,
0x00010820,0x00010830,0x20010820,0x20010830,
0x00080000,0x00080010,0x20080000,0x20080010,
0x00090000,0x00090010,0x20090000,0x20090010,
0x00080800,0x00080810,0x20080800,0x20080810,
0x00090800,0x00090810,0x20090800,0x20090810,
0x00080020,0x00080030,0x20080020,0x20080030,
0x00090020,0x00090030,0x20090020,0x20090030,
0x00080820,0x00080830,0x20080820,0x20080830,
0x00090820,0x00090830,0x20090820,0x20090830,
/* for C bits (numbered as per FIPS 46) 7 8 10 11 12 13 */
0x00000000,0x02000000,0x00002000,0x02002000,
0x00200000,0x02200000,0x00202000,0x02202000,
0x00000004,0x02000004,0x00002004,0x02002004,
0x00200004,0x02200004,0x00202004,0x02202004,
0x00000400,0x02000400,0x00002400,0x02002400,
0x00200400,0x02200400,0x00202400,0x02202400,
0x00000404,0x02000404,0x00002404,0x02002404,
0x00200404,0x02200404,0x00202404,0x02202404,
0x10000000,0x12000000,0x10002000,0x12002000,
0x10200000,0x12200000,0x10202000,0x12202000,
0x10000004,0x12000004,0x10002004,0x12002004,
0x10200004,0x12200004,0x10202004,0x12202004,
0x10000400,0x12000400,0x10002400,0x12002400,
0x10200400,0x12200400,0x10202400,0x12202400,
0x10000404,0x12000404,0x10002404,0x12002404,
0x10200404,0x12200404,0x10202404,0x12202404,
/* for C bits (numbered as per FIPS 46) 14 15 16 17 19 20 */
0x00000000,0x00000001,0x00040000,0x00040001,
0x01000000,0x01000001,0x01040000,0x01040001,
0x00000002,0x00000003,0x00040002,0x00040003,
0x01000002,0x01000003,0x01040002,0x01040003,
0x00000200,0x00000201,0x00040200,0x00040201,
0x01000200,0x01000201,0x01040200,0x01040201,
0x00000202,0x00000203,0x00040202,0x00040203,
0x01000202,0x01000203,0x01040202,0x01040203,
0x08000000,0x08000001,0x08040000,0x08040001,
0x09000000,0x09000001,0x09040000,0x09040001,
0x08000002,0x08000003,0x08040002,0x08040003,
0x09000002,0x09000003,0x09040002,0x09040003,
0x08000200,0x08000201,0x08040200,0x08040201,
0x09000200,0x09000201,0x09040200,0x09040201,
0x08000202,0x08000203,0x08040202,0x08040203,
0x09000202,0x09000203,0x09040202,0x09040203,
/* for C bits (numbered as per FIPS 46) 21 23 24 26 27 28 */
0x00000000,0x00100000,0x00000100,0x00100100,
0x00000008,0x00100008,0x00000108,0x00100108,
0x00001000,0x00101000,0x00001100,0x00101100,
0x00001008,0x00101008,0x00001108,0x00101108,
0x04000000,0x04100000,0x04000100,0x04100100,
0x04000008,0x04100008,0x04000108,0x04100108,
0x04001000,0x04101000,0x04001100,0x04101100,
0x04001008,0x04101008,0x04001108,0x04101108,
0x00020000,0x00120000,0x00020100,0x00120100,
0x00020008,0x00120008,0x00020108,0x00120108,
0x00021000,0x00121000,0x00021100,0x00121100,
0x00021008,0x00121008,0x00021108,0x00121108,
0x04020000,0x04120000,0x04020100,0x04120100,
0x04020008,0x04120008,0x04020108,0x04120108,
0x04021000,0x04121000,0x04021100,0x04121100,
0x04021008,0x04121008,0x04021108,0x04121108,
/* for D bits (numbered as per FIPS 46) 1 2 3 4 5 6 */
0x00000000,0x10000000,0x00010000,0x10010000,
0x00000004,0x10000004,0x00010004,0x10010004,
0x20000000,0x30000000,0x20010000,0x30010000,
0x20000004,0x30000004,0x20010004,0x30010004,
0x00100000,0x10100000,0x00110000,0x10110000,
0x00100004,0x10100004,0x00110004,0x10110004,
0x20100000,0x30100000,0x20110000,0x30110000,
0x20100004,0x30100004,0x20110004,0x30110004,
0x00001000,0x10001000,0x00011000,0x10011000,
0x00001004,0x10001004,0x00011004,0x10011004,
0x20001000,0x30001000,0x20011000,0x30011000,
0x20001004,0x30001004,0x20011004,0x30011004,
0x00101000,0x10101000,0x00111000,0x10111000,
0x00101004,0x10101004,0x00111004,0x10111004,
0x20101000,0x30101000,0x20111000,0x30111000,
0x20101004,0x30101004,0x20111004,0x30111004,
/* for D bits (numbered as per FIPS 46) 8 9 11 12 13 14 */
0x00000000,0x08000000,0x00000008,0x08000008,
0x00000400,0x08000400,0x00000408,0x08000408,
0x00020000,0x08020000,0x00020008,0x08020008,
0x00020400,0x08020400,0x00020408,0x08020408,
0x00000001,0x08000001,0x00000009,0x08000009,
0x00000401,0x08000401,0x00000409,0x08000409,
0x00020001,0x08020001,0x00020009,0x08020009,
0x00020401,0x08020401,0x00020409,0x08020409,
0x02000000,0x0A000000,0x02000008,0x0A000008,
0x02000400,0x0A000400,0x02000408,0x0A000408,
0x02020000,0x0A020000,0x02020008,0x0A020008,
0x02020400,0x0A020400,0x02020408,0x0A020408,
0x02000001,0x0A000001,0x02000009,0x0A000009,
0x02000401,0x0A000401,0x02000409,0x0A000409,
0x02020001,0x0A020001,0x02020009,0x0A020009,
0x02020401,0x0A020401,0x02020409,0x0A020409,
/* for D bits (numbered as per FIPS 46) 16 17 18 19 20 21 */
0x00000000,0x00000100,0x00080000,0x00080100,
0x01000000,0x01000100,0x01080000,0x01080100,
0x00000010,0x00000110,0x00080010,0x00080110,
0x01000010,0x01000110,0x01080010,0x01080110,
0x00200000,0x00200100,0x00280000,0x00280100,
0x01200000,0x01200100,0x01280000,0x01280100,
0x00200010,0x00200110,0x00280010,0x00280110,
0x01200010,0x01200110,0x01280010,0x01280110,
0x00000200,0x00000300,0x00080200,0x00080300,
0x01000200,0x01000300,0x01080200,0x01080300,
0x00000210,0x00000310,0x00080210,0x00080310,
0x01000210,0x01000310,0x01080210,0x01080310,
0x00200200,0x00200300,0x00280200,0x00280300,
0x01200200,0x01200300,0x01280200,0x01280300,
0x00200210,0x00200310,0x00280210,0x00280310,
0x01200210,0x01200310,0x01280210,0x01280310,
/* for D bits (numbered as per FIPS 46) 22 23 24 25 27 28 */
0x00000000,0x04000000,0x00040000,0x04040000,
0x00000002,0x04000002,0x00040002,0x04040002,
0x00002000,0x04002000,0x00042000,0x04042000,
0x00002002,0x04002002,0x00042002,0x04042002,
0x00000020,0x04000020,0x00040020,0x04040020,
0x00000022,0x04000022,0x00040022,0x04040022,
0x00002020,0x04002020,0x00042020,0x04042020,
0x00002022,0x04002022,0x00042022,0x04042022,
0x00000800,0x04000800,0x00040800,0x04040800,
0x00000802,0x04000802,0x00040802,0x04040802,
0x00002800,0x04002800,0x00042800,0x04042800,
0x00002802,0x04002802,0x00042802,0x04042802,
0x00000820,0x04000820,0x00040820,0x04040820,
0x00000822,0x04000822,0x00040822,0x04040822,
0x00002820,0x04002820,0x00042820,0x04042820,
0x00002822,0x04002822,0x00042822,0x04042822,
};


/* spr.h */
/* Copyright (C) 1993 Eric Young - see README for more details */
static unsigned long des_SPtrans[8][64]={
/* nibble 0 */
0x00820200, 0x00020000, 0x80800000, 0x80820200,
0x00800000, 0x80020200, 0x80020000, 0x80800000,
0x80020200, 0x00820200, 0x00820000, 0x80000200,
0x80800200, 0x00800000, 0x00000000, 0x80020000,
0x00020000, 0x80000000, 0x00800200, 0x00020200,
0x80820200, 0x00820000, 0x80000200, 0x00800200,
0x80000000, 0x00000200, 0x00020200, 0x80820000,
0x00000200, 0x80800200, 0x80820000, 0x00000000,
0x00000000, 0x80820200, 0x00800200, 0x80020000,
0x00820200, 0x00020000, 0x80000200, 0x00800200,
0x80820000, 0x00000200, 0x00020200, 0x80800000,
0x80020200, 0x80000000, 0x80800000, 0x00820000,
0x80820200, 0x00020200, 0x00820000, 0x80800200,
0x00800000, 0x80000200, 0x80020000, 0x00000000,
0x00020000, 0x00800000, 0x80800200, 0x00820200,
0x80000000, 0x80820000, 0x00000200, 0x80020200,

/* nibble 1 */
0x10042004, 0x00000000, 0x00042000, 0x10040000,
0x10000004, 0x00002004, 0x10002000, 0x00042000,
0x00002000, 0x10040004, 0x00000004, 0x10002000,
0x00040004, 0x10042000, 0x10040000, 0x00000004,
0x00040000, 0x10002004, 0x10040004, 0x00002000,
0x00042004, 0x10000000, 0x00000000, 0x00040004,
0x10002004, 0x00042004, 0x10042000, 0x10000004,
0x10000000, 0x00040000, 0x00002004, 0x10042004,
0x00040004, 0x10042000, 0x10002000, 0x00042004,
0x10042004, 0x00040004, 0x10000004, 0x00000000,
0x10000000, 0x00002004, 0x00040000, 0x10040004,
0x00002000, 0x10000000, 0x00042004, 0x10002004,
0x10042000, 0x00002000, 0x00000000, 0x10000004,
0x00000004, 0x10042004, 0x00042000, 0x10040000,
0x10040004, 0x00040000, 0x00002004, 0x10002000,
0x10002004, 0x00000004, 0x10040000, 0x00042000,

/* nibble 2 */
0x41000000, 0x01010040, 0x00000040, 0x41000040,
0x40010000, 0x01000000, 0x41000040, 0x00010040,
0x01000040, 0x00010000, 0x01010000, 0x40000000,
0x41010040, 0x40000040, 0x40000000, 0x41010000,
0x00000000, 0x40010000, 0x01010040, 0x00000040,
0x40000040, 0x41010040, 0x00010000, 0x41000000,
0x41010000, 0x01000040, 0x40010040, 0x01010000,
0x00010040, 0x00000000, 0x01000000, 0x40010040,
0x01010040, 0x00000040, 0x40000000, 0x00010000,
0x40000040, 0x40010000, 0x01010000, 0x41000040,
0x00000000, 0x01010040, 0x00010040, 0x41010000,
0x40010000, 0x01000000, 0x41010040, 0x40000000,
0x40010040, 0x41000000, 0x01000000, 0x41010040,
0x00010000, 0x01000040, 0x41000040, 0x00010040,
0x01000040, 0x00000000, 0x41010000, 0x40000040,
0x41000000, 0x40010040, 0x00000040, 0x01010000,

/* nibble 3 */
0x00100402, 0x04000400, 0x00000002, 0x04100402,
0x00000000, 0x04100000, 0x04000402, 0x00100002,
0x04100400, 0x04000002, 0x04000000, 0x00000402,
0x04000002, 0x00100402, 0x00100000, 0x04000000,
0x04100002, 0x00100400, 0x00000400, 0x00000002,
0x00100400, 0x04000402, 0x04100000, 0x00000400,
0x00000402, 0x00000000, 0x00100002, 0x04100400,
0x04000400, 0x04100002, 0x04100402, 0x00100000,
0x04100002, 0x00000402, 0x00100000, 0x04000002,
0x00100400, 0x04000400, 0x00000002, 0x04100000,
0x04000402, 0x00000000, 0x00000400, 0x00100002,
0x00000000, 0x04100002, 0x04100400, 0x00000400,
0x04000000, 0x04100402, 0x00100402, 0x00100000,
0x04100402, 0x00000002, 0x04000400, 0x00100402,
0x00100002, 0x00100400, 0x04100000, 0x04000402,
0x00000402, 0x04000000, 0x04000002, 0x04100400,

/* nibble 4 */
0x02000000, 0x00004000, 0x00000100, 0x02004108,
0x02004008, 0x02000100, 0x00004108, 0x02004000,
0x00004000, 0x00000008, 0x02000008, 0x00004100,
0x02000108, 0x02004008, 0x02004100, 0x00000000,
0x00004100, 0x02000000, 0x00004008, 0x00000108,
0x02000100, 0x00004108, 0x00000000, 0x02000008,
0x00000008, 0x02000108, 0x02004108, 0x00004008,
0x02004000, 0x00000100, 0x00000108, 0x02004100,
0x02004100, 0x02000108, 0x00004008, 0x02004000,
0x00004000, 0x00000008, 0x02000008, 0x02000100,
0x02000000, 0x00004100, 0x02004108, 0x00000000,
0x00004108, 0x02000000, 0x00000100, 0x00004008,
0x02000108, 0x00000100, 0x00000000, 0x02004108,
0x02004008, 0x02004100, 0x00000108, 0x00004000,
0x00004100, 0x02004008, 0x02000100, 0x00000108,
0x00000008, 0x00004108, 0x02004000, 0x02000008,

/* nibble 5 */
0x20000010, 0x00080010, 0x00000000, 0x20080800,
0x00080010, 0x00000800, 0x20000810, 0x00080000,
0x00000810, 0x20080810, 0x00080800, 0x20000000,
0x20000800, 0x20000010, 0x20080000, 0x00080810,
0x00080000, 0x20000810, 0x20080010, 0x00000000,
0x00000800, 0x00000010, 0x20080800, 0x20080010,
0x20080810, 0x20080000, 0x20000000, 0x00000810,
0x00000010, 0x00080800, 0x00080810, 0x20000800,
0x00000810, 0x20000000, 0x20000800, 0x00080810,
0x20080800, 0x00080010, 0x00000000, 0x20000800,
0x20000000, 0x00000800, 0x20080010, 0x00080000,
0x00080010, 0x20080810, 0x00080800, 0x00000010,
0x20080810, 0x00080800, 0x00080000, 0x20000810,
0x20000010, 0x20080000, 0x00080810, 0x00000000,
0x00000800, 0x20000010, 0x20000810, 0x20080800,
0x20080000, 0x00000810, 0x00000010, 0x20080010,

/* nibble 6 */
0x00001000, 0x00000080, 0x00400080, 0x00400001,
0x00401081, 0x00001001, 0x00001080, 0x00000000,
0x00400000, 0x00400081, 0x00000081, 0x00401000,
0x00000001, 0x00401080, 0x00401000, 0x00000081,
0x00400081, 0x00001000, 0x00001001, 0x00401081,
0x00000000, 0x00400080, 0x00400001, 0x00001080,
0x00401001, 0x00001081, 0x00401080, 0x00000001,
0x00001081, 0x00401001, 0x00000080, 0x00400000,
0x00001081, 0x00401000, 0x00401001, 0x00000081,
0x00001000, 0x00000080, 0x00400000, 0x00401001,
0x00400081, 0x00001081, 0x00001080, 0x00000000,
0x00000080, 0x00400001, 0x00000001, 0x00400080,
0x00000000, 0x00400081, 0x00400080, 0x00001080,
0x00000081, 0x00001000, 0x00401081, 0x00400000,
0x00401080, 0x00000001, 0x00001001, 0x00401081,
0x00400001, 0x00401080, 0x00401000, 0x00001001,

/* nibble 7 */
0x08200020, 0x08208000, 0x00008020, 0x00000000,
0x08008000, 0x00200020, 0x08200000, 0x08208020,
0x00000020, 0x08000000, 0x00208000, 0x00008020,
0x00208020, 0x08008020, 0x08000020, 0x08200000,
0x00008000, 0x00208020, 0x00200020, 0x08008000,
0x08208020, 0x08000020, 0x00000000, 0x00208000,
0x08000000, 0x00200000, 0x08008020, 0x08200020,
0x00200000, 0x00008000, 0x08208000, 0x00000020,
0x00200000, 0x00008000, 0x08000020, 0x08208020,
0x00008020, 0x08000000, 0x00000000, 0x00208000,
0x08200020, 0x08008020, 0x08008000, 0x00200020,
0x08208000, 0x00000020, 0x00200020, 0x08008000,
0x08208020, 0x00200000, 0x08200000, 0x08000020,
0x00208000, 0x00008020, 0x08008020, 0x08200000,
0x00000020, 0x08208000, 0x00208020, 0x00000000,
0x08000000, 0x08200020, 0x00008000, 0x00208020};


int des_check_key=0;

void des_set_odd_parity(des_cblock *key)
	{
	int i;

	for (i=0; i<DES_KEY_SZ; i++)
		(*key)[i]=odd_parity[(*key)[i]];
	}

static int check_parity(des_cblock *key)
	{
	int i;

	for (i=0; i<DES_KEY_SZ; i++)
		{
		if ((*key)[i] != odd_parity[(*key)[i]])
			return(0);
		}
	return(1);
	}

/* Weak and semi week keys as take from
 * %A D.W. Davies
 * %A W.L. Price
 * %T Security for Computer Networks
 * %I John Wiley & Sons
 * %D 1984
 * Many thanks to smb@ulysses.att.com (Steven Bellovin) for the reference
 * (and actual cblock values).
 */
#define NUM_WEAK_KEY	16
static des_cblock weak_keys[NUM_WEAK_KEY]={
	/* weak keys */
	0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
	0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,
	0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,
	0xE0,0xE0,0xE0,0xE0,0xE0,0xE0,0xE0,0xE0,
	/* semi-weak keys */
	0x01,0xFE,0x01,0xFE,0x01,0xFE,0x01,0xFE,
	0xFE,0x01,0xFE,0x01,0xFE,0x01,0xFE,0x01,
	0x1F,0xE0,0x1F,0xE0,0x0E,0xF1,0x0E,0xF1,
	0xE0,0x1F,0xE0,0x1F,0xF1,0x0E,0xF1,0x0E,
	0x01,0xE0,0x01,0xE0,0x01,0xF1,0x01,0xF1,
	0xE0,0x01,0xE0,0x01,0xF1,0x01,0xF1,0x01,
	0x1F,0xFE,0x1F,0xFE,0x0E,0xFE,0x0E,0xFE,
	0xFE,0x1F,0xFE,0x1F,0xFE,0x0E,0xFE,0x0E,
	0x01,0x1F,0x01,0x1F,0x01,0x0E,0x01,0x0E,
	0x1F,0x01,0x1F,0x01,0x0E,0x01,0x0E,0x01,
	0xE0,0xFE,0xE0,0xFE,0xF1,0xFE,0xF1,0xFE,
	0xFE,0xE0,0xFE,0xE0,0xFE,0xF1,0xFE,0xF1};

int des_is_weak_key(des_cblock *key)
	{
	int i;

	for (i=0; i<NUM_WEAK_KEY; i++)
		/* Added == 0 to comparision, I obviously don't run
		 * this section very often :-(, thanks to
		 * engineering@MorningStar.Com for the fix
		 * eay 93/06/29 */
		if (memcmp(weak_keys[i],key,sizeof(key)) == 0) return(1);
	return(0);
	}

/* NOW DEFINED IN des.h
 * See ecb_encrypt.c for a pseudo description of these macros. 
 * #define PERM_OP(a,b,t,n,m) ((t)=((((a)>>(n))^(b))&(m)),\
 * 	(b)^=(t),\
 * 	(a)=((a)^((t)<<(n))))
 */

#define HPERM_OP(a,t,n,m) ((t)=((((a)<<(16-(n)))^(a))&(m)),\
	(a)=(a)^(t)^(t>>(16-(n))))

static char shifts2[16]={0,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0};

/* return 0 if key parity is odd (correct),
 * return -1 if key parity error,
 * return -2 if illegal weak key.
 */
int des_set_key(des_cblock *key, des_key_schedule schedule)
	{
	register unsigned long c,d,t,s;
	register unsigned char *in;
	register unsigned long *k;
	register int i;

	if (des_check_key)
		{
		if (!check_parity(key))
			return(-1);

		if (des_is_weak_key(key))
			return(-2);
		}

	k=(unsigned long *)schedule;
	in=(unsigned char *)key;

	c2l(in,c);
	c2l(in,d);

	/* do PC1 in 60 simple operations */ 
/*	PERM_OP(d,c,t,4,0x0f0f0f0f);
	HPERM_OP(c,t,-2, 0xcccc0000);
	HPERM_OP(c,t,-1, 0xaaaa0000);
	HPERM_OP(c,t, 8, 0x00ff0000);
	HPERM_OP(c,t,-1, 0xaaaa0000);
	HPERM_OP(d,t,-8, 0xff000000);
	HPERM_OP(d,t, 8, 0x00ff0000);
	HPERM_OP(d,t, 2, 0x33330000);
	d=((d&0x00aa00aa)<<7)|((d&0x55005500)>>7)|(d&0xaa55aa55);
	d=(d>>8)|((c&0xf0000000)>>4);
	c&=0x0fffffff; */

	/* I now do it in 47 simple operations :-)
	 * Thanks to John Fletcher (john_fletcher@lccmail.ocf.llnl.gov)
	 * for the inspiration. :-) */
	PERM_OP (d,c,t,4,0x0f0f0f0f);
	HPERM_OP(c,t,-2,0xcccc0000);
	HPERM_OP(d,t,-2,0xcccc0000);
	PERM_OP (d,c,t,1,0x55555555);
	PERM_OP (c,d,t,8,0x00ff00ff);
	PERM_OP (d,c,t,1,0x55555555);
	d=	(((d&0x000000ff)<<16)| (d&0x0000ff00)     |
		 ((d&0x00ff0000)>>16)|((c&0xf0000000)>>4));
	c&=0x0fffffff;

	for (i=0; i<ITERATIONS; i++)
		{
		if (shifts2[i])
			{ c=((c>>2)|(c<<26)); d=((d>>2)|(d<<26)); }
		else
			{ c=((c>>1)|(c<<27)); d=((d>>1)|(d<<27)); }
		c&=0x0fffffff;
		d&=0x0fffffff;
		/* could be a few less shifts but I am to lazy at this
		 * point in time to investigate */
		s=	des_skb[0][ (c    )&0x3f                ]|
			des_skb[1][((c>> 6)&0x03)|((c>> 7)&0x3c)]|
			des_skb[2][((c>>13)&0x0f)|((c>>14)&0x30)]|
			des_skb[3][((c>>20)&0x01)|((c>>21)&0x06) |
						  ((c>>22)&0x38)];
		t=	des_skb[4][ (d    )&0x3f                ]|
			des_skb[5][((d>> 7)&0x03)|((d>> 8)&0x3c)]|
			des_skb[6][ (d>>15)&0x3f                ]|
			des_skb[7][((d>>21)&0x0f)|((d>>22)&0x30)];

		/* table contained 0213 4657 */
		*(k++)=((t<<16)|(s&0x0000ffff))&0xffffffff;
		s=     ((s>>16)|(t&0xffff0000));
		
		s=(s<<4)|(s>>28);
		*(k++)=s&0xffffffff;
		}
	return(0);
	}

/* ecb_enc.c */
/* Copyright (C) 1993 Eric Young - see README for more details */

int des_ecb_encrypt(des_cblock *input, des_cblock *output, des_key_schedule ks, int encrypt)
	{
	register unsigned long l0,l1,t;
	register unsigned char *in,*out;
	unsigned long ll[2];

	in=(unsigned char *)input;
	out=(unsigned char *)output;
	c2l(in,l0);
	c2l(in,l1);
	ll[0]=l0;
	ll[1]=l1;
	des_encrypt(ll,ll,ks,encrypt);
	l0=ll[0];
	l1=ll[1];
	l2c(l0,out);
	l2c(l1,out);
	l0=l1=ll[0]=ll[1]=0;
	return(0);
	}

int des_encrypt(unsigned long *input, unsigned long *output,des_key_schedule ks, int encrypt)
	{
	register unsigned long l,r,t,u;
#ifdef ALT_ECB
	register unsigned char *des_SP=(unsigned char *)des_SPtrans;
#endif
	union fudge {
		unsigned long  l;
		unsigned short s[2];
		unsigned char  c[4];
		} U,T;
	register int i;
	register unsigned long *s;

	l=input[0];
	r=input[1];

	IP(l,r,t);
	/* Things have been modified so that the initial rotate is
	 * done outside the loop.  This required the
	 * des_SPtrans values in sp.h to be rotated 1 bit to the right.
	 * One perl script later and things have a 5% speed up on a sparc2.
	 * Thanks to Richard Outerbridge <71755.204@CompuServe.COM>
	 * for pointing this out. */
	t=(r<<1)|(r>>31);
	r=(l<<1)|(l>>31);
	l=t;

	/* clear the top bits on machines with 8byte longs */
	l&=0xffffffff;
	r&=0xffffffff;

	s=(unsigned long *)ks;
	/* I don't know if it is worth the effort of loop unrolling the
	 * inner loop */
	if (encrypt)
		{
		for (i=0; i<32; i+=4)
			{
			D_ENCRYPT(l,r,i+0); /*  1 */
			D_ENCRYPT(r,l,i+2); /*  2 */
			}
		}
	else
		{
		for (i=30; i>0; i-=4)
			{
			D_ENCRYPT(l,r,i-0); /* 16 */
			D_ENCRYPT(r,l,i-2); /* 15 */
			}
		}
	l=(l>>1)|(l<<31);
	r=(r>>1)|(r<<31);
	/* clear the top bits on machines with 8byte longs */
	l&=0xffffffff;
	r&=0xffffffff;

	FP(r,l,t);
	output[0]=l;
	output[1]=r;
	l=r=t=u=0;
	return(0);
	}
