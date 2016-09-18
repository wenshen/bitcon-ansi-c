/*
 * Inventory.c
 *
 *  Created on: Nov 9, 2012
 *      Author: apopoola
 */

#include "Inventory.h"
#include <assert.h>

Inventory * createNewInventory(void (*onErrorReceived)(Error error,char *,...))
{
	assert(onErrorReceived != NULL);

	Inventory * self = malloc(sizeof(*self));
	if (!self) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,"Can! allocate %i bytes of memory in createNewInventory\n", sizeof(*self));
		return NULL;
	}

	getObject(self)->destroy = destroyInventory;

	if(initializeInventory(self, onErrorReceived)) {
		return self;
	}

	destroyObject(self);
	return NULL;
}

Inventory * createNewInventoryFromData(ByteArray * data, void (*onErrorReceived)(Error error,char *,...))
{
	assert(data != NULL);
	assert(onErrorReceived != NULL);

	Inventory * self = malloc(sizeof(*self));
	if (!self) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,"Can! allocate %i bytes of memory in createNewInventoryFromData\n",sizeof(*self));
		return NULL;
	}

	getObject(self)->destroy = destroyInventory;
	if(initializeInventoryFromData(self,data,onErrorReceived)){
		return self;
	}

	destroyObject(self);
	return NULL;
}

Inventory * getInventory(void * self)
{
	assert(self!= NULL);

	return self;
}

int initializeInventory(Inventory * self, void (*onErrorReceived)(Error error,char *,...))
{
	assert(self != NULL);
	assert(onErrorReceived != NULL);

	self->count = 0;
	self->inventory = NULL;
	if (initializeMessageFromObject(getMessage(self), onErrorReceived)){
		return TRUE;
	}
	return FALSE;
}

int initializeInventoryFromData(Inventory * self, ByteArray * data, void (*onErrorReceived)(Error error,char *,...))
{
	assert(self != NULL);
	assert(data != NULL);
	assert(onErrorReceived != NULL);

	self->count = 0;
	self->inventory = NULL;
	if (initializeMessageFromByteArrayData(getMessage(self), data, onErrorReceived)){
		return TRUE;
	}
	return FALSE;
}

void destroyInventory(void * selfIn)
{
	assert(selfIn != NULL);

	Inventory * self = selfIn;

	uint16_t i = 0;
	for (; i < self->count; i++) {
		decrementReferenceCount(self->inventory[i]); /* destroy InventoryVector*/
	}
	destroyObject(self->inventory); /* destroy Inventory */
	destroyMessage(self);
}


uint32_t calculateInventoryLength(Inventory * self)
{
	assert(self != NULL);

	return getSizeOfVarLenInt(self->count) + (self->count * INVENTORYVECTOR_TOTAL_SIZE); /* Size of count field and total size of all InventoryVectors */
}

uint32_t deserializeInventory(Inventory * self)
{
	assert(self != NULL);

	ByteArray * bytes = getMessage(self)->bytes;
	if (! bytes) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_NULL_BYTES,"Attempting to deserialise an Inventory with no bytes.");
		return 0;
	}
	if (bytes->length < 37) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,"Attempting to deserialise an Inventory with less bytes than required for one item.");
		return 0;
	}

	VarLenInt numInventoryVectors = decodeVarLenInt(bytes, 0);
	if (numInventoryVectors.value > 1388) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,"Attempting to deserialise a Inventory with a var int over 1388.");
		return 0;
	}

	/* Run through the items and deserialise each one. */
	self->inventory = malloc(sizeof(*self->inventory) * (size_t)numInventoryVectors.value);
	if (!self->inventory) {
		getMessage(self)->onErrorReceived(ERROR_OUT_OF_MEMORY,"Can! allocate %i bytes of memory in deserializeInventory", sizeof(*self->inventory) * (size_t)numInventoryVectors.value);
		return 0;
	}

	self->count = numInventoryVectors.value;
	uint16_t cursor = numInventoryVectors.storageSize;

	int i = 0;
	for (; i < numInventoryVectors.value; i++) {
		/* Deserialize and create new InventoryVectors from bytes */
		ByteArray * data = getByteArraySubsectionReference(bytes, cursor, bytes->length-cursor);

		if (!data) {
			getMessage(self)->onErrorReceived(ERROR_INIT_FAIL,"Could not create a new ByteArray in deserializeInventory for inventory broadcast number %u.",i);
			return 0;
		}
		self->inventory[i] = createNewInventoryVectorFromData(data, getMessage(self)->onErrorReceived);
		if (!self->inventory[i]) {
			getMessage(self)->onErrorReceived(ERROR_INIT_FAIL,"Could not create a new InventoryItem in deserializeInventory for inventory broadcast number %u.",i);
			decrementReferenceCount(data);
			return 0;
		}

		/* Deserialise */
		uint8_t len = deserializeInventoryVector(self->inventory[i]);
		if (!len){
			getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,"Inventory cannot be deserialised because of an error with the InventoryVector number %u.",i);
			decrementReferenceCount(data);
			return 0;
		}

		/* Adjust length */
		data->length = len;
		decrementReferenceCount(data); /*release pointer to this InventoryVector */
		cursor += len;
	}
	return cursor;
}

uint32_t serializeInventory(Inventory * self)
{
	assert(self != NULL);

	ByteArray * bytes = getMessage(self)->bytes;

	if (! bytes) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_SERIALISATION_NULL_BYTES,"Attempting to serialise a Inventory with no bytes.");
		return 0;
	}

	VarLenInt numInventoryVectors = createVarLenIntFromUInt64(self->count);
	if (bytes->length < numInventoryVectors.storageSize + 36 * self->count) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,"Attempting to deserialise a Inventory with less bytes than required.");
		return 0;
	}

	encodeVarLenInt(bytes, 0, numInventoryVectors);
	uint16_t cursor = numInventoryVectors.storageSize;

	int i = 0;
	for (; i < numInventoryVectors.value; i++) {
		getMessage(self->inventory[i])->bytes = getByteArraySubsectionReference(bytes, cursor, bytes->length-cursor);

		if (! getMessage(self->inventory[i])->bytes) {
			getMessage(self)->onErrorReceived(ERROR_INIT_FAIL,"Cannot create a new ByteArray subsection reference in serialiseInventory");
			return 0;
		}
		uint32_t len = serializeInventoryVector(self->inventory[i]);

		if (! len) {
			getMessage(self)->onErrorReceived(ERROR_MESSAGE_SERIALISATION_BAD_BYTES,"Inventory cannot be serialised because of an error with the InventoryVector number %u.",i);

			/* Release ByteArray objects to avoid problems overwritting pointer without release, if serialisation is tried again. */
			int j = 0;
			for (; j < i + 1; j++) {
				decrementReferenceCount(getMessage(self->inventory[j])->bytes);
			}
			return 0;
		}
		getMessage(self->inventory[i])->bytes->length = len;
		cursor += len;
	}
	return cursor;
}
