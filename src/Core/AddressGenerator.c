/*
 * AddressGenerator.c
 *
 * Created on: Nov 20, 2012
 * Created by: Wen Shen
 * Copyright (c) 2012 MIBitcoin Project Team
 */

#include <stdio.h>
#include <ctype.h>
#include <openssl/ssl.h>
#include <openssl/ripemd.h>
#include <openssl/rand.h>
#include "../Object/Network/Address.h"
#include "../Object/ByteArray.h"
#include "../Object/Object.h"
#include "../Object/VersionChecksumBytes.h"
#include "../Constants.h"

void err(Error a, char * format, ...);

void err(Error a, char * format, ...) {

	va_list argptr;
	va_start(argptr, format);
	vfprintf(stderr, format, argptr);
	va_end(argptr);
	printf("\n");
}

void getLine(char * ptr);

void getLine(char * ptr) {

	int c;
	int len = 30;

	for (;;) {

		c = fgetc(stdin);

		if (c == EOF) {

			break;
		}

		if (--len == 0) {

			break;
		}

		*ptr = c;

		if (c == '\n') {

			break;
		}

		ptr++;
	}

	*ptr = 0;
}

int main() {

	printf("OpenSSL version: %s\n", OPENSSL_VERSION_TEXT);
	printf("Enter the number of keys: ");
	fflush(stdout);
	char stringMatch[31];
	getLine(stringMatch);
	unsigned long int i = strtol(stringMatch, NULL, 0);
	printf("Enter a string of text for the key (30 max): ");
	fflush(stdout);
	getLine(stringMatch);
	printf("Waiting for entropy... Move the cursor around...\n");
	fflush(stdout);
	char entropy[32];
	FILE * f = fopen("/dev/random", "r");

	if (fread(entropy, 32, 1, f) != 1) {

		printf("FAILURING GETTING ENTROPY!");
		return 1;
	}

	RAND_add(entropy, 32, 32);
	fclose(f);
	printf("Making %lu addresses for \"%s\"\n\n", i, stringMatch);
	EC_KEY * key = EC_KEY_new_by_curve_name(NID_secp256k1);
	uint8_t * pubKey = NULL;
	int pubSize = 0;
	uint8_t * privKey = NULL;
	int privSize = 0;
	uint8_t * shaHash = malloc(32);
	uint8_t * ripemdHash = malloc(20);
	unsigned int x;

	for (x = 0; x < i;) {

		if (!EC_KEY_generate_key(key)) {

			printf("GENERATE KEY FAIL\n");
			return 1;
		}

		int pubSizeNew = i2o_ECPublicKey(key, NULL);

		if (!pubSizeNew) {

			printf("PUB KEY TO DATA ZERO\n");
			return 1;
		}

		if (pubSizeNew != pubSize) {

			pubSize = pubSizeNew;
			pubKey = realloc(pubKey, pubSize);
		}
		uint8_t * pubKey2 = pubKey;

		if (i2o_ECPublicKey(key, &pubKey2) != pubSize) {

			printf("PUB KEY TO DATA FAIL\n");
			return 1;
		}

		SHA256(pubKey, pubSize, shaHash);
		RIPEMD160(shaHash, 32, ripemdHash);
		Address * address = createNewAddressFromRIPEMD160Hash(ripemdHash, 0, 0,
				err);
		ByteArray * string = getStringForVersionChecksumBytes(
				getVersionChecksumBytes(address));
		decrementReferenceCount(address);
		uint8_t offset = 1;
		size_t matchSize = strlen(stringMatch);
		uint8_t y;
		/* Get private key*/
		const BIGNUM * privKeyNum = EC_KEY_get0_private_key(key);

		if (!privKeyNum) {
			printf("PRIV KEY TO BN FAIL\n");
		}

		int privSizeNew = BN_num_bytes(privKeyNum);

		if (privSizeNew != privSize) {
			privSize = privSizeNew;
			privKey = realloc(privKey, privSize);
		}

		int res = BN_bn2bin(privKeyNum, privKey);

		if (res != privSize) {
			printf("PRIV KEY TO DATA FAIL\n");
		}

		/* Print data to stdout*/
		printf("Private key (hex): ");
		int i;

		for (i = 0; i < privSize; i++) {
			printf(" %.2X", privKey[i]);
		}

		printf("\nPublic key (hex): ");
		int j;

		for (j = 0; j < pubSize; j++) {
			printf(" %.2X", pubKey[j]);
		}

		printf("\nAddress (base-58): %s\n\n", getByteArrayData(string));
		x++; /*Move to next*/

		decrementReferenceCount(string);
	}

	free(shaHash);
	free(ripemdHash);
	EC_KEY_free(key);
	return 0;
}
