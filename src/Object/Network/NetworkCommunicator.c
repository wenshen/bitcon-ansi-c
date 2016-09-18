/*
 * NetworkCommunicator.c
 *
 * Created on: Nov 12, 2012
 * Created by: Wen Shen
 * Copyright (c) 2012 MIBitcoin Project Team
 */

#include "NetworkCommunicator.h"
#include "Version.h"
#include <assert.h>


/* Constructor */

NetworkCommunicator * createNewNetworkCommunicator(void (*onErrorReceived)(Error error,char *,...))
{
	assert(onErrorReceived != NULL);

	NetworkCommunicator * self = malloc(sizeof(*self));
	if (! self) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory in createNewNetworkCommunicator\n",sizeof(*self));
		return NULL;
	}
	getObject(self)->destroy = destroyNetworkCommunicator;

	if (initializeNetworkCommunicator(self,onErrorReceived)) {
		return self;
	}

	free(self);
	return NULL;
}

NetworkCommunicator * getNetworkCommunicator(void * self)
{
	assert(self!= NULL);

	return self;
}

boolean initializeNetworkCommunicator(NetworkCommunicator * self, void (*onErrorReceived)(Error error,char *,...))
{
	assert(self!= NULL);
	assert(onErrorReceived != NULL);

	/* Set fields. */
	self->attemptingOrWorkingConnections = 0;
	self->numIncomingConnections = 0;
	self->eventLoop = 0;
	self->acceptEventIPv4 = 0;
	self->acceptEventIPv6 = 0;
	self->isListeningIPv4 = FALSE;
	self->isListeningIPv6 = FALSE;
	self->ourIPv4 = NULL;
	self->ourIPv6 = NULL;
	self->pingTimer = 0;
	self->nonce = 0;
	self->stoppedListening = FALSE;
	self->onErrorReceived = onErrorReceived;
	if (!setObjectRefCountToOne(getObject(self))) {
		return FALSE;
	}
	return TRUE;
}

void destroyNetworkCommunicator(void * vself)
{
	assert(vself!= NULL);

	/* When this is called all references to the object should be released, so no additional syncronisation is required */
	NetworkCommunicator * self = vself;
	if (self->isStarted) {
		stopNetworkCommunicator(self);
	}
	if (self->alternativeMessages) {
		decrementReferenceCount(self->alternativeMessages);
	}

	decrementReferenceCount(self->addresses);
	if (self->ourIPv4) {
		decrementReferenceCount(self->ourIPv4);
	}

	if (self->ourIPv6) {
		decrementReferenceCount(self->ourIPv6);
	}

	free(self->altMaxSizes);
	destroyObject(self);
}

void acceptIncomingConnection(void * vself, uint64_t socket)
{
	NetworkCommunicator * self = vself;
	uint64_t connectSocketID;
	if (!socketAccept(socket, &connectSocketID)) { /*FIXME*/
		return;
	}
	/* Connected, add NetworkAddress */

	Peer * peer = createNewPeerFromNetworkAddress(createNewNetworkAddress(0, NULL, 0, 0, self->onErrorReceived));
	if (! peer) {
		return;
	}

	peer->incoming = TRUE;
	peer->socketID = connectSocketID;
	peer->acceptedTypes = MESSAGE_TYPE_VERSION; /* The peer connected to us, we want the version from them. */

	/* Set up receive event */
	if (socketCanReceiveEvent(&peer->receiveEvent,self->eventLoop,peer->socketID, onPeerReadyForReadingCallback, peer)) {
		/* The event works FIXME*/
		if(socketAddEvent(peer->receiveEvent, self->responseTimeOut)){ /* Begin receive event.*/
			/*Success*/
			if (socketCanSendEvent(&peer->sendEvent,self->eventLoop, peer->socketID, onPeerReadyForWritingCallback, peer)) {
				/* Both onErrorReceived work. Take the peer.*/
				if (addPeerToAddressManager(self->addresses, peer)){
					if (self->addresses->peersNum == 1 && ((self->flags & NETWORK_COMMUNICATOR_AUTO_PING))) {
						/* Got first peer, start pings */

						startNetworkCommunicatorPingTimer(self);
					}

					peer->connectionWorking = TRUE;
					self->attemptingOrWorkingConnections++;
					self->numIncomingConnections++;
					if (self->numIncomingConnections == self->maxIncommingConnections || self->attemptingOrWorkingConnections == self->maxConnections) {
						/* Reached maximum connections, stop listening. */
						stopListeningToIncomingConnections(self);
						self->stoppedListening = TRUE;
					}
					return;
				}
			}
		}
		/* Could create receive event but there was a failure afterwards so free it. */
		socketFreeEvent(self->eventLoop,peer->receiveEvent); /*FIXME*/
	}
	/*Failure, release peer. */
	closeSocket(connectSocketID);  /*FIXME*/
	decrementReferenceCount(peer);
}

ConnectionReturn connectToPeerUsingNetworkCommunicator(NetworkCommunicator * self, Peer * peer)
{
	assert(self != NULL);
	assert(peer != NULL);

	if (!isReachableIPType(self->addresses, getNetworkAddress(peer)->type)){
		return CONNECT_NO_SUPPORT;
	}

	boolean isIPv6 = getNetworkAddress(peer)->type & IP_IPv4;

	/* Make socket*/
	SocketReturn res = createNewSocket(&peer->socketID, isIPv6); /*FIXME*/

	if(res == SOCKET_NO_SUPPORT){
		if (isIPv6) {
			setIPTypeReachability(self->addresses, IP_IPv6, FALSE);
		}
		else {
			setIPTypeReachability(self->addresses, IP_IPv4, FALSE);
		}

		if (!isReachableIPType(self->addresses, IP_IPv6 | IP_IPv4)){
			self->onNetworkError(self->callbackHandler,self); /* IPv6 and IPv4 not reachable.*/
		}
		return CONNECT_NO_SUPPORT;
	}else if (res == SOCKET_BAD) {
		if (!self->addresses->peersNum) {
			self->onNetworkError(self->callbackHandler,self);
		}
		return CONNECT_FAIL;
	}
	printf("Connecting to peer\n");
	/* Connect FIXME*/
	if(socketConnect(peer->socketID, getByteArrayData(getNetworkAddress(peer)->ip), isIPv6, getNetworkAddress(peer)->port)){
		/* Add event for connection*/
		if (socketDidConnectEvent(&peer->connectEvent,self->eventLoop, peer->socketID, onPeerConnectCallback, peer)) {
			if (socketAddEvent(peer->connectEvent, self->connectionTimeOut)) {
				/* Connection is fine. Retain for waiting for connect.*/
				incrementReferenceCount(peer);
				self->attemptingOrWorkingConnections++;
				return CONNECT_OK;
			}else {
				socketFreeEvent(self->eventLoop,peer->connectEvent);
			}
		}
		closeSocket(peer->socketID); /*FIXME*/
		return CONNECT_FAIL;
	}

	closeSocket(peer->socketID);  /*FIXME*/
	return CONNECT_BAD;
}
void onPeerConnectCallback(void * vself, void * vpeer)
{
	printf("Connected to peer\n");
	NetworkCommunicator * self = vself;
	Peer * peer = vpeer;
	socketFreeEvent(self->eventLoop,peer->connectEvent); /* No longer need this event. FIXME */

	/* Check to see if in the meantime, that we have not been connected to by the peer. Double connections are bad m'kay. */
	if (!getPeerNodeFromAddressManager(self->addresses, getNetworkAddress(peer))){
		/* Make receive event */
		if (socketCanReceiveEvent(&peer->receiveEvent,self->eventLoop, peer->socketID, onPeerReadyForReadingCallback, peer)) {
			/* Make send event */
			if (socketCanSendEvent(&peer->sendEvent,self->eventLoop, peer->socketID, onPeerReadyForWritingCallback, peer)) {
				if (socketAddEvent(peer->sendEvent, self->sendTimeOut)) {
					if(addPeerToAddressManager(self->addresses, peer)){
						if (self->addresses->peersNum == 1 && ((self->flags & NETWORK_COMMUNICATOR_AUTO_PING))) {
							/* Got first peer, start pings */
							startNetworkCommunicatorPingTimer(self);
						}
						peer->connectionWorking = TRUE;

						/* Connection OK, so begin handshake if auto handshaking is enabled.*/
						boolean fail = FALSE;
						if (self->flags & NETWORK_COMMUNICATOR_AUTO_HANDSHAKE){
							Version * version = getNetworkCommunicatorVersion(self,getNetworkAddress(peer));
							if (version) {
								getMessage(version)->expectResponse = MESSAGE_TYPE_VERACK; /* Expect a response for this message. */
								getMessage(version)->type = MESSAGE_TYPE_VERSION;
								peer->acceptedTypes = MESSAGE_TYPE_VERACK; /* Accept the acknowledgement. */
								sendMessageToPeer(self, peer, getMessage(version));
								decrementReferenceCount(version);
								peer->versionSent = TRUE;
							}else
								fail = TRUE;
						}
						if (! fail)
							return;
					}
				}
				socketFreeEvent(self->eventLoop,peer->sendEvent); /*FIXME*/
			}
			socketFreeEvent(self->eventLoop,peer->receiveEvent);
		}
	}
	/* Close socket on failure*/
	closeSocket(peer->socketID); /*FIXME*/
	self->attemptingOrWorkingConnections--;
	if (! self->attemptingOrWorkingConnections) {
		self->onNetworkError(self->callbackHandler,self);
	}
	if (getNetworkAddress(peer)->public){
		/* No penalty here since it was definitely our fault. */
		NetworkAddress * addr = realloc(peer, sizeof(NetworkAddress));
		if (addr){
			addAddressToAddressManager(self->addresses, addr);
		}
	}else {
		decrementReferenceCount(peer);
	}
}

