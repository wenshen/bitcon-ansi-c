/*
 * TransactionInput.c
 *
 *  Created on: 07/10/2012
 *  Created by: Abdulfatai Popoola
 *  Modified by: Wen Shen & Krasnoshtan Dmytro
 *  Copyright (c) 2012 Bitcoin Project Team
 */

#include "TransactionInput.h"
#include "../../Utils/VariableLengthInteger/VarLenInt.h"
#include "assert.h"

/* CONSTRUCTORS*/

TransactionInput * createNewTransactionInput(Script * script, uint32_t sequence, ByteArray * prevOutputHash, uint32_t prevOutputIndex, void (*onErrorReceived)(Error error,char *,...))
{
	assert(sequence >= 0);
	assert(prevOutputHash != NULL);
	assert(prevOutputIndex >= 0);
	assert(onErrorReceived != NULL);

	/*try to allocate memory for the object*/
	TransactionInput * self = malloc(sizeof(*self));

	if (!self) {
		/* if memory was not allocated - exit */
		onErrorReceived(ERROR_OUT_OF_MEMORY,	"Cannot allocate %i bytes of memory for createNewTransactionInput\n", sizeof(*self));
		return NULL;
	}

	getObject(self)->destroy = destroyTransactionInput; /*assign destructor function to object*/

	if (initializeTransactionInput(self, script, sequence, prevOutputHash, prevOutputIndex, onErrorReceived)) {
		return self;
	}

	/* Initialization failed; Clean up everything and return NULL*/
	free(self);
	return NULL;
}

TransactionInput * createNewTransactionInputFromData(ByteArray * data,void (*onErrorReceived)(Error error,char *,...))
{
	TransactionInput * self = malloc(sizeof(*self));

	if (!self) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory in createNewTransactionInputFromData\n", sizeof(*self));
		return NULL;
	}

	getObject(self)->destroy = destroyTransactionInput; /*assign destructor function to object*/

	if(initializeTransactionInputFromData(self, data, onErrorReceived)) {
		return self;
	}

	/* Initialization from Data failed; Clean up everything and return NULL*/
	free(self);
	return NULL;
}

/*INITIALIZERS*/
int initializeTransactionInput(TransactionInput * self, Script * script, uint32_t sequence, ByteArray * prevOutputHash, uint32_t prevOutputIndex, void (*onErrorReceived)(Error error,char *,...))
{
	assert(self != NULL);
	assert(prevOutputHash != NULL);

	if (script) {
		self->scriptObject = script;
		incrementReferenceCount(script);
	} else {
		self->scriptObject = NULL;
	}

	if (prevOutputHash) {
		self->prevOutput.hash = prevOutputHash;
		incrementReferenceCount(prevOutputHash);
	}


	self->prevOutput.index = prevOutputIndex;
	self->sequence = sequence;

	int isSuccessful = initializeMessageFromObject(getMessage(self), onErrorReceived);
	if (!isSuccessful) {
		return FALSE;
	}

	return TRUE;
}

int initializeTransactionInputFromData(TransactionInput * self, ByteArray * data, void (*onErrorReceived)(Error error,char *,...))
{
	assert(self != NULL);
	assert(data != NULL);

	self->scriptObject = NULL;
	self->prevOutput.hash = NULL;

	int isSuccessful = initializeMessageFromByteArrayData(getMessage(self), data, onErrorReceived);
	if (!isSuccessful) {
		return FALSE;
	}
	return TRUE;
}


TransactionInput * getTransactionInput(void * self)
{
	assert(self != NULL);
	return self;
}

/*DESTRUCTORS*/
void destroyTransactionInput(void * txIn)
{
	assert (txIn != NULL);

	TransactionInput * self = txIn;

	if (self->scriptObject){
		decrementReferenceCount(self->scriptObject);
	}

	if (self->prevOutput.hash) {
		decrementReferenceCount(self->prevOutput.hash);
	}

	destroyMessage(self);
}

