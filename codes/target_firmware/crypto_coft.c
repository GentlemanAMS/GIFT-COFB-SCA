#include "crypto_coft.h"
#include "hal.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "gift128.h"


#define FIRST_IMPLEMENTATION 0
#define ATTACK_NONCE 1



typedef unsigned char block[16];
typedef unsigned char half_block[8];

#define PLAINTEXT_LENGTH 16
#define HEADER_LENGTH 16

#define TAGBYTES 16

uint8_t attributes[HEADER_LENGTH]; 
uint8_t text[PLAINTEXT_LENGTH]; 
block enckey;
block nonce;


void set_key(uint8_t * k)
{
	for(uint8_t i=0; i < 16; i++){
		enckey[i] = k[i];
	}
}


void set_nonce(uint8_t * n)
{
	for(uint8_t i=0; i < 16; i++){
		nonce[i] = n[i];
	}
}

void set_header(uint8_t * header)
{
    for(uint8_t i =0; i < HEADER_LENGTH; i++){
        attributes[i] = header[i];
    }
}


void set_plaintext(uint8_t * plaintext)
{
    for(uint8_t i =0; i < PLAINTEXT_LENGTH; i++){
        text[i] = plaintext[i];
    }
}

void encryption_starts(uint8_t* ct, uint8_t ctlen)
{
	cofb_encrypt(ct, enckey, nonce, attributes, HEADER_LENGTH, text, PLAINTEXT_LENGTH);
}
















#define COFB_ENCRYPT 1
#define COFB_DECRYPT 0

void cofb_encrypt(unsigned char *c, unsigned char *k, unsigned char *n,
                 unsigned char *a, unsigned abytes,
                 unsigned char *p, unsigned pbytes) {
    cofb_crypt(c, k, n, a, abytes, p, pbytes, COFB_ENCRYPT);
}

static int cofb_crypt(unsigned char *out, unsigned char *k, unsigned char *n,
                     unsigned char *a, unsigned alen,
                     unsigned char *in, unsigned inlen, int encrypting) {

    unsigned i;
    unsigned emptyA, emptyM;

    if ( ! encrypting ) {
        if (inlen < TAGBYTES) return -1;
        inlen -= TAGBYTES;
    }

    if(alen==0)
        emptyA=1;
    else
        emptyA=0;

    if(inlen==0)
        emptyM=1;
    else
        emptyM=0;

    /*Mask-Gen*/
    block Y,input;
    half_block offset;
    /*nonce is 128-bit*/
    for(i=0;i<16;i++)
        input[i] = n[i];

    //Triggers Power Trace Capture
	trigger_high();
    giftb128(input,k,Y);
    trigger_low();
    //Ends Power Trace Capture

    for(i=0;i<8;i++)
        offset[i] = Y[i];


        /*Process AD*/
        /*non-empty A*/
    /*full blocks*/
    while(alen>16){
        /* X[i] = (A[i] + G(Y[i-1])) + offset */
        pho1(input,Y,a,16);
        /* offset = 2*offset */
        double_half_block(offset,offset);
        xor_topbar_block(input, input, offset);
        /* Y[i] = E(X[i]) */
        giftb128(input, k, Y);

        a = a + 16;
        alen -= 16;
    }

    /* last block */
    /* full block: offset = 3*offset */
    /* partial block: offset = 3^2*offset */
    triple_half_block(offset,offset);
    if((alen%16!=0)||(emptyA)){
        triple_half_block(offset,offset);
    }

    if(emptyM){
        /* empty M: offset = 3^2*offset */
        triple_half_block(offset,offset);
        triple_half_block(offset,offset);
    }

    /* X[i] = (pad(A[i]) + G(Y[i-1])) + offset */
    pho1(input,Y,a,alen);

    xor_topbar_block(input, input, offset);
    /* Y[a] = E(X[a]) */
    giftb128(input, k, Y);


    /* Process M */
    /* full blocks */
    while (inlen>16){
        double_half_block(offset,offset);
        /* C[i] = Y[i+a-1] + M[i]*/
        /* X[i] = M[i] + G(Y[i+a-1]) + offset */
        if(encrypting){
            pho(Y,in,input,out,16);
        }
        else{
            phoprime(Y,in,input,out,16);
        }

        xor_topbar_block(input,input,offset);
        /* Y[i] = E(X[i+a]) */
        giftb128(input, k, Y);

        in = in + 16;
        out = out + 16;
        inlen -= 16;
    }

    if(!emptyM){
        /* full block: offset = 3*offset */
        /* empty data / partial block: offset = 3^2*offset */
        triple_half_block(offset,offset);
        if(inlen%16!=0){
            triple_half_block(offset,offset);
        }
        /* last block */
        /* C[m] = Y[m+a-1] + M[m]*/
        /* X[a+m] = M[m] + G(Y[m+a-1]) + offset */
        if(encrypting){
            pho(Y,in,input,out,inlen);
            out += inlen;
        }
        else{
            phoprime(Y,in,input,out,inlen);
            in += inlen;
        }


        xor_topbar_block(input,input,offset);
        /* T = E(X[m+a]) */
        giftb128(input, k, Y);
    }

    if (encrypting) {
        memcpy(out, Y, TAGBYTES);
        return 0;
    } else
        return (memcmp(in,Y,TAGBYTES) ? -1 : 0);     /* Check for validity */
}