void disconnectPeer(NetworkCommunicator * self,Peer * peer,uint16_t penalty,boolean stopping)
{
	/* Apply the penalty given*/
	getNetworkAddress(peer)->score -= penalty;
	/*Free onErrorReceived if they exist*/
	if (peer->receiveEvent) socketFreeEvent(self->eventLoop,peer->receiveEvent);
	if (peer->sendEvent) socketFreeEvent(self->eventLoop,peer->sendEvent);
	/* Close the socket*/
	closeSocket(peer->socketID);
	/* Release the receiving message object if it exists.*/
	if (peer->receive) decrementReferenceCount(peer->receive);
	/* Release all messages in the send queue*/
	uint8_t x;
	for ( x = 0; x < peer->sendQueueSize; x++)
		decrementReferenceCount(peer->sendQueue[(peer->sendQueueFront + x) % SEND_QUEUE_MAX_SIZE]);

	/* If incoming, lower the incoming connections number */
	if (peer->incoming) {
		self->numIncomingConnections--;
	}
	if (self->stoppedListening && self->numIncomingConnections < self->maxIncommingConnections) {
		/* Start listening again*/
		startListeningToIncomingConnections(self);
	}

	/* Lower the attempting or working connections number*/
	self->attemptingOrWorkingConnections--;
	/* If this is a working connection, remove from the address manager.*/
	if (peer->connectionWorking){
		removePeerNodeFromAddressManager(self->addresses, peer);
	}
	else{
		/* Else we release the object from control of the NetworkCommunicator*/
		socketFreeEvent(self->eventLoop,peer->connectEvent); /*FIXME*/
		decrementReferenceCount(peer);
	}
	if (self->addresses->peersNum == 0 && (self->flags & NETWORK_COMMUNICATOR_AUTO_PING)) {
		/* No more peers so stop pings*/
		stopNetworkCommunicatorPingTimer(self);
	}
	if (! self->attemptingOrWorkingConnections && ! stopping) {
		/* No more connections so give a network error*/
		self->onNetworkError(self->callbackHandler,self);
	}
}

Version * getNetworkCommunicatorVersion(NetworkCommunicator * self, NetworkAddress * addRecv)
{
	assert(self != NULL);
	assert(addRecv != NULL);

	NetworkAddress * sourceAddr;
	if ((addRecv->type & IP_IPv6) && self->ourIPv6) {
		sourceAddr = self->ourIPv6;
	}
	else if (self->ourIPv4) {
		sourceAddr = self->ourIPv4;
	}

	self->nonce = rand();
	Version * version = createNewVersion(self->version, self->services, time(NULL), addRecv, sourceAddr, self->nonce, self->userAgent, self->blockHeight, self->onErrorReceived);
	return version;
}

OnMessageReceivedAction processMessageForAutoDiscovery(NetworkCommunicator * self, Peer * peer)
{
	assert(self!= NULL);
	assert(peer != NULL);

	if (peer->receive->type == MESSAGE_TYPE_ADDR) {
		/* Received addresses.*/
		AddressBroadcast * addrs = getAddressBroadcast(peer->receive);
		/* Only accept no timestaps when we have less than 100 addresses.*/
		if(peer->versionMessage->version < ADDR_TIME_VERSION
		   && numOfStoredAddresses(self->addresses) > 1000) {
			return MESSAGE_ACTION_CONTINUE;
		}

		/* Store addresses.*/
		uint64_t now = time(NULL) + self->addresses->networkTimeOffset;
		/* Loop through addresses*/
		boolean didAdd = FALSE; /* True when we add an address.*/

		uint16_t i;
		for (i = 0; i < addrs->addrNum; i++) {
			/* Check if we have the address as a connected peer*/
			Peer * peerB = getNetworkAddressFromAddressManager(self->addresses,addrs->addresses[i]);
			if(! peerB){
				/*Do not already have this address as a peer*/
				if (addrs->addresses[i]->type & IP_INVALID) {
					/* Address broadcasts should not contain invalid addresses.*/
					return MESSAGE_ACTION_DISCONNECT;
				}
				if (addrs->addresses[i]->type & IP_LOCAL && ! (getNetworkAddress(peer)->type & IP_LOCAL)) {
					/* Do not allow peers to send local addresses to non-local peers.*/
					return MESSAGE_ACTION_DISCONNECT;
				}
				/*Adjust time priority*/
				if (addrs->addresses[i]->score > now + 600) {
					/* Address is advertised more than ten minutes from now. Give low priority at 5 days ago */
					addrs->addresses[i]->score = (uint32_t)(now - 432000);  /* Needs fixing for integer overflow in the year 2106.*/
				}
				else if (addrs->addresses[i]->score > now - 900) { /*Sooner than fifteen minutes ago. Give highest priority.*/
					addrs->addresses[i]->score = (uint32_t)now;
				}

				/* Else leave the time*/
				/* Check if we have the address as a stored address*/
				NetworkAddress * addr = getNetworkAddressFromAddressManager(self->addresses,addrs->addresses[i]);
				if(addr){
					/* Already have the address. Modify the existing object.*/
					addr->score = addrs->addresses[i]->score;
					addr->services = addrs->addresses[i]->services;
					addr->version = addrs->addresses[i]->version;
				}else if (isReachableIPType(self->addresses, addrs->addresses[i]->type)){
					/* Do not have the address so add it if we believe we can reach it and if it is not us.*/
					if ((self->ourIPv4
						 && addrs->addresses[i]->type & IP_IPv4
						 && compareNetworkAddresses(self->ourIPv4, addrs->addresses[i]))
						|| (self->ourIPv6
							&& addrs->addresses[i]->type & IP_IPv6
							&& compareNetworkAddresses(self->ourIPv6, addrs->addresses[i])))
								/* Is us*/
								continue;
					/* Make copy of the address since otherwise we will corrupt the AddressBroadcast when trying the connection.*/
					NetworkAddress * copy = createNewNetworkAddress(addrs->addresses[i]->score, addrs->addresses[i]->ip, addrs->addresses[i]->port, addrs->addresses[i]->services, getMessage(addrs->addresses[i])->onErrorReceived);
					if (copy){
						addAddressToAddressManager(self->addresses, copy);
						decrementReferenceCount(copy);
						didAdd = TRUE;
					}else {
						return MESSAGE_ACTION_DISCONNECT;
					}
				}
			}else
				/* We have an advertised peer. This means it is public and should return to the address store.*/
				getNetworkAddress(peerB)->public = TRUE;
		}
		if (! peer->getAddresses && addrs->addrNum < 10) {
			/* Unsolicited addresses. Less than ten so relay to two random peers. bitcoin-qt does something weird. We wont. Keep it simple... Right ???*/
			uint16_t i, j;
			for ( i = rand() % (self->addresses->peersNum - 1),j = 0; i < self->addresses->peersNum && j < 2; i++,j++) {
				if (self->addresses->peers[i] == peer) {
					continue;
				}
				sendMessageToPeer(self, self->addresses->peers[i], getMessage(addrs));
			}
		}
		if (didAdd) {
			/* We have new address information so try connecting to addresses.*/
			tryConnectingToStoredAddresses(self);
		}
		/* Got addresses.*/
		peer->getAddresses = FALSE;
	} else if (peer->receive->type == MESSAGE_TYPE_GETADDR) {
		/* Give 33 peers with the highest times with a some randomisation added. Try connected peers first. Do not send empty addr.*/
		AddressBroadcast * addrBroadcast = createNewAddressBroadcast(self->version >= ADDR_TIME_VERSION && peer->versionMessage->version >= ADDR_TIME_VERSION, self->onErrorReceived);
		if (!addrBroadcast) {
			return MESSAGE_ACTION_DISCONNECT;
		}
		getMessage(addrBroadcast)->type = MESSAGE_TYPE_ADDR;

		/* Try connected peers. Only send peers that are public (private addresses are those which connect to us but haven't relayed their address).*/
		uint16_t peersSent = 0;
		uint16_t y = 0;
		while (peersSent < 28 && y < self->addresses->peersNum) { /* 28 to have room for 5 addresses.*/
			if (self->addresses->peers[y] != peer /* Not the peer we are sending to*/
				&& getNetworkAddress(self->addresses->peers[y])->public /* Public*/
				&& (getNetworkAddress(self->addresses->peers[y])->type != IP_LOCAL /* OK if not local*/
					|| getNetworkAddress(peer)->type == IP_LOCAL)) { /*               Or if the peer we are sending to is local*/
						if(!addNetworkAddressToAddressBroadcast(addrBroadcast, getNetworkAddress(self->addresses->peers[y]))){
							/* Memory problem. Free space by disconnecting node.*/
							decrementReferenceCount(addrBroadcast);
							return MESSAGE_ACTION_DISCONNECT;
						}
						peersSent++;
			}
			y++;
		}
		/* Now add stored addresses*/
		NetworkAddressLocator * addrs = getAddressesfromAddressManager(self->addresses, 33 - peersSent);
		uint8_t x;
		for (x = 0; (addrs + x)->addr != NULL; x++){
			if ((addrs + x)->addr->type != IP_LOCAL
				|| getNetworkAddress(peer)->type == IP_LOCAL)
				/* If the address is not local or if the peer we are sending to is local, the address is acceptable.*/
				if(! addNetworkAddressToAddressBroadcast(addrBroadcast, (addrs + x)->addr)){
					decrementReferenceCount(addrBroadcast);
					uint8_t y;
					for (y = x; (addrs + x)->addr != NULL; y++) {
						decrementReferenceCount((addrs + y)->addr);
					}
					return MESSAGE_ACTION_DISCONNECT;
				}
			decrementReferenceCount((addrs + x)->addr);
		}
		free(addrs);
		/* Send address broadcast, if we have at least one.*/
		if (addrBroadcast->addrNum) {
			sendMessageToPeer(self, peer, getMessage(addrBroadcast));
		}
		decrementReferenceCount(addrBroadcast);
	}

	/* Use opportunity to discover if we should broadcast our own addresses for receiving incoming connections.*/
	if (self->maxIncommingConnections /* Only share address if we allow for incoming connections.*/
		&& (self->isListeningIPv4 || self->isListeningIPv6) /* Share when we are listening for incoming connections.*/
		&& peer->timeBroadcast < time(NULL) - NUM_SECONDS_IN_A_DAY /*Every 24 hours*/
		&& peer->acceptedTypes & MESSAGE_TYPE_ADDR) { /* Only share address if they are allowed.*/

		peer->timeBroadcast = time(NULL);
		AddressBroadcast * addrBroadcast = createNewAddressBroadcast(self->version >= ADDR_TIME_VERSION && peer->versionMessage->version >= ADDR_TIME_VERSION, self->onErrorReceived);
		if (! addrBroadcast)
			return MESSAGE_ACTION_DISCONNECT;
		getMessage(addrBroadcast)->type = MESSAGE_TYPE_ADDR;
		if (self->ourIPv6) {
			self->ourIPv6->score = (uint32_t)time(NULL) + self->addresses->networkTimeOffset;
			if(!addNetworkAddressToAddressBroadcast(addrBroadcast,self->ourIPv6)){
				/* Memory issue*/
				decrementReferenceCount(addrBroadcast);
				return MESSAGE_ACTION_DISCONNECT;
			}
		}
		if (self->ourIPv4) {
			self->ourIPv4->score = (uint32_t)time(NULL) + self->addresses->networkTimeOffset;
			if(!addNetworkAddressToAddressBroadcast(addrBroadcast,self->ourIPv4)){
				decrementReferenceCount(addrBroadcast);
				return MESSAGE_ACTION_DISCONNECT;
			}
		}
		sendMessageToPeer(self, peer, getMessage(addrBroadcast));
		decrementReferenceCount(addrBroadcast);
	}
	return MESSAGE_ACTION_CONTINUE; /*Do not disconnect.*/
}

