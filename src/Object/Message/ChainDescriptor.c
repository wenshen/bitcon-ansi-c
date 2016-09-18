/*
 * ChainDescriptor.c
 *
 * Created on: Nov 8, 2012
 * Created by: Wen Shen
 * Copyright (c) 2012 MIBitcoin Project Team
 */

#include "ChainDescriptor.h"
#include "assert.h"


ChainDescriptor * newChainDescriptor(void (*onErrorReceived)(Error error,char *,...)){
	assert(onErrorReceived!= NULL);
	ChainDescriptor * self = malloc(sizeof(*self));
	if (! self) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory in NewChainDescriptor\n",sizeof(*self));
		return NULL;
	}
	getObject(self)->destroy = freeChainDescriptor;
	if(initChainDescriptor(self,onErrorReceived))
		return self;
	free(self);
	return NULL;
}
ChainDescriptor * newChainDescriptorFromData(ByteArray * data,void (*onErrorReceived)(Error error,char *,...)){
	assert(data != NULL && onErrorReceived != NULL);
	ChainDescriptor * self = malloc(sizeof(*self));
	if (! self) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory in NewChainDescriptorFromData\n",sizeof(*self));
		return NULL;
	}
	getObject(self)->destroy = freeChainDescriptor;
	if(initChainDescriptorFromData(self,data,onErrorReceived))
		return self;
	free(self);
	return NULL;
}

ChainDescriptor * getChainDescriptor(void * self){
	assert(self != NULL);
	return self;
}


int initChainDescriptor(ChainDescriptor * self,void (*onErrorReceived)(Error error,char *,...)){
	assert(self != NULL && onErrorReceived != NULL);
	self->numOfHash = 0;
	self->hashes = NULL;
	if (! initializeMessageFromObject(getMessage(self), onErrorReceived))
		return FALSE;
	return TRUE;
}
int initChainDescriptorFromData(ChainDescriptor * self,ByteArray * data,void (*onErrorReceived)(Error error,char *,...)){
	assert(self !=NULL && data != NULL && onErrorReceived != NULL);
	self->numOfHash = 0;
	self->hashes = NULL;
	if (! initializeMessageFromByteArrayData(getMessage(self), data, onErrorReceived))
		return FALSE;
	return TRUE;
}

void freeChainDescriptor(void * vself){
	assert (vself!= NULL);
	ChainDescriptor * self = vself;
	uint16_t x;
	for ( x = 0; x < self->numOfHash; x++) {
		destroyObject(self->hashes[x]);
	}
	free(self->hashes);
	destroyMessage(self);
}


int addHashToChainDescriptor(ChainDescriptor * self,ByteArray * hash){
	assert(self != NULL && hash != NULL);
	incrementReferenceCount(hash);
	return takeHashForChainDescriptor(self,hash);
}
uint16_t deserializeChainDescriptor(ChainDescriptor * self){
	assert(self != NULL);
	ByteArray * bytes = getMessage(self)->bytes;
	if (! bytes) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_NULL_BYTES,"Attempting to deserialise a ChainDescriptor with no bytes.");
		return 0;
	}
	if (bytes->length < 33) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,"Attempting to deserialise a ChainDescriptor with less bytes than required for one hash.");
		return 0;
	}
	VarLenInt numOfHash = decodeVarLenInt(bytes, 0);
	if (numOfHash.value > 500) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,"Attempting to deserialise a ChainDescriptor with a var int over 500.");
		return 0;
	}
	if (bytes->length < numOfHash.storageSize + numOfHash.value * 32) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,"Attempting to deserialise a ChainDescriptor with less bytes than required for the hashes.");
		return 0;
	}
	/* Deserialize each hash */
	self->hashes = malloc(sizeof(*self->hashes) * (size_t)numOfHash.value);
	if (! self->hashes) {
		getMessage(self)->onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory in ChainDescriptorDeserialise\n",sizeof(*self->hashes) * (size_t)numOfHash.value);
		return 0;
	}
	self->numOfHash = numOfHash.value;
	uint16_t cursor = numOfHash.storageSize;
	uint16_t x;
	for ( x = 0; x < self->numOfHash; x++) {
		self->hashes[x] = getByteArraySubsectionReference(bytes, cursor, 32);
		if (! self->hashes[x]){
			getMessage(self)->onErrorReceived(ERROR_INIT_FAIL,"Cannot create new ByteArray in ChainDescriptorDeserialise\n");
			return 0;
		}
		cursor += 32;
	}
	return cursor;
}
uint16_t serializeChainDescriptor(ChainDescriptor * self){
	assert(self != NULL);
	ByteArray * bytes = getMessage(self)->bytes;
	if (! bytes) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_SERIALISATION_NULL_BYTES,"Attempting to serialise a ChainDescriptor with no bytes.");
		return 0;
	}
	VarLenInt numOfHash = createVarLenIntFromUInt64(self->numOfHash);
	if (bytes->length < numOfHash.storageSize + self->numOfHash * 32) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_SERIALISATION_BAD_BYTES,"Attempting to serialise a ChainDescriptor with less bytes than required for the hashes.");
		return 0;
	}
	encodeVarLenInt(bytes, 0, numOfHash);
	uint16_t cursor = numOfHash.storageSize;
	uint16_t x;
	for ( x = 0; x < self->numOfHash; x++) {
		copyByteArrayToByteArray(bytes, cursor, self->hashes[x]);
		changeByteArrayDataReference(self->hashes[x], bytes, cursor);
		cursor += 32;
	}
	return cursor;
}
int takeHashForChainDescriptor(ChainDescriptor * self,ByteArray * hash){
	assert(self != NULL && hash != NULL);
	self->numOfHash++;
	ByteArray ** temp = realloc(self->hashes, sizeof(*self->hashes) * self->numOfHash);
	if (! temp)
		return FALSE;
	self->hashes = temp;
	self->hashes[self->numOfHash-1] = hash;
	return TRUE;
}
