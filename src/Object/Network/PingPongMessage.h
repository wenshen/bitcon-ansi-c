/*
 * PingPongMessageMessage.h
 *
 *  Created on: 04/10/2012
 *  Created by: Abdulfatai Popoola
 *  Copyright (c) 2012 Bitcoin Project Team
 */


/**
 @file
 @brief This message contains the nonce ID used in ping and pong messages for protocol versions over 60000.
 When the protocol version as negotiated in the "ver" message is greater than 60000,
 the "ping" message must contain a uint64 field called "nonce".
 A peer sending "ping" should set the nonce to a random value,
 and it is then echoed back by the recipient in a new "pong" message that also contains a single uint64 field. see https://en.bitcoin.it/wiki/BIP_0031
 A NetworkCommunicator will accept this message for compatible protocol numbers and use an empty message for pings otherwise. Inherits Message
*/

#ifndef PingPongMessageH
#define PingPongMessageH

#include "../Message/Message.h"

/**
 @brief Structure for PingPongMessage objects. @see PingPongMessage.h
*/
typedef struct{
	Message base; /**< Message base structure */
	uint64_t nonceID; /**< Used to identify a ping/pong. Set ping to zero if no identification is needed. Set pongs to the same as the request ping. */
} PingPongMessage;

/**
 @brief Creates a new PingPongMessage object.
 @param ID The identifier used in a ping/pong communication. Set to zero if you don't want to overlap ping messages
 @returns A new PingPongMessage object.
*/
PingPongMessage * createNewPingPongMessage(uint64_t ID,void (*onErrorReceived)(Error error,char *,...));
/**
@brief Creates a new PingPongMessage object from serialised data.
 @param data Serialised PingPongMessage data.
 @returns A new PingPongMessage object.
*/
PingPongMessage * createNewPingPongMessageFromData(ByteArray * data,void (*onErrorReceived)(Error error,char *,...));

/**
 @brief Gets a PingPongMessage from another object. Use this to avoid casts.
 @param self The object to obtain the PingPongMessage from.
 @returns The PingPongMessage object.
*/
PingPongMessage * getPingPongMessage(void * self);

/**
 @brief Initialises a PingPongMessage object
 @param self The PingPongMessage object to initialise
 @param ID The identifier used in a ping/pong communcation. Set to zero if you don't want to overlap ping messages
 @returns One on success, Zero on failure.
*/
int initializePingPongMessage(PingPongMessage * self, uint64_t ID, void (*onErrorReceived)(Error error,char *,...));
/**
 @brief Initialises a PingPongMessage object from serialised data
 @param self The PingPongMessage object to initialise
 @param data The serialised data.
 @returns One on success, Zero on failure.
*/
int initializePingPongMessageFromData(PingPongMessage * self,ByteArray * data,void (*onErrorReceived)(Error error,char *,...));

/**
 @brief Frees a PingPongMessage object.
 @param self The PingPongMessage object to free.
 */
void destroyPingPongMessage(void * self);

/**
 @brief Deserialises a PingPongMessage so that it can be used as an object.
 @param self The PingPongMessage object
 @returns Length read if successful, zero otherwise.
*/
uint8_t deserializePingPongMessage(PingPongMessage * self);
/**
 @brief Serialises a PingPongMessage to the byte data.
 @param self The PingPongMessage object
 @returns Length written if successful, zero otherwise.
*/
uint8_t serializePingPongMessage(PingPongMessage * self);

#endif