/* BYTE SERIALIZATION AND DESERIALIZATION*/
uint32_t serializeTransactionInput(TransactionInput * self)
{
	assert(self != NULL);

	ByteArray * bytes = getMessage(self)->bytes;

	if (!bytes) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_SERIALISATION_NULL_BYTES, "Attempting to serialize a TransactionInput with no bytes.\n");
		return 0;
	}


	if (!self->prevOutput.hash) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_SERIALISATION_BAD_DATA, "Attempting to serialize a TransactionInput without previous output hash.\n");
		return 0;
	}

	if (!self->scriptObject) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_SERIALISATION_BAD_DATA, "Attempting to serialize a TransactionInput without scriptObject.\n");
		return 0;
	}

	VarLenInt scriptLen = createVarLenIntFromUInt64(getByteArray(self->scriptObject)->length);


	uint32_t requiredLen = 40 + scriptLen.storageSize + getByteArray(self->scriptObject)->length;

	if (bytes->length < requiredLen) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_SERIALISATION_BAD_BYTES, "TransactionInput has fewer bytes than expected. TransactionInput byte-length: %i < required: %i\n",bytes->length, requiredLen);
		return 0;
	}

	/* Serialize data into the ByteArray*/
	printf("byte: %i", bytes->length);
	printf("prev: %x", self->prevOutput.hash->sharedData->data);
	copyByteArrayToByteArray(bytes, 0, self->prevOutput.hash);

	printf("we r here 4th.\n");
	changeByteArrayDataReference(self->prevOutput.hash, bytes, 0);
	writeInt32AsLittleEndianIntoByteArray(bytes, 32, self->prevOutput.index);
	encodeVarLenInt(bytes, 36, scriptLen);
	printf("we r here 5th.\n");

	copyByteArrayToByteArray(bytes, 36 + scriptLen.storageSize, getByteArray(self->scriptObject));
	changeByteArrayDataReference(getByteArray(self->scriptObject), bytes, 36 + scriptLen.storageSize);
	writeInt32AsLittleEndianIntoByteArray(bytes,36 + scriptLen.storageSize + getByteArray(self->scriptObject)->length, self->sequence);

	return requiredLen;
}

uint32_t deserializeTransactionInput(TransactionInput * self)
{
	assert(self != NULL);

	ByteArray * bytes = getMessage(self)->bytes;
	/* TODO add to constants when Abdul will release the file */
	if (!bytes) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_NULL_BYTES, "Attempting to deserialize a TransactionInput with no bytes.");
		return 0;
	}

	if (bytes->length < 41) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,	"Attempting to deserialise a TransactionInput with less than 41 bytes.");
		return 0;
	}

	VarLenInt scriptLen = decodeVarLenInt(bytes, 36);

	if (scriptLen.value > 10000) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_BAD_BYTES, "Attempting to deserialise a TransactionInput that is too large.");
		return 0;
	}

	uint32_t requiredLen = (uint32_t) (40 + scriptLen.storageSize + scriptLen.value);

	if (bytes->length < requiredLen) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,	"Attempted deserialization of TransactionInput with fewer bytes than expected. Byte-length: %i < required: %i\n",bytes->length, requiredLen);
		return 0;
	}
	/* Deserialize by subreferencing byte arrays and reading integers.*/
	self->prevOutput.hash = getByteArraySubsectionReference(bytes, 0, 32);

	if (!self->prevOutput.hash) {
		getMessage(self)->onErrorReceived(ERROR_INIT_FAIL, "Cannot create a new ByteArray in deserializeTransactionInput");
		return 0;
	}

	self->prevOutput.index = readInt32AsLittleEndianFromByteArray(bytes, 32);
	self->scriptObject = createNewScriptFromReference(bytes, 36 + scriptLen.storageSize, (uint32_t) scriptLen.value);

	if (!self->scriptObject) {
		getMessage(self)->onErrorReceived(ERROR_INIT_FAIL, "Cannot create a new Script in TransactionInputDeserialise");
		decrementReferenceCount(self->prevOutput.hash);
		return 0;
	}

	self->sequence = readInt32AsLittleEndianFromByteArray(bytes, (uint32_t) (36 + scriptLen.storageSize + scriptLen.value));
	return requiredLen;
}
