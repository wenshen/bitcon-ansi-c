/*
 * ChainDescriptor.h
 *
 * Created on: Nov 8, 2012
 * Created by: Wen Shen
 * Copyright (c) 2012 MIBitcoin Project Team
 */

/**
 @file
 @brief Stores up to 500 block hashes for a block chain to describe a block chain for a peer so that a peer can send relevent block data. Inherits from Message
 @details The Chain Descriptor Class
*/
#ifndef CHAINDESCRIPTOR_H_
#define CHAINDESCRIPTOR_H_

#include "Message.h"

/**
 @brief Structure for ChainDescriptor objects. @see ChainDescriptor.h
*/
typedef struct{
	Message base; /**< Message base structure */
	uint16_t numOfHash; /**< Number of block hashes to describe the block chain. Up to 500. */
	ByteArray ** hashes; /**< Hashes used to describe the block chain. This should contain hashes in the blockchain but not all of them. The maximum allowed is 500. The usual behaviour is to have the 10 last block hashes and then each hash below those going down to the genesis block has a gap that doubles (See https://en.bitcoin.it/wiki/Protocol_specification#getblocks ). The newest block hashes should come first. This should be NULL if empty. The GetBlocks object will release each ByteArray and free the array when the object is freed. */
} ChainDescriptor;

/**
 @brief Creates a new ChainDescriptor object.
 @returns A new ChainDescriptor object.
*/
ChainDescriptor * newChainDescriptor(void (*onErrorReceived)(Error error,char *,...));
/**
@brief Creates a new ChainDescriptor object from serialized data.
 @param data Serialized ChainDescriptor data.
 @returns A new ChainDescriptor object.
*/
ChainDescriptor * newChainDescriptorFromData(ByteArray * data,void (*onErrorReceived)(Error error,char *,...));

/**
 @brief Gets a ChainDescriptor from another object. Use this to avoid casts.
 @param self The object to obtain the ChainDescriptor from.
 @returns The ChainDescriptor object.
*/
ChainDescriptor * getChainDescriptor(void * self);

/**
 @brief Initializes a ChainDescriptor object
 @param self The ChainDescriptor object to initialize
 @returns true on success, false on failure.
*/
int initChainDescriptor(ChainDescriptor * self,void (*onErrorReceived)(Error error,char *,...));
/**
 @brief Initializes a ChainDescriptor object from serialized data
 @param self The ChainDescriptor object to initialize
 @param data The serialized data.
 @returns true on success, false on failure.
*/
int initChainDescriptorFromData(ChainDescriptor * self,ByteArray * data,void (*onErrorReceived)(Error error,char *,...));

/**
 @brief Frees a ChainDescriptor object.
 @param self The ChainDescriptor object to free.
 */
void freeChainDescriptor(void * self);

/**
 @brief Adds a hash to the ChainDescriptor onto the end.
 @param self The ChainDescriptor object
 @param hash The hash to add.
 @returns true if the hash was added successfully, false on error.
 */
int addHashToChainDescriptor(ChainDescriptor * self,ByteArray * hash);
/**
 @brief Deserializes a ChainDescriptor so that it can be used as an object.
 @param self The ChainDescriptor object
 @returns The length read on success, 0 on failure.
*/
uint16_t deserializeChainDescriptor(ChainDescriptor * self);
/**
 @brief Serializes a ChainDescriptor to the byte data.
 @param self The ChainDescriptor object
 @returns The length written on success, 0 on failure.
*/
uint16_t serializeChainDescriptor(ChainDescriptor * self);
/**
 @brief Takes a hash for the ChainDescriptor and puts it on the end. The hash is not retained so the calling function is releasing control.
 @param self The ChainDescriptor object
 @param hash The hash to take.
 @returns true if the hash was taken successfully, false on error.
 */
int takeHashForChainDescriptor(ChainDescriptor * self,ByteArray * hash);


#endif /* CHAINDESCRIPTOR_H_ */
