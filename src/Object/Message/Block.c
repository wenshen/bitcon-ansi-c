/*
 * Block.c
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

#include "Block.h"
#include "assert.h"
#include "../ByteArray.h"
#include "ctype.h"
#include "../../Utils/VariableLengthInteger/VarLenInt.h"
#include "Transaction.h"
#include "../../Utils/Cryptography/Crypt.h"
#include "Message.h"

/**
 @fn Block * createNewBlock(void(*onErrorReceived)(Error error, char *, ...)) 
 @brief Creates a new Block
 @returns Block
 */

Block * createNewBlock(void(*onErrorReceived)(Error error, char *, ...)) {

	assert(onErrorReceived != NULL);
	Block * block = malloc(sizeof(*block));
	if (!block) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,
				"Cannot allocate %i bytes of memory in createNewBlock\n",
				sizeof(*block));
		return NULL;
	}
	getObject(block)->destroy = destroyBlock;
	int isSuccessful = initBlock(block, onErrorReceived);

	if (isSuccessful) {
		return block; /**< created successfully */
	}
	free(block); /**< free the memory */
	return NULL; /**<  created failed */
}


/**
 @fn Block * createNewBlockFromByteArray(ByteArray * info, void(*onErrorReceived)(Error error, char *, ...)) 
 @brief Create a new Block from a given ByteArray data
 @param info
 @returns Block
 */

Block * createNewBlockFromByteArray(ByteArray * info,
		void(*onErrorReceived)(Error error, char *, ...)) {
	assert(info != NULL);
	assert(onErrorReceived != NULL);
	Block * block = malloc(sizeof(*block));
	if (!block) {
		onErrorReceived(
				ERROR_OUT_OF_MEMORY,
				"Cannot allocate %i bytes of memory in createNewBlockFromByteArray\n",
				sizeof(*block));
		return NULL;
	}
	getObject(block)->destroy = destroyBlock;
	int isSuccessful = initBlockFromByteArray(block, info, onErrorReceived);
	if (isSuccessful) {
		return block; /**< created successfully */
	}
	free(block); /**< free the memory */
	return NULL; /**<  created failed */
}

/**
 @fn Block * createNewBlockFromOriginalBlock(void(*onErrorReceived)(Error error, char *, ...)) 
 @brief Create a new Block from a default Block
 @returns Block
 */
Block * createNewBlockFromOriginalBlock(
		void(*onErrorReceived)(Error error, char *, ...)) {

	assert(onErrorReceived != NULL);
	Block * block = malloc(sizeof(*block));
	if (!block) {
		onErrorReceived(
				ERROR_OUT_OF_MEMORY,
				"Cannot allocate %i bytes of memory in createNewBlockFromOriginalBlock\n",
				sizeof(*block));
		return NULL;
	}
	getObject(block)->destroy = destroyBlock;
	int isSuccessful = initBlockWithGenesisBlockInfo(block, onErrorReceived);
	if (isSuccessful) {
		return block; /**< created successfully */
	}
	free(block); /**< free the memory */
	return NULL; /**< created failed */
}

/**
 @fn Block * getBlock(void * block)
 @brief get a block Object
 @param block
 @returns Block
 */

Block * getBlock(void * block)
{
	assert(block!= NULL);

	return block;
}

/**
 @fn int initBlock(Block * self, void(*onErrorReceived)(Error error, char *, ...))
 @brief Initialize the Block from 
 @param self
 @returns TRUE or FALSE
 */

int initBlock(Block * self, void(*onErrorReceived)(Error error, char *, ...)) {
	assert(self!= NULL);
	assert(onErrorReceived != NULL);
	self->previousBlockHash = NULL;
	self->merkleTreeRootHash = NULL;
	self->transactions = NULL;
	self->numOfTransactions = 0;

	int isSuccessful = initializeMessageFromObject(getMessage(self), onErrorReceived);
	if (isSuccessful){
		return TRUE;
	}else{
		return FALSE;
	}

}

/**
 @fn int initBlockFromByteArray(Block * self, ByteArray * data, void(*onErrorReceived)(Error error, char *, ...))
 @brief Initialize the Block from ByteArray
 @param self
 @param data
 @returns TRUE or FALSE
 */
