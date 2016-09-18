/*
 * Crypt.c
 *
 *  Created on: 05/10/2012
 *  Created by: Abdulfatai Popoola
 *  Modified by: Wen Shen
 *  Copyright (c) 2012 Bitcoin Project Team
 */

#include "Crypt.h"
#include <stdio.h>
#include <stdlib.h>
#include <openssl/sha.h>
#include <openssl/ripemd.h>
#include <openssl/ssl.h>
#include <assert.h>

void Sha160(uint8_t * data, uint16_t len, uint8_t * output)
{
	assert(data != NULL );
	assert(len >=0);
	assert(output != NULL);

    SHA1(data, len, output);
}
void Sha256(uint8_t * data, uint16_t len, uint8_t * output)
{
	assert(data != NULL );
	assert(len >=0);
	assert(output != NULL);

	SHA256(data, len, output);
}

void Ripemd160(uint8_t * data, uint16_t len, uint8_t * output)
{
	assert(data != NULL);
	assert(len >=0);
	assert(output != NULL);

	RIPEMD160(data, len, output);
}

int ECDSAVerify(uint8_t * signature, uint8_t signatureLen, uint8_t * hash, const uint8_t * pubKey, uint8_t keyLen)
{
	assert(signature != NULL);
	assert(signatureLen >=0);
	assert(hash != NULL);
	assert(pubKey != NULL);
	assert(keyLen >=0);

	EC_KEY * key = EC_KEY_new_by_curve_name(NID_secp256k1);
	o2i_ECPublicKey(&key, &pubKey, keyLen);
	int res = ECDSA_verify(0, hash, 32, signature, signatureLen, key);
	EC_KEY_free(key);
	return res == 1;
}

