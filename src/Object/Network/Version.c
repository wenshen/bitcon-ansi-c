/*
 * Version.c
 *
 * Created on: Nov 9, 2012
 * Created by: Wen Shen
 * Copyright (c) 2012 MIBitcoin Project Team
 */

#include "Version.h"
#include "assert.h"

/*  Constructor */

Version * createNewVersion(int32_t version,VersionServices services,int64_t time,NetworkAddress * addRecv,NetworkAddress * addSource,uint64_t nounce,ByteArray * userAgent,int32_t blockHeight,void (*onErrorReceived)(Error error,char *,...)){
	Version * self = malloc(sizeof(*self));
	assert(addRecv != NULL && userAgent != NULL && onErrorReceived != NULL);
	if (! self) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory in newVersion\n",sizeof(*self));
		return NULL;
	}
	getObject(self)->destroy = destroyVersion;
	if(initVersion(self,version,services,time,addRecv,addSource,nounce,userAgent,blockHeight,onErrorReceived))
		return self;
	free(self);
	return NULL;
}
Version * createNewVersionFromData(ByteArray * data,void (*onErrorReceived)(Error error,char *,...)){

	Version * self = malloc(sizeof(*self));
	assert(data != NULL && onErrorReceived != NULL);

	if (! self) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory in newVersionFromData\n",sizeof(*self));
		return NULL;
	}
	getObject(self)->destroy = destroyVersion;
	if(initVersionFromData(self, data, onErrorReceived))
		return self;
	free(self);
	return NULL;
}

/* Object Getter */

Version * getVersion(void * self){
	assert(self != NULL);
	return self;
}

/*  Initializer */

int initVersion(Version * self,int32_t version,VersionServices services,int64_t time,NetworkAddress * addRecv,NetworkAddress * addSource,uint64_t nounce,ByteArray * userAgent,int32_t blockHeight,void (*onErrorReceived)(Error error,char *,...)){
	assert(self != NULL && addRecv != NULL && addSource != NULL && onErrorReceived != NULL );
	self->version = version;
	self->services = services;
	self->time = time;
	self->addRecv = addRecv;
	incrementReferenceCount(addRecv);
	self->addSource = addSource;
	incrementReferenceCount(addSource);
	self->nounce = nounce;
	self->userAgent = userAgent;
	incrementReferenceCount(userAgent);
	self->blockHeight = blockHeight;
	if (! initializeMessageFromObject(getMessage(self), onErrorReceived))
		return FALSE;
	return TRUE;
}

int initVersionFromData(Version * self,ByteArray * data,void (*onErrorReceived)(Error error,char *,...)){
	assert(self != NULL && data != NULL && onErrorReceived != NULL);
	self->addRecv = NULL;
	self->userAgent = NULL;
	self->addSource = NULL;
	if (! initializeMessageFromByteArrayData(getMessage(self), data, onErrorReceived))
		return FALSE;
	return TRUE;
}

/*  Destructor */

void destroyVersion(void * vself){
	Version * self = vself;
	assert(vself!= NULL);

	if (self->addRecv) destroyObject(self->addRecv);
	if (self->addSource) destroyObject(self->addSource);
	if (self->userAgent) destroyObject(self->userAgent);
	destroyMessage(self);
}

/*  Functions */

