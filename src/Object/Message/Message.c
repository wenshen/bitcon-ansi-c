/*
 * Message.c
 *
 *  Created on: 04/10/2012
 *  Created by: Abdulfatai Popoola
 *  Modified by: Wen Shen & Dmytro Krasnoshtan & Issak Gezehei
 *  Copyright (c) 2012 Bitcoin Project Team
 */
 
/** @file
 @brief This file is Message.c
 @details locks contain transaction information and use a proof of work system to show that they are legitimate.
 */ 
 

#include "Message.h"
#include <assert.h>

/*
TODO pull out anonymous error handler callback
checking that this object was created from the getObject
linking new destroy function
setting the referenceCount to 1
*/

/**
 @fn Message * createNewMessage(void (*onErrorReceived)(Error error,char *,...))
 @brief Creates a new Message
 @returns Message
 */
Message * createNewMessage(void (*onErrorReceived)(Error error,char *,...))
{
	assert(onErrorReceived != NULL);

	Message * self = malloc(sizeof(*self)); /**<Create memory for Message */

	if (!self) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory for Message\n",sizeof(*self));
		return NULL;
	}

	/**< checking that this object was created from the getObject */
	if(getObject(self) != NULL){
		/**< retrieve the base object and use it to update the underlying base object details */
		getObject(self)->destroy = destroyMessage; /**<assign destructor function to object*/
		getObject(self)->referenceCount = 1;

		/**< Set other Message fields */
		self->bytes = NULL;
		self->onErrorReceived = onErrorReceived;
		self->expectResponse = FALSE;
		return self;
	}


	/**< creation failed; Clean up everything and return NULL */
	free(self);
	return NULL;
}

/**
 @fn Message * createMessageFromByteArray(Message * self, ByteArray * data, void (*onErrorReceived)(Error error,char *,...))
 @brief Creates a new Message the same as createNewMessage with additional parameters  Message, ByteArray
 @param self
 @param data
 @returns int TRUE or FALSE
 */

int * setByteArrayInMessage(Message * self, ByteArray * data, void (*onErrorReceived)(Error error,char *,...))
{
	assert(self != NULL);
	assert(data != NULL);
	assert(onErrorReceived != NULL);

	/**< Check to see that the base Object class can be retrieved and then update details */
	if(getObject(self) != NULL){
		/**< retrieve the base object and use it to update the underlying object details */
		getObject(self)->referenceCount = 1;

		/**< Set other Message fields */
		self->bytes = data;  /**< set Message data to the ByteArray data that was passed in */
		incrementReferenceCount(data); /**< Increment the number of references to the ByteArray data */

		self->onErrorReceived = onErrorReceived;
		self->expectResponse = FALSE;
		return TRUE;
	}

	/**< creation failed; Clean up everything and return NULL */
	free(self);
	return FALSE;
}

int initializeMessageFromObject(Message * self, void (*onErrorReceived)(Error error,char *,...))
{
    assert(self != NULL);
    assert(onErrorReceived != NULL);

	if (!setObjectRefCountToOne(getObject(self))) { /*TODO Rename to resetReferenceCount? */
		return FALSE;
	}
	self->bytes = NULL;
	self->onErrorReceived = onErrorReceived;
	self->expectResponse = FALSE;
	return TRUE;
}

int initializeMessageFromByteArrayData(Message * self, ByteArray * data, void (*onErrorReceived)(Error error,char *,...))
{
    assert(self != NULL);
    assert(data != NULL);
    assert(onErrorReceived != NULL);

	if (!setObjectRefCountToOne(getObject(self))) { /*TODO Rename to resetReferenceCount?*/
		return FALSE;
	}

	self->bytes = data;
	incrementReferenceCount(data);
	self->onErrorReceived = onErrorReceived;
	self->expectResponse = FALSE;
	return TRUE;
}

/**
 @fn Message * getMessage(void * self)
 @brief getter
 @param self
 @returns Message Object
 */
Message * getMessage(void * self)
{
	assert(self != NULL);
	return self;
}



 /**
 @fn void destroyMessage(void * message)
 @brief destroy Message 
 @param message
 @returns
 */
void destroyMessage(void * message)
{
	assert(message != NULL);

	Message * self = message;
	if (self->bytes != NULL){
		decrementReferenceCount(self->bytes); /*Reduce number of references to these bytes*/
	}
	destroyObject(self);
}