int initBlockFromByteArray(Block * self, ByteArray * data,
		void(*onErrorReceived)(Error error, char *, ...)) {
	assert(self!= NULL);
	assert(onErrorReceived != NULL);
	self->previousBlockHash = NULL;
	self->merkleTreeRootHash = NULL;
	self->transactions = NULL;
	self->numOfTransactions = 0;
	int isSuccessful = initializeMessageFromByteArrayData(getMessage(self), data, onErrorReceived);
	if (isSuccessful){
		return TRUE;
	}
	return FALSE;

}

/**
 @fn int initBlockWithOriginalBlock(Block * self,void(*onErrorReceived)(Error error, char *, ...))
 @brief Initialize the Block from ByteArray
 @param self
 @returns TRUE or FALSE
 */
int initBlockWithGenesisBlockInfo(Block * self,void(*onErrorReceived)(Error error, char *, ...)) {
	assert(self!= NULL);
	assert(onErrorReceived != NULL);
	uint8_t uint_data[285] = {0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3B,0xA3,0xED,0xFD,0x7A,0x7B,0x12,0xB2,0x7A,0xC7,0x2C,0x3E,0x67,0x76,0x8F,0x61,0x7F,0xC8,0x1B,0xC3,0x88,0x8A,0x51,0x32,0x3A,0x9F,0xB8,0xAA,0x4B,0x1E,0x5E,0x4A,0x29,0xAB,0x5F,0x49,0xFF,0xFF,0x00,0x1D,0x1D,0xAC,0x2B,0x7C,0x01,0x01,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0x4D,0x04,0xFF,0xFF,0x00,0x1D,0x01,0x04,0x45,0x54,0x68,0x65,0x20,0x54,0x69,0x6D,0x65,0x73,0x20,0x30,0x33,0x2F,0x4A,0x61,0x6E,0x2F,0x32,0x30,0x30,0x39,0x20,0x43,0x68,0x61,0x6E,0x63,0x65,0x6C,0x6C,0x6F,0x72,0x20,0x6F,0x6E,0x20,0x62,0x72,0x69,0x6E,0x6B,0x20,0x6F,0x66,0x20,0x73,0x65,0x63,0x6F,0x6E,0x64,0x20,0x62,0x61,0x69,0x6C,0x6F,0x75,0x74,0x20,0x66,0x6F,0x72,0x20,0x62,0x61,0x6E,0x6B,0x73,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0xF2,0x05,0x2A,0x01,0x00,0x00,0x00,0x43,0x41,0x04,0x67,0x8A,0xFD,0xB0,0xFE,0x55,0x48,0x27,0x19,0x67,0xF1,0xA6,0x71,0x30,0xB7,0x10,0x5C,0xD6,0xA8,0x28,0xE0,0x39,0x09,0xA6,0x79,0x62,0xE0,0xEA,0x1F,0x61,0xDE,0xB6,0x49,0xF6,0xBC,0x3F,0x4C,0xEF,0x38,0xC4,0xF3,0x55,0x04,0xE5,0x1E,0xC1,0x12,0xDE,0x5C,0x38,0x4D,0xF7,0xBA,0x0B,0x8D,0x57,0x8A,0x4C,0x70,0x2B,0x6B,0xF1,0x1D,0x5F,0xAC,0x00,0x00,0x00,0x00};

	ByteArray *data = createNewByteArrayUsingDataCopy(uint_data, 285, onErrorReceived);
	if (data == NULL){
		return FALSE;
	}
	uint8_t genesisHash[32] = {0x6F,0xE2,0x8C,0x0A,0xB6,0xF1,0xB3,0x72,0xC1,0xA6,0xA2,0x46,0xAE,0x63,0xF7,0x4F,0x93,0x1E,0x83,0x65,0xE1,0x5A,0x08,0x9C,0x68,0xD6,0x19,0x00,0x00,0x00,0x00,0x00};
	memcpy(self->hash, genesisHash, 32);
	int isSuccessful = initializeMessageFromByteArrayData(getMessage(self), data, onErrorReceived);
	if (! isSuccessful) {
		decrementReferenceCount(data); /**< Decrease the count of  the ByteArray data*/
		decrementReferenceCount(self->hash);/**< Decrease the count of the hash*/
		return FALSE;
	}
	decrementReferenceCount(data);/**< Decrease the count of  the ByteArray data*/
	deserializeBlock(self, TRUE);/**< Deserialize the Block so that it can be used as an object*/
	return TRUE;
}