OnMessageReceivedAction processMessageForAutoHandshake(NetworkCommunicator * self, Peer * peer)
{
	assert(self != NULL);
	assert(peer != NULL);

	boolean endHandshake = FALSE;
	if (peer->receive->type == MESSAGE_TYPE_VERSION) {
		/* Check Node version and nounce. */
		if (getVersion(peer->receive)->version < MIN_PROTO_VERSION
			|| (getVersion(peer->receive)->nounce == self->nonce && self->nonce > 0))
			/* Disconnect peer */
			return MESSAGE_ACTION_DISCONNECT;
		else{
			/* Version OK; Check if we have a connection to this peer already */
			Peer * peerCheck = getNetworkAddressFromAddressManager(self->addresses, getVersion(peer->receive)->addSource);
			if (peerCheck && peerCheck != peer){
				/* One of the connections must now be dropped.
				 * To avoid both connections being closed an arbitrary comparison that can be shared between the nodes is used.
				 * The network addresses are compared for this purpose.
				 */
				int cmp;
				if (getVersion(peer->receive)->addSource->type & IP_IPv6){
					cmp = compareByteArrays(getVersion(peer->receive)->addSource->ip, self->ourIPv6->ip);
					if (!cmp)
						cmp = (getVersion(peer->receive)->addSource->port > self->ourIPv6->port) ? 1 : -1;
				}else{
					cmp = memcmp(getByteArrayData(getVersion(peer->receive)->addSource->ip) + 12,getByteArrayData(self->ourIPv4->ip) + 12,4);
					if (! cmp)
						cmp = (getVersion(peer->receive)->addSource->port > self->ourIPv4->port) ? 1 : -1;
				}
				if (cmp > 0)
					/* Disconnect this connection. */
					return MESSAGE_ACTION_DISCONNECT;
				else
					/* Disconnect the other connection. */
					disconnectPeer(self, peerCheck, 0, FALSE);
			}

			/* Remove the source address from the address manager if it has it.
			 * It will be added again if the peer wishes to broadcast their address.
			 * Now that the peer is connected, we identify it by the source address and do not want to try connections to the same address.
			 */

			removeNetworkAddressFromAddressManager(self->addresses, getNetworkAddress(getVersion(peer->receive)->addSource));
			/* Save version message */
			peer->versionMessage = getVersion(peer->receive);
			incrementReferenceCount(peer->versionMessage);

			/* Change version 10300 to 300 */
			if (peer->versionMessage->version == 10300) {
				peer->versionMessage->version = 300;
			}

			/* Copy over reported version and services for the NetworkAddress. */
			getNetworkAddress(peer)->version = peer->versionMessage->version;
			getNetworkAddress(peer)->services = peer->versionMessage->services;
			getNetworkAddress(peer)->score = (uint32_t)time(NULL); /* Loss of integer precision */

			/* Change port and IP number to the port and IP the peer wants us to recognize them with. */
			getNetworkAddress(peer)->port = peer->versionMessage->addSource->port;
			getNetworkAddress(peer)->ip = peer->versionMessage->addSource->ip;
			getNetworkAddress(peer)->type = getIPType(getByteArrayData(getNetworkAddress(peer)->ip));

			/* Adjust network time */
			peer->timeOffset = peer->versionMessage->time - time(NULL); /* Set the time offset for this peer.*/

			/* Disallow version from here. */
			peer->acceptedTypes &= ~MESSAGE_TYPE_VERSION;

			/* Send our acknowledgment */
			Message * ack = createNewMessage(self->onErrorReceived);
			if (!ack) {
				return MESSAGE_ACTION_DISCONNECT;
			}

			ack->type = MESSAGE_TYPE_VERACK;
			boolean ok = sendMessageToPeer(self,peer,ack);
			decrementReferenceCount(ack);
			if (!ok) {
				return MESSAGE_ACTION_DISCONNECT; /* If we cannot send an acknowledgment, exit. */
			}

			/* Send version next if we have not already. */
			if (!peer->versionSent) {
				Version * version = getNetworkCommunicatorVersion(self, getNetworkAddress(peer));
				if (version) {
					getMessage(version)->expectResponse = MESSAGE_TYPE_VERACK; /* Expect a response for this message. */
					peer->acceptedTypes = MESSAGE_TYPE_VERACK; /* Ony allow version acknowledgment from here. */
					getMessage(version)->type = MESSAGE_TYPE_VERSION;
					boolean ok = sendMessageToPeer(self, peer, getMessage(version));
					decrementReferenceCount(version);
					if (!ok) {
						return MESSAGE_ACTION_DISCONNECT;
					}
					peer->versionSent = TRUE;
				}else {
					return MESSAGE_ACTION_DISCONNECT;
				}
			}else {
				/* Sent already. End of handshake */
				endHandshake = TRUE;
			}
		}
	}else if (peer->receive->type == MESSAGE_TYPE_VERACK) {
		peer->versionAck = TRUE;
		peer->acceptedTypes &= ~MESSAGE_TYPE_VERACK; /* Got the version acknowledgment. Do not need it again */
		if (peer->versionMessage) {
			/* We already have their version message. That's all! */
			endHandshake = TRUE;
		}
		else {
			/* We need their version. */
			peer->acceptedTypes |= MESSAGE_TYPE_VERSION;
		}
	}
	if (endHandshake) {
		/* The end of the handshake. Allow pings, inventory advertisments, address requests, address broadcasts and alerts. */
		peer->acceptedTypes |= MESSAGE_TYPE_PING;
		peer->acceptedTypes |= MESSAGE_TYPE_INV;
		peer->acceptedTypes |= MESSAGE_TYPE_GETADDR;
		peer->acceptedTypes |= MESSAGE_TYPE_ADDR;
		peer->acceptedTypes |= MESSAGE_TYPE_ALERT;
		if ((self->flags & NETWORK_COMMUNICATOR_AUTO_PING)) {
			/* Request addresses */
			Message * getaddr = createNewMessage(self->onErrorReceived);
			if (! getaddr) {
				return  MESSAGE_ACTION_DISCONNECT;
			}
			getaddr->type = MESSAGE_TYPE_GETADDR;
			/* Send the message without an expected response.
			 * We do not expect an "addr" message because the peer may not send us anything if they do not have addresses to give.*/
			boolean ok = sendMessageToPeer(self,peer,getaddr);
			decrementReferenceCount(getaddr);
			if (!ok) {
				return MESSAGE_ACTION_DISCONNECT;
			}
			peer->getAddresses = TRUE;
		}
	}
	return MESSAGE_ACTION_CONTINUE;
}

