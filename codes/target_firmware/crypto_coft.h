#ifndef CRYTPO_COFTB
#define CRYTPO_COFTB

#include "crypto_coft.h"
#include "hal.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "gift128.h"


typedef unsigned char block[16];
typedef unsigned char half_block[8];


#define PLAINTEXT_LENGTH 16
#define HEADER_LENGTH 16

void set_key(uint8_t * key);
void set_nonce(uint8_t * n);
void set_header(uint8_t * header);
void set_plaintext(uint8_t * plaintext);
void encryption_starts(uint8_t* ct, uint8_t ctlen);




static void padding(block d, block s, unsigned no_of_bytes);
static void xor_block(block d, block s1, block s2, unsigned no_of_bytes);
static void xor_topbar_block(block d, block s1, half_block s2);
static void double_half_block(half_block d, half_block s);
static void triple_half_block(half_block d, half_block s);
static void G(block d, block s);
static void pho1(block d, block Y, block M, int no_of_bytes);
static void pho(block Y, block M, block X, block C, int no_of_bytes);
static void phoprime(block Y, block C, block X, block M, int no_of_bytes);

static int cofb_crypt(unsigned char *out, unsigned char *k, unsigned char *n,
                     unsigned char *a, unsigned alen,
                     unsigned char *in, unsigned inlen, int encrypting);

void cofb_encrypt(unsigned char *c, unsigned char *k, unsigned char *n,
                 unsigned char *a, unsigned abytes,
                 unsigned char *p, unsigned pbytes);

#endif