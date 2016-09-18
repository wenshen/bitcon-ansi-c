/*
 * VersionChecksumBytes.c
 *
 *  Created on: 03/10/2012
 *  Created by: Hayk Baluyan
 *  Copyright (c) 2012 Bitcoin Project Team
 */

#include "VersionChecksumBytes.h"
#include "../BigInt/BigInt.h"
#include "../Base58/Base58.h"
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>


VersionChecksumBytes * createNewVersionChecksumBytesFromString(ByteArray * string,uint8_t cacheString,void (*onErrorReceived)(Error error,char *,...))
{
	VersionChecksumBytes * self = malloc(sizeof(*self));
	if (! self) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory in NewVersionChecksumBytesFromString\n",sizeof(*self));
		return NULL;
	}

	getObject(self)->destroy = destroyVersionChecksumBytes;
	if(initializeVersionChecksumBytesFromString(self,string,cacheString,onErrorReceived)){
		return self;
	}

	free(self);
	return NULL;
}

uint8_t initializeVersionChecksumBytesFromString(VersionChecksumBytes * self,ByteArray * string,uint8_t cacheString,void (*onErrorReceived)(Error error,char *,...))
{
	/* Cache string if needed */
	if (cacheString) {
		self->cachedString = string;
		incrementReferenceCount(string);
	} else {
		self->cachedString = NULL;
	}

	self->cacheString = cacheString;
	/* Get bytes from string conversion*/
	BigInt bytes;
	BigIntAlloc(&bytes, 25); /*25 is the number of bytes for bitcoin addresses.*/
	bytes=decodeBase58Checked((char *)getByteArrayData(string), onErrorReceived);

	if (&bytes==NULL){
		return FALSE;
	}

	/* Take over the bytes with the ByteArray*/
	if (! initializeNewByteArrayFromData(getByteArray(self), bytes.data, bytes.length, onErrorReceived)){
		return FALSE;
	}
	reverseBytes(getByteArray(self)); /* BigInt is in little-endian. Conversion needed to make bitcoin address the right way.*/
	return TRUE;
}


VersionChecksumBytes * createNewVersionChecksumBytesFromBytes(uint8_t * bytes,uint32_t size,int cacheString,void (*onErrorReceived)(Error error,char *,...)){
	VersionChecksumBytes * self = malloc(sizeof(*self));
	if (! self) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory in NewVersionChecksumBytesFromBytes\n",sizeof(*self));
		return NULL;
	}
	getObject(self)->destroy = destroyVersionChecksumBytes;
	if(initializeVersionChecksumBytesFromBytes(self,bytes,size,cacheString,onErrorReceived)){
		return self;
	}
	free(self);
	return NULL;
}



uint8_t initializeVersionChecksumBytesFromBytes(VersionChecksumBytes * self,uint8_t * bytes,uint32_t size,uint8_t cacheString,void (*onErrorReceived)(Error error,char *,...)){	self->cacheString = cacheString;
	self->cachedString = NULL;
	if (! initializeNewByteArrayFromData(getByteArray(self), bytes, size, onErrorReceived)){
		return FALSE;
	}
	return TRUE;
}

/*  Object Getter */

VersionChecksumBytes * getVersionChecksumBytes(void * self)
{
	assert(self!=NULL);

	return self;
}



void destroyVersionChecksumBytes(void * vself){
	VersionChecksumBytes * self = vself;
	if (self->cachedString){
		decrementReferenceCount(self->cachedString);
	}
	destroyByteArray(getByteArray(self));
}

/*  Functions */

uint8_t getNetVersionByteForVersionChecksumBytes(VersionChecksumBytes * self){

	return getByteFromByteArray(getByteArray(self), 0);
}

ByteArray * getStringForVersionChecksumBytes(VersionChecksumBytes * self){
	if (self->cachedString) {
		incrementReferenceCount(self->cachedString);
		return self->cachedString;
	} else {
		reverseBytes(getByteArray(self));

		BigInt bytes;
		BigIntAlloc(&bytes, getByteArray(self)->length);
		bytes.length = getByteArray(self)->length;
		memcpy(bytes.data, getByteArrayData(getByteArray(self)), bytes.length);

		char * string = encodeBase58(bytes.data,bytes.length);

		if (! string){
			return NULL;
		}
		ByteArray * str = createNewByteArrayFromString(string, TRUE, getByteArray(self)->onErrorReceived);
		if (! str) {
			free(string);
			return NULL;
		}
		reverseBytes(getByteArray(self));
		if (self->cacheString) {
			self->cachedString = str;
			incrementReferenceCount(str);
		}

		return str;
	}
}

