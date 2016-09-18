/*
 *  Peer.h
 *
 *  Created on: 9/11/2012.
 *  Created by: Prajowal Manandhar
 *  Modified by: Attila - Peter Toth
 *  Copyright (c) 2012 Bitcoin Project Team
 *  please read https://en.bitcoin.it/wiki/Protocol_Specification
 */

/**
 @file
 @brief Contains data for managing peer connections. Inherits NetworkAddress
 */

#ifndef NODE_H_
#define NODE_H_

#include "NetworkAddress.h"
#include "Version.h"
#include <time.h>
#include "../Message/Message.h"

/**
 @brief Structure for Peer objects. @see Peer.h
 */
typedef struct {
	NetworkAddress base; /**< NetworkAddress base structure */
	uint64_t socketID; /**< Not used in the bitcoin protocol. This is used by bitcoin to store a socket ID for a connection to a NetworkAddress. The socket here is not closed when the NetworkAddress is freed so needs to be closed elsewhere. */
	Message * receive; /**< Receiving message. NULL if not receiving. This message is exclusive to the peer. */
	Message * sendQueue[10];/**< Messages to send to this peer. NULL if not sending anything. */
	uint8_t sendQueueSize; /**< Up to 10 messages in queue */
	uint8_t sendQueueFront; /**< Index of the front of the queue */
	uint32_t messageSent; /**< Used by a NetworkCommunicator to store the message length send. When the header is sent, 24 bytes are taken off. */
	short int sentHeader; /**< 1 if the sending message's header has been sent. */
	uint8_t * sendingHeader; /**< Stores header to send */
	short int getAddresses; /* 1 is asked for addresses, 0 otherwise. */
	uint64_t receiveEvent; /**< Event for receiving data from this peer */
	uint64_t sendEvent; /**< Event for sending data from this peer */
	uint64_t connectEvent; /**< Event for connecting to the peer. */
	short int versionSent; /**< 1 if the version was sent to this peer */
	Version * versionMessage; /**< The version message from this peer. */
	short int versionAck; /**< This peer acknowledged the version message. */
	uint8_t * headerBuffer; /**< Used by a NetworkCommunicator to read the message header before processing. */
	uint32_t messageReceived; /**< Used by a NetworkCommunicator to store the message length received. When the header is received 24 bytes are taken off. */
	uint16_t acceptedTypes; /**< Set messages that will be accepted on receiving. When a peer tries to send another message, drop the peer. Starts empty */
	short int receivedHeader; /**< 1 if the receiving message's header has been received. */
	int16_t timeOffset; /**< The offset from the system time this peer has */
	uint64_t timeBroadcast; /**< Time of the last own address broadcast. */
	short int connectionWorking; /**< 1 when the connection has been successful and the peer has ben added to the AddressManager. */
	MessageType typesExpected[3]; /**< List of expected responses */
	uint8_t typesExpectedNum; /**< Number of expected responses */
	short int incoming; /**< Node from an incoming connection if 1 */
	clock_t downloadTime; /**< Downloaded time for this peer, not taking the latency into account. Use for determining effeciency. */
	uint32_t downloadAmount; /**< Downloaded bytes measured for this peer. */
	clock_t latencyTime; /**< Total measured response time */
	uint16_t responses; /**< Number of measured responses. */
	clock_t latencyTimerStart; /**< Used to measure latency. */
	clock_t downloadTimerStart; /**< Used to measure download time. */
	MessageType latencyExpected; /**< Message expected for the latency measurement */
} Peer;

/**
 @brief Creates a new Peer object.
 @returns A new Peer object.
 */

Peer * createNewPeerFromNetworkAddress(NetworkAddress * addr);

/**
 @brief Gets a Peer from another object. Use this to avoid casts.
 @param self The object to obtain the Peer from.
 @returns The Peer object or NULL in case of faliure.
 */
Peer * getNode(void * self);

/**
 @brief Initializes a Peer object
 @param self The Peer object to initialize
 @returns 1 on success, 0 on failure.
 */
short int initNodeByTakingNetworkAddress(Peer * self);

/**
 @brief Frees a Peer object.
 @param self The Peer object to free.
 */
void destroyNode(void * self);

#endif
