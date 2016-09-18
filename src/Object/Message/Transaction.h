/*
 * Transcation.h
 *
 *  Created on: 03/10/2012
 *  Created by: Abdulfatai Popoola
 *  Copyright (c) 2012 Bitcoin Project Team
 */

/** @file
 @brief Transaction object
 @details The Transaction object handles all details of transactions such as hashing, serialization and deserialization, transaction inputs and transaction outputs.
 It also handles locking, versioning and calculating the length of transactions.
 */

#ifndef TRANSACTION_H_
#define TRANSACTION_H_

#include "Message.h"
#include "TransactionInput.h"
#include "TransactionOutput.h"
#include "../../Utils/Cryptography/Crypt.h"

/**
 @struct Header file for the Transaction Object; contains the Transaction base structure.
 @brief Structure for Transaction objects; contains all the fields defined in the bitcoin protocol. @see Transaction.h
*/

typedef struct{
    Message base; /**< Message base structure> */
	uint8_t hash[32]; /**< The hash for this transaction. Null by default*/
	uint32_t version; /**< Version of the transaction data format. 1 by default*/
	uint32_t numOfTransactionInput; /**< Number of TransactionInputs. How many inputs are already added to the transaction*/
	TransactionInput ** inputs; /**< TransactionInput base structure*/
	uint32_t numOfTransactionOutput; /**< Number of TransactionOutputs. How many outputs are already added to the transaction */
	TransactionOutput ** outputs; /**< TransactionOutput base structure*/
	uint32_t lockTime; /**< The block number or timestamp at which this transaction is locked; A non-locked transaction must not be included in blocks. */
} Transaction;

/**
 @fn Transaction createNewTransaction(uint32_t lockTime, uint32_t version, void (*onErrorReceived)(Error error,char *,...))
 @brief Creates a new Transaction object with no inputs or outputs.
 @param lockTime The timestamp which determines if this transaction is locked. 0 means the transaction is always locked; values < 500000000 indicate the block number at which this transaction is locked while values >= 500000000 indicate the UNIX timestamp at which this transaction is locked
 @param version Transaction data format version
 @param onErrorReceived Error handler
 @returns A new Transaction object.
 */
Transaction * createNewTransaction(uint32_t lockTime, uint32_t version, void (*onErrorReceived)(Error error,char *,...));

/**
 @fn Transaction createNewTransactionFromByteArray(ByteArray * bytes, void (*onErrorReceived)(Error error,char *,...))
 @brief Creates a new Transaction object from byte data. Should be serialised for object data.
 @param bytes Array of bytes to be used for creation of transaction
 @param onErrorReceived Error handler
 @returns A new Transaction object.
 */
Transaction * createNewTransactionFromByteArray(ByteArray * bytes, void (*onErrorReceived)(Error error,char *,...));

/**
 @fn int setUpTransactionData(Transaction * self, uint32_t lockTime, uint32_t version, void (*onErrorReceived)(Error error,char *,...))
 @brief Sets up the fields of a Transaction object
 @param self The Transaction object whose fields are to be set
 @param lockTime The timestamp which determines if this transaction is locked. 0 means the transaction is always locked; values < 500000000 indicate the block number at which this transaction is locked while values >= 500000000 indicate the UNIX timestamp at which this transaction is locked
 @param version Transaction data format version
 @param onErrorReceived Error handler
 @returns 1 on success, 0 on failure.
 */
int setUpTransactionData(Transaction * self, uint32_t lockTime, uint32_t version, void (*onErrorReceived)(Error error,char *,...));

/**
 @fn int setUpTransactionDataUsingByteArray(Transaction * self, ByteArray * data,void (*onErrorReceived)(Error error,char *,...))
 @brief Sets up the fields of a Transaction object using a byteArray object that was passed in
 @param bytes Array of bytes to be used for creation of transaction
 @param onErrorReceived Error handler
 @returns 1 on success, 0 on failure.
 */
int setUpTransactionDataUsingByteArray(Transaction * self, ByteArray * data,void (*onErrorReceived)(Error error,char *,...));

/**
 @fn void destroyTransaction(void * self)
 @brief Destructor; destroys a Transaction object.
 @param self The Transaction object to free.
 @returns void
 */
void destroyTransaction(void * self);

