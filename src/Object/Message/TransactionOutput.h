/*
 * TransactionInput.c
 *
 *  Created on: 07/10/2012
 *  Created by: Abdulfatai Popoola
 *  Modified by: Wen Shen
 *  Copyright (c) 2012 Bitcoin Project Team
 */
#ifndef TRANSACTIONOUTPUT_H_
#define TRANSACTIONOUTPUT_H_
/*An output contains instructions for sending bitcoins*/

#include "Message.h"
#include "../Script.h"


typedef struct{
    Message base; /**< Message base structure */
	uint64_t value; /**< The transaction value is the number of Satoshi (1 BTC = 100,000,000 Satoshi) that this output will be worth when claimed*/
	Script * scriptObject; /**< The output script object */
} TransactionOutput;


/* CONSTRUCTORS */
/**
 @brief Creates a new TransactionOutput object.
 @returns A new TransactionOutput object.
 */
TransactionOutput * createNewTransactionOutput(uint64_t value, Script * script,void (*onErrorReceived)(Error error,char *,...));
/**
 @brief Creates a new TransactionOutput object from byte data. Should be serialised for object data.
 @returns A new TransactionOutput object.
 */
TransactionOutput * createNewTransactionOutputFromData(ByteArray * data, void (*onErrorReceived)(Error error,char *,...));


/**
 @brief Initialises a TransactionOutput object.
 @param self The TransactionOutput object to initialise.
 @returns true on success, false on failure.
 */
int initializeTransactionOutput(TransactionOutput * self, uint64_t value, Script * script,void (*onErrorReceived)(Error error,char *,...));
/**
 @brief Initialises a TransactionOutput object.
 @param self The TransactionOutput object to initialise.
 @returns true on success, false on failure.
 */
int initializeTransactionOutputFromData(TransactionOutput * self, ByteArray * data,void (*onErrorReceived)(Error error,char *,...));


/**
 @brief Gets a TransactionOutput from another object. Use this to avoid casts.
 @param self The object to obtain the TransactionOutput from.
 @returns The TransactionOutput object.
 */
TransactionOutput * getTransactionOutput(void * self);



/**
 @brief Frees a TransactionOutput object.
 @param self The TransactionOutput object to free.
 */
void destroyTransactionOutput(void * self);

/* BYTE SERIALIZATION AND DESERIALIZATION */
/**
 @brief Serialises a TransactionOutput to byte data.
 @param self The TransactionOutput object
 @returns The length written on success, 0 on failure.
 */
uint32_t serializeTransactionOutput(TransactionOutput * self);

/**
 @brief Deserialises a TransactionOutput so that it can be used as an object.
 @param self The TransactionOutput object
 @returns The length read on success, 0 on failure.
 */
uint32_t deserializeTransactionOutput(TransactionOutput * self);

#endif /* TRANSACTION_H_ */
