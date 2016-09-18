/*
 * GetBlocks.h
 *
 * Created on: Nov 9, 2012
 * Created by: Wen Shen
 * Copyright (c) 2012 MIBitcoin Project Team
 */

#ifndef GETBLOCKS_H_
#define GETBLOCKS_H_
/**
 @file
 @brief A message to ask for an inventory of blocks or the block headers (getheaders message) up to the "stopAtHash",
 the last known block or 500 blocks, whichever comes first. The message comes with a block chain descriptor so the recipient
  can discover how the blockchain is different so they can send the necessary blocks (starting from a point of agreement). Inherits Message
 @details The GetBlocks Class
*/

#include "ChainDescriptor.h"

/**
 @brief Structure for GetBlocks objects. @see GetBlocks.h
*/
typedef struct{
	Message base; /**< Message base structure */
	uint32_t version; /**< Protocol version for this message */
	ChainDescriptor * chainDescriptor; /**< ChainDescriptor for determining what blocks are needed. */
	ByteArray * stopAtHash; /**< The block hash to stop at when retreiving an inventory. Else up to 500 block hashes can be given. */
} GetBlocks;

/**
 @brief Creates a new GetBlocks object.
 @returns A new GetBlocks object.
*/
GetBlocks * newGetBlocks(uint32_t version,ChainDescriptor * chainDescriptor,ByteArray * stopAtHash,void (*onErrorReceived)(Error error,char *,...));
/**
@brief Creates a new GetBlocks object from serialized data.
 @param data Serialized GetBlocks data.
 @returns A new GetBlocks object.
*/
GetBlocks * newGetBlocksFromData(ByteArray * data,void (*onErrorReceived)(Error error,char *,...));

/**
 @brief Gets a GetBlocks from another object. Use this to avoid casts.
 @param self The object to obtain the GetBlocks from.
 @returns The GetBlocks object.
*/
GetBlocks * retrieveGetBlocks(void * self);

/**
 @brief Initialises a GetBlocks object
 @param self The GetBlocks object to initialise
 @returns true on success, false on failure.
*/
int initGetBlocks(GetBlocks * self,uint32_t version,ChainDescriptor * chainDescriptor,ByteArray * stopAtHash,void (*onErrorReceived)(Error error,char *,...));
/**
 @brief Initialises a GetBlocks object from serialised data
 @param self The GetBlocks object to initialise
 @param data The serialised data.
 @returns true on success, false on failure.
*/
int initGetBlocksFromData(GetBlocks * self,ByteArray * data,void (*onErrorReceived)(Error error,char *,...));

/**
 @brief Frees a GetBlocks object.
 @param self The GetBlocks object to free.
 */
void destroyGetBlocks(void * self);

/**
 @brief Calculates the length needed to serialise the object.
 @param self The GetBlocks object.
 @returns The length read on success, 0 on failure.
 */
uint32_t calculateGetBlocksLength(GetBlocks * self);
/**
 @brief Deserializes a GetBlocks so that it can be used as an object.
 @param self The GetBlocks object
 @returns The length read on success, 0 on failure.
*/
uint16_t deserializeGetBlocks(GetBlocks * self);
/**
 @brief Serializes a GetBlocks to the byte data.
 @param self The GetBlocks object
 @returns The length written on success, 0 on failure.
*/
uint16_t serializeGetBlocks(GetBlocks * self);

#endif /* GETBLOCKS_H_ */
