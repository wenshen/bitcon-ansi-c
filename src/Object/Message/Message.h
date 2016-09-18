/*
 * Message.h
 *
 *  Created on: 03/10/2012
 *  Created by: Abdulfatai Popoola
 *  Modified by: Wen Shen & Dmytro Krasnoshtan & Issak Gezehei & Prajowal Manandhar
 *  Copyright (c) 2012 Bitcoin Project Team
 */

/** @file
 @brief Header file for the Message object.
 @details The Message class is the base class for all kinds of messages that can be transferred over the bitcoin network.
 */

#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <stdlib.h>
#include <stdint.h>
#include "../../Constants.h"
#include "../ByteArray.h"
#include "../Object.h"
#include "../../Utils/VariableLengthInteger/VarLenInt.h"

/**
 @struct Message
 @brief Structure for Message objects. @see Message.h
 */
typedef struct Message{
	Object base; /**< Object base structure */
	MessageType type; /**< The type of the message */
	uint8_t * altText; /**< For an alternative message: This is the type text. */
	ByteArray * bytes; /**< Raw message data minus the message header. When serialising this should be assigned to a ByteArray large enough to hold the serialised data. */
	uint8_t checksum[4]; /**< The message checksum. When sending messages using a NetworkCommunicator, this is calculated for you. */
	void (*onErrorReceived)(Error error,char *,...); /**< Pointer to error callback */
	MessageType expectResponse; /**< Set to zero if no message expected or the type of message expected as a response. */
	short int serialized;
} Message;

/**
 @fn Message * createNewMessage(void (*onErrorReceived)(Error error,char *,...))
 @brief Creates a new Message object. This message will be created with object data and not with byte data. The message can be serialised for the byte data used over the network.
 @returns A new Message object.
 */
Message * createNewMessage(void (*onErrorReceived)(Error error,char *,...));

/**
 @fn Message * getMessage(void * self)
 @brief Gets a Message from another object. Use this to avoid casts.
 @param self The object to obtain the Message from.
 @returns The Message object.
 */
Message * getMessage(void * self);


/**
 @fn Message * createMessageFromByteArray(Message * self,ByteArray * data,void (*onErrorReceived)(Error error,char *,...))
 @brief Sets the ByteArray field of a Message with the passed in ByteArray data
 @param self The Message object
 @param data The byte data for the object. The data will not be copied but retained by this object.
 @returns int a binary value representing success or failure
 */
int * setByteArrayInMessage(Message * self,ByteArray * data,void (*onErrorReceived)(Error error,char *,...));

/**
 @brief Initialises a Message object
 @param self The Message object to initialise
 @returns TRUE on success, FALSE on failure.
 */
int initializeMessageFromObject(Message * self, void (*onErrorReceived)(Error error,char *,...));

/**
 @brief Initialises a Message object from byteArray data.
 @param self The Message object to initialise
 @param data The byte data for the object. The data will not be copied but referenced by this object.
 @returns TRUE on success, FALSE on failure.
 */
int initializeMessageFromByteArrayData(Message * self, ByteArray * data, void (*onErrorReceived)(Error error,char *,...));

/**
 @fn void destroyMessage(void * self)
 @brief destroys the memory allocated to a Message object.
 @param self The Message object to free.
 */
void destroyMessage(void * self);

/*  FUNCTIONS */


#endif /* MESSAGE_H_ */
