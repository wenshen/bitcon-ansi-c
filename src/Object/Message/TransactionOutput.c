/*
 * TransactionOutput.c
 *
 *  Created on: 07/10/2012
 *  Created by: Abdulfatai Popoola
 *  Modified by: Wen Shen
 *  Copyright (c) 2012 Bitcoin Project Team
 */

#include "TransactionOutput.h"
#include "../../Utils/VariableLengthInteger/VarLenInt.h"
#include "assert.h"

/*  Constructors */

TransactionOutput * createNewTransactionOutput(uint64_t value, Script * script, void (*onErrorReceived)(Error error,char *,...))
{
	assert(value >= 0);
	assert(script != NULL);
	assert(onErrorReceived != NULL);

	TransactionOutput * self = malloc(sizeof(*self));

	if (!self) {
		onErrorReceived(ERROR_OUT_OF_MEMORY, "Cannot allocate %i bytes of memory in createNewTransactionOutput\n",sizeof(*self));
		return NULL;
	}

	getObject(self)->destroy = destroyTransactionOutput; /*assign destructor function to object*/

	if(initializeTransactionOutput(self, value, script, onErrorReceived)) {
		return self;
	}

	/* Initialization failed; Clean up everything and return NULL*/
	free(self);
	return NULL;
}

TransactionOutput * createNewTransactionOutputFromData(ByteArray * data, void (*onErrorReceived)(Error error,char *,...))
{
	assert(data != NULL);
	assert(onErrorReceived != NULL);

	TransactionOutput * self = malloc(sizeof(*self));

	if (!self) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory in createNewTransactionOutputFromData\n", sizeof(*self));
		return NULL;
	}

	getObject(self)->destroy = destroyTransactionOutput; /*assign destructor function to object */
	if(initializeTransactionOutputFromData(self, data, onErrorReceived)) {
		return self;
	}

	/* Initialization failed; Clean up everything and return NULL */
	free(self);
	return NULL;
}

/* INITIALIZERS*/

int initializeTransactionOutput(TransactionOutput * self, uint64_t value, Script * script, void (*onErrorReceived)(Error error,char *,...))
{
	assert(self != NULL);
	assert(value >= 0);
	assert(script != NULL);
	assert(onErrorReceived != NULL);

	if (script) {
		self->scriptObject = script;
		incrementReferenceCount(script);
	} else {
		self->scriptObject = NULL;
	}

	self->value = value;

	int isSuccessful = initializeMessageFromObject(getMessage(self), onErrorReceived);
	if (!isSuccessful) {
		return FALSE;
	}

	return TRUE;
}

int initializeTransactionOutputFromData(TransactionOutput * self, ByteArray * data, void (*onErrorReceived)(Error error,char *,...))
{
	assert(self != NULL);
	assert(data != NULL);
	assert(onErrorReceived != NULL);

	self->scriptObject = NULL;
	int isSuccessful = initializeMessageFromByteArrayData(getMessage(self), data, onErrorReceived);
	if (isSuccessful){
		return TRUE;
	}

	return TRUE;
}

/* GETTERS*/

TransactionOutput * getTransactionOutput(void * self)
{
	assert(self != NULL);
	return self;
}

/* DESTRUCTOR */

void destroyTransactionOutput(void * txOut)
{
	assert(txOut != NULL);

	TransactionOutput * self = txOut;

	if (self->scriptObject) {
		decrementReferenceCount(self->scriptObject);
	}

	destroyMessage(self);
}

/*BYTE SERIALIZATION AND DESERIALIZATION*/
uint32_t serializeTransactionOutput(TransactionOutput * self)
{
	assert(self!= NULL);
	ByteArray * bytes = getMessage(self)->bytes;

	if (!bytes) {
		getMessage(self)->onErrorReceived(
				ERROR_MESSAGE_SERIALISATION_NULL_BYTES,
				"Attempting to serialize a TransactionOutput with no bytes.");
		return 0;
	}

	if (!self->scriptObject) {
		getMessage(self)->onErrorReceived(
				ERROR_MESSAGE_SERIALISATION_BAD_DATA,
				"Attempting to serialize a TransactionOutput without a scriptObject.");
		return 0;
	}

	VarLenInt scriptLen = createVarLenIntFromUInt64(getByteArray(self->scriptObject)->length);

	uint32_t requiredLen = 8 + scriptLen.storageSize + getByteArray(self->scriptObject)->length;

	if (bytes->length < requiredLen) {
		getMessage(self)->onErrorReceived(
				ERROR_MESSAGE_SERIALISATION_BAD_BYTES,
				"TransactionOutput has fewer bytes than expected. TransactionOutput byte-length: %i < required: %i\n",
				bytes->length, requiredLen);
		return 0;
	}

	/* Serialize data into the ByteArray and reference objects to this ByteArray to save memory. */
	writeInt64AsLittleEndianIntoByteArray(bytes, 0, self->value);
	encodeVarLenInt(bytes, 8, scriptLen);

	copyByteArrayToByteArray(bytes, 8 + scriptLen.storageSize, getByteArray(self->scriptObject));
	changeByteArrayDataReference(getByteArray(self->scriptObject), bytes, 8 + scriptLen.storageSize);
	return requiredLen;
}

uint32_t deserializeTransactionOutput(TransactionOutput * self)
{

	assert(self != NULL);

	ByteArray * bytes = getMessage(self)->bytes;

	if (!bytes) {
		getMessage(self)->onErrorReceived(
				ERROR_MESSAGE_DESERIALISATION_NULL_BYTES,
				"Attempting to deserialise a TransactionOutput with no bytes."); /*TODO Code duplication; fixme and merge */
		return 0;
	}

	if (bytes->length < 9) {
		getMessage(self)->onErrorReceived(
				ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,
				"Attempting to deserialise a TransactionOutput with less than 9 bytes.");
		return 0;
	}

	uint8_t x = getByteFromByteArray(bytes, 8); /* Check length for decoding; change to header? */

	if (x < 253) {
		x = 9;
	} else if (x == 253) {
		x = 11;
	} else if (x == 254) {
		x = 13;
	} else {
		x = 17;
	}

	if (bytes->length < x) {
		getMessage(self)->onErrorReceived(
				ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,
				"Attempting to deserialise a TransactionOutput with less than %i bytes required.", x);
		return 0;
	}

	VarLenInt scriptLen = decodeVarLenInt(bytes, 8); /* Safe decoding after all checks */

	if (scriptLen.value > 10000) {
		getMessage(self)->onErrorReceived(
				ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,
				"Attempted deserialisation of a TransactionOutput with a script that is too large.");
		return 0;
	}

	uint32_t requiredLen = (uint32_t) (8 + scriptLen.storageSize + scriptLen.value);

	if (bytes->length < requiredLen) {
		getMessage(self)->onErrorReceived(
				ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,
				"Attempted deserialization of TransactionInput with fewer bytes than expected. Byte-length: %i < required: %i\n",bytes->length, requiredLen);
		return 0;
	}

	/* Deserialize by subreferencing byte arrays and reading integers. */
	self->value = readInt64AsLittleEndianFromByteArray(bytes, 0);
	self->scriptObject = createNewScriptFromReference(bytes, 8 + scriptLen.storageSize, (uint32_t) scriptLen.value);
	if (!self->scriptObject) {
		getMessage(self)->onErrorReceived(
				ERROR_INIT_FAIL,
				"Cannot create a new Script in deserializeTransactionOutput");
		return 0;
	}
	return requiredLen;
}
