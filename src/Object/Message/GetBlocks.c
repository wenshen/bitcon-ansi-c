/*
 * GetBlocks.c
 *
 * Created on: Nov 9, 2012
 * Created by: Wen Shen
 * Copyright (c) 2012 MIBitcoin Project Team
 */

#include "GetBlocks.h"
#include "assert.h"


GetBlocks * newGetBlocks(uint32_t version,ChainDescriptor * chainDescriptor,ByteArray * stopAtHash,void (*onErrorReceived)(Error error,char *,...)){
	assert(chainDescriptor != NULL && stopAtHash != NULL && onErrorReceived != NULL);
	GetBlocks * self = malloc(sizeof(*self));
	if (! self) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory in newGetBlocks\n",sizeof(*self));
		return NULL;
	}
	getObject(self)->destroy = destroyGetBlocks;
	if(initGetBlocks(self,version,chainDescriptor,stopAtHash,onErrorReceived))
		return self;
	free(self);
	return NULL;
}
GetBlocks * newGetBlocksFromData(ByteArray * data,void (*onErrorReceived)(Error error,char *,...)){
	assert(data != NULL && onErrorReceived != NULL);
	GetBlocks * self = malloc(sizeof(*self));
	if (! self) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory in newGetBlocksFromData\n",sizeof(*self));
		return NULL;
	}
	getObject(self)->destroy = destroyGetBlocks;
	if(initGetBlocksFromData(self,data,onErrorReceived))
		return self;
	free(self);
	return NULL;
}


GetBlocks * retrieveGetBlocks(void * self){
	assert(self != NULL);
	return self;
}


int initGetBlocks(GetBlocks * self,uint32_t version,ChainDescriptor * chainDescriptor,ByteArray * stopAtHash,void (*onErrorReceived)(Error error,char *,...)){
	assert (self != NULL && chainDescriptor != NULL && stopAtHash != NULL && onErrorReceived != NULL);
	self->version = version;
	self->chainDescriptor = chainDescriptor;
	incrementReferenceCount(chainDescriptor);
	self->stopAtHash = stopAtHash;
	incrementReferenceCount(stopAtHash);
	if (! initializeMessageFromObject(getMessage(self), onErrorReceived))
		return FALSE;
	return TRUE;
}

int initGetBlocksFromData(GetBlocks * self,ByteArray * data,void (*onErrorReceived)(Error error,char *,...)){
	assert(self != NULL && data != NULL && onErrorReceived != NULL);
	self->chainDescriptor = NULL;
	self->stopAtHash = NULL;
	if (! initializeMessageFromByteArrayData(getMessage(self), data, onErrorReceived))
		return FALSE;
	return TRUE;
}


void destroyGetBlocks(void * vself){
	assert(vself != NULL);
	GetBlocks * self = vself;
	if (self->chainDescriptor) destroyObject(self->chainDescriptor);
	if (self->stopAtHash) destroyObject(self->stopAtHash);
	destroyMessage(self);
}

uint32_t calculateGetBlocksLength(GetBlocks * self){
	assert(self != NULL);
	return 36 + self->chainDescriptor->numOfHash * 32 + getSizeOfVarLenInt(self->chainDescriptor->numOfHash);
}

uint16_t deserializeGetBlocks(GetBlocks * self){
	assert(self != NULL);
	ByteArray * bytes = getMessage(self)->bytes;
	if (! bytes) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_NULL_BYTES,"Attempting to deserialise a GetBlocks with no bytes.");
		return 0;
	}
	if (bytes->length < 69) { /* 4 version bytes, 33 chain descriptor bytes, 32 stop at hash bytes */
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,"Attempting to deserialise a GetBlocks with less bytes than required for one hash.");
		return 0;
	}
	self->version = readInt32AsLittleEndianFromByteArray(bytes, 0);
	/* Deserialize the ChainDescriptor */
	ByteArray * data  = getByteArraySubsectionReference(bytes, 4, bytes->length-4);
	if (! data) {
		getMessage(self)->onErrorReceived(ERROR_INIT_FAIL,"Cannot create new ByteArray in deserializeGetBlocks for the chain descriptor.\n");
		return 0;
	}
	self->chainDescriptor = newChainDescriptorFromData(data, getMessage(self)->onErrorReceived);
	if (! self->chainDescriptor){
		getMessage(self)->onErrorReceived(ERROR_INIT_FAIL,"Cannot create new ChainDescriptor in deserializeGetBlocks\n");
		destroyObject(data);
		return 0;
	}
	uint16_t len = deserializeChainDescriptor(self->chainDescriptor);
	if (! len){
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,"GetBlocks cannot be deserialised because of an error with the ChainDescriptor.");
		destroyObject(data);
		return 0;
	}
	data->length = len; /* Re-adjust length for the chain descriptor object's reference. */
	destroyObject(data); /* Release this reference. */
	/* Deserialise stopAtHash */
	if (bytes->length < len + 36) { /* The chain descriptor length plus the version and stopAtHash bytes. */
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,"Attempting to deserialise a GetBlocks with less bytes than required for the stopAtHash.");
		return 0;
	}
	self->stopAtHash = getByteArraySubsectionReference(bytes, len + 4, 32);
	if (! self->stopAtHash) {
		getMessage(self)->onErrorReceived(ERROR_INIT_FAIL,"Cannot create new ByteArray in deserializeGetBlocks\n");
		destroyObject(self->chainDescriptor);
	}
	return len + 36;
}

uint16_t serializeGetBlocks(GetBlocks * self){
	assert(self != NULL);
	ByteArray * bytes = getMessage(self)->bytes;
	if (! bytes) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_SERIALISATION_NULL_BYTES,"Attempting to serialize a GetBlocks with no bytes.");
		return 0;
	}
	if (bytes->length < 36 + self->chainDescriptor->numOfHash * 32 + getSizeOfVarLenInt(self->chainDescriptor->numOfHash)) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_SERIALISATION_BAD_BYTES,"Attempting to serialize a GetBlocks with less bytes than required.");
		return 0;
	}
	setByteInByteArray(bytes, 0, self->version);
	/* Serialize chain descriptor */
	getMessage(self->chainDescriptor)->bytes = getByteArraySubsectionReference(bytes, 4, bytes->length-4);
	if (! getMessage(self->chainDescriptor)->bytes) {
		getMessage(self)->onErrorReceived(ERROR_INIT_FAIL,"Cannot create a new ByteArray sub reference in GetBlocksSerialise");
		return 0;
	}
	uint32_t len = serializeChainDescriptor(self->chainDescriptor);
	if (! len) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_SERIALISATION_BAD_BYTES,"GetBlocks cannot be serialised because of an error with the chain descriptor. This error should never occur... :o");
		/* Release bytes to avoid problems overwritting pointer without release, if serialisation is tried again. */
		destroyObject(getMessage(self->chainDescriptor)->bytes);
		return 0;
	}
	getMessage(self->chainDescriptor)->bytes->length = len;
	/* Serialize stopAtHash */
	copyByteArrayToByteArray(bytes, len + 4, self->stopAtHash);
	changeByteArrayDataReference(self->stopAtHash, bytes, len + 4);
	return len + 36;
}