OnMessageReceivedAction processMessageForAutoPingPongs(NetworkCommunicator * self, Peer * peer)
{
	assert(self != NULL);
	assert(peer != NULL);

	if (peer->receive->type == MESSAGE_TYPE_PING
		&& peer->versionMessage->version >= PONG_VERSION
		&& self->version >= PONG_VERSION){
		peer->receive->type = MESSAGE_TYPE_PONG;
		if(!sendMessageToPeer(self, peer, peer->receive)) {
			return MESSAGE_ACTION_DISCONNECT;
		}
	} else if (peer->receive->type == MESSAGE_TYPE_PONG){
		if(self->version < PONG_VERSION || peer->versionMessage->version < PONG_VERSION) {
			return MESSAGE_ACTION_DISCONNECT; /* This peer should not be sending pong messages. */
		}
		/* No longer want a pong */
		peer->acceptedTypes &= ~MESSAGE_TYPE_PONG;
	}
	return MESSAGE_ACTION_CONTINUE;
}

void onPeerReadyForReadingCallback(void * vself, void * vpeer)
{
	assert(vself != NULL);
	assert(vpeer != NULL);

	NetworkCommunicator * self = vself;
	Peer * peer = vpeer;
	/* Node kindly has some data available in the socket buffer. */

	if (!peer->receive) {
		/* New message to be received. */
		peer->receive = createNewMessage(self->onErrorReceived);
		if (!peer->receive) {
			disconnectPeer(self, peer, 0, FALSE);
			return;
		}
		peer->headerBuffer = malloc(24); /* Twenty-four bytes for the message header. */
		if (!peer->headerBuffer) {
			/* If we run out of memory we want to disconnect as nothing more can be done here. */
			disconnectPeer(self, peer, 0, FALSE);
			return;
		}
		peer->messageReceived = 0; /* So far received nothing. */
		socketAddEvent(peer->receiveEvent, self->recvTimeOut); /* From now on use timeout for receiving data. FIXME*/
		/* Start download timer */
		peer->downloadTimerStart = clock();
	}
	if (!peer->receivedHeader) { /* Not received the complete message header yet. */
		int32_t num = socketReceive(peer->socketID,	peer->headerBuffer + peer->messageReceived,	24 - peer->messageReceived); /*FIXME*/
		switch (num) {
			case SOCKET_CONNECTION_CLOSE:
				free(peer->headerBuffer); /* Free the buffer */
				disconnectPeer(self, peer, 7200, FALSE); /* Remove with penalty for disconnection */
				return;
			case SOCKET_FAILURE:
				/* Failure so remove peer. */
				free(peer->headerBuffer); /* Free the buffer */
				disconnectPeer(self, peer, 0, FALSE);
				return;
			default:
				/* Did read some bytes */
				peer->messageReceived += num;
				if (peer->messageReceived == 24)
					/* The header has been received. */
					onHeaderReceivedCallback(self, peer);
				return;
		}
	} else {
		/* Means we have payload to fetch as we have the header but not the payload */
		int32_t num = socketReceive(peer->socketID,	getByteArrayData(peer->receive->bytes)	+ peer->messageReceived, peer->receive->bytes->length - peer->messageReceived);
		switch (num) {
			case SOCKET_CONNECTION_CLOSE:
				disconnectPeer(self, peer, 7200, FALSE); /* Remove with penalty for disconnection */
				return;
			case SOCKET_FAILURE:
				/* Failure so remove peer. */
				disconnectPeer(self, peer, 0, FALSE);
				return;
			default:
				/* Did read some bytes */
				peer->messageReceived += num;
				if (peer->messageReceived == peer->receive->bytes->length) {
					/* We now have the message. */
					onMessageReceivedCallback(self, peer);
				}
				return;
		}
	}
}

void onPeerReadyForWritingCallback(void * vself, void * vpeer)
{
	assert(vself != NULL);
	assert(vpeer != NULL);

	NetworkCommunicator * self = vself;
	Peer * peer = vpeer;
	boolean finished = FALSE;

	/* Once this event is called, it is possible to write data to the peer */
	if (!peer->sentHeader) {
		/* Check to see if header has to be sent */
		if (!peer->messageSent) {
			/* Create header */
			peer->sendingHeader = malloc(24);
			if (!peer->sendingHeader) {
				disconnectPeer(self, peer, 0, FALSE);
				return;
			}

			peer->sendingHeader[0] = self->networkID;
			peer->sendingHeader[1] = self->networkID >> 8;
			peer->sendingHeader[2] = self->networkID >> 16;
			peer->sendingHeader[3] = self->networkID >> 24;

			/* Get the message we are sending. */
			Message * toSend = peer->sendQueue[peer->sendQueueFront];
			/* Message type text */
			switch (toSend->type) {
				case MESSAGE_TYPE_VERSION:
					memcpy(peer->sendingHeader + 4, "version\0\0\0\0\0", 12);
					break;
				case MESSAGE_TYPE_VERACK:
					memcpy(peer->sendingHeader + 4, "verack\0\0\0\0\0\0", 12);
					break;
				case MESSAGE_TYPE_ADDR:
					memcpy(peer->sendingHeader + 4, "addr\0\0\0\0\0\0\0\0", 12);
					break;
				case MESSAGE_TYPE_INV:
					memcpy(peer->sendingHeader + 4, "inv\0\0\0\0\0\0\0\0\0", 12);
					break;
				case MESSAGE_TYPE_GETDATA:
					memcpy(peer->sendingHeader + 4, "getdata\0\0\0\0\0", 12);
					break;
				case MESSAGE_TYPE_GETBLOCKS:
					memcpy(peer->sendingHeader + 4, "getBlocks\0\0\0", 12);
					break;
				case MESSAGE_TYPE_GETHEADERS:
					memcpy(peer->sendingHeader + 4, "getheaders\0\0", 12);
					break;
				case MESSAGE_TYPE_TX:
					memcpy(peer->sendingHeader + 4, "tx\0\0\0\0\0\0\0\0\0\0", 12);
					break;
				case MESSAGE_TYPE_BLOCK:
					memcpy(peer->sendingHeader + 4, "block\0\0\0\0\0\0\0", 12);
					break;
				case MESSAGE_TYPE_HEADERS:
					memcpy(peer->sendingHeader + 4, "headers\0\0\0\0\0", 12);
					break;
				case MESSAGE_TYPE_GETADDR:
					memcpy(peer->sendingHeader + 4, "getaddr\0\0\0\0\0", 12);
					break;
				case MESSAGE_TYPE_PING:
					memcpy(peer->sendingHeader + 4, "ping\0\0\0\0\0\0\0\0", 12);
					break;
				case MESSAGE_TYPE_PONG:
					memcpy(peer->sendingHeader + 4, "pong\0\0\0\0\0\0\0\0", 12);
					break;
				case MESSAGE_TYPE_ALERT:
					memcpy(peer->sendingHeader + 4, "alert\0\0\0\0\0\0\0", 12);
					break;
				default:
					memcpy(peer->sendingHeader + 4, toSend->altText, 12);
					break;
			}

			/* Length */
			if (toSend->bytes){
				peer->sendingHeader[16] = toSend->bytes->length;
				peer->sendingHeader[17] = toSend->bytes->length >> 8;
				peer->sendingHeader[18] = toSend->bytes->length >> 16;
				peer->sendingHeader[19] = toSend->bytes->length >> 24;
			}else{
				memset(peer->sendingHeader + 16, 0, 4);
			}

			/* Checksum */
			peer->sendingHeader[20] = toSend->checksum[0];
			peer->sendingHeader[21] = toSend->checksum[1];
			peer->sendingHeader[22] = toSend->checksum[2];
			peer->sendingHeader[23] = toSend->checksum[3];
		}

		int32_t len = socketSend(peer->socketID, peer->sendingHeader + peer->messageSent, 24 - peer->messageSent); /*FIXME*/
		if (len == SOCKET_FAILURE) {
			free(peer->sendingHeader);
			disconnectPeer(self, peer, 0, FALSE);
		} else {
			peer->messageSent += len;
			if (peer->messageSent == 24) {
				/* Done header */
				free(peer->sendingHeader);
				if (peer->sendQueue[peer->sendQueueFront]->bytes) {
					/* Now send the bytes */
					peer->messageSent = 0;
					peer->sentHeader = TRUE;
				}else{
					/* Nothing more to send! */
					finished = TRUE;
				}
			}
		}
	}else{
		/* Sent header */
		int32_t len = socketSend(peer->socketID, getByteArrayData(peer->sendQueue[peer->sendQueueFront]->bytes) + peer->messageSent, peer->sendQueue[peer->sendQueueFront]->bytes->length - peer->messageSent);/*FIXME*/
		if (len == SOCKET_FAILURE) {
			disconnectPeer(self, peer, 0, FALSE);
		}
		else{
			peer->messageSent += len;
			if (peer->messageSent == peer->sendQueue[peer->sendQueueFront]->bytes->length) {
				/* Sent the entire payload. */
				finished = TRUE;
			}
		}
	}
	if (finished) {
		/* Reset variables for next send. */
		peer->messageSent = 0;
		peer->sentHeader = FALSE;
		/* Done sending message. */
		if (peer->sendQueue[peer->sendQueueFront]->expectResponse){
			socketAddEvent(peer->receiveEvent, self->responseTimeOut); /* Expect response. */
			/* Add to list of expected responses. */
			if (peer->typesExpectedNum == MAX_RESPONSES_EXPECTED) {
				disconnectPeer(self, peer, NUM_SECONDS_IN_A_DAY, FALSE);
				return;
			}
			peer->typesExpected[peer->typesExpectedNum] = peer->sendQueue[peer->sendQueueFront]->expectResponse;
			peer->typesExpectedNum++;
			/* Start timer for latency measurement, if not already waiting. */
			if (!peer->latencyTimerStart){
				peer->latencyTimerStart = clock();
				peer->latencyExpected = peer->sendQueue[peer->sendQueueFront]->expectResponse;
			}
		}

		/* Remove message from queue.*/
		peer->sendQueueSize--;
		decrementReferenceCount(peer->sendQueue[peer->sendQueueFront]);
		if (peer->sendQueueSize) {
			peer->sendQueueFront++;
			if (peer->sendQueueFront == SEND_QUEUE_MAX_SIZE) {
				peer->sendQueueFront = 0;
			}
		}else{
			/* Remove send event as we have nothing left to send */
			socketRemoveEvent(peer->sendEvent); /*FIXME*/
		}
	}
}

