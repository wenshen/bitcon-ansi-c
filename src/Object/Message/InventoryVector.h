/*
 * InventoryVector.h
 *
 *  Created on: 04/10/2012
 *  Created by: Abdulfatai Popoola
 *  Copyright (c) 2012 Bitcoin Project Team
 */

/**
 @file
 @brief Inventory vectors are used for notifying other nodes about objects they have or data which is being requested. see https://en.bitcoin.it/wiki/Protocol_specification#Inventory_Vectors
 Inventory vector could be transaction messages, block messages or errors
*/

#ifndef INVENTORYVECTORH
#define INVENTORYVECTORH

#include "Message.h"

/**
 @brief Structure for InventoryVector objects. @see InventoryVector.h
*/
typedef struct{
	Message base; /**< Message base structure */
	InventoryVectorType type; /**< The type of the inventory vector. It can be an error, a transaction or a block */
	ByteArray * hash; /**< The hash of the inventory vector */
} InventoryVector;

/**
 @brief Creates a new InventoryVector object.
 @returns A new InventoryVector object.
*/
InventoryVector * createNewInventoryVector(InventoryVectorType type, ByteArray * hash, void (*onErrorReceived)(Error error,char *,...));

/**
@brief Creates a new InventoryVector object from serialised data.
 @param data Serialised InventoryVector data.
 @returns A new InventoryVector object.
*/

InventoryVector * createNewInventoryVectorFromData(ByteArray * data, void (*onErrorReceived)(Error error,char *,...));

/**
 @brief Gets a InventoryVector from another object. Use this to avoid casts.
 @param self The object to obtain the InventoryVector from.
 @returns The InventoryVector object.
*/
InventoryVector * getInventoryVector(void * self);

/**
 @brief Initialises a InventoryVector object
 @param self The InventoryVector object to initialise
 @returns true on success, false on failure.
*/
int initializeInventoryVector(InventoryVector * self, InventoryVectorType type, ByteArray * hash, void (*onErrorReceived)(Error error,char *,...));
/**
 @brief Initialises a InventoryVector object from serialised data
 @param self The InventoryVector object to initialise
 @param data The serialised data.
 @returns true on success, false on failure.
*/
int initializeInventoryVectorFromData(InventoryVector * self, ByteArray * data, void (*onErrorReceived)(Error error,char *,...));

/**
 @brief Frees a InventoryVector object.
 @param self The InventoryVector object to free.
 */
void destroyInventoryVector(void * self);

/**
 @brief Deserialises a InventoryVector so that it can be used as an object.
 @param self The InventoryVector object
 @returns The length read on success, 0 on failure.
*/
uint32_t deserializeInventoryVector(InventoryVector * self);
/**
 @brief Serialises a InventoryVector to the byte data.
 @param self The InventoryVector object
 @returns The length written on success, 0 on failure.
*/
uint32_t serializeInventoryVector(InventoryVector * self);

#endif
