/*
 * Base58.h
 *
 *  Created on: 03/10/2012
 *  Created by: Hayk Baluyan
 *  Copyright (c) 2012 Bitcoin Project Team
 */

#include "Base58.h"
#include "../BigInt/BigInt.h"
#include <stdint.h>

BigInt decodeBase58(char * str) {


	BigInt bi;
	bi.data = malloc(1);

	if (! bi.data) {

		bi.length = 0;
		return bi;
	}

	bi.data[0] = 0;
	bi.length = 1;

	uint8_t temp[189];
	uint8_t x;

	for (x = strlen(str) - 1;; x--) {

		uint8_t alphaIndex = str[x];

		if (alphaIndex != 49) {

			if (str[x] < 58) {

				alphaIndex -= 49;
			} else if (str[x] < 73) {

				alphaIndex -= 56;
			} else if (str[x] < 79) {

				alphaIndex -= 57;
			} else if (str[x] < 91) {

				alphaIndex -= 58;
			} else if (str[x] < 108) {

				alphaIndex -= 64;
			} else {

				alphaIndex -= 65;
			}

			BigInt bi2;
			bi2.data = malloc (strlen(str));

			BigIntFromPowUInt8 (&bi2, 58, strlen (str) - 1 - x);
				memset(temp, 0, bi2.length + 1);
			BigIntEqualsMultiplicationByUInt8(&bi2, alphaIndex, temp);



			BigIntEqualsAdditionByBigInt(&bi, &bi2);

			free(bi2.data);
		}
		if (!x) {

			break;
		}
	}

	uint8_t zeros = 0;
	for (x = 0; x < strlen(str); x++) {

		if (str[x] == '1') {

			zeros++;
		}
		else {

			break;
		}
	}

	if (zeros) {

		bi.length += zeros;
		uint8_t * temp = realloc(bi.data, bi.length);

		if (! temp) {

			free(bi.data);
			bi.length = 0;
			return bi;
		}

		bi.data = temp;
		memset(bi.data + bi.length - zeros, 0, zeros);
	}
	return bi;
}

/*Returns null if the checksum is invalid*/
uint8_t * verifyAndRemoveCheckSum(uint8_t* data) {

	int l;
	uint8_t* result = malloc(21);
	result = data;
	uint8_t* givenCheckSum = malloc(4);

	memcpy(givenCheckSum, data + 21, 4);
	uint8_t* correctCheckSum = malloc(4);
	correctCheckSum = getCheckSum(data);

	for(l = 0; l < 4; l++) {

		if( givenCheckSum[l] != correctCheckSum[l] ) {

			return NULL;
		}
	}
	return result;
}

BigInt decodeBase58Checked(char * str,void (*onErrorReceived)(Error error,char *,...)) {

	BigInt bi = decodeBase58(str);

	/*printf("\nString: %s\nDecoded String: ", str);
	printf("\n");
	int p;
	for(p=0; p<bi.length; p++)
		printf("%u ", bi.data[p]);*/


	if (bi.length < 4) {

		onErrorReceived(ERROR_BASE58_DECODE_CHECK_TOO_SHORT,"The string passed into DecodeBase58Checked decoded into data that was too short or there was a memory failure.");
		bi.length = 0;
		free(bi.data);
		bi.data = NULL;
		return bi;
	}

	uint8_t * reversed = malloc(bi.length - 4);

	if (!reversed) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,"Can! allocate %i bytes of memory in decodeBase58Checked",bi.length-4);
		bi.length = 0;
		bi.data = NULL;
		return bi;
	}

	uint8_t x;


	for (x = 4; x < bi.length; x++) {

		reversed[bi.length - 1 -x ] = bi.data[x];
	}




	uint8_t checksum[32];
	uint8_t checksum2[32];
	Sha256(reversed, bi.length - 4, checksum);
	Sha256(checksum, 32, checksum2);


	int ok = TRUE;

	for (x = 0; x < 4; x++){

		/*printf("\ncs1: %u cs2: %u bi:%u",checksum[x], checksum2[x] , bi.data[3 - x]);*/


		 /* this should be checked*/
		if (checksum2[x] != bi.data[3-x]){
			printf("\nbrak %u %u",checksum2[x],bi.data[3-x]);
			ok = FALSE;
		}
	}

	if(!ok) {
     /* this should be checked*/
		onErrorReceived(ERROR_BASE58_DECODE_CHECK_INVALID,"The data passed to DecodeBase58Checked is invalid. Checksum does not match.");
		bi.length = 1;
		bi.data[0] = 0;
		return bi;
	}

	return bi;
}

char * encodeBase58(uint8_t * bytes, uint8_t len) {

	uint8_t x = 0;
	char * str = malloc(len);

	if (! str) {

		return NULL;
	}

	uint8_t size = len;
	/* Zeros */
	uint8_t y;

	for (y = len - 1;; y--) {

			if (! bytes[y]) {

				str[x] = '1';
				x++;

				if (! y) {

					break;
				}

			} else {

				break;
			}
	}
	uint8_t zeros = x;

	BigInt bi;
	bi.data = malloc(len);

	if (! bi.data) {

		free(str);
		return NULL;
	}

	memmove(bi.data, bytes, len);
	bi.length = len;
	BigIntNormalise(&bi);

	uint8_t * temp = malloc(len);

	if (! temp) {

		free(str);
		free(bi.data);
		return NULL;
	}
	/* Encode */
	uint8_t mod;

	for (;BigIntCompareTo58(&bi) >= 0;x++) {

		mod = BigIntModuloWith58(&bi);

		if (size < x + 3) {
			size = x + 3;
			char * temp = realloc(str, size);

			if (! temp) {

				free(str);
				return NULL;
			}
			str = temp;
		}
		str[x] = base58Characters[mod];
		BigIntEqualsSubtractionByUInt8(&bi, mod);
		memset(temp, 0, len);
		BigIntEqualsDivisionBy58(&bi, temp);
	}
	str[x] = base58Characters[bi.data[bi.length - 1]];
	free(bi.data);
	x++;

	for (y = 0; y < (x-zeros) / 2; y++) {

		char temp = str[y+zeros];
		str[y + zeros] = str[x - y - 1];
		str[x - y - 1] = temp;
	}

	str[x] = '\0';

	free(temp);
	return str;
}

uint8_t * getCheckSum(uint8_t* data) {



	uint8_t checksum[32];

	Sha256(data,21,checksum);
	Sha256(checksum,32,checksum);

	return checksum;
}