void onHeaderReceivedCallback(NetworkCommunicator * self, Peer * peer)
{
	assert(self != NULL);
	assert(peer != NULL);

	/* Make a ByteArray. ??? Could be modified not to use a ByteArray, but it is cleaner this way and easier to maintain. */
	ByteArray * header = createNewByteArrayFromData(peer->headerBuffer, 24, self->onErrorReceived);
	if (!header){
		disconnectPeer(self, peer, 0, FALSE);
	}

	if (readInt32AsLittleEndianFromByteArray(header, 0) != self->networkID){
		/* The network ID bytes is not what we are looking for. We will have to remove the peer. */
		decrementReferenceCount(header);
		disconnectPeer(self, peer, NUM_SECONDS_IN_A_DAY, FALSE);
	}

	MessageType type;
	uint32_t size;
	boolean error = FALSE;
	ByteArray * typeBytes = createNewByteArraySubsectionReference(header, 4, 12);
	if (!typeBytes){
		decrementReferenceCount(header);
		disconnectPeer(self, peer, 0, FALSE);
	}

	/*TODO: Change to case/select statements/constructs*/
	if (!memcmp(getByteArrayData(typeBytes),"version\0\0\0\0\0",12)) {
		/* Version message: Check if we are accepting version messages at the moment. */
		if (peer->acceptedTypes & MESSAGE_TYPE_VERSION){
			/* Check payload size */
			size = readInt32AsLittleEndianFromByteArray(header, 16);
			if (size > VERSION_MAX_SIZE) { /* Illegal size. cbitcoin will not accept this. */
				error = TRUE;
			}
			else {
				type = MESSAGE_TYPE_VERSION;
			}
		}else {
			error = TRUE;
		}
	}else if (! memcmp(getByteArrayData(typeBytes), "verack\0\0\0\0\0\0", 12)){
		/* Version acknowledgement message */
		if (peer->acceptedTypes & MESSAGE_TYPE_VERACK){
			size = readInt32AsLittleEndianFromByteArray(header, 16);
			if (size) { /* Must contain zero payload. */
				error = TRUE;
			}
			else {
				type = MESSAGE_TYPE_VERACK;
			}
		}else {
			error = TRUE;
		}
	}else if (!memcmp(getByteArrayData(typeBytes), "addr\0\0\0\0\0\0\0\0", 12)){
		/* Address broadcast message */
		if (peer->acceptedTypes & MESSAGE_TYPE_ADDR){
			size = readInt32AsLittleEndianFromByteArray(header, 16);
			if (size > ADDRESS_BROADCAST_MAX_SIZE) {
				error = TRUE;
			}
			else {
				type = MESSAGE_TYPE_ADDR;
			}
		}else {
			error = TRUE;
		}
	}else if (!memcmp(getByteArrayData(typeBytes), "inv\0\0\0\0\0\0\0\0\0", 12)){
		/* Inventory broadcast message */
		if (peer->acceptedTypes & MESSAGE_TYPE_INV){
			size = readInt32AsLittleEndianFromByteArray(header, 16);
			if (size > INVENTORY_BROADCAST_MAX_SIZE) {
				error = TRUE;
			}
			else {
				type = MESSAGE_TYPE_INV;
			}
		}else {
			error = TRUE;
		}
	}else if (!memcmp(getByteArrayData(typeBytes), "getdata\0\0\0\0\0", 12)){
		/* Get data message */
		if (peer->acceptedTypes & MESSAGE_TYPE_GETDATA){
			size = readInt32AsLittleEndianFromByteArray(header, 16);
			if (size > GET_DATA_MAX_SIZE) {
				error = TRUE;
			}
			else {
				type = MESSAGE_TYPE_GETDATA;
			}
		}else {
			error = TRUE;
		}
	}else if (!memcmp(getByteArrayData(typeBytes), "getBlocks\0\0\0", 12)){
		/* Get blocks message */
		if (peer->acceptedTypes & MESSAGE_TYPE_BLOCK){
			size = readInt32AsLittleEndianFromByteArray(header, 16);
			if (size > GET_BLOCKS_MAX_SIZE) {
				error = TRUE;
			}
			else {
				type = MESSAGE_TYPE_GETBLOCKS;
			}
		}else {
			error = TRUE;
		}
	}else if (!memcmp(getByteArrayData(typeBytes), "getheaders\0\0", 12)){
		/* Get headers message */
		if (peer->acceptedTypes & MESSAGE_TYPE_GETHEADERS){
			size = readInt32AsLittleEndianFromByteArray(header, 16);
			if (size > GET_HEADERS_MAX_SIZE) {
				error = TRUE;
			}
			else {
				type = MESSAGE_TYPE_GETHEADERS;
			}
		}else {
			error = TRUE;
		}
	}else if (!memcmp(getByteArrayData(typeBytes), "tx\0\0\0\0\0\0\0\0\0\0", 12)){
		/* Transaction message */
		if (peer->acceptedTypes & MESSAGE_TYPE_TX){
			size = readInt32AsLittleEndianFromByteArray(header, 16);
			if (size > TRANSACTION_MAX_SIZE) {
				error = TRUE;
			}
			else {
				type = MESSAGE_TYPE_TX;
			}
		}else {
			error = TRUE;
		}
	}else if (! memcmp(getByteArrayData(typeBytes), "block\0\0\0\0\0\0\0", 12)){
		/* Block message */
		if (peer->acceptedTypes & MESSAGE_TYPE_BLOCK){
			size = readInt32AsLittleEndianFromByteArray(header, 16);
			if (size > BLOCK_MAX_SIZE + 89) { /* Plus 89 for header. */
				error = TRUE;
			}
			else {
				type = MESSAGE_TYPE_BLOCK;
			}
		}else {
			error = TRUE;
		}
	}else if (! memcmp(getByteArrayData(typeBytes), "headers\0\0\0\0\0", 12)){
		/* Block headers message */
		if (peer->acceptedTypes & MESSAGE_TYPE_HEADERS){
			size = readInt32AsLittleEndianFromByteArray(header, 16);
			if (size > BLOCK_HEADERS_MAX_SIZE) {
				error = TRUE;
			}
			else {
				type = MESSAGE_TYPE_HEADERS;
			}
		}else {
			error = TRUE;
		}
	}else if (!memcmp(getByteArrayData(typeBytes), "getaddr\0\0\0\0\0", 12)){
		/* Get Addresses message */
		if (peer->acceptedTypes & MESSAGE_TYPE_GETADDR){
			size = readInt32AsLittleEndianFromByteArray(header, 16);
			if (size) { /* Should be empty */
				error = TRUE;
			}
			else {
				type = MESSAGE_TYPE_GETADDR;
			}
		}else {
			error = TRUE;
		}
	}else if (!memcmp(getByteArrayData(typeBytes), "ping\0\0\0\0\0\0\0\0", 12)){
		/* Ping message */
		if (peer->acceptedTypes & MESSAGE_TYPE_PING){
			size = readInt32AsLittleEndianFromByteArray(header, 16);
			if (size > 8 || ((peer->versionMessage->version < PONG_VERSION || self->version < PONG_VERSION) && size)) {
			/*Should be empty before version 60000 or 8 bytes. */
				error = TRUE;
			}
			else {
				type = MESSAGE_TYPE_PING;
			}
		}else {
			error = TRUE;
		}
	}else if (!memcmp(getByteArrayData(typeBytes), "pong\0\0\0\0\0\0\0\0", 12)){
		/* Pong message */
		if (peer->acceptedTypes & MESSAGE_TYPE_PONG){
			size = readInt32AsLittleEndianFromByteArray(header, 16);
			if (size > 8) {
				error = TRUE;
			}
			else {
				type = MESSAGE_TYPE_PONG;
			}
		}else {
			error = TRUE;
		}
	}else if (!memcmp(getByteArrayData(typeBytes), "alert\0\0\0\0\0\0\0", 12)){
		/* Alert message*/
		if (peer->acceptedTypes & MESSAGE_TYPE_ALERT){
			size = readInt32AsLittleEndianFromByteArray(header, 16);
			if (size > ALERT_MAX_SIZE) {
				error = TRUE;
			}
			else {
				type = MESSAGE_TYPE_ALERT;
			}
		}else {
			error = TRUE;
		}
	}else{
		/* Either alternative message or invalid */
		error = TRUE; /* Default error until valid alternative message found. */
		if (self->alternativeMessages) {
			int i = 0;
			for (; i < self->alternativeMessages->length / 12; i++) {
				/* Check this alternative message */
				if (!memcmp(getByteArrayData(typeBytes), getByteArrayData(self->alternativeMessages) + 12 * i, 12)){
					/* Alternative message */
					type = MESSAGE_TYPE_ALT;
					/* Check payload size */
					size = readInt32AsLittleEndianFromByteArray(header, 16);
					uint32_t altSize;
					if (self->altMaxSizes) {
						altSize = self->altMaxSizes[i];
					}
					else {
						altSize = BLOCK_MAX_SIZE;
					}
					if (size <= altSize) { /* Should correspond to the user given value */
						error = FALSE;
					}
					break;
				}
			}
		}
	}
	decrementReferenceCount(typeBytes);
	if (error) {
		/* Error with the message header type or length */
		disconnectPeer(self, peer, NUM_SECONDS_IN_A_DAY, FALSE);
		decrementReferenceCount(header);
	}

	/* If this is a response we have been waiting for, no longer wait for it */
	int i = 0;
	for (; i < peer->typesExpectedNum; i++) {
		if (type == peer->typesExpected[i]) {
			/* Record latency */
			if (peer->latencyTimerStart && peer->latencyExpected == type) {
				peer->latencyTime += clock() - peer->latencyTimerStart;
				peer->responses++;
				peer->latencyTimerStart = 0;
			}

			/* Remove */
			boolean remove = TRUE;
			if (self->flags & NETWORK_COMMUNICATOR_AUTO_HANDSHAKE)
				if (type == MESSAGE_TYPE_VERACK && ! peer->versionMessage){
					/* We expect the version */
					peer->typesExpected[i] = MESSAGE_TYPE_VERSION;
					remove = FALSE;
				}
			if (remove) {
				peer->typesExpectedNum--;
				memmove(peer->typesExpected + i, peer->typesExpected + i + 1, peer->typesExpectedNum - i);
			}
			break;
		}
	}

	/* The type and size is OK, make the messages */
	peer->receive->type = type;
	/* Get checksum */
	peer->receive->checksum[0] = getByteFromByteArray(header, 20);
	peer->receive->checksum[1] = getByteFromByteArray(header, 21);
	peer->receive->checksum[2] = getByteFromByteArray(header, 22);
	peer->receive->checksum[3] = getByteFromByteArray(header, 23);

	/* Message is now ready. Free the header. */
	decrementReferenceCount(header); /* Took the header buffer which should be freed here. */
	if (size) {
		peer->receive->bytes = createNewByteArrayOfSize(size,self->onErrorReceived);
		if (! peer->receive->bytes) {
			disconnectPeer(self, peer, 0, FALSE);
		}
		/* Change variables for receiving the payload. */
		peer->receivedHeader = TRUE;
		peer->messageReceived = 0;
	}else { /* Got the message */
		onMessageReceivedCallback(self,peer);
	}
}