/**
 @fn void deleteBlock(void * selfCopy)
 @brief Destroy the Block
 @param selfCopy
  */
void destroyBlock(void * selfCopy) {
	assert(selfCopy != NULL);
	Block * self = selfCopy;

	/**< Destroy the previous Block hash*/
	if (self->previousBlockHash) {
		decrementReferenceCount(self->previousBlockHash);
	}
	/**< Destroy merkle Tree Root hash*/
	if (self->merkleTreeRootHash) {
		decrementReferenceCount(self->merkleTreeRootHash);
	}
	/** Destroy transactions*/
	if (self->transactions) { /**<  Check for the loop since the transaction number can be set without having any transactions. */
		uint32_t i;
		for (i = 0; i < self->numOfTransactions; i++){
			if (self->transactions[i]){
				decrementReferenceCount(self->transactions[i]);
			}
		}
		free(self->transactions);
	}
	/**< Destroy hash*/
	if (self->hash) {
		decrementReferenceCount(self->hash);
	}
	/**< Destroy the Message base*/
	destroyMessage(getObject(self));
}


/**
 @fn void calculateBlockHash(Block * self, uint8_t * hash) {
 @brief Calculate the hash for a Block.
 @param self
 @param hash
  */
void calculateBlockHash(Block * self, uint8_t * hash) {
	assert(self!=NULL && hash != NULL);
	uint8_t * headerData = getByteArrayData(getMessage(self)->bytes);
	uint8_t hash2[32];
	Sha256(headerData, 80, hash2);
	Sha256(hash2, 32, hash);
}


/**
 @fn uint32_t calculateBlockLength(Block * self, int transactions)
 @brief Calculate the length of Block that is needed to serialize the object
 @param self
 @param transactions
 @return blocklength
 */
uint32_t calculateBlockLength(Block * self, int transactions) {
	assert(self!=NULL && transactions != NULL);
	uint32_t len = 80 + getSizeOfVarLenInt(self->numOfTransactions);
	/**< calculate TransactinLength*/
	if (transactions) {
		uint32_t x;
		for (x = 0; x < self->numOfTransactions; x++) {
			uint32_t tLen = calculateTransactionLength(self->transactions[x]);
			if (!tLen)
				return 0;
			len += tLen;
		}
		return len;
	} else
		return len + 1; /**< Plus the stupid pointless null byte. */
}

/**
 @fn uint32_t deserializeBlock(Block * self, int transactions)
 @brief Deserialize the Block
 @param self
 @param transactions
 @return 0 
 */
uint32_t deserializeBlock(Block * self, int transactions) {
	assert(self!= NULL );
	/*Parameters checking*/
	assert(transactions != NULL && !isalpha(transactions) && transactions/1.00 == (int) transactions);
	ByteArray * bytes = getMessage(self)->bytes;
	if (bytes== NULL) {
		getMessage(self)->onErrorReceived(
				ERROR_MESSAGE_DESERIALISATION_NULL_BYTES,
				"Attempting to deserialize a Block with no bytes.");
		return 0;
	}
	if (bytes->length < 82) {
		getMessage(self)->onErrorReceived(
				ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,
				"Attempting to deserialize a Block with less than 82 bytes. Minimum for header (With null byte).");
		return 0;
	}
	self->version = readInt32AsLittleEndianFromByteArray(bytes, 0);
	self->previousBlockHash = getByteArraySubsectionReference(bytes, 4, 32);
	if (!self->previousBlockHash) {
		getMessage(self)->onErrorReceived(ERROR_INIT_FAIL,
				"Cannot create the previous block hash ByteArray in deserializeBlock.");
		return 0;
	}
	self->merkleTreeRootHash = getByteArraySubsectionReference(bytes, 36, 32);
	if (!self->merkleTreeRootHash) {
		getMessage(self)->onErrorReceived(ERROR_INIT_FAIL,
				"Cannot create the merkleTree root ByteArray in deserializeBlock.");
		return 0;
	}
	self->time = readInt32AsLittleEndianFromByteArray(bytes, 68);
	self->target = readInt32AsLittleEndianFromByteArray(bytes, 72);
	self->nonce = readInt32AsLittleEndianFromByteArray(bytes, 76);
	/**< If first VarLenInt byte is zero, then stop here for headers, otherwise look for 8 more bytes and continue */
	uint8_t firstByte = getByteFromByteArray(bytes, 80);
	if (transactions && firstByte) {
		 deserializeWithTransactions(self,bytes);
	} else { /**< Just header*/
		deserializeWithOnlyHeaders(self, firstByte, bytes);
	}
}

