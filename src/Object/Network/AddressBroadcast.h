/*
 * AddressBroadcast.h
 *
 *  Created on: 03/10/2012
 *  Created by: Hayk Baluyan
 *  Copyright (c) 2012 Bitcoin Project Team
 */

/**
 @file
 @brief Message for broadcasting NetworkAddresses over the network. This is used to share socket information for peers that are accepting incomming connections. When making an object, get the addresses you want to add and then set the "addrNum" and "addresses" fields. When managing the addresses you want to share, it's best to keep one object and change the addresses when necessary by reorganising the pointers with the "addresses" field. Inherits Message
*/

#ifndef ADDRESSBROADCASTH
#define ADDRESSBROADCASTH

#include "NetworkAddress.h"

/**
 @brief Structure for AddressBroadcast objects. @see AddressBroadcast.h
*/
typedef struct{
	Message base; /**< Message base structure */
	uint8_t timeStamps; /**< If TRUE, timestamps are included with the NetworkAddresses */
	uint8_t addrNum; /**< Number of addresses. Maximum is 30. */
	NetworkAddress ** addresses; /**< List of NetworkAddresses shared so that peers can find new peers. */
} AddressBroadcast;

/**
 @brief Creates a new AddressBroadcast object.
 @returns A new AddressBroadcast object.
 */
AddressBroadcast * createNewAddressBroadcast(uint8_t timeStamps,void(*onErrorReceived)(Error error, char *, ...));
/**
@brief Creates a new AddressBroadcast object from serialised data.
 @param data Serialised AddressBroadcast data.
 @returns A new AddressBroadcast object.
*/
AddressBroadcast * createNewAddressBroadcastFromSerialisedData(ByteArray * data,uint8_t timeStamps,void(*onErrorReceived)(Error error, char *, ...));

/**
 @brief Gets a AddressBroadcast from another object. Use this to avoid casts.
 @param self The object to obtain the AddressBroadcast from.
 @returns The AddressBroadcast object.
 */
AddressBroadcast * getAddressBroadcast(void * self);

/**
 @brief Initialises a AddressBroadcast object
 @param self The AddressBroadcast object to initialise
 @returns TRUE on success, FALSE on failure.
*/
uint8_t initializeAddressBroadcast(AddressBroadcast * self,uint8_t timeStamps,void(*onErrorReceived)(Error error, char *, ...));
/**
 @brief Initialises a AddressBroadcast object from serialised data
 @param self The AddressBroadcast object to initialise
 @param data The serialised data.
 @returns TRUE on success, FALSE on failure.
*/
uint8_t initializeAddressBroadcastFromSerialisedData(AddressBroadcast * self,uint8_t timeStamps,ByteArray * data,void(*onErrorReceived)(Error error, char *, ...));

/**
 @brief Frees a AddressBroadcast object.
 @param self The AddressBroadcast object to free.
 */
void destroyAddressBroadcast(void * self);
 
/**
 @brief Calculates the length needed to serialise the object.
 @param self The AddressBroadcast object.
 @returns The length read on success, 0 on failure.
 */
uint32_t lengthOfAddressBroadcast(AddressBroadcast * self);
/**
 @brief Deserialises a AddressBroadcast so that it can be used as an object.
 @param self The AddressBroadcast object
 @returns The length read on success, 0 on failure.
*/
uint32_t deserialiseAddressBroadcast(AddressBroadcast * self);
/**
 @brief Serialises a AddressBroadcast to the byte data.
 @param self The AddressBroadcast object
 @returns The length written on success, 0 on failure.
*/
uint32_t serialiseAddressBroadcast(AddressBroadcast * self);

/**
 @brief Adds a NetworkAddress to the list for broadcasting.
 @param self The AddressBroadcast object
 @param address The NetworkAddress to add.
 @returns TRUE if the network address was added successfully, FALSE if there was an error in doing so.
 */
uint8_t addNetworkAddressToAddressBroadcast(AddressBroadcast * self,NetworkAddress * address);

/**
 @brief Takes a NetworkAddress to the list for broadcasting. This does not retain the NetworkAddress so you can pass an NetworkAddress into this while releasing control in the calling function.
 @param self The AddressBroadcast object
 @param address The NetworkAddress to take.
 @returns TRUE if the network address was taken successfully, FALSE if there was an error in doing so.
 */
uint8_t addressBroadcastTakeNetworkAddress(AddressBroadcast * self,NetworkAddress * address);

#endif
