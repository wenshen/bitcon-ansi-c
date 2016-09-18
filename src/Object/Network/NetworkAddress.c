/*
 * NetworkAddress.c
 *
 *  Created on: 03/10/2012
 *  Created by: Hayk Baluyan
 *  Copyright (c) 2012 Bitcoin Project Team
 */

#include "NetworkAddress.h"
#include "assert.h"



NetworkAddress * createNewNetworkAddress(uint32_t time,ByteArray * ip,uint16_t port,uint64_t services,void(*onErrorReceived)(Error error, char *, ...)){
	NetworkAddress * self = malloc(sizeof(*self));
	if (! self) {
			onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory in CBNewNetworkAddress\n",sizeof(*self));
			return NULL;
		}
	getObject(self)->destroy = destroyNetworkAddress;
	if (initializeNetworkAddress(self,time,ip,port,services,onErrorReceived))
		return self;
	free(self);
	return NULL;
}
NetworkAddress * createNewNetworkAddressFromSerialisedData(ByteArray * data,void(*onErrorReceived)(Error error, char *, ...))
{
	NetworkAddress * self = malloc(sizeof(*self));
	if (!self) {
			onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory in CBNewNetworkAddress\n",sizeof(*self));
			return NULL;
		}
	getObject(self)->destroy = destroyNetworkAddress;
	if(initializeNetworkAddressFromSerialisedData(self,data,onErrorReceived))
		return self;
	free(self);
	return NULL;
}



NetworkAddress * getNetworkAddress(void * self)
{
	assert(self != NULL);

	return self;
}



uint8_t initializeNetworkAddress(NetworkAddress * self,uint32_t time,ByteArray * ip,uint16_t port,uint64_t services,void(*onErrorReceived)(Error error, char *, ...)){
	self->time = time;
	self->ip = ip;
	if (! ip) {
		ip = createNewByteArrayOfSize(16,onErrorReceived );
		if (! ip)
			return FALSE;
		memset(getByteArrayData(ip), 0, 16);
		self->type = IP_INVALID;
	}
	else{

				self->type = getIPType(getByteArrayData(ip));

				incrementReferenceCount(ip);
	}
	self->port = port;
	self->services = services;

	if (! initializeMessageFromObject(getMessage(self), onErrorReceived)){
		decrementReferenceCount(ip);
		return FALSE;
	}
	return TRUE;
}

uint8_t initializeNetworkAddressFromSerialisedData(NetworkAddress * self,ByteArray * data,void(*onErrorReceived)(Error error, char *, ...)){
	self->ip = NULL;
	self->time=0;
	if (! initializeMessageFromByteArrayData(getMessage(self), data,onErrorReceived))
		return FALSE;

	return TRUE;
}



void destroyNetworkAddress(void * vself){
	NetworkAddress * self = vself;
	if (self->ip) decrementReferenceCount(self->ip);
	destroyMessage(self);
}


uint8_t deserialiseNetworkAddress(NetworkAddress * self,uint8_t time){

	ByteArray * bytes = getMessage(self)->bytes;

	if (! bytes) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_NULL_BYTES,"Attempting to deserialise a NetworkAddress with no bytes.");
		return 0;
	}
	if (bytes->length < 26 + time * 4) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,"Attempting to deserialise a NetworkAddress with less bytes than required.");
		return 0;
	}
	uint8_t cursor;
	if (time) {
		self->time = readInt32AsLittleEndianFromByteArray(bytes, 0);

		cursor = 4;
	}else{
		self->time = 0;
		cursor = 0;
	}
	self->services = readInt64AsLittleEndianFromByteArray(bytes, cursor);
	cursor += 8;
	self->ip = createNewByteArrayFromSubsection(bytes, cursor, 16);


	if (! self->ip)
		return 0;

	self->type = getIPType(getByteArrayData(self->ip));
	cursor += 16;
	/* read port */
	uint16_t result = bytes->sharedData->data[bytes->offset+cursor + 1];
	result |= (uint16_t)bytes->sharedData->data[bytes->offset+cursor] << 8;
	self->port = result;

	return cursor + 2;
}
uint8_t compareNetworkAddresses(NetworkAddress * self,NetworkAddress * addr){
	return (self->ip
			&& addr->ip
			&& compareByteArrays(self->ip, addr->ip)
			&& self->port == addr->port);
}
uint8_t serialiseNetworkAddress(NetworkAddress * self,uint8_t time){
	ByteArray * bytes = getMessage(self)->bytes;
	if (! bytes) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_SERIALISATION_NULL_BYTES,"Attempting to serialise a NetworkAddress with no bytes.");
		return 0;
	}
	if (bytes->length < 26 + time * 4) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_SERIALISATION_BAD_BYTES,"Attempting to serialise a NetworkAddress with less bytes than required.");
		return 0;
	}
	uint8_t cursor;
	if (time) {
		writeInt32AsLittleEndianIntoByteArray(bytes, 0, self->time);
		cursor = 4;
	}else cursor = 0;

	writeInt64AsLittleEndianIntoByteArray(bytes, cursor, self->services);
	cursor += 8;
	copyByteArrayToByteArray(bytes, cursor, self->ip);
	changeByteArrayDataReference(self->ip, bytes, cursor);
	cursor += 16;
	/*set port */
	bytes->sharedData->data[bytes->offset+cursor + 1] = self->port;
	bytes->sharedData->data[bytes->offset+cursor] = (self->port) >> 8;

	return cursor + 2;

}