/**
 @fn uint32_t deserializeWithTransactions (Block * self, ByteArray * bytes)
 @brief Deserialize With Transaction
 @param self
 @param bytes
 @return 0 
 */
uint32_t deserializeWithTransactions (Block * self, ByteArray * bytes){
	assert(self!= NULL && bytes != NULL);
	if (bytes->length < 89) {
		getMessage(self)->onErrorReceived(
				ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,
				"Attempting to deserialize a Block with a non-zero varint with less than 89 bytes.");
		return 0;
	}
	VarLenInt numOfTransactionsVarInt = decodeVarLenInt(bytes, 80);
	if (numOfTransactionsVarInt.value * 60 > bytes->length - 81) {
		getMessage(self)->onErrorReceived(
				ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,
				"Attempting to deserialize a Block with too many transactions for the byte data length.");
		return 0;
	}
	self->numOfTransactions = (uint32_t) numOfTransactionsVarInt.value;
	self->transactions = malloc(
			sizeof(*self->transactions) * self->numOfTransactions);
	if (!self->numOfTransactions) {
		getMessage(self)->onErrorReceived(
				ERROR_OUT_OF_MEMORY,
				"Cannot allocate %i bytes of memory in deserializeWithTransactions\n",
				sizeof(*self->transactions) * self->numOfTransactions);
		return 0;
	}
	uint32_t cursor = 80 + numOfTransactionsVarInt.storageSize;
	uint16_t i;
	for (i = 0; i < self->numOfTransactions; i++) {
		ByteArray * data = getByteArraySubsectionReference(bytes,
				cursor, bytes->length - cursor);
		if (!data) {
			getMessage(self)->onErrorReceived(
					ERROR_INIT_FAIL,
					"Could not create a new ByteArray in deserializeWithTransactions for the transaction number %u.",
					i);
		}
		Transaction * transaction =
				createNewTransactionFromByteArray(data,
						getMessage(self)->onErrorReceived);
		if (!transaction) {
			getMessage(self)->onErrorReceived(
					ERROR_INIT_FAIL,
					"Could not create a new Transaction in deserializeWithTransactions for the transaction number %u.",
					i);
			decrementReferenceCount(data);
			return 0;
		}
		uint32_t len = deserializeTransaction(transaction);
		if (!len) {
			getMessage(self)->onErrorReceived(
					ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,
					"Block cannot be deserialized because of an error with the transaction number %u.",
					i);
			decrementReferenceCount(data);
			return 0;
		}
		/**< Readjust ByteArray length */
		data->length = len;
		decrementReferenceCount(data);
		self->transactions[i] = transaction;
		cursor += len;
	}
			return cursor;
}

/**
 @fn uint32_t deserializeWithOnlyHeaders(Block * self, uint8_t firstByte, ByteArray * bytes)
 @brief Deserialize With Transaction
 @param self
 @param firstByte
 @param bytes
 @return 0 
 */
uint32_t deserializeWithOnlyHeaders(Block * self, uint8_t firstByte, ByteArray * bytes){
	assert(self!= NULL && firstByte != NULL && bytes!=NULL);
	uint8_t i;
	if (firstByte < 253) {
		i = 1;
	} else if (firstByte == 253) {
		i = 2;
	} else if (firstByte == 254) {
		i = 4;
	} else {
		i = 8;
	}
	if (bytes->length < 80 + i + 1) {
		getMessage(self)->onErrorReceived(
				ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,
				"Attempting to deserialize a Block header with not enough space to cover the var int.");
		return 0;
	}
	self->numOfTransactions = (uint32_t) decodeVarLenInt(bytes, 80).value; /**< This value is undefined in the protocol. Should best be zero when getting the headers since there is not supposed to be any transactions. Would have probably been better if the var int was dropped completely for headers only.*/
	/**< Ensure null byte is null. This null byte is a bit of a nuissance but it exists in the protocol when there are no transactions. */
	if (getByteFromByteArray(bytes, 80 + i) != 0) {
		getMessage(self)->onErrorReceived(
				ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,
				"Attempting to deserialize a Block header with a final byte which is not null. This is not what it is supposed to be but you already knew that right?");
		return 0;
	}
	return 80 + i + 1; /**< 80 header bytes, the var int and the null byte */
}


