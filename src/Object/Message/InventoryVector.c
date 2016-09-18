/*
 * InventoryVector.c
 *
 *  Created on: 04/10/2012
 *  Created by: Abdulfatai Popoola
 *  Copyright (c) 2012 Bitcoin Project Team
 */

#include "InventoryVector.h"
#include <assert.h>
#include <stdlib.h>

InventoryVector * createNewInventoryVector(InventoryVectorType type, ByteArray * hash, void (*onErrorReceived)(Error error,char *,...))
{
	assert(type != NULL);
	assert(hash != NULL);
	assert(onErrorReceived != NULL);

	InventoryVector * self = malloc(sizeof(*self));
	if (!self) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,
				"Cannot allocate %i bytes of memory in createNewInventoryVector\n",
				sizeof(*self));
		return NULL;
	}

	getObject(self)->destroy = destroyInventoryVector; /**<assign destructor function to object*/
	if (initializeInventoryVector(self, type, hash, onErrorReceived)) {
		return self;
	}

	free(self);
	return NULL;
}

InventoryVector * createNewInventoryVectorFromData(ByteArray * data, void (*onErrorReceived)(Error error,char *,...))
{
	assert(data != NULL);
	assert(onErrorReceived != NULL);

	InventoryVector * self = malloc(sizeof(*self));
	if (!self) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,
				"Cannot allocate %i bytes of memory in createNewInventoryVectorFromData\n",
				sizeof(*self));
		return NULL;
	}

	getObject(self)->destroy = destroyInventoryVector; /**<assign destructor function to object*/
	if (initializeInventoryVectorFromData(self, data, onErrorReceived)) {
		return self;
	}

	free(self);
	return NULL;
}

InventoryVector * getInventoryVector(void * self)
{
	assert(self != NULL);

	return self;
}

int initializeInventoryVector(InventoryVector * self, InventoryVectorType type, ByteArray * hash, void (*onErrorReceived)(Error error,char *,...))
{
	assert(self != NULL);
	assert(hash != NULL);
	assert(type != NULL);
	assert(onErrorReceived != NULL);

	self->type = type;
	self->hash = hash;
	incrementReferenceCount(hash);
	if (!initializeMessageFromObject(getMessage(self), onErrorReceived)) {
		return FALSE;
	}

	return TRUE;
}

int initializeInventoryVectorFromData(InventoryVector * self, ByteArray * data, void (*onErrorReceived)(Error error,char *,...))
{
	assert(self != NULL);
	assert(data != NULL);
	assert(onErrorReceived != NULL);

	self->hash = NULL;
	if (!initializeMessageFromByteArrayData(getMessage(self), data, onErrorReceived)) {
		return FALSE;
	}

	return TRUE;
}

void destroyInventoryVector(void * selfIn)
{
	assert(selfIn != NULL);

	InventoryVector * self = selfIn;
	decrementReferenceCount(self->hash);
	destroyMessage(self);
}

uint32_t deserializeInventoryVector(InventoryVector * self)
{
	assert(self != NULL);

	ByteArray * bytes = getMessage(self)->bytes;
	if (!bytes) {
		getMessage(self)->onErrorReceived(
				ERROR_MESSAGE_DESERIALISATION_NULL_BYTES,
				"Attempting to deserialise a InventoryVector with no bytes.");
		return 0;
	}
	if (bytes->length < INVENTORYVECTOR_TOTAL_SIZE) {
		getMessage(self)->onErrorReceived(
				ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,
				"Attempting to deserialise a InventoryVector with less than 36 bytes.");
		return 0;
	}
	self->type = readInt32AsLittleEndianFromByteArray(bytes, 0);
	self->hash = getByteArraySubsectionReference(bytes, INVENTORYVECTOR_TYPE_FIELD_SIZE, INVENTORYVECTOR_HASH_FIELD_SIZE);
	if (!self->hash) {
		return 0;
	}

	return INVENTORYVECTOR_TOTAL_SIZE;
}

uint32_t serializeInventoryVector(InventoryVector * self)
{
	assert(self != NULL);

	ByteArray * bytes = getMessage(self)->bytes;
	if (!bytes) {
		getMessage(self)->onErrorReceived(
				ERROR_MESSAGE_SERIALISATION_NULL_BYTES,
				"Attempting to serialise a InventoryVector with no bytes.");
		return 0;
	}
	if (bytes->length < INVENTORYVECTOR_TOTAL_SIZE) {
		getMessage(self)->onErrorReceived(
				ERROR_MESSAGE_SERIALISATION_BAD_BYTES,
				"Attempting to serialise a InventoryVector with less than 36 bytes.");
		return 0;
	}

	writeInt32AsLittleEndianIntoByteArray(bytes, 0, self->type);
	copyByteArrayToByteArray(bytes, INVENTORYVECTOR_TYPE_FIELD_SIZE, self->hash);
	changeByteArrayDataReference(self->hash, bytes, 4);
	return 36;
}
