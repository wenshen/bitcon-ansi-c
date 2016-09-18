/*
 * Address.h
 *
 *  Created on: 03/10/2012
 *  Created by: Hayk Baluyan
 *  Copyright (c) 2012 Bitcoin Project Team
 */
/**
 @file
 @brief Contains IP, time, port and services information for a peer as well as data and code for managing individual peers. Used to advertise peers. "bytesTransferred/timeUsed" can be used to rank peers for the most efficient ones which can be useful when selecting prefered peers for download. Inherits Object
*/

#ifndef NETWORKADDRESSH
#define NETWORKADDRESSH

#include "../Message/Message.h"
#include "NetworkFunctions.h"

/**
 @brief Structure for NetworkAddress objects. @see NetworkAddress.h
*/
typedef struct {
	Message base; /**< Object base structure */
	uint32_t score; /**< Address score. */
	uint32_t time;		/* Time field*/
	uint64_t services; /**< Services bit field */
	ByteArray * ip; /**< IP address. Should be 16 bytes for the IPv6 compatible format. The NetworkAddress should have exclusive access to the ByteArray to avoid potential threading issues as the NetworkAddresses are protected by mutexes maintained by NetworkCommunicators but the ip is not. */
	uint16_t port; /**< Port number */
	IPType type; /**< The type of the IP */
	int32_t version; /**< Protocol version of peer. Set to CB_NODE_VERSION_NOT_SET before it is set once a CBVersion message is received. */
	bool public; /**< If true the address is public and should be relayed. If true, upon a lost or failed connection, return to addresses list. This doesn't include failure from "CBNetworkCommunicatorConnect". If false the address is private and should be forgotten when connections are closed. */
} NetworkAddress;

/**
 @brief Creates a new NetworkAddress object.
 @param ip The IP address in a 16 byte IPv6 format. If NULL, the IP will be a 16 byte ByteArray set will all zero.
 @param port is Port Number
 @param time 4 bytes for Time
 @param services 8 bytes for Services
 @param onErrorReceived Callback for errors.
 @returns A new NetworkAddress object.
 */
NetworkAddress * createNewNetworkAddress(uint32_t time,ByteArray * ip,uint16_t port,uint64_t services,void(*onErrorReceived)(Error error, char *, ...));
/**
 @brief Creates a new NetworkAddress object from serialised data.
 @param time 4 bytes for Time
 @param data for Serialized Data
 @param onErrorReceived Callback for errors.
 @returns A new NetworkAddress object.
 */
NetworkAddress * createNewNetworkAddressFromSerialisedData(ByteArray * data,void(*onErrorReceived)(Error error, char *, ...));

/**
 @brief Gets a NetworkAddress from another object. Use this to avoid casts.
 @param self The object to obtain the NetworkAddress from.
 @returns The NetworkAddress object.
 */
NetworkAddress * getNetworkAddress(void * self);

/**
 @brief Initialises a NetworkAddress object
 @param self The NetworkAddress object to initialise
 @param port is Port Number
 @param time 4 bytes for Time
 @param services 8 bytes for Services
 @param onErrorReceived Callback for errors.
 @returns true on success, false on failure.
 */
uint8_t initializeNetworkAddress(NetworkAddress * self,uint32_t time,ByteArray * ip,uint16_t port,uint64_t services,void(*onErrorReceived)(Error error, char *, ...));
/**
 @brief Initialises a NetworkAddress object from serialised data
 @param self The NetworkAddress object to initialise
 @param time 4 bytes for Time
 @param data for Serialized Data
 @param onErrorReceived Callback for errors.
 @returns true on success, false on failure.
 */
uint8_t initializeNetworkAddressFromSerialisedData(NetworkAddress * self,ByteArray * data,void(*onErrorReceived)(Error error, char *, ...));

/**
 @brief destroys a NetworkAddress object.
 @param self The NetworkAddress object to free.
 */
void destroyNetworkAddress(void * self);
 


/**
 @brief Deserialises a NetworkAddress so that it can be used as an object.
 @param self The NetworkAddress object
 @param time If true a timestamp is expected, else it is not. If a timestamp is not expected then "time" will not be set and will be the previous value.
 @returns The length read on success, 0 on failure.
 */
uint8_t deserialiseNetworkAddress(NetworkAddress * self, uint8_t time);
/**
 @brief Compares two network addresses
 @param self The NetworkAddress object
 @param addr The NetworkAddress for comparison
 @returns true if the IP and port match and the IP is not NULL. False otherwise.
 */
uint8_t compareNetworkAddresses(NetworkAddress * self,NetworkAddress * addr);
/**
 @brief Serialises a NetworkAddress to the byte data.
 @param self The NetworkAddress object
 @param time If true the time will be included, else it will not.
 @returns The length written on success, 0 on failure.
 */
uint8_t serialiseNetworkAddress(NetworkAddress * self, uint8_t time);

#endif