uint32_t deserializeVersion(Version * self){

	ByteArray * bytes = getMessage(self)->bytes;
	ByteArray * data = getByteArraySubsectionReference(bytes, 20, bytes->length-20); /* Get data from 20 bytes to the end of the byte array to deserialise the recieving network address. */
	uint32_t len = deserialiseNetworkAddress(self->addRecv, FALSE); /* No time from version message.*/
	uint8_t x = getByteFromByteArray(bytes, 80); /* Check length for decoding VarInt */
	VarLenInt varInt = decodeVarLenInt(bytes, 80);

	assert(self != NULL);

	if (! bytes) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_NULL_BYTES,"Attempting to deserialize a Version with no bytes.");
		return 0;
	}
	if (bytes->length < 46) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,"Attempting to deserialize a Version with less than 46 bytes.");
		return 0;
	}
	self->version = readInt32AsLittleEndianFromByteArray(bytes, 0);
	self->services = (VersionServices) readInt64AsLittleEndianFromByteArray(bytes, 4);
	self->time = readInt64AsLittleEndianFromByteArray(bytes, 12);

	if (! data) {
		getMessage(self)->onErrorReceived(ERROR_INIT_FAIL,"Cannot create a new ByteArray in deserializeVersion for the receiving address.");
		return 0;
	}

	self->addRecv = createNewNetworkAddressFromSerialisedData( data, getMessage(self)->onErrorReceived);
	if (! self->addRecv) {
		getMessage(self)->onErrorReceived(ERROR_INIT_FAIL,"Cannot create a new NetworkAddress in deserializeVersion for the receiving address.");
		destroyObject(data);
		return 0;
	}

	if (! len){
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,"Version cannot be deserialized because of an error with the receiving address.");
		destroyObject(data);
		return 0;
	}
	/* Readjust ByteArray length for receiving address */
	data->length = len;
	destroyObject(data);
	if (self->version >= 106) {
		if (bytes->length < 85) {
			getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,"Attempting to deserialize a Version with less than 85 bytes required.");
			return 0;
		}
		/* Source address deserialization */
		data = getByteArraySubsectionReference(bytes, 46, bytes->length-46);
		if (! data) {
			getMessage(self)->onErrorReceived(ERROR_INIT_FAIL,"Cannot create a new ByteArray in deserializeVersion for the source address.");
			return 0;
		}
		/*Attention!!!*/
		self->addSource = createNewNetworkAddressFromSerialisedData( data, getMessage(self)->onErrorReceived);
		if (! self->addSource) {
			getMessage(self)->onErrorReceived(ERROR_INIT_FAIL,"Cannot create a new NetworkAddress in deserializeVersion for the source address.");
			destroyObject(data);
			return 0;
		}
		/*Attention !!!*/

		if (! len){
			getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,"Version cannot be deserialized because of an error with the source address.");
			destroyObject(data);
			return 0;
		}
		/*Readjust ByteArray length for source address*/
		data->length = len;
		destroyObject(data);
		self->nounce = readInt16AsLittleEndianFromByteArray(bytes, 72);

		if (x > 253){
			getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,"Attempting to deserialize a Version with a var string that is too big.");
			return 0;
		}

		if (bytes->length < 84 + varInt.storageSize + varInt.value) {
			getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,"Attempting to deserialize a Version without enough space to cover the userAgent and block height.");
			return 0;
		}
		if (varInt.value > 400) { /* cbitcoin accepts userAgents upto 400 bytes */
			getMessage(self)->onErrorReceived(ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,"Attempting to deserialize a Version with a userAgent over 400 bytes.");
			return 0;
		}
		self->userAgent = getByteArraySubsectionReference(bytes, 80 + varInt.storageSize, (uint32_t)varInt.value);
		if (! self->userAgent) {
			getMessage(self)->onErrorReceived(ERROR_INIT_FAIL,"Cannot create a new ByteArray in deserializeVersion");
			return 0;
		}
		self->blockHeight = readInt32AsLittleEndianFromByteArray(bytes, 80 + varInt.storageSize + (uint32_t)varInt.value);
		return 84 + varInt.storageSize + (uint32_t)varInt.value;
	}else return 46; /* Not the further message. */
}
uint32_t calculateVersionLength(Version * self){
	uint32_t len = 46; /* Version, services, time and receiving address. */
	assert(self != NULL);

	if (self->version >= 106) {
		if (self->userAgent->length > 400)
			return 0;
		len += 38 + getSizeOfVarLenInt(self->userAgent->length) + self->userAgent->length; /* Source address, nounce, user-agent and block height. */
	}
	return len;
}
uint32_t serializeVersion(Version * self){
	ByteArray * bytes = getMessage(self)->bytes;
	uint32_t len = serialiseNetworkAddress(self->addRecv,FALSE);
	VarLenInt varInt = createVarLenIntFromUInt64(self->userAgent->length);

	if (! bytes) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_SERIALISATION_NULL_BYTES,"Attempting to serialize a Version with no bytes.");
		return 0;
	}
	if (bytes->length < 46) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_SERIALISATION_BAD_BYTES,"Attempting to serialize a Version with less than 46 bytes.");
		return 0;
	}
	writeInt32AsLittleEndianIntoByteArray(bytes, 0, self->version);
	writeInt64AsLittleEndianIntoByteArray(bytes, 4, self->services);
	writeInt64AsLittleEndianIntoByteArray(bytes, 12, self->time);
	getMessage(self->addRecv)->bytes = getByteArraySubsectionReference(bytes, 20, bytes->length-20);
	if (! getMessage(self->addRecv)->bytes) {
		getMessage(self)->onErrorReceived(ERROR_INIT_FAIL,"Cannot create a new ByteArray sub reference in serializeVersion for receiving address.");
		return 0;
	}

	if (! len) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_SERIALISATION_BAD_BYTES,"Version cannot be serialized because of an error with the receiving NetworkAddress");
		/* Release bytes to avoid problems overwriting pointer without release, if serialization is tried again. */
		destroyObject(getMessage(self->addRecv)->bytes);
		return 0;
	}
	getMessage(self->addRecv)->bytes->length = len;
	if (self->version >= 106) {
		if (bytes->length < 85) {
			getMessage(self)->onErrorReceived(ERROR_MESSAGE_SERIALISATION_BAD_BYTES,"Attempting to serialize a Version with less than 85 bytes required.");
			return 0;
		}
		if (self->userAgent->length > 400) {
			getMessage(self)->onErrorReceived(ERROR_MESSAGE_SERIALISATION_BAD_BYTES,"Attempting to serialize a Version with a userAgent over 400 bytes.");
			return 0;
		}
		getMessage(self->addSource)->bytes = getByteArraySubsectionReference(bytes, 46, bytes->length-46);
		if (! getMessage(self->addSource)->bytes) {
			getMessage(self)->onErrorReceived(ERROR_INIT_FAIL,"Cannot create a new ByteArray sub reference in serializeVersion for source address.");
			return 0;
		}
		uint32_t len = serialiseNetworkAddress(self->addSource,FALSE);

		if (! len) {
			getMessage(self)->onErrorReceived(ERROR_MESSAGE_SERIALISATION_BAD_BYTES,"Version cannot be serialized because of an error with the source NetworkAddress");
			/* Release bytes to avoid problems overwritting pointer without release, if serialisation is tried again. */
			destroyObject(getMessage(self->addSource)->bytes);
			return 0;
		}
		getMessage(self->addSource)->bytes->length = len;
		writeInt64AsLittleEndianIntoByteArray(bytes, 72, self->nounce);

		encodeVarLenInt(bytes, 80, varInt);
		if (bytes->length < 84 + varInt.storageSize+ varInt.value) {
			getMessage(self)->onErrorReceived(ERROR_MESSAGE_SERIALISATION_BAD_BYTES,"Attempting to deserialize a Version without enough space to cover the userAgent and block height.");
			return 0;
		}
		copyByteArrayToByteArray(bytes, 80 + varInt.storageSize,self->userAgent);
		changeByteArrayDataReference(self->userAgent,bytes,80 + varInt.storageSize);
		writeInt32AsLittleEndianIntoByteArray(bytes, 80 + varInt.storageSize + (uint32_t)varInt.value, self->blockHeight);
		return 84 + varInt.storageSize + (uint32_t)varInt.value;
	}else return 46; /* Not the further message. */
}