void onSocketEventLoopErrorCallback(void * vself)
{
	assert(vself != NULL);

	NetworkCommunicator * self = vself;
	getMessage(self)->onErrorReceived(ERROR_NETWORK_COMMUNICATOR_LOOP_FAIL,"The socket event loop failed. Stopping the NetworkCommunicator...");
	self->eventLoop = 0;
	stopNetworkCommunicator(self);
}

void onMessageReceivedCallback(NetworkCommunicator * self, Peer * peer)
{
	assert(self != NULL);
	assert(peer != NULL);

	/* Record download time */
	peer->downloadTime += clock() - peer->downloadTimerStart;
	peer->downloadAmount += 24 + (peer->receive->bytes ? peer->receive->bytes->length : 0);

	/* Change timeout of receive event, depending on wHether or not we want more responses. */
	socketAddEvent(peer->receiveEvent,peer->typesExpectedNum ? self->responseTimeOut : self->timeOut); /*FIXME*/

	/* Check checksum*/
	uint8_t hash[32];
	uint8_t hash2[32];
	if (peer->receive->bytes) {
		Sha256(getByteArrayData(peer->receive->bytes), peer->receive->bytes->length, hash);
		Sha256(hash, 32, hash2);
	}else{
		hash2[0] = 0x5D;
		hash2[1] = 0xF6;
		hash2[2] = 0xE0;
		hash2[3] = 0xE2;
	}
	if (memcmp(hash2, peer->receive->checksum, 4)) {
		/* Checksum failure. There is no excuse for this. Drop the peer. Why have checksums anyway??? */
		disconnectPeer(self, peer, NUM_SECONDS_IN_A_DAY, FALSE);
		return;
	}
	/* Deserialise and give the onMessageReceived or onAlternativeMessageReceived event */
	uint32_t len;
	switch (peer->receive->type) {
		case MESSAGE_TYPE_VERSION:
			peer->receive = realloc(peer->receive, sizeof(Version)); /* For storing additional data */
			if (!peer->receive) {
				disconnectPeer(self, peer, 0, FALSE);
				return;
			}
			getObject(peer->receive)->destroy = destroyVersion;
			len = deserializeVersion(getVersion(peer->receive));
			break;
		case MESSAGE_TYPE_ADDR:
			peer->receive = realloc(peer->receive, sizeof(AddressBroadcast));
			if (! peer->receive) {
				disconnectPeer(self, peer, 0, FALSE);
				return;
			}
			getObject(peer->receive)->destroy = destroyAddressBroadcast;
			getAddressBroadcast(peer->receive)->timeStamps = peer->versionMessage->version > 31401; /* Timestamps start version 31402 and up. */
			len = deserialiseAddressBroadcast(getAddressBroadcast(peer->receive));
			break;
		case MESSAGE_TYPE_INV:
		case MESSAGE_TYPE_GETDATA:
			peer->receive = realloc(peer->receive, sizeof(Inventory));
			if (! peer->receive) {
				disconnectPeer(self, peer, 0, FALSE);
				return;
			}
			getObject(peer->receive)->destroy = destroyInventory;
			len = deserializeInventory(getInventory(peer->receive));
			break;
		case MESSAGE_TYPE_GETBLOCKS:
		case MESSAGE_TYPE_GETHEADERS:
			peer->receive = realloc(peer->receive, sizeof(GetBlocks));
			if (! peer->receive) {
				disconnectPeer(self, peer, 0, FALSE);
				return;
			}
			getObject(peer->receive)->destroy = destroyGetBlocks;
			len = deserializeGetBlocks(retrieveGetBlocks(peer->receive));
			break;
		case MESSAGE_TYPE_TX:
			peer->receive = realloc(peer->receive, sizeof(Transaction));
			if (!peer->receive) {
				disconnectPeer(self, peer, 0, FALSE);
				return;
			}
			getObject(peer->receive)->destroy = destroyTransaction;
			len = deserializeTransaction(getTransaction(peer->receive));
			break;
		case MESSAGE_TYPE_BLOCK:
			peer->receive = realloc(peer->receive, sizeof(Block));
			if (! peer->receive) {
				disconnectPeer(self, peer, 0, FALSE);
				return;
			}
			getObject(peer->receive)->destroy = destroyBlock;
			len = deserializeBlock(getBlock(peer->receive),TRUE); /* TRUE -> Including transactions. */
			break;
		case MESSAGE_TYPE_HEADERS:
			peer->receive = realloc(peer->receive, sizeof(BlockHeaders));
			if (! peer->receive) {
				disconnectPeer(self, peer, 0, FALSE);
				return;
			}
			getObject(peer->receive)->destroy = destroyBlockHeaders;
			len = deserializeBlockHeaders(getBlockHeaders(peer->receive));
			break;
		case MESSAGE_TYPE_PING:
			if (peer->versionMessage->version >= 60000 && self->version >= 60000){
				peer->receive = realloc(peer->receive, sizeof(PingPongMessage));
				if (! peer->receive) {
					disconnectPeer(self, peer, 0, FALSE);
					return;
				}
				getObject(peer->receive)->destroy = destroyPingPongMessage;
				len = deserializePingPongMessage(getPingPongMessage(peer->receive));
			}else {
				len = 0;
			}
		case MESSAGE_TYPE_PONG:
			peer->receive = realloc(peer->receive, sizeof(PingPongMessage));
			if (! peer->receive) {
				disconnectPeer(self, peer, 0, FALSE);
				return;
			}
			getObject(peer->receive)->destroy = destroyPingPongMessage;
			len = deserializePingPongMessage(getPingPongMessage(peer->receive));
			break;
		case MESSAGE_TYPE_ALERT:
			peer->receive = realloc(peer->receive, sizeof(Alert));
			if (! peer->receive) {
				disconnectPeer(self, peer, 0, FALSE);
				return;
			}
			getObject(peer->receive)->destroy = destroyAlert;
			len = deserializeAlert(getAlert(peer->receive));
			break;
		default:
			len = 0; /* Zero default */
			break;
	}
	/* Check that the deserialised message's size read matches the message length. */
	if (!peer->receive->bytes) {
		if (len) {
			/* Error */
			disconnectPeer(self, peer, NUM_SECONDS_IN_A_DAY, FALSE);
			return;
		}
	}else if (len != peer->receive->bytes->length){
		/* Error */
		disconnectPeer(self, peer, NUM_SECONDS_IN_A_DAY, FALSE);
		return;
	}

	/* Deserialization was sucessful. */
	/* Call event callback */
	OnMessageReceivedAction action = self->onMessageReceived(self->callbackHandler,self,peer);
	if (action == MESSAGE_ACTION_CONTINUE) {
		/* Automatic responses */
		if (self->flags & NETWORK_COMMUNICATOR_AUTO_HANDSHAKE)
			/* Auto handshake responses */
			action = processMessageForAutoHandshake(self,peer);
		if ((self->flags & NETWORK_COMMUNICATOR_AUTO_DISCOVERY) && action == MESSAGE_ACTION_CONTINUE)
			/* Auto discovery responses */
			action = processMessageForAutoDiscovery(self,peer);
		if ((self->flags & NETWORK_COMMUNICATOR_AUTO_PING) && action == MESSAGE_ACTION_CONTINUE)
			/* Auto ping response */
			action = processMessageForAutoPingPongs(self, peer);
	}

	/* Release objects and get ready for next message */
	switch (action) {
		case MESSAGE_ACTION_DISCONNECT:
			/* Node misbehaving. Disconnect. */
			disconnectPeer(self, peer, NUM_SECONDS_IN_A_DAY, FALSE);
			break;
		case MESSAGE_ACTION_STOP:
			stopNetworkCommunicator(self);
			break;
		case MESSAGE_ACTION_CONTINUE:
			decrementReferenceCount(peer->receive);
			/* Reset variables for receiving the next message */
			peer->receivedHeader = FALSE;
			peer->receive = NULL;
		default:
			break;
	}
}
void onTimeOutErrorCallback(void * vself, void * vpeer, TimeOutType type)
{
	assert(vself != NULL);
	assert(vpeer != NULL);

	NetworkCommunicator * self = vself;
	Peer * peer = vpeer;

	if (type == TIMEOUT_RECEIVE && !peer->messageReceived && !peer->receivedHeader) {
		/* Not responded */
		if (peer->typesExpectedNum) {
			type = TIMEOUT_RESPONSE;
		}
		else {
			/* No response expected but peer did not send any information for a long time. */
			type = TIMEOUT_NO_DATA;
		}
	}

	if (getNetworkAddress(peer)->public && getNetworkAddress(peer)->ip){ /* Cannot take peer as address in the case where the IP is NULL */
		incrementReferenceCount(peer); /* Increment reference count for peer for returning to addresses list */
		disconnectPeer(self,peer, NUM_SECONDS_IN_A_DAY, FALSE); /* Remove NetworkAddress. 1 day penalty. */
	}else
		disconnectPeer(self,peer, NUM_SECONDS_IN_A_DAY, FALSE); /* Remove NetworkAddress. 1 day penalty. */
	/* Send event for network timeout. The peer will be NULL if it wasn't retained elsewhere. */
	self->onTimeOut(self->callbackHandler, self, peer, type);
}

