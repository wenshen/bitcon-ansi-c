/*
 * BlockHeaders.c
 *
 * Created on: Nov 8, 2012
 * Created by: Wen Shen
 * Copyright (c) 2012 MIBitcoin Project Team
 */


#include "BlockHeaders.h"
#include "assert.h"


BlockHeaders * newBlockHeaders(void (*onErrorReceived)(Error error,char *,...)){
	assert(onErrorReceived != NULL && onErrorReceived != NULL);
	BlockHeaders * self = malloc(sizeof(*self));
	if (!self) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory in newBlockHeaders\n",sizeof(*self));
		return NULL;
	}
	getObject(self)->destroy = destroyBlockHeaders;
	if(initBlockHeaders(self,onErrorReceived)){
		return self;
	}
	free(self);
	return NULL;
}
BlockHeaders * newBlockHeadersFromData(ByteArray * data,void (*onErrorReceived)(Error error,char *,...)){
	assert(data != NULL && onErrorReceived != NULL);
	BlockHeaders * self = malloc(sizeof(*self));
	if (!self) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory in newBlockHeadersFromData\n",sizeof(*self));
		return NULL;
	}
	getObject(self)->destroy = destroyBlockHeaders;
	if(initBlockHeadersFromData(self,data,onErrorReceived))
		return self;
	free(self);
	return NULL;
}

/* Object Getter */

BlockHeaders * getBlockHeaders(void * self){
	assert(self!= NULL);
	return self;
}


int initBlockHeaders(BlockHeaders * self,void (*onErrorReceived)(Error error,char *,...)){
	assert(self != NULL && onErrorReceived != NULL);
	self->numOfHeaders = 0;
	self->blockHeaders = NULL;
	if (!initializeMessageFromObject(getMessage(self), onErrorReceived))
		return FALSE;
	return TRUE;
}
int initBlockHeadersFromData(BlockHeaders * self,ByteArray * data,void (*onErrorReceived)(Error error,char *,...)){
	assert(self!= NULL && data != NULL && onErrorReceived != NULL);
	self->numOfHeaders = 0;
	self->blockHeaders = NULL;
	if (! initializeMessageFromByteArrayData(getMessage(self), data, onErrorReceived))
		return FALSE;
	return TRUE;
}



void destroyBlockHeaders(void * vself){
	assert(vself != NULL);
	BlockHeaders * self = vself;
	uint16_t x;
	for ( x = 0; x < self->numOfHeaders; x++) {
		destroyObject(self->blockHeaders[x]);
	}
	free(self->blockHeaders);
	destroyMessage(self);
}


int addBlockHeaderToBlockHeaderList(BlockHeaders * self,Block * header){
	assert(self != NULL && header != NULL);
	incrementReferenceCount(header);
	return takeBlockFromBlockHeaderList(self,header);
}
uint32_t calculateBlockHeadersLength(BlockHeaders * self)
{
	assert(self != NULL);
	return getSizeOfVarLenInt(self->numOfHeaders) + self->numOfHeaders * 81;
}

uint32_t deserializeBlockHeaders(BlockHeaders * self){
	assert(self != NULL);
	ByteArray * bytes = getMessage(self)->bytes;
	if (!bytes) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_NULL_BYTES,"Attempting to deserialize a BlockHeaders with no bytes.");
		return 0;
	}
	if (bytes->length < 82) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,"Attempting to deserialize a BlockHeaders with less bytes than required for one header.");
		return 0;
	}
	VarLenInt numOfHeaders = decodeVarLenInt(bytes, 0);
	if (numOfHeaders.value > 2000) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,"Attempting to deserialize a BlockHeaders with a var int over 2000.");
		return 0;
	}
	/* Deserialise each header */
	self->blockHeaders = malloc(sizeof(*self->blockHeaders) * (size_t)numOfHeaders.value);
	if (! self->blockHeaders) {
		getMessage(self)->onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory in deserializeBlockHeaders\n",sizeof(*self->blockHeaders) * (size_t)numOfHeaders.value);
		return 0;
	}
	self->numOfHeaders = numOfHeaders.value;
	uint16_t cursor = numOfHeaders.storageSize;
	uint16_t x;
	for ( x = 0; x < numOfHeaders.value; x++) {
		/* Make new Block from the rest of the data. */
		ByteArray * data = getByteArraySubsectionReference(bytes, cursor, bytes->length-cursor);
		if (! data) {
			getMessage(self)->onErrorReceived(ERROR_INIT_FAIL,"Cannot create a new ByteArray in deserializeBlockHeaders for the header number %u.",x);
			return 0;
		}
		self->blockHeaders[x] = createNewBlockFromByteArray(data, getMessage(self)->onErrorReceived);
		if (! self->blockHeaders[x]){
			getMessage(self)->onErrorReceived(ERROR_INIT_FAIL,"Cannot create a new Block in deserializeBlockHeaders for the header number %u.",x);
			destroyObject(data);
			return 0;
		}
		/* Deserialize */
		uint8_t len = deserializeBlock(self->blockHeaders[x],false); /* false for no transactions. Only the header. */
		if (! len){
			getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,"BlockHeaders cannot be deserialised because of an error with the Block number %u.",x);
			destroyObject(data);
			return 0;
		}
		/* Adjust length */
		data->length = len;
		destroyObject(data);
		cursor += len;
	}
	return cursor;
}
uint32_t serializeBlockHeaders(BlockHeaders * self){
	assert(self != NULL);
	ByteArray * bytes = getMessage(self)->bytes;
	if (! bytes) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_SERIALISATION_NULL_BYTES,"Attempting to serialize a BlockHeaders with no bytes.");
		return 0;
	}
	if (bytes->length < 81 * self->numOfHeaders) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,"Attempting to deserialize a BlockHeaders with less bytes than minimally required.");
		return 0;
	}
	VarLenInt num = createVarLenIntFromUInt64(self->numOfHeaders);
	encodeVarLenInt(bytes, 0, num);
	uint16_t cursor = num.storageSize;
	uint16_t x;
	for ( x = 0; x < num.value; x++) {
		getMessage(self->blockHeaders[x])->bytes = getByteArraySubsectionReference(bytes, cursor, bytes->length-cursor);
		if (! getMessage(self->blockHeaders[x])->bytes) {
			getMessage(self)->onErrorReceived(ERROR_INIT_FAIL,"Cannot create a new ByteArray sub reference in serializeBlockHeaders for the header number %u",x);
			return 0;
		}
		uint32_t len = serializeBlock(self->blockHeaders[x],false); /* false for no transactions.*/
		if (!len) {
			getMessage(self)->onErrorReceived(ERROR_MESSAGE_SERIALISATION_BAD_BYTES,"BlockHeaders cannot be serialized because of an error with the Block number %u.",x);
			/* Release ByteArray objects to avoid problems overwritting pointer without release, if serialisation is tried again. */
			uint8_t y;
			for ( y = 0; y < x + 1; y++) {
				destroyObject(getMessage(self->blockHeaders[y])->bytes);
			}
			return 0;
		}
		getMessage(self->blockHeaders[x])->bytes->length = len;
		cursor += len;
	}
	return cursor;
}

int takeBlockFromBlockHeaderList(BlockHeaders * self,Block * header){
	assert(self != NULL && header != NULL);
	self->numOfHeaders++;
	Block ** temp = realloc(self->blockHeaders, sizeof(*self->blockHeaders) * self->numOfHeaders);
	if (!temp)
		return false;
	self->blockHeaders = temp;
	self->blockHeaders[self->numOfHeaders-1] = header;
	return TRUE;
}
