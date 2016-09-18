/*
 * VersionChecksumBytes.h
 *
 *  Created on: 03/10/2012
 *  Created by: Hayk Baluyan
 *  Copyright (c) 2012 Bitcoin Project Team
 */

/**
 @file
 @brief Represents a key, begining with a version byte and ending with a checksum. Inherits ByteArray
*/

#ifndef VERSIONCHECKSUMBYTESH
#define VERSIONCHECKSUMBYTESH

#include "../Base58/Base58.h"
#include <stdint.h>
#include "ByteArray.h"
/**
 @brief Structure for VersionChecksumBytes objects. @see VersionChecksumBytes.h
*/
typedef struct{
	ByteArray base; /**< ByteArray base structure */
	uint8_t cacheString; /**< If TRUE, cache string */
	ByteArray * cachedString; /**< Pointer to cached ByteArray string */
} VersionChecksumBytes;

/**
 @brief Creates a new VersionChecksumBytes object from a base-58 encoded string. The base-58 string will be validated by it's checksum. This returns NULL if the string is invalid. The _ERROR_BASE58_DECODE_CHECK_TOO_SHORT error is given if the decoded data is less than 4 bytes. _ERROR_BASE58_DECODE_CHECK_INVALID is given if the checksum does not match.
 @param string A base-58 encoded String to make a VersionChecksumBytes object with a termination character.
 @param cacheString If true, the bitcoin string for this object will be cached in memory.
 @param onErrorReceived A Engine for errors.
 @returns A new VersionChecksumBytes object or NULL on failure.
 */
VersionChecksumBytes * createNewVersionChecksumBytesFromString(ByteArray * string,uint8_t cacheString,void (*onErrorReceived)(Error error,char *,...));
/**
 @brief Creates a new VersionChecksumBytes object from bytes.
 @param bytes The bytes for the VersionChecksumBytes object.
 @param size The size of the byte data.
 @param cacheString If true, the bitcoin string for this object will be cached in memory.
 @param onErrorReceived A Engine for errors.
 @returns A new VersionChecksumBytes object.
 */
VersionChecksumBytes * creatNewVersionChecksumBytesFromBytes(uint8_t * bytes,uint32_t size,uint8_t cacheString,void (*onErrorReceived)(Error error,char *,...));

/**
 @brief Gets a VersionChecksumBytes from another object. Use this to avoid casts.
 @param self The object to obtain the VersionChecksumBytes from.
 @returns The VersionChecksumBytes object.
 */
VersionChecksumBytes * getVersionChecksumBytes(void * self);

/**
 @brief Initialises a VersionChecksumBytes object from a string.
 @param self The VersionChecksumBytes object to initialise.
 @param string A String to make a VersionChecksumBytes object with a termination character.
 @param cacheString If true, the bitcoin string for this object will be cached in memory.
 @param onErrorReceived A Engine for errors.
 @returns true on success, false on failure.
 */
uint8_t initializeVersionChecksumBytesFromString(VersionChecksumBytes * self,ByteArray * string,uint8_t cacheString,void (*onErrorReceived)(Error error,char *,...));
/**
 @brief Initialises a new VersionChecksumBytes object from bytes.
 @param self The VersionChecksumBytes object to initialise.
 @param bytes The bytes for the VersionChecksumBytes object.
 @param size The size of the byte data.
 @param cacheString If true, the bitcoin string for this object will be cached in memory.
 @param onErrorReceived A Engine for errors.
 @returns true on success, false on failure.
 */
uint8_t initializeVersionChecksumBytesFromBytes(VersionChecksumBytes * self,uint8_t * bytes,uint32_t size,uint8_t cacheString,void (*onErrorReceived)(Error error,char *,...));

/**
 @brief Frees a VersionChecksumBytes object.
 @param self The VersionChecksumBytes object to free.
 */
void destroyVersionChecksumBytes(void * self);
 


/**
 @brief Gets the version for a VersionChecksumBytes object.
 @param self The VersionChecksumBytes object.
 @returns The version code. The Macros _PRODUCTION_NETWORK and _TEST_NETWORK should correspond to this. 
 */
uint8_t getNetVersionByteForVersionChecksumBytes(VersionChecksumBytes * self);
/**
 @brief Gets the string representation for a VersionChecksumBytes object as a base-58 encoded String.
 @param self The VersionChecksumBytes object.
 @returns The object represented as a base-58 encoded String. Do not modify this. Copy if modification is required.
 */
ByteArray * getStringForVersionChecksumBytes(VersionChecksumBytes * self);


/*  Functions*/
#endif
