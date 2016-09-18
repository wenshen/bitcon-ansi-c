/*
 * Block.h
 *
 *  Created on: 04/10/2012
 *  Created by: Wen Shen
 *  Modified by: Issak Gezehei
 *  Copyright (c) 2012 Bitcoin Project Team
 */

/** @file
 @brief This file defines a bitcoin block.
 @details Blocks contain transaction information and use a proof of work system to show that they are legitimate.
 */
#ifndef BLOCK_H_
#define BLOCK_H_

#include "Transaction.h"
#include "../../BigInt/BigInt.h"
#include "../../Constants.h"


/**
 @struct Block
 @brief Base class
 */
typedef struct {
	Message base; /**< Message base structure */
	uint8_t hash[32]; /**< The hash for this block. NULL if it needs to be calculated or set. */
	uint32_t version; /**< block version */
	ByteArray * previousBlockHash; /**< The previous block hash. */
	ByteArray * merkleTreeRootHash; /**< The merkle tree root hash. To limit the amount of data transferred when synchronizing*/
	uint32_t time; /**< Timestamp for the block. The network uses 32 bits. The protocol can be future proofed by detecting overflows when going through the blocks. So if a block's time overflows such that the time is less than the median of the last 10 blocks, the block can be seen by adding the first 32 bits of the network time and finally the timestamp can be tested against the network time. The overflow problem can therefore be fixed by a workaround but it is a shame Satoshi did not use 64 bits. */
	uint32_t target; /**< The compact target representation. */
	uint32_t nonce; /**<  A 32-bit (4-byte) field whose value is set so that the hash of the block will contain a run of zeros in generating blocks*/
	uint32_t numOfTransactions; /**< Number of transactions in the block. */
	Transaction ** transactions; /**< The transactions included in this block. NULL if only the header has been received. */
} Block;

/**
 @fn Block createNewBlock(void(*onErrorReceived)(Error error, char *, ...))
 @brief Creates a new Block
 @returns Block
 */
Block * createNewBlock(void(*onErrorReceived)(Error error, char *, ...));

/**
 @fn Block createNewBlockFromByteArray(ByteArray * data, void(*onErrorReceived)(Error error, char *, ...))
 @brief Creates a new Block
 @param data
 @returns Block
 */
Block * createNewBlockFromByteArray(ByteArray * data,
		void(*onErrorReceived)(Error error, char *, ...));
      
/**
 @fn Block * createNewBlockWithOriginalBlockvoid(*onErrorReceived)(Error error, char *, ...))
 @brief Creates a new Block object with the original block for the bitcoin block chain. This will have serialised data as well as object data.
 @param data Serialised block data.
 @returns A new Block object.
 */
Block * createNewBlockWithOriginalBlock(
		void(*onErrorReceived)(Error error, char *, ...));

/**
 @fn Block * getBlock(void * self)
 @brief Gets a Block from another object. Use this to avoid casts.
 @param self The object to obtain the Block from.
 @returns The Block object.
 */
Block * getBlock(void * self);

/**
 @fn initBlock(Block * self void(*onErrorReceived)(Error error, char *, ...))
 @brief Initializes a Block object
 @param self The Block object to initialize
 @returns true on success, false on failure.
 */
int initBlock(Block * self,
		void(*onErrorReceived)(Error error, char *, ...));
      
/**
 @fn int initBlockFromByteArray(Block * self, ByteArray * data,void(*onErrorReceived)(Error error, char *, ...))
 @brief Initializes a Block object from serialized data
 @param self The Block object to initialize
 @param data The serialized data.
 @returns true on success, false on failure.
 */
int initBlockFromByteArray(Block * self, ByteArray * data,
		void(*onErrorReceived)(Error error, char *, ...));

/**
 @fn int initBlockWithOriginalBlock(Block * self, void(*onErrorReceived)(Error error, char *, ...))
 @brief Initializes a Block object with the original block for the bitcoin block chain. This will have serialized data as well as object data.
 @param self The Block object to initialize.
 @param onErrorReceived
 @returns A new Block object.
 */
int initBlockWithGenesisBlockInfo(Block * self,
		void(*onErrorReceived)(Error error, char *, ...));

/**
 @fn void deleteBlock(void * vself)
 @brief Frees a Block object.
 @param self The Block object to free.
 */
void destroyBlock(void * vself);

/**
 @fn void calculateBlockHash(Block * self, uint8_t * hash)
 @brief Calculates the hash for a block.
 @param self The Block object. This should be serialized.
 @param The hash for the block to be set. This should be 32 bytes long.
 */
void calculateBlockHash(Block * self, uint8_t * hash);

/**
 @fn uint32_t calculateBlockLength(Block * self, int transactions)
 @brief Calculates the length needed to serialize the object.
 @param self The Block object.
 @param transactions If true, the full block, if not true just the header.
 @returns The length read on success, 0 on failure.
 */
uint32_t calculateBlockLength(Block * self, int transactions);

/**
 @fn uint32_t deserializeBlock(Block * self, int transactions)
 @brief Deserializes a Block so that it can be used as an object.
 @param self The Block object
 @param transactions If true deserialize transactions. If false there do not deserialize for transactions.
 @returns The length read on success, 0 on failure.
 */
uint32_t deserializeBlock(Block * self, int transactions);

/**
 @fn uint32_t deserializeWithOnlyHeaders(Block * self, uint8_t firstByte, ByteArray * bytes)
 @ breif deserialize Block only with headers
 @ param self The Block object
 @ param firstByte uint8_t
 @ param bytes ByteArray object
 @ The length read on success, o on failure.
 */
uint32_t deserializeWithOnlyHeaders(Block * self, uint8_t firstByte, ByteArray * bytes);

/**
 @fn uint32_t deserializeWithTransactions (Block * self, ByteArray * bytes)
 @ breif deserialize Block with transactions
 @ param self The Block object
 @ param bytes ByteArray object
 @ The length read on success, o on failure.
 */
uint32_t deserializeWithTransactions (Block * self, ByteArray * bytes);

/**
 @fn uint8_t * getBlockHash(Block * self)
 @brief Retrieves or calculates the hash for a block. Hashes taken from this function are cached.
 @param self The Block object. This should be serialised.
 @returns The hash for the block. This is a 32 byte long, double SHA-256 hash and is a pointer to the hash field in the block.
 */
uint8_t * getBlockHash(Block * self);

/**
 @fn uint32_t serializeBlock(Block * self, int transactions)
 @brief Serializes a Block to the byte data.
 @param self The Block object
 @param transactions If true serialize transactions. If false there do not serialize for transactions.
 @returns The length read on success, 0 on failure.
 */
uint32_t serializeBlock(Block * self, int transactions);

/**
 @fn void dealWithSerializationHeader(Block * self, ByteArray * bytes)
 @brief To deal with the header issue for serializeBlock.
 @param self The Block object
 @param bytes The ByteArray
 */
void dealWithSerializationHeader(Block * self, ByteArray * bytes);

/**
 @fn uint32_t dealWithSerializationTran(Block * self, ByteArray * bytes,uint32_t cursor, int transactions)
 @briefTo dealt with the transaction issue for serializeBlock.
 @param self The Block object
 @param bytes The ByteArray
 @param cursor The initial cursor
 @returns The cursor
 */
uint32_t dealWithSerializationTran(Block * self, ByteArray * bytes,
		uint32_t cursor, int transactions);

#endif /* BLOCK_H_ */
