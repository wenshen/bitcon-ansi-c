/*
 *  Peer.c
 *
 *  Created on: 9/11/2012.
 *  Created by: Prajowal Manandhar
 *  Modified by:
 *  Copyright (c) 2012 Bitcoin Project Team
 *  please read https://en.bitcoin.it/wiki/Protocol_Specification
 */

#include "Peer.h"

Peer * createNewPeerFromNetworkAddress(NetworkAddress * addr)
{
	Peer * self = getNode(addr);
	self = realloc(self, sizeof(*self));
	if (!self) {
		getMessage(self)->onErrorReceived(ERROR_OUT_OF_MEMORY,
				"Cannot reallocate to %i bytes of memory in NewNodeByTakingNetworkAddress\n",
				sizeof(*self));
		return NULL;
	}

	getObject(self)->destroy = destroyNode;
	if (initNodeByTakingNetworkAddress(self)) {
		return self;
	}

	free(self);
	return NULL;
}

Peer * getNode(void * self) {
	return self;
}

short int initNodeByTakingNetworkAddress(Peer * self) {
	self->receive = NULL;
	self->receivedHeader = 0;
	self->versionSent = 0;
	self->versionAck = 0;
	self->versionMessage = NULL;
	self->acceptedTypes = 0;
	self->timeOffset = 0;
	self->sentHeader = 0;
	self->messageSent = 0;
	self->timeBroadcast = 0;
	self->connectionWorking = 0;
	self->typesExpectedNum = 0;
	self->downloadTime = 0;
	self->downloadAmount = 0;
	self->latencyTime = 0;
	self->responses = 0;
	self->latencyTimerStart = 0;
	self->downloadTimerStart = 0;
	self->sendQueueFront = 0;
	self->sendQueueSize = 0;
	self->messageReceived = 0;
	return 1;
}

void destroyNode(void * self) {
	destroyNetworkAddress(self);
}

