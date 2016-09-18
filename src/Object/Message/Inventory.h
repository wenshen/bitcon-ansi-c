/*
 * Inventory.h
 *
 *  Created on: Nov 9, 2012
 *      Author: apopoola
 */


/**
 @file
 @brief Allows a node to advertise its knowledge of one or more objects. It can be received unsolicited, or in reply to getblocks.
 see https://en.bitcoin.it/wiki/Protocol_Specification#inv
 maximum payload length: 1.8 Megabytes or 50000 entries
*/


#ifndef INVENTORY_H_
#define INVENTORY_H_

#include "Message.h"
#include "InventoryVector.h"

/**
 @brief Structure for Inventory objects. @see Inventory.h
*/
typedef struct{
	Message base; /**< Message base structure */
	uint16_t count; /**< Number of inventory entries. */
	InventoryVector ** inventory; /**< The Inventory vectors. This should be a memory block of pointers to InventoryVectors. It will be freed when the Inventory is freed. When adding an item it should be retained. When removing an item it should be released. Leave or set as NULL if empty. */
} Inventory;

/**
 @brief Creates a new Inventory object.
 @returns A new Inventory object.
*/
Inventory * createNewInventory(void (*onErrorReceived)(Error error,char *,...));
/**
@brief Creates a new Inventory object from serialised data.
 @param data serialised Inventory data.
 @returns A new Inventory object.
*/
Inventory * createNewInventoryFromData(ByteArray * data,void (*onErrorReceived)(Error error,char *,...));

/**
 @brief Gets a Inventory from another object. Use this to avoid casts.
 @param self The object to obtain the Inventory from.
 @returns The Inventory object.
 */
Inventory * getInventory(void * self);

/**
 @brief Initialises an Inventory object
 @param self The Inventory object to initialise
 @returns one on success, zero on failure.
*/
int initializeInventory(Inventory * self,void (*onErrorReceived)(Error error,char *,...));
/**
 @brief Initialises an Inventory object from serialised data
 @param self The Inventory object to initialise
 @param data The serialised data.
 @returns one on success, zero on failure.
*/
int initializeInventoryFromData(Inventory * self,ByteArray * data,void (*onErrorReceived)(Error error,char *,...));

/**
 @brief destroys an Inventory object.
 @param self The Inventory object to destroy.
*/
void destroyInventory(void * self);

/**
 @brief Calculates the length needed to serialise the object.
 @param self The Inventory object.
 @returns The length read on success, 0 on failure.
 */
uint32_t calculateInventoryLength(Inventory * self);
/**
 @brief Deserialises a Inventory so that it can be used as an object.
 @param self The Inventory object
 @returns The length read on success, 0 on failure.
*/
uint32_t deserialiseInventory(Inventory * self);
/**
 @brief Serialises a Inventory to the byte data.
 @param self The Inventory object
 @returns The length written on success, 0 on failure.
*/
uint32_t serializeInventory(Inventory * self);

#endif /* INVENTORY_H_ */
