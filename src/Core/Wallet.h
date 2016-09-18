/*
 * Wallet.h
 *
 * Created on: Nov 27, 2012
 * Created by: Wen Shen
 * Copyright (c) 2012 MIBitcoin Project Team
 */

#ifndef WALLET_H_
#define WALLET_H_
#include "../Object/Object.h"
#include "../Object/Message/Transaction.h"
#include "../Object/Network/Address.h"
#include <stdio.h>
/*A record that stores the transaction detalis*/
typedef struct {
  Object base;
  char * date;
  RecordType type;
  Address * to;
  Address * from;
  float amount;
} Record;
/*A keypair that stores the privateKey and publicKey of the address */
typedef struct {
	Object base;
	uint8_t privateKey;
	uint8_t publicKey;
}Keypair;

/*A wallet that stores the transactions details and keypairs*/
typedef struct {
	Object base;
	uint32_t numOfRecords;
	Record ** records;
	uint32_t numOfKeypairs;
	Keypair ** keypairs;
    void (*onErrorReceived)(Error error,char *,...);
} Wallet;

/**
 @brief create a new wallet
 @param onErrorReceived Events for errors.
 @returns Wallet
 */
Wallet * createWallet(void(*onErrorReceived)(Error error, char *, ...));

/**
 @brief create a new Record
 @param onErrorReceived Events for errors.
 @returns Record
 */
Record * createRecord();


/**
 @brief Initializes a Wallet object.
 @param self A Wallet object
 @param onErrorReceived Events for errors.
 @returns true on success, false on failure.
 */
boolean initWallet(Wallet *self, void(*onErrorReceived)(Error error, char *, ...));
/**
 @brief Store the Wallet into the storage file.
 @param self A Wallet object.
 @returns true on success, false on failure.
 */
boolean storeWallet( Wallet * self );
/**
 @brief Read Wallet from the storage file.
 @returns Wallet object
 */
Wallet * readWallet();
/**
 @brief Add a keypair the the wallet.
 @param self Wallet
 @param keypair  Keypair
 @returns true on success, false on failure.
 */
boolean addKeypairToWallet(Wallet * self, Keypair *keypair );
/**
 @brief Add a transaction record to the wallet.
 @param self Wallet.
 @param record Record
 @returns true on success, false on failure.
 */
boolean  addRecordToWallet(Wallet * self, Record * record);
/**,
 @brief Get the balance of the wallet.
 @param self The Wallet object.
 @returns double The balance.
 */
float getBalance(Wallet * self);
/**
 @brief Update Wallet self with another Wallet wallet.
 @param self The Wallet object to be updated.
 @param wallet The Wallet object will be used to update.
 @returns void.
 */
void updateWallet(Wallet * self, Wallet *wallet);

/**
 @brief Receive certain amount bitcoin from Address from to Address to.
 @param self The Wallet object.
 @param jsonRecord the sent record in JSON format
 @returns true on success, false on failure.
 */
boolean receiveCoins(Wallet * self, Record *jsonRecord);
/**
 @brief Find the address by Keypair in the wallet.
 @param self The Wallet object.
 @param keypair The Keypair need to be used in the search.
 @param addressType Type fo address: Address_FROM or Address_TO.
 @returns Address The address founded in the wallet; if not found, NULL.
 */
Address * getAddressByKeyPair(Wallet * self, Keypair * keypair, int addressType);
/**
 @brief Verify whether the address is in the wallet.
 @param self The Wallet object.
 @param address The Address object to be verified.
 @param addressType Type fo address: Address_FROM or Address_TO.
 @returns true on success, false on failure.
 */
boolean isMyPublicKey(Wallet * self, Address * address, int addressType);
/**
 @brief Destroy the wallet object.
 @param self The Wallet object to be destroyed.
 @returns void.
 */
void destroyWallet(Wallet * self);

/**
 @brief Destroy the Record object.
 @param self The Record object to be destroyed.
 @returns void.
 */

void destroyRecord(Record *self);


/**
 @fn boolean readRecordsFromWalletFile(Wallet *self, FILE *file);
 @brief Reads transaction records from a wallet file and stores them into the wallet file in memory
 @param Wallet the wallet to store the records in
 @param file The file name to read from
 @returns true on success; false on errors
 */
boolean * readRecordsFromWalletFile(Wallet *self, char *file);

boolean *addTransactionToWallet(Wallet *self, Transaction *transaction);

#endif /* WALLET_H_ */
