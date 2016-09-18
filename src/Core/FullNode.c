/*
 * FullNode.c
 *
 * Created on: Nov 24, 2012
 * Created by: Wen Shen
 * Copyright (c) 2012 MIBitcoin Project Team
 */

#include "FullNode.h"
#include "../Object/Object.h"
#include "../Object/ByteArray.h"
#include "../Object/Network/AddressManager.h"
#include "../Object/Network/Address.h"
#include "../Object/Network/NetworkCommunicator.h"
#include <pwd.h>

FullNode * newFullNode(void (*logError)(char *,...)) {

	FullNode * self = malloc(sizeof(*self));

	if (! self) {

		logError("Cannot allocate %i bytes of memory in newFullNode\n",sizeof(*self));
		return NULL;
	}

	getObject(self)->destroy = destroyFullNode;

	if (initFullNode(self,logError)) {

		return self;
	}

	free(self);
	return NULL;
}

/*  Object Getter*/

FullNode * getFullNode(void * self) {

	return self;
}

/*  Initializer*/

bool initFullNode(FullNode * self,void (*logError)(char *,...)) {

	if (! initializeNetworkCommunicator(getNetworkCommunicator(self), self->onErrorReceived)) {

		return FALSE;
	}
	/* Set network communicator fields.*/
	getNetworkCommunicator(self)->blockHeight = 0;
	getNetworkCommunicator(self)->callbackHandler = self;
	getNetworkCommunicator(self)->flags = NETWORK_COMMUNICATOR_AUTO_DISCOVERY | NETWORK_COMMUNICATOR_AUTO_HANDSHAKE | NETWORK_COMMUNICATOR_AUTO_PING;
	getNetworkCommunicator(self)->version = PONG_VERSION;
	setAlternativeMessages(getNetworkCommunicator(self), NULL, NULL);
	/* Find home directory.*/
	const char * homeDir;
	struct passwd * pwd = getpwuid(getuid());

	if (! pwd) {

		return FALSE;
	}

	homeDir = pwd->pw_dir;
	unsigned long homeLen = strlen(homeDir);
	/* Open or create a new address store*/
	unsigned long dataDirLen = strlen(DATA_DIRECTORY);
	char * addressFilePath = malloc(homeLen + dataDirLen + strlen(ADDRESS_DATA_FILE) + 1);
	memcpy(addressFilePath, homeDir, homeLen);
	memcpy(addressFilePath + homeLen, DATA_DIRECTORY, strlen(DATA_DIRECTORY));
	strcpy(addressFilePath + homeLen + dataDirLen, ADDRESS_DATA_FILE);
	self->addressFile = fopen(addressFilePath, "rb+");

	if (self->addressFile) {

		/* The address store exists.*/
		free(addressFilePath);
		/* Get the file length*/
		fseek(self->addressFile, 0, SEEK_END);
		unsigned long fileLen = ftell(self->addressFile);
		fseek(self->addressFile, 0, SEEK_SET);
		/* Read file into a ByteArray*/
		ByteArray * buffer = createNewByteArrayOfSize((uint32_t)fileLen, self->onErrorReceived);

		if (! buffer) {

			fclose(self->addressFile);
			return FALSE;
		}

		if(fread(getByteArrayData(buffer), fileLen, 1, self->addressFile) != fileLen) {

			decrementReferenceCount(buffer);
			fclose(self->addressFile);
			return FALSE;
		}

		/* Create the AddressManager*/
		getNetworkCommunicator(self)->addresses = newAddressManagerFromData(buffer, logError, fullNodeOnBadTime);
		decrementReferenceCount(buffer);

		if (! addressManagerDeserialise(getNetworkCommunicator(self)->addresses,getNetworkCommunicator(self)->onErrorReceived)) {

			fclose(self->addressFile);
			decrementReferenceCount(getNetworkCommunicator(self)->addresses);
			logError("There was an error when deserialising the AddressManager for the FullNode.");
			return FALSE;
		}

	} else {

		/* The address store does not exist*/
		getNetworkCommunicator(self)->addresses = newAddressManager(logError, fullNodeOnBadTime);

		if (! getNetworkCommunicator(self)->addresses) {

			return FALSE;
		}
		/* Create the file*/
		self->addressFile = fopen(addressFilePath, "wb");
		free(addressFilePath);

		if (! self->addressFile) {

			decrementReferenceCount(getNetworkCommunicator(self)->addresses);
			return FALSE;
		}
	}
	/* Create block validator*/

	return TRUE;
}

/*  Destructor*/

void destroyFullNode(void * self) {

	fclose(getFullNode(self)->addressFile);
	decrementReferenceCount(getNetworkCommunicator(self)->addresses);
	destroyNetworkCommunicator(self);
}


void fullNodeOnBadTime(void * self) {

	destroyFullNode(self);

}