boolean sendMessageToPeer(NetworkCommunicator * self, Peer * peer, Message * message)
{
	assert(self != NULL);
	assert(peer != NULL);

	if (peer->sendQueueSize == SEND_QUEUE_MAX_SIZE) {
		return FALSE;
	}

	/* Serialise message if needed. */
	if (!message->bytes) {
		uint32_t len;
		switch (message->type) {
			case MESSAGE_TYPE_VERSION:
				len = calculateVersionLength(getVersion(message));
				if (!len) {
					return FALSE;
				}
				message->bytes = createNewByteArrayOfSize(len, self->onErrorReceived);
				if (!message->bytes) {
					return FALSE;
				}
				len = serializeVersion(getVersion(message));
				break;
			case MESSAGE_TYPE_ADDR:
				/* "lengthOfAddressBroadcast" cannot fail. */
				message->bytes = createNewByteArrayOfSize(lengthOfAddressBroadcast(getAddressBroadcast(message)), self->onErrorReceived);
				if (! message->bytes) {
					return FALSE;
				}
				len = serialiseAddressBroadcast(getAddressBroadcast(message));
				break;
			case MESSAGE_TYPE_INV:
			case MESSAGE_TYPE_GETDATA:
				/* "calculateInventoryLength" cannot fail. */
				message->bytes = createNewByteArrayOfSize(calculateInventoryLength(getInventory(message)), self->onErrorReceived);
				if (! message->bytes) {
					return FALSE;
				}
				len = serializeInventory(getInventory(message));
				break;
			case MESSAGE_TYPE_GETBLOCKS:
			case MESSAGE_TYPE_GETHEADERS:
				/* "calculateGetBlocksLength" cannot fail. */
				message->bytes = createNewByteArrayOfSize(calculateGetBlocksLength(retrieveGetBlocks(message)), self->onErrorReceived);
				if (! message->bytes) {
					return FALSE;
				}
				len = serializeGetBlocks(retrieveGetBlocks(message));
				break;
			case MESSAGE_TYPE_TX:
				len = calculateTransactionLength(getTransaction(message));
				if (!len) {
					return FALSE;
				}
				message->bytes = createNewByteArrayOfSize(len, self->onErrorReceived);
				if (!message->bytes) {
					return FALSE;
				}
				len = serializeTransaction(getTransaction(message));
				break;
			case MESSAGE_TYPE_BLOCK:
				len = calculateBlockLength(getBlock(message),TRUE);
				if (!len) {
					return FALSE;
				}
				message->bytes = createNewByteArrayOfSize(len, self->onErrorReceived);
				if (!message->bytes) {
					return FALSE;
				}
				len = serializeBlock(getBlock(message),TRUE); /* TRUE -> Including transactions. */
				break;
			case MESSAGE_TYPE_HEADERS:
				/* "calculateBlockHeadersLength" cannot fail. */
				message->bytes = createNewByteArrayOfSize(calculateBlockHeadersLength(getBlockHeaders(message)), self->onErrorReceived);
				if (!message->bytes) {
					return FALSE;
				}
				len = serializeBlockHeaders(getBlockHeaders(message));
				break;
			case MESSAGE_TYPE_PING:
				if (peer->versionMessage->version >= 60000 && self->version >= 60000){
					message->bytes = createNewByteArrayOfSize(8, self->onErrorReceived);
					if (!message->bytes) {
						return FALSE;
					}
					len = serializePingPongMessage(getPingPongMessage(message));
				}
			case MESSAGE_TYPE_PONG:
				message->bytes = createNewByteArrayOfSize(8, self->onErrorReceived);
				if (!message->bytes) {
					return FALSE;
				}
				len = serializePingPongMessage(getPingPongMessage(message));
				break;
			case MESSAGE_TYPE_ALERT:
				/* This should have been serialized before! */
				return FALSE;
				break;
			default:
				break;
		}
		if (message->bytes) {
			if (message->bytes->length != len)
				return FALSE;
		}
	}

	if (message->bytes) {
		/* Make checksum */
		uint8_t hash[32];
		uint8_t hash2[32];
		Sha256(getByteArrayData(message->bytes), message->bytes->length, hash);
		Sha256(hash, 32, hash2);
		message->checksum[0] = hash2[0];
		message->checksum[1] = hash2[1];
		message->checksum[2] = hash2[2];
		message->checksum[3] = hash2[3];
	}else{
		/* Empty bytes checksum */
		message->checksum[0] = 0x5D;
		message->checksum[1] = 0xF6;
		message->checksum[2] = 0xE0;
		message->checksum[3] = 0xE2;
	}

	peer->sendQueue[(peer->sendQueueFront + peer->sendQueueSize) % SEND_QUEUE_MAX_SIZE] = message; /* Node will send the message from this. */
	if (peer->sendQueueSize == 0) {
		boolean ok = socketAddEvent(peer->sendEvent, self->sendTimeOut); /*FIXME*/
		if (!ok)
			return FALSE;
	}
	peer->sendQueueSize++;
	incrementReferenceCount(message);
	return TRUE;
}
void pingConnectedPeers(void * vself)
{
	assert(vself != NULL);

	NetworkCommunicator * self = vself;
	Message * ping = createNewMessage(self->onErrorReceived);
	if (!ping) {
		return;
	}

	ping->type = MESSAGE_TYPE_PING;

	if (self->version >= PONG_VERSION) {
		PingPongMessage * pingPong = createNewPingPongMessage(rand(), self->onErrorReceived);
		getMessage(pingPong)->type = MESSAGE_TYPE_PING;

		int i = 0;
		for (; i < self->addresses->peersNum; i++) {
			if (self->addresses->peers[i]->acceptedTypes & MESSAGE_TYPE_PING){ /* Only send ping if they can send ping. */
				if(self->addresses->peers[i]->versionMessage->version >= PONG_VERSION){
					sendMessageToPeer(self, self->addresses->peers[i], getMessage(pingPong));
					getMessage(pingPong)->expectResponse = MESSAGE_TYPE_PONG; /* Expect a pong. */
					self->addresses->peers[i]->acceptedTypes |= MESSAGE_TYPE_PONG;
				}else {
					sendMessageToPeer(self, self->addresses->peers[i], getMessage(ping));
				}
			}
		}
		decrementReferenceCount(pingPong);
	} else { /*TODO: Check if this conditional branch is not buggy*/
		int i = 0;
		for (0; i < self->addresses->peersNum; i++) {
			if (self->addresses->peers[i]->acceptedTypes & MESSAGE_TYPE_PING) /* Only send ping if they can send ping. */
				sendMessageToPeer(self, self->addresses->peers[i], getMessage(ping));
		}
	}

	decrementReferenceCount(ping);
}

