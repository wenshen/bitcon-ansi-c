/*
 * testAddress.c
 *
 *  Created on: 21/10/2012
 *  Created by: Hayk Baluyan
 *  Copyright (c) 2012 Bitcoin Project Team
 */

/** @file
 @brief A test case for Address
 @details Test the Address struct
 */

#include <stdio.h>
#include <stdarg.h>
#include "../src/Object/Network/Address.h"
#include <time.h>

void onErrorReceived(Error a, char * format, ...);
void onErrorReceived(Error a, char * format, ...) {
	va_list argptr;
	va_start(argptr, format);
	vfprintf(stderr, format, argptr);
	va_end(argptr);
	printf("\n");
}

int main() {

	unsigned int session = (unsigned int) time(NULL);
	session = 1337544566;
	printf("Session = %u\n", session);
	srand(session);

	/* creating Address of Bitcoin pubkey hash type(starting with 1) */
	ByteArray * addressString = createNewByteArrayFromString(
			"1D5A1q5d192j5gYuWiP3CSE5fcaaZxe6E9", TRUE, onErrorReceived);
	int index;
	printf("\nAddress string consists of the following %i  bytes:\n",addressString->length);
	for(index=0;index<addressString->length;index++)
		printf("%u ",addressString->sharedData->data[index]);


	Address * address = createNewAddressFromString(addressString, FALSE, onErrorReceived);
	decrementReferenceCount(addressString);
	printf("\n\nAddress consists of the following %i bytes:\n",address->base.base.length);
			for(index=0;index<address->base.base.length;index++)
				printf("%u ",address->base.base.sharedData->data[index]);

	uint8_t netVersionByte = getNetVersionByteForVersionChecksumBytes(getVersionChecksumBytes(address));
	printf("\n\nVersion Byte is the first byte%u",netVersionByte);
	if (netVersionByte != ADDRESS_PUBKEYHASH) {
		printf("PRODUCTION NET VERSION DOES NOT MATCH %i != 0\n", netVersionByte);
		return 1;
	}

	ByteArray * addressStringFromVersionChecksum = getStringForVersionChecksumBytes(getVersionChecksumBytes(address));

	printf("\n\nAddress as a String is: %s",(char *) getByteArrayData(addressStringFromVersionChecksum));
	if (strcmp((char *) getByteArrayData(addressStringFromVersionChecksum), "1D5A1q5d192j5gYuWiP3CSE5fcaaZxe6E9")) {
		printf(	"\nNOT CACHED STRING WRONG %s != 1D5A1q5d192j5gYuWiP3CSE5fcaaZxe6E9\n",
				(char *) getByteArrayData(addressStringFromVersionChecksum));
		return 1;
	}
	decrementReferenceCount(addressStringFromVersionChecksum);
	decrementReferenceCount(address);

	/* creating Address of Bitcoin testnet pubkey hash type(starting with 111) */
	addressString = createNewByteArrayFromString("mzCk9JXXF9we7MB2Gdt59tcfj6Lr2rSzpu", TRUE, onErrorReceived);
	address = createNewAddressFromString(addressString, FALSE, onErrorReceived);
	decrementReferenceCount(addressString);

	netVersionByte = getNetVersionByteForVersionChecksumBytes(getVersionChecksumBytes(address));
	/*netVersionByte=111; /* FIXME */
	if (netVersionByte != ADDRESS_TESTNETPUBKEYHASH) {
		printf("TEST NET VERSION DOES NOT MATCH %i != 111\n", netVersionByte);
		return 1;
	}
	decrementReferenceCount(address);



	/* Test building from fake RIPEMD160 hash*/
		uint8_t * hash = malloc(20);
		int x;
		for (x = 0; x < 20; x++)
			hash[x] = rand();
		address = createNewAddressFromRIPEMD160Hash(hash, ADDRESS_PUBKEYHASH, false,onErrorReceived);
		free(hash);
		netVersionByte = getNetVersionByteForVersionChecksumBytes(getVersionChecksumBytes(address));
		if (netVersionByte != ADDRESS_PUBKEYHASH) {
			printf("PRODUCTION NET VERSION FOR RIPEMD160 TEST DOES NOT MATCH %i != 0\n",netVersionByte);
			return 1;
		}
		decrementReferenceCount(address);
		hash = malloc(20);
		for (x = 0; x < 20; x++)
			hash[x] = rand();
		address = createNewAddressFromRIPEMD160Hash(hash, ADDRESS_TESTNETPUBKEYHASH, false, onErrorReceived);
		free(hash);
		netVersionByte = getNetVersionByteForVersionChecksumBytes(getVersionChecksumBytes(address));
		if (netVersionByte != ADDRESS_TESTNETPUBKEYHASH) {
			printf("TEST NET VERSION FOR RIPEMD160 TEST DOES NOT MATCH %i != 111\n",netVersionByte);
			return 1;
		}
		decrementReferenceCount(address);
		/* Test createNewAddressFromRIPEMD160Hash for pre-known address*/
		hash = malloc(20);

		for (x = 0; x < 20; x++)
			hash[x] = x;
		address = createNewAddressFromRIPEMD160Hash(hash, ADDRESS_PUBKEYHASH, false, onErrorReceived);
		free(hash);
		addressString = getStringForVersionChecksumBytes(getVersionChecksumBytes(address));
		if (strcmp((char *)getByteArrayData(addressString), "112D2adLM3UKy4Z4giRbReR6gjWuvHUqB")){
			printf("RIPEMD160 TEST STRING WRONG %s != 112D2adLM3UKy4Z4giRbReR6gjWuvHUqB\n",(char *)getByteArrayData(addressString));
			return 1;
		}
		decrementReferenceCount(addressString);
		decrementReferenceCount(address);



	return 0;
}

