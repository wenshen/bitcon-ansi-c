/*
 * PingPongMessage.
 *
 *  Created on: 04/10/2012
 *  Created by: Abdulfatai Popoola
 *  Copyright (c) 2012 Bitcoin Project Team
 */

#include <assert.h>
#include <stdlib.h>
#include "PingPongMessage.h"

PingPongMessage * createNewPingPongMessage(uint64_t nonceID, void (*onErrorReceived)(Error error,char *,...))
{
	assert(nonceID >= 0);
	assert(onErrorReceived != NULL);

	PingPongMessage * self = malloc(sizeof(*self));
	if (!self) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory in createNewPingPongMessage\n", sizeof(*self));
		return NULL;
	}

	getObject(self)->destroy = destroyPingPongMessage; /**<assign destructor function to object*/
	if (initializePingPongMessage(self, nonceID, onErrorReceived)) {
		return self;
	}

	free(self);
	return NULL;
}

PingPongMessage * createNewPingPongMessageFromData(ByteArray * data, void (*onErrorReceived)(Error error,char *,...))
{
	assert(data != NULL);
	assert(onErrorReceived != NULL);

	PingPongMessage * self = malloc(sizeof(*self));

	if (!self) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory in createNewPingPongMessageFromData\n",sizeof(*self));
		return NULL;
	}

	getObject(self)->destroy = destroyPingPongMessage; /**<assign destructor function to object*/
	if(initializePingPongMessageFromData(self, data, onErrorReceived)) {
		return self;
	}

	free(self);
	return NULL;
}

PingPongMessage * getPingPongMessage(void * self)
{
	assert(self != NULL);

	return self;
}

int initializePingPongMessage(PingPongMessage * self, uint64_t ID, void (*onErrorReceived)(Error error,char *,...))
{
	assert(self != NULL);
	assert(ID >= 0);
	assert(onErrorReceived != NULL);

	self->nonceID = ID;
	if (!initializeMessageFromObject(getMessage(self), onErrorReceived))
		return FALSE;
	return TRUE;
}

int initializePingPongMessageFromData(PingPongMessage * self, ByteArray * data, void (*onErrorReceived)(Error error,char *,...))
{
	assert(self != NULL);
	assert(data != NULL);
	assert(onErrorReceived != NULL);

	if(!initializeMessageFromByteArrayData(getMessage(self), data, onErrorReceived)) {
		return FALSE;
	}

	return TRUE;
}


void destroyPingPongMessage(void * self)
{
	assert(self != NULL);

	destroyMessage(self);
}

uint8_t deserializePingPongMessage(PingPongMessage * self)
{
	assert(self != NULL);

	ByteArray * bytes = getMessage(self)->bytes;
	if (! bytes) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_NULL_BYTES,"Attempting to deserialise a PingPongMessage with no bytes.");
		return 0;
	}
	if (bytes->length < 8) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,"Attempting to deserialise a PingPongMessage with less than 8 bytes.");
		return 0;
	}
	self->nonceID = readInt64AsLittleEndianFromByteArray(bytes, 0);
	return 8;
}

uint8_t serializePingPongMessage(PingPongMessage * self)
{
	assert(self != NULL);

	ByteArray * bytes = getMessage(self)->bytes;
	if (! bytes) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_SERIALISATION_NULL_BYTES,"Attempting to serialise a PingPongMessage with no bytes.");
		return 0;
	}
	if (bytes->length < 8) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_SERIALISATION_BAD_BYTES,"Attempting to serialise a PingPongMessage with less than 8 bytes.");
		return 0;
	}
	writeInt64AsLittleEndianIntoByteArray(bytes, 0, self->nonceID);
	return 8;
}

