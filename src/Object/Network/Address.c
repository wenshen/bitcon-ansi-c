/*
 * Address.c
 *
 *  Created on: 03/10/2012
 *  Created by: Hayk Baluyan
 *  Copyright (c) 2012 Bitcoin Project Team
 */


#include "Address.h"
#include "../VersionChecksumBytes.h"

Address * createNewAddressFromRIPEMD160Hash(uint8_t * hash,uint8_t netVersionByte,uint8_t cacheString,void (*onErrorReceived)(Error error,char *,...)){

	Address * self = malloc(sizeof(*self));
	if (!self) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory in createNewAddressFromRIPEMD160Hash\n",sizeof(*self));
		return NULL;
	}
	getObject(self)->destroy = destroyAddress;
	if (initializeAddressFromRIPEMD160Hash(self,netVersionByte,hash,cacheString,onErrorReceived)) {
		return self;
	}
	free(self);
	return NULL;
}

Address * createNewAddressFromString(ByteArray * string,uint8_t cacheString,void (*onErrorReceived)(Error error,char *,...)){
	Address * self = malloc(sizeof(*self));
	if ( !self ) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory in NewAddressFromString\n",sizeof(*self));
		return NULL;
	}
	getObject(self)->destroy = destroyAddress;
	if (initializeAddressFromString(self,string,cacheString,onErrorReceived))
		return self;

	free(self);
	return NULL;
}


Address * getAddress(void * self){
	return self;
}



uint8_t initializeAddressFromRIPEMD160Hash(Address * self,uint8_t netVersionByte,uint8_t * hash,uint8_t cacheString,void (*onErrorReceived)(Error error,char *,...))
{
	/* Build address and then complete intitialization with VersionChecksumBytes*/
	uint8_t * addressData = malloc(25); /* 1 version byte, 20 hash bytes, 4 checksum bytes.*/
	if ( addressData==NULL) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate 25 bytes of memory in initializeAddressFromRIPEMD160Hash\n");
		return FALSE;
	}
	/* Set version byte */
	addressData[0] = netVersionByte;
	/* Move hash */
	memmove(addressData+1, hash, 20);
	/* Make checksum and move it into address; check https://en.bitcoin.it/wiki/Technical_background_of_Bitcoin_addresses for step by step explanation */
	uint8_t checksum[32];
	uint8_t checksum2[32];
	Sha256(addressData,21,checksum);
	Sha256(checksum,32,checksum2);
	memmove(addressData+21, checksum2, 4);
	/* initialise VersionChecksumBytes*/
	if (! initializeVersionChecksumBytesFromBytes(getVersionChecksumBytes(self), addressData, 25,cacheString, onErrorReceived)) {
		return FALSE;
	}
	return TRUE;
}

uint8_t initializeAddressFromString(Address * self,ByteArray * string,uint8_t cacheString,void (*onErrorReceived)(Error error,char *,...)){
	if (! initializeVersionChecksumBytesFromString(getVersionChecksumBytes(self), string, cacheString, onErrorReceived))
		return FALSE;
	return TRUE;
}

void destroyAddress(void * self){
	destroyVersionChecksumBytes(getVersionChecksumBytes(self));
}

