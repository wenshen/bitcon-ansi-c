/*
 * Address.h
 *
 *  Created on: 03/10/2012
 *  Created by: Hayk Baluyan
 *  Copyright (c) 2012 Bitcoin Project Team
 */

/**
 @file
 @brief Based upon an ECDSA public key and a network version code. Used for receiving bitcoins. Inherits VersionChecksumBytes.
 @details Here is a diagram of how a bitcoin address is structured created by Alan Reiner, developer of Bitcoin Armory (http://bitcoinarmory.com/):
 \image html Address.png
*/

#include "../VersionChecksumBytes.h"

#ifndef ADDRESSH
#define ADDRESSH

/*  Includes */

/**
 @brief Structure for Address objects. @see Address.h
*/
typedef struct{
	VersionChecksumBytes base; /**< VersionChecksumBytes base structure */
} Address;

/**
 @brief Creates a new Address object from a RIPEMD-160 hash.
 @param netVersionByte Shows version byte of the address(i.e. 1 for public key hush, 3 for script hash)
 @param hash The RIPEMD-160 hash. Must be 20 bytes.
 @param cacheString If true, the bitcoin string for this object will be cached in memory.
 @param onErrorReceived Callback for errors.
 @returns A new Address object.
 */
Address * createNewAddressFromRIPEMD160Hash(uint8_t * hash,uint8_t netVersionByte,uint8_t cacheString,void (*onErrorReceived)(Error error,char *,...));
/**
 @brief Creates a new Address object from a base-58 encoded string.
 @param string The base-58 encoded String with a termination character.
 @param cacheString If true, the bitcoin string for this object will be cached in memory.
 @param onErrorReceived Callback for errors.
 @returns A new Address object. Returns NULL on failure such as an invalid bitcoin address.
 */
Address * createNewAddressFromString(ByteArray * string,uint8_t cacheString,void (*onErrorReceived)(Error error,char *,...));

/**
 @brief Gets a Address from another object. Use this to avoid casts.
 @param self The object to obtain the Address from.
 @returns The Address object.
 */
Address * getAddress(void * self);

/**
 @brief Initializes a Address object from a RIPEMD-160 hash.
 @param self The Address object to initialize.
 @param network A NetworkParameters object with the network information.
 @param hash The RIPEMD-160 hash. Must be 20 bytes.
 @param cacheString If true, the bitcoin string for this object will be cached in memory.
 @param onErrorReceived Events for errors.
 @returns true on success, false on failure.
 */
uint8_t initializeAddressFromRIPEMD160Hash(Address * self,uint8_t netVersionByte,uint8_t * hash,uint8_t cacheString,void (*onErrorReceived)(Error error,char *,...));
/**
 @brief Initializes a Address object from a base-58 encoded string.
 @param self The Address object to initialize.
 @param netVersionByte Shows version byte of the address(i.e. 1 for public key hush, 3 for script hash)
 @param string The base-58 encoded String with a termination character.
 @param cacheString If true, the bitcoin string for this object will be cached in memory.
 @param onErrorReceived Events for errors.
 @returns true on success, false on failure.
 */
uint8_t initializeAddressFromString(Address * self,ByteArray * string,uint8_t cacheString,void (*onErrorReceived)(Error error,char *,...));

/**
 @brief destroys an Address object.
 @param self The Address object to free.
 */
void destroyAddress(void * self);
 


#endif