void setAddressManager(NetworkCommunicator * self, AddressManager * manager)
{
	assert(self != NULL);
	assert(manager != NULL);


	incrementReferenceCount(manager);
	self->addresses = manager;
}

void setAlternativeMessages(NetworkCommunicator * self, ByteArray * altMessages, uint32_t * altMaxSizes)
{
	assert(self != NULL);
	assert(altMessages != NULL);

	if (altMessages){
		incrementReferenceCount(altMessages);
	}

	self->alternativeMessages = altMessages;
	self->altMaxSizes = altMaxSizes;
}

void setOwnIPv4Address(NetworkCommunicator * self,NetworkAddress * myIPv4Address)
{
	assert(self != NULL);
	assert(myIPv4Address != NULL);

	incrementReferenceCount(myIPv4Address);
	self->ourIPv4 = myIPv4Address;
}

void setOwnIPv6Address(NetworkCommunicator * self, NetworkAddress * myIPv6Address)
{
	assert(self != NULL);
	assert(myIPv6Address != NULL);

	incrementReferenceCount(myIPv6Address);
	self->ourIPv6 = myIPv6Address;
}

void setUserAgent(NetworkCommunicator * self, ByteArray * userAgent)
{
	assert(self != NULL);
	assert(userAgent != NULL);

	incrementReferenceCount(userAgent);
	self->userAgent = userAgent;
}

boolean startNetworkCommunicator(NetworkCommunicator * self)
{
	assert(self != NULL);

	/* Create the socket event loop */
	if (!newEventLoop(&self->eventLoop, onSocketEventLoopErrorCallback, onTimeOutErrorCallback, self)){ /*FIXME*/
		/* Cannot create event loop*/
		getMessage(self)->onErrorReceived(ERROR_NETWORK_COMMUNICATOR_LOOP_CREATE_FAIL,"The NetworkCommunicator event loop could not be created.");
		return FALSE;
	}
	self->isStarted = TRUE;
	return TRUE;
}

void startListeningToIncomingConnections(NetworkCommunicator * self)
{
	assert(self != NULL);

	if (isReachableIPType(self->addresses, IP_IPv4)) {
		/* Create new bound IPv4 socket */
		if (socketBind(&self->listeningSocketIPv4, FALSE, self->ourIPv4->port)){ /*FIXME*/
			/* Add event for accepting connection for both sockets */
			if (socketCanAcceptEvent(&self->acceptEventIPv4, self->eventLoop, self->listeningSocketIPv4, acceptIncomingConnection)) {
				if(socketAddEvent(self->acceptEventIPv4, 0)) /* No timeout for listening for incoming connections. */
					/* Start listening on IPv4 */
					if(socketListen(self->listeningSocketIPv4, self->maxIncommingConnections)){
						/* client is in IPv4listening state*/
						self->isListeningIPv4 = TRUE;
					}
				if (!self->isListeningIPv4) {
					/* Failure to add event */
					socketFreeEvent(self->eventLoop,self->acceptEventIPv4);
				}
			}
		}
		if (! self->isListeningIPv4)
			/* Failure to create event */
			closeSocket(self->listeningSocketIPv4);
	}

	/* Now for the IPv6 */
	if (isReachableIPType(self->addresses, IP_IPv6)) {
		/* Create new bound IPv6 socket */
		if (socketBind(&self->listeningSocketIPv6, TRUE, self->ourIPv6->port)){	/* Add event for accepting connection for both sockets */
			if (socketCanAcceptEvent(&self->acceptEventIPv6,self->eventLoop, self->listeningSocketIPv6, acceptIncomingConnection)) { /*FIXME*/
				if(socketAddEvent(self->acceptEventIPv6, 0)) /* No timeout for listening for incoming connections. */
					/* Start listening on IPv6 */
					if(socketListen(self->listeningSocketIPv6, self->maxIncommingConnections)){
						/* client is in IPv6listening state*/
						self->isListeningIPv6 = TRUE;
					}
				if (! self->isListeningIPv6){
					/* Failure to add event */
					socketFreeEvent(self->eventLoop,self->acceptEventIPv6);
				}
			}
		}
		if (! self->isListeningIPv6)
			/* Failure to create event */
			closeSocket(self->listeningSocketIPv6);
	}
}
void startNetworkCommunicatorPingTimer(NetworkCommunicator * self)
{
	assert(self != NULL);

	startTimer(self->eventLoop, &self->pingTimer, self->heartBeat, pingConnectedPeers, self); /*FIXME*/
}

void stopNetworkCommunicator(NetworkCommunicator * self)
{
	assert(self != NULL);

	if (self->isListeningIPv4 || self->isListeningIPv6) {
		stopListeningToIncomingConnections(self);
	}
	self->isStarted = FALSE;

	/* Disconnect all the peers */
	uint16_t removeNum = self->addresses->peersNum; /* Assign to temporary variable because "self->addresses->peersNum" will decrement each iteration. */

	int i = 0;
	for (; i < removeNum; i++) {
		disconnectPeer(self, self->addresses->peers[i], 0, TRUE); /* "TRUE" we are stopping. */
	}
	/* Stop event loop */
	exitEventLoop(self->eventLoop); /*FIXME*/
}

void stopListeningToIncomingConnections(NetworkCommunicator * self)
{
	assert(self != NULL);

	if (self->isListeningIPv4) {
		/* Stop listening on IPv4 */
		socketFreeEvent(self->eventLoop,self->acceptEventIPv4); /*FIXME*/
		closeSocket(self->listeningSocketIPv4);
		self->isListeningIPv4 = FALSE;
	}
	if (self->isListeningIPv6) {
		/* Stop listening on IPv6 */
		socketFreeEvent(self->eventLoop,self->acceptEventIPv6); /*FIXME*/
		closeSocket(self->listeningSocketIPv6);
		self->isListeningIPv6 = FALSE;
	}
}

void stopNetworkCommunicatorPingTimer(NetworkCommunicator * self)
{
	assert(self != NULL);

	if (self->pingTimer) {
		endTimer(self->pingTimer); /*FIXME*/
	}

	self->pingTimer = 0;
}

void tryConnectingToStoredAddresses(NetworkCommunicator * self)
{
	assert(self != NULL);

	if (self->attemptingOrWorkingConnections >= self->maxConnections) {
		return; /* Cannot connect to any more peers; maximum exceeded */
	}

	uint16_t numPossibleConnections = self->maxConnections - self->attemptingOrWorkingConnections;
	uint64_t numAddresses = numOfStoredAddresses(self->addresses);
	if (numAddresses < numPossibleConnections) {
		numPossibleConnections = numAddresses; /* Cannot connect to any more than the address we have */
	}

	NetworkAddressLocator * addrs = getAddressesfromAddressManager(self->addresses, numPossibleConnections);

	int i = 0;
	for (; (addrs + i)->addr != NULL; i++) {
		Peer * peer = createNewPeerFromNetworkAddress((addrs + i)->addr);
		if (!peer){
			free(addrs);
			return;
		}

		peer->incoming = FALSE;
		uint8_t bucketIndex = (addrs + i)->bucketIndex;
		uint16_t addrIndex = (addrs + i)->addrIndex;

		ConnectionReturn result = connectToPeerUsingNetworkCommunicator(self, peer);
		boolean remove = FALSE;

		if ((result == CONNECT_BAD || result == CONNECT_FAIL) && getNetworkAddress(peer)->public) {
			/* Convert back to address and set the pointer in the bucket. */
			(self->addresses->buckets + bucketIndex)->addresses[addrIndex] = realloc(peer, sizeof(NetworkAddress));
			if (! (self->addresses->buckets + bucketIndex)->addresses[addrIndex]) {
				remove = TRUE;
			}
			else if (result == CONNECT_BAD) {
				/* Add penalty if bad */
				(self->addresses->buckets + bucketIndex)->addresses[addrIndex]->score -= 3600;
			}
		}else {
			remove = TRUE;
		}

		if (remove){
			/* Remove from addresses; the connection was ok, we had a failure and we should not return the address or there is no support for this address. */
			Bucket * bucket = (self->addresses->buckets + bucketIndex);
			bucket->numOfAddresses--;
			if (bucket->numOfAddresses) {
				/* Move memory down. */
				if (bucket->numOfAddresses - addrIndex) {
					memmove(bucket->addresses + addrIndex, bucket->addresses + addrIndex + 1, (bucket->numOfAddresses - addrIndex) * sizeof(*bucket->addresses));
				}

				/* Reallocate memory. */
				NetworkAddress ** temp = realloc(bucket->addresses, sizeof(*bucket->addresses) * bucket->numOfAddresses);
				if (temp) {
					bucket->addresses = temp;
				}
			}
			/* Release from addresses. */
			decrementReferenceCount(peer);
		}
		decrementReferenceCount(peer);
	}
	free(addrs);
}
