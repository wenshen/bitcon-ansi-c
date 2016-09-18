/*
 * BlockHeaders.h
 *
 * Created on: Nov 8, 2012
 * Created by: Wen Shen
 * Copyright (c) 2012 MIBitcoin Project Team
 */

/** @file
 @brief A message used to send and receive block headers. Inherits Message
 @details The headers of a block
 */

#ifndef BLOCKHEADERS_H_
#define BLOCKHEADERS_H_


#include "Block.h"

/**
 @brief Structure for BlockHeaders objects. @see BlockHeaders.h
*/
typedef struct{
	Message base; /**< Message base structure */
	uint16_t numOfHeaders; /**< The number of headers. */
	Block ** blockHeaders; /**< The block headers as Block objects with no transactions. The number of transactions is given however. */
} BlockHeaders;

/**
 @brief Creates a new BlockHeaders object.
 @returns A new BlockHeaders object.
*/
BlockHeaders * newBlockHeaders(void (*onErrorReceived)(Error error,char *,...));
/**
@brief Creates a new BlockHeaders object from serialized data.
 @param data Serialized BlockHeaders data.
 @returns A new BlockHeaders object.
*/
BlockHeaders * newBlockHeadersFromData(ByteArray * data,void (*onErrorReceived)(Error error,char *,...));

/**
 @brief Gets a BlockHeaders from another object. Use this to avoid casts.
 @param self The object to obtain the BlockHeaders from.
 @returns The BlockHeaders object.
*/
BlockHeaders * getBlockHeaders(void * self);

/**
 @brief Initializes a BlockHeaders object
 @param self The BlockHeaders object to initialize
 @returns true on success, false on failure.
*/
int initBlockHeaders(BlockHeaders * self,void (*onErrorReceived)(Error error,char *,...));
/**
 @brief Initializes a BlockHeaders object from serialized data
 @param self The BlockHeaders object to initialize
 @param data The serialized data.
 @returns true on success, false on failure.
*/
int initBlockHeadersFromData(BlockHeaders * self,ByteArray * data,void (*onErrorReceived)(Error error,char *,...));

/**
 @brief Frees a BlockHeaders object.
 @param self The BlockHeaders object to free.
 */
void destroyBlockHeaders(void * self);

/**
 @brief Adds a Block into the block header list.
 @param self The BlockHeaders object
 @param address The Block to add.
 @returns true if the block header was added successfully or false on error.
 */
int addBlockHeaderToBlockHeaderList(BlockHeaders * self,Block * header);
/**
 @brief Calculates the length needed to serialize the object.
 @param self The BlockHeaders object.
 @returns The length read on success, 0 on failure.
 */
uint32_t calculateBlockHeadersLength(BlockHeaders * self);
/**
 @brief Deserializes a BlockHeaders so that it can be used as an object.
 @param self The BlockHeaders object
 @returns The length read on success, 0 on failure.
*/
uint32_t deserializeBlockHeaders(BlockHeaders * self);
/**
 @brief Serializes a BlockHeaders to the byte data.
 @param self The BlockHeaders object
 @returns The length written on success, 0 on failure.
*/
uint32_t serializeBlockHeaders(BlockHeaders * self);
/**
 @brief Takes a Block for the block header list. This does not retain the Block so you can pass an Block into this while releasing control from the calling function.
 @param self The BlockHeaders object
 @param address The Block to take.
 @returns true if the block header was taken successfully or false on error.
 */
int takeBlockFromBlockHeaderList(BlockHeaders * self,Block * header);

#endif /* BLOCKHEADERS_H_ */
