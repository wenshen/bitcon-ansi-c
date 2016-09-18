/*
 * AddressBroadcast.c
 *
 *  Created on: 03/10/2012
 *  Created by: Hayk Baluyan
 *  Copyright (c) 2012 Bitcoin Project Team
 */

#include "AddressBroadcast.h"
#include <assert.h>

AddressBroadcast * createNewAddressBroadcast(uint8_t timeStamps,void(*onErrorReceived)(Error error, char *, ...)){
	AddressBroadcast * self = malloc(sizeof(*self));
	if (! self) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory in NewAddressBroadcast\n",sizeof(*self));
		return NULL;
	}
	getObject(self)->destroy = destroyAddressBroadcast;
	if (initializeAddressBroadcast(self,timeStamps,onErrorReceived))
		return self;
	free(self);
	return NULL;
}
AddressBroadcast * createNewAddressBroadcastFromSerialisedData(ByteArray * data,uint8_t timeStamps,void(*onErrorReceived)(Error error, char *, ...)){
	AddressBroadcast * self = malloc(sizeof(*self));
	if (! self) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory in NewAddressBroadcast\n",sizeof(*self));
		return NULL;
	}
	getObject(self)->destroy = destroyAddressBroadcast;
	if (initializeAddressBroadcastFromSerialisedData(self,timeStamps,data,onErrorReceived))
		return self;
	free(self);
	return NULL;
}

AddressBroadcast * getAddressBroadcast(void * self)
{
	return self;
}

uint8_t initializeAddressBroadcast(AddressBroadcast * self,uint8_t timeStamps,void(*onErrorReceived)(Error error, char *, ...))
{
	self->timeStamps = timeStamps;
	self->addrNum = 0;
	self->addresses = NULL;
	if (! initializeMessageFromObject(getMessage(self), onErrorReceived))
		return FALSE;
	return TRUE;
}
uint8_t initializeAddressBroadcastFromSerialisedData(AddressBroadcast * self,uint8_t timeStamps, ByteArray * data,void(*onErrorReceived)(Error error, char *, ...)){
	self->timeStamps = timeStamps;
	self->addrNum = 0;
	self->addresses = NULL;
	if (! initializeMessageFromByteArrayData(getMessage(self), data, onErrorReceived))
		return FALSE;
	return TRUE;
}



void destroyAddressBroadcast(void * vself){
	AddressBroadcast * self = vself;
	uint8_t x;
	for (x = 0; x < self->addrNum; x++){
		decrementReferenceCount(self->addresses[x]);
	}
	if (self->addresses) {
		free(self->addresses);
	}
	destroyObject(self);
}



uint8_t addNetworkAddressToAddressBroadcast(AddressBroadcast * self,NetworkAddress * address)
{
	assert(self != NULL);
	assert(address != NULL);

	incrementReferenceCount(address);
	return addressBroadcastTakeNetworkAddress(self,address);
}

uint32_t deserialiseAddressBroadcast(AddressBroadcast * self)
{
	ByteArray * bytes = getMessage(self)->bytes;
	if (! bytes) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_NULL_BYTES,"Attempting to deserialise a AddressBroadcast with no bytes.");
		return 0;
	}
	if (bytes->length < 26 + self->timeStamps * 4) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,"Attempting to deserialise a AddressBroadcast without enough bytes to cover one address.");
		return 0;
	}
	VarLenInt num = decodeVarLenInt(bytes, 0);
	if (num.value > 30) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,"Attempting to deserialise a AddressBroadcast with a var int over 30.");
		return 0;
	}
	self->addresses = malloc(sizeof(*self->addresses) * (size_t)num.value);
	if (! self->addresses) {
		getMessage(self)->onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory in AddressBroadcastDeserialise\n",sizeof(*self->addresses) * (size_t)num.value);
		return 0;
	}
	self->addrNum = num.value;

	uint16_t cursor = num.storageSize;

	uint8_t x ;
	for (x = 0; x < num.value; x++) {
		/* Make new NetworkAddress from the rest of the data.*/
		uint8_t len;

		ByteArray * data = createNewByteArrayFromSubsection(bytes, cursor, bytes->length-cursor);


		if (data) {


			self->addresses[x] = createNewNetworkAddressFromSerialisedData(data, getMessage(self)->onErrorReceived);

			if (self->addresses[x]){

				len = deserialiseNetworkAddress(self->addresses[x], self->timeStamps);

				if (! len)
					getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,"AddressBroadcast cannot be deserialised because of an error with the NetworkAddress number %u.",x);
			}else{
				len = 0;
				getMessage(self)->onErrorReceived(ERROR_INIT_FAIL,"Could not create NetworkAddress in AddressBroadcastDeserialise for network address %u.",x);
			}
		}else{
			len = 0;
			getMessage(self)->onErrorReceived(ERROR_INIT_FAIL,"Could not create ByteArray in AddressBroadcastDeserialise for network address %u.",x);
		}
		if (! len) {

			decrementReferenceCount(data);
			return 0;
		}

		data->length = len;
		decrementReferenceCount(data);
		cursor += len;



	}

	return cursor;
}
uint32_t lengthOfAddressBroadcast(AddressBroadcast * self)
{
	assert(self!=NULL);

	return getSizeOfVarLenInt(self->addrNum) + self->addrNum * (self->timeStamps ? 30 : 26);
}

uint32_t serialiseAddressBroadcast(AddressBroadcast * self){
	ByteArray * bytes = getMessage(self)->bytes;
	if (! bytes) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_SERIALISATION_NULL_BYTES,"Attempting to serialise a AddressBroadcast with no bytes.");
		return 0;
	}
	if (bytes->length < (26 + self->timeStamps * 4) * self->addrNum) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_SERIALISATION_BAD_BYTES,"Attempting to serialise a AddressBroadcast without enough bytes.");
		return 0;
	}
	VarLenInt num = createVarLenIntFromUInt64(self->addrNum);
	encodeVarLenInt(bytes, 0, num);
	uint16_t cursor = num.storageSize;
	uint8_t x;
	for (x = 0; x < num.value; x++) {
		getMessage(self->addresses[x])->bytes = createNewByteArrayFromSubsection(bytes, cursor, bytes->length-cursor);


		uint32_t len = serialiseNetworkAddress(self->addresses[x],self->timeStamps);
		if (! len) {
			getMessage(self)->onErrorReceived(ERROR_MESSAGE_SERIALISATION_BAD_BYTES,"AddressBroadcast cannot be serialised because of an error with the NetworkAddress number %u.",x);
				uint8_t y;
			for (y = 0; y < x + 1; y++) {
				decrementReferenceCount(getMessage(self->addresses[y])->bytes);
			}
			return 0;
		}
		getMessage(self->addresses[x])->bytes->length = len;
		cursor += len;
	}

	return cursor;
}
uint8_t addressBroadcastTakeNetworkAddress(AddressBroadcast * self,NetworkAddress * address){
	self->addrNum++;
	NetworkAddress ** temp = realloc(self->addresses, sizeof(*self->addresses) * self->addrNum);
	if (! temp) {
		return FALSE;
	}
	self->addresses = temp;
	self->addresses[self->addrNum-1] = address;
	return TRUE;
}