/**
 @fn Transaction getTransaction(void * self)
 @brief Gets a Transaction from another object. Use this to avoid explicit casts.
 @param self The object to obtain the Transaction from.
 @returns A Transaction object.
 */
Transaction * getTransaction(void * self);

/**
 @fn int addTransactionInput(Transaction * self, TransactionInput * input)
 @brief Adds a TransactionInput to a Transaction Object
 @param self The Transaction object.
 @param input The TransactionInput object to add to the self Transaction
 @returns 1 if the transaction input was added successfully and 0 on error.
 */
int addTransactionInput(Transaction * self, TransactionInput * input);
/**
 @fn int addTransactionOutput(Transaction * self, TransactionOutput * output)
 @brief Adds an TransactionOutput to the Transaction.
 @param self The Transaction object.
 @param output The TransactionOutput object to add to the self Transaction.
 @returns 1 if the transaction output was added successfully and 0 on error.
 */
int addTransactionOutput(Transaction * self, TransactionOutput * output);
/**
 @fn void calculateTransactionHash(Transaction * self, uint8_t * hash)
 @brief Calculates the hash for a transaction.
 @param self The Transaction object. This should be serialised.
 @returns void
 */
void calculateTransactionHash(Transaction * self);

/**
 @fn uint8_t getTransactionHash(Transaction * self)
 @brief Retrieves or calculates the hash for a transaction. Hashes taken from this function are cached.
 @param self The Transaction object. This should be serialised.
 @returns The hash for the transaction. This is a 32 byte long, double SHA-256 hash and is a pointer to the hash field in the transaction.
 */
uint8_t * getTransactionHash(Transaction * self);

/**
 @fn uint8_t calculateTransactionLength(Transaction * self)
 @brief Calculates the length of an Transaction; this is needed to serialise the object.
 @param self The Transaction object.
 @returns The length of the Transaction on success, 0 on failure.
 */
uint32_t calculateTransactionLength(Transaction * self);

/**
 @fn int isCoinBase(Transaction * self)
 @brief Determines if a transaction is a coinbase transaction or not.
 @param self The Transaction object.
 @returns 1 if the transaction is a coin-base transaction or 0 if not.
 */
int isCoinBase(Transaction * self);
/**
 @fn uint32_t serializeTransaction(Transaction * self)
 @brief Serialises a Transaction to byte data.
 @param self The Transaction object.
 @returns The length read on success, 0 on failure.
 */
uint32_t serializeTransaction(Transaction * self);

/**
 @fn uint32_t deserializeTransaction(Transaction * self)
 @brief Deserialises a Transaction so that it can be used as an object.
 @param self The Transaction object
 @returns The length read on success, 0 on failure.
 */

uint32_t deserializeTransaction(Transaction * self);

/** TRANSACTION SIGNING */
/**
 @fn TransactionHashStatus getTransactionInputHashForVerification(void * vself, ByteArray * prevOutSubScript, uint32_t input, SignatureType signType, uint8_t * hash)
 @brief Gets the hash for signing or signature checking for a transaction input. The transaction input needs to contain the outPointerHash, outPointerIndex and sequence. If these are modifed afterwards then the signiture is invalid.
 @param vself The Transaction object.
 @param prevOutSubScript The sub script from the output. Must be the correct one or the signiture will be invalid.
 @param input The index of the input to sign.
 @param signType The type of signature to get the data for.
 @param hash The 32 byte data hash for signing or checking signatures.
 @returns TX_HASH_OK if the hash has been retreived with no problems. TX_HASH_BAD is returned if the hash is invalid and TX_HASH_ERR is returned upon an error.
 */
TransactionHashStatus getTransactionInputHashForVerification(void * vself, ByteArray * prevOutSubScript, uint32_t input, SignatureType signType, uint8_t * hash);
/**
 @brief Adds an TransactionInput to the Transaction without retaining it.
 @param self The Transaction object.
 @param input The TransactionInput object.
 @returns true if the transaction input was taken successfully and false on error.
 */
int takeTransactionInput(Transaction * self, TransactionInput * input);
/**
 @brief Adds an TransactionInput to the Transaction without retaining it.
 @param self The Transaction object.
 @param input The TransactionOutput object.
 @returns true if the transaction output was taken successfully and false on error.
 */
int takeTransactionOutput(Transaction * self, TransactionOutput * output);

#endif