/**
 @fn uint8_t * getBlockHash(Block * self)
 @brief Get the hash of the Block
 @param self
 @return hash
 */
uint8_t * getBlockHash(Block * self) {
	assert(self != NULL);
	if (!self->hash)
		calculateBlockHash(self, self->hash);
	return self->hash;
}

/*
 @fn uint32_t serializeBlock(Block * self, int transactions)
 @brief Serialize Block
 @param self
 @param transactions
 @return 0
 */
uint32_t serializeBlock(Block * self, int transactions) {
	assert(self!= NULL );
	assert(transactions >= 0);
	ByteArray * bytes = getMessage(self)->bytes;
	if (!bytes) {
		getMessage(self)->onErrorReceived(
				ERROR_MESSAGE_SERIALISATION_NULL_BYTES,
				"Attempting to serialize a Block with no bytes.");
		return 0;
	}
	VarLenInt numOfTransactions = createVarLenIntFromUInt64(
			self->numOfTransactions);
	uint32_t cursor = 80 + numOfTransactions.storageSize;
	if (bytes->length < cursor + 1) {
		getMessage(self)->onErrorReceived(
				ERROR_MESSAGE_SERIALISATION_BAD_BYTES,
				"Attempting to serialize a Block with less bytes than required for the header, transaction number var int and at least a null byte. %i < %i\n",
				bytes->length, cursor);
		return 0;
	}
	/**< Deal with the header */
	dealWithSerializationHeader(self, bytes);

	/**< Deal with the Transactions */
	encodeVarLenInt(bytes, 80, numOfTransactions);
	cursor = dealWithSerializationTran(self, bytes, cursor, transactions);
	return cursor;

}

/*
 @fn void dealWithSerializationHeader(Block * self, ByteArray * bytes)
 @brief Serialize the header in a Block
 @param self
 @param bytes
 */

void dealWithSerializationHeader(Block * self, ByteArray * bytes) {
	assert(self!=NULL && bytes!=NULL);
	writeInt32AsLittleEndianIntoByteArray(bytes, 0, self->version);
	copyByteArrayToByteArray(bytes, 4, self->previousBlockHash);
	changeByteArrayDataReference(self->previousBlockHash, bytes, 4);
	copyByteArrayToByteArray(bytes, 36, self->merkleTreeRootHash);
	changeByteArrayDataReference(self->merkleTreeRootHash, bytes, 36);
	writeInt32AsLittleEndianIntoByteArray(bytes, 68, self->time);
	writeInt32AsLittleEndianIntoByteArray(bytes, 72, self->target);
	writeInt32AsLittleEndianIntoByteArray(bytes, 76, self->nonce);
}

/*
 @fn void dealWithSerializationHeader(Block * self, ByteArray * bytes)
 @brief Serialize the transactions in a Block
 @param self
 @param bytes
 */
uint32_t dealWithSerializationTran(Block * self, ByteArray * bytes,
		uint32_t cursor, int transactions) {
	assert(self!=NULL && bytes!=NULL && cursor >= 0);
	if (transactions) {
		uint32_t x;
		for (x = 0; x < self->numOfTransactions; x++) {
			getMessage(self->transactions[x])->bytes
					= getByteArraySubsectionReference(bytes, cursor,
							bytes->length - cursor);
			if (!getMessage(self->transactions[x])->bytes) {
				getMessage(self)->onErrorReceived(
						ERROR_INIT_FAIL,
						"Cannot create a new ByteArray sub reference in dealWithSerializationTran for the transaction number %u",
						x);
				return 0;
			}
			uint32_t len = serializeTransaction(self->transactions[x]);
			if (!len) {
				getMessage(self)->onErrorReceived(
						ERROR_MESSAGE_SERIALISATION_BAD_BYTES,
						"Block cannot be serialized because of an error with the transaction number %u.",
						x);
				return 0;
			}
			getMessage(self->transactions[x])->bytes->length = len;
			cursor += len;
		}
	} else {
		/**< Add null byte since there are to be no transactions (header only). */
		setByteInByteArray(bytes, cursor, 0);
		cursor++;
	}
	return cursor;
}