/* ------------------------------------------------------------------------- */

static void padding(block d, block s, unsigned no_of_bytes){
    unsigned i;
    block tmp;
    if(no_of_bytes==0){
        for(i=0; i<16; i++)
            tmp[i] = 0;
        tmp[0] = 0x80;
    }
    else if (no_of_bytes<16){
        for(i=0; i<no_of_bytes; i++)
            tmp[i] = s[i];
        tmp[no_of_bytes] = 0x80;
        for(i=no_of_bytes+1; i<16; i++)
            tmp[i] = 0;
    }
    else{
        for(i=0; i<16; i++)
            tmp[i] = s[i];
    }
    for(i=0; i<16; i++)
            d[i] = tmp[i];
}

/* ------------------------------------------------------------------------- */

static void xor_block(block d, block s1, block s2, unsigned no_of_bytes) {
    unsigned i;
    for (i=0; i<no_of_bytes; i++)
        d[i] = s1[i] ^ s2[i];
}

static void xor_topbar_block(block d, block s1, half_block s2) {
    unsigned i;
    block tmp;
    for (i=0; i<8; i++)
        tmp[i] = s1[i] ^ s2[i];
    for (i=8; i<16; i++)
        tmp[i] = s1[i];

    for(i=0; i<16; i++)
        d[i] = tmp[i];
}

/* ------------------------------------------------------------------------- */

static void double_half_block(half_block d, half_block s) {
    unsigned i;
    half_block tmp;
    /*x^{64} + x^4 + x^3 + x + 1*/
    for (i=0; i<7; i++)
        tmp[i] = (s[i] << 1) | (s[i+1] >> 7);
    tmp[7] = (s[7] << 1) ^ ((s[0] >> 7) * 27);

    for(i=0; i<8; i++)
        d[i] = tmp[i];
}

static void triple_half_block(half_block d, half_block s) {
    unsigned i;
    half_block tmp;
    double_half_block(tmp,s);
    for (i=0; i<8; i++)
        d[i] = s[i] ^ tmp[i];
}
/* ------------------------------------------------------------------------- */

static void G(block d, block s){
    unsigned i;
    block tmp;
    /*Y[1],Y[2] -> Y[2],Y[1]<<<1*/
    for(i=0; i<8; i++){
        tmp[i] = s[8+i];
    }
    for(i=0; i<7; i++){
        tmp[i+8] = s[i]<<1 | s[i+1]>>7;
    }
    tmp[7+8] = s[7]<<1 | s[0]>>7;

    for(i=0; i<16; i++)
        d[i] = tmp[i];
}

static void pho1(block d, block Y, block M, int no_of_bytes) {
    block tmpM;
    G(Y,Y);
    padding(tmpM,M,no_of_bytes);
    xor_block(d,Y,tmpM,16);
}

static void pho(block Y, block M, block X, block C, int no_of_bytes) {
    xor_block(C,Y,M,no_of_bytes);
    pho1(X,Y,M,no_of_bytes);
}

static void phoprime(block Y, block C, block X, block M, int no_of_bytes) {
    xor_block(M,Y,C,no_of_bytes);
    pho1(X,Y,M,no_of_bytes);

}

/* ------------------------------------------------------------------------- */


/* ------------------------------------------------------------------------- */

