/*
 * TransactionInput.h
 *
 *  Created on: 07/10/2012
 *  Created by: Abdulfatai Popoola
 *  Modified by: Wen Shen, Krasnoshtan Dmytro
 *  Copyright (c) 2012 Bitcoin Project Team
 */


/** @file
 @brief TransactionInput object
 @details handles additional structure (transactionIn) for transaction.
*/

#ifndef TRANSACTIONINPUT_H_
#define TRANSACTIONINPUT_H_

/*An TransactionOutput is a reference to an output in a different transaction. Multiple inputs are often listed in a transaction. The values of the referenced outputs are added up, and the total is usable in the outputs of this transaction. */

#include "Message.h"
#include "../Script.h"
#include "TransactionOutput.h"

/**
 @struct PreviousOutput additional structure to help with prev_out
 @brief contains hash of the transaction and the number of outputs. Example of the structure:
 {
	hash: "0000000000000000000000000000000000000000000000000000000000000000",
	n: 4294967295
 }
*/
typedef struct{
	ByteArray * hash; /**< Hash of the transaction that includes the output for this input */
	uint32_t index; /**< The index of the output for this input */
} PreviousOutput;

/**
 @struct TransactionInput The structure that handles input in transactions
 @brief contains PreviousOutput structure and  Example of the structure:
 {
	prev_out: {
		hash: "0000000000000000000000000000000000000000000000000000000000000000",
		n: 4294967295
	},
	coinbase: "044c86041b0152"
 }
*/


typedef struct{
	Message base; /**< Message base structure */
	uint32_t sequence; /**< The version of this transaction input. Not used in protocol v0.3.18.00. Set to 0 for transactions that may someday be open to change after broadcast, set to _TRANSACTION_INPUT_FINAL if this input never needs to be changed after broadcast. */
	Script * scriptObject; /**< Contains script information as a Script. */
	PreviousOutput prevOutput; /**< A locator for a previous output being spent. */
} TransactionInput;

/**
 @fn createNewTransactionInput(Script * script, uint32_t sequence, ByteArray * prevOutputHash, uint32_t prevOutputIndex, void (*onErrorReceived)(Error error,char *,...))
 @brief Creates a new TransactionInput object with no inputs or outputs.

 @param script Computational Script for confirming transaction authorization
 @param sequence Transaction version as defined by the sender. Intended for "replacement" of transactions when information is updated before inclusion into a block.
 @param prevOutputHash. The hash of the previous transaction.
 @param prevOutputIndex Index of the previous transaction
 @param onErrorReceived Error handler
 @returns new TransactionInput object.
 */
TransactionInput * createNewTransactionInput(Script * script, uint32_t sequence, ByteArray * prevOutputHash, uint32_t prevOutputIndex, void (*onErrorReceived)(Error error,char *,...));


/**
 @fn createNewTransactionInputFromData(ByteArray * data,void (*onErrorReceived)(Error error,char *,...));
 @brief Creates a new TransactionInput from byteArray.

 @param data. Raw data to create a transactionInput
 @param onErrorReceived Error handler
 @returns new TransactionInput object.
 */
TransactionInput * createNewTransactionInputFromData(ByteArray * data,void (*onErrorReceived)(Error error,char *,...));

/**
 @fn initializeTransactionInput(TransactionInput * self, Script * script, uint32_t sequence, ByteArray * prevOutHash, uint32_t prevOutIndex, void (*onErrorReceived)(Error error,char *,...))
 @brief Initializes a TransactionInput object.

 @param self The TransactionInput object
 @param script Computational Script for confirming transaction authorization
 @param sequence Transaction version as defined by the sender. Intended for "replacement" of transactions when information is updated before inclusion into a block.
 @param prevOutputHash. The hash of the previous transaction.
 @param prevOutputIndex Index of the previous transaction
 @param onErrorReceived Error handler
 @returns TRUE on success, FALSE on failure.
 */
int initializeTransactionInput(TransactionInput * self, Script * script, uint32_t sequence, ByteArray * prevOutHash, uint32_t prevOutIndex, void (*onErrorReceived)(Error error,char *,...));

/**
 @fn initializeTransactionInputFromData(TransactionInput * self, ByteArray * data, void (*onErrorReceived)(Error error,char *,...))
 @brief Initializes a new TransactionInput object from the ByteArray

 @param self The TransactionInput object to initialize
 @param data The byte data.
 @param onErrorReceived Error handler
 @returns true on success, false on failure.
 */
int initializeTransactionInputFromData(TransactionInput * self, ByteArray * data, void (*onErrorReceived)(Error error,char *,...));


/**
 @fn getTransactionInput(void * self)
 @brief brief Gets a TransactionInput from another object. Use this to avoid casts

 @param self The TransactionInput object
 @returns TransactionInput object
 */
TransactionInput * getTransactionInput(void * self);

/**
 @fn destroyTransactionInput(void * self)
 @brief Frees a TransactionInput object.

 @param self the TransactionInput object
 @returns void
 */
void destroyTransactionInput(void * self);

/**
 @fn serializeTransactionInput(TransactionInput * self)
 @brief Serializes a TransactionInput to byte data

 @param self the TransactionInput object
 @returns the length written on success, 0 on failure
 */
uint32_t serializeTransactionInput(TransactionInput * self);

/**
 @fn deserializeTransactionInput(TransactionInput * self)
 @brief deserializes a TransactionInput so that it can be used as an object

 @param self The TransactionInput object
 @returns The length read on success, 0 on failure.
 */
uint32_t deserializeTransactionInput(TransactionInput * self);

#endif /* TRANSACTIONINPUT_H_ */
