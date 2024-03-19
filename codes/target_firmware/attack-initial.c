#include "hal.h"
#include <stdint.h>
#include <stdlib.h>

#include "simpleserial.h"
#include "crypto_coft.h"

#define TAGBYTES 16
#define PLAINTEXT_LENGTH 16
#define HEADER_LENGTH 16


//Sets the Key
uint8_t get_key(uint8_t* k, uint8_t len)
{
	set_key(k);
	// simpleserial_put('r', 16, k);
	return 0x00;
}

//Sets the Nonce
uint8_t get_nonce(uint8_t* n, uint8_t len)
{
	set_nonce(n);
	// simpleserial_put('r', 16, n);
	return 0x00;
}

//Sets the Header
uint8_t get_header(uint8_t* h, uint8_t len)
{
    set_header(h);
    // simpleserial_put('r', HEADER_LENGTH, h);
	return 0x00;
}

//Sets the Plaintext
uint8_t get_plaintext(uint8_t* pt, uint8_t len)
{
    set_plaintext(pt);
    // simpleserial_put('r', PLAINTEXT_LENGTH, pt);
	return 0x00;
}

//Stores the Cipher Text
uint8_t ciphertext[PLAINTEXT_LENGTH + TAGBYTES]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

//Starts encryption. Calls the Gift-COFT function. Stores the cipher text + tag generated in the ciphertext variable
uint8_t encryption(uint8_t* e, uint8_t len)
{
	encryption_starts(ciphertext, (PLAINTEXT_LENGTH + TAGBYTES));
	simpleserial_put('r', (PLAINTEXT_LENGTH + TAGBYTES), ciphertext); //Returns Cipher text
	return 0x00;
}

//Return Ciphertext
uint8_t return_ciphertext(uint8_t* c, uint8_t len)
{
	simpleserial_put('r', (PLAINTEXT_LENGTH + TAGBYTES), ciphertext);
	return 0x00;
}


uint8_t reset(uint8_t* x, uint8_t len)
{
	return 0x00;
}


int main(void)
{
    platform_init();
	init_uart();
	trigger_setup();	

	simpleserial_init();

    //Adding commands
	simpleserial_addcmd('k', 16, get_key);
	simpleserial_addcmd('n', 16, get_nonce);
	simpleserial_addcmd('h', HEADER_LENGTH, get_header);
	simpleserial_addcmd('p', PLAINTEXT_LENGTH, get_plaintext);

	simpleserial_addcmd('e', 1, encryption);
	simpleserial_addcmd('c', 1, return_ciphertext);

	simpleserial_addcmd('x', 0, reset);

	while(1)
		simpleserial_get();
}
