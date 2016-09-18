/*
 * Block_tests.c
 *
 *  Created on: 21/10/2012
 *  Created by: Wen Shen
 *  Copyright (c) 2012 Bitcoin Project Team
 */

/** @file
 @brief A test case for Block
 @details Test the Block class
 */

#include <stdio.h>
#include "../src/Object/Message/Block.h"
#include "../src/Object/ByteArray.h"
#include "../src/Constants.h"
#include <time.h>
#include "openssl/ssl.h"
#include "openssl/ripemd.h"
#include "openssl/rand.h"
#include "stdarg.h"
#include "assert.h"



void onErrorReceived(Error a,char * format,...);
/**
 @fn void onErrorReceived(Error a,char * format,...)
 @brief Error alert message printing
 @param a
 @param format
 @return void
 */
void onErrorReceived(Error a,char * format,...){
	va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
	printf("\n");
}
/**
 @fn int main()
 @ brief main function in testBlock
 @return int
 */
int main(int argc, char *argv[]){
	unsigned int s = (unsigned int)time(NULL);
	s = 1337544566;
	printf("Session = %ui\n",s);
	srand(s);
	/* Test genesis block */
	ByteArray * genesisMerkleRoot = createNewByteArrayUsingDataCopy((uint8_t []){0x3B,0xA3,0xED,0xFD,0x7A,0x7B,0x12,0xB2,0x7A,0xC7,0x2C,0x3E,0x67,0x76,0x8F,0x61,0x7F,0xC8,0x1B,0xC3,0x88,0x8A,0x51,0x32,0x3A,0x9F,0xB8,0xAA,0x4B,0x1E,0x5E,0x4A}, 32, onErrorReceived);

	ByteArray * genesisInScript = createNewByteArrayUsingDataCopy((uint8_t [77]){0x04,0xFF,0xFF,0x00,0x1D,0x01,0x04,0x45,0x54,0x68,0x65,0x20,0x54,0x69,0x6D,0x65,0x73,0x20,0x30,0x33,0x2F,0x4A,0x61,0x6E,0x2F,0x32,0x30,0x30,0x39,0x20,0x43,0x68,0x61,0x6E,0x63,0x65,0x6C,0x6C,0x6F,0x72,0x20,0x6F,0x6E,0x20,0x62,0x72,0x69,0x6E,0x6B,0x20,0x6F,0x66,0x20,0x73,0x65,0x63,0x6F,0x6E,0x64,0x20,0x62,0x61,0x69,0x6C,0x6F,0x75,0x74,0x20,0x66,0x6F,0x72,0x20,0x62,0x61,0x6E,0x6B,0x73}, 77, onErrorReceived);

	ByteArray * genesisOutScript = createNewByteArrayUsingDataCopy((uint8_t [67]){0x41,0x04,0x67,0x8A,0xFD,0xB0,0xFE,0x55,0x48,0x27,0x19,0x67,0xF1,0xA6,0x71,0x30,0xB7,0x10,0x5C,0xD6,0xA8,0x28,0xE0,0x39,0x09,0xA6,0x79,0x62,0xE0,0xEA,0x1F,0x61,0xDE,0xB6,0x49,0xF6,0xBC,0x3F,0x4C,0xEF,0x38,0xC4,0xF3,0x55,0x04,0xE5,0x1E,0xC1,0x12,0xDE,0x5C,0x38,0x4D,0xF7,0xBA,0x0B,0x8D,0x57,0x8A,0x4C,0x70,0x2B,0x6B,0xF1,0x1D,0x5F,0xAC}, 67, onErrorReceived);

	Block *genesisBlock = createNewBlockFromOriginalBlock(onErrorReceived);

	/* Test hash */
	uint8_t calcHash[32];
	printf("6--Hello world!\n");
	calculateBlockHash(genesisBlock,calcHash);
	printf("5 hello world!!\n");
	if(memcmp(genesisBlock->hash, calcHash,32)){
		printf("GENESIS BLOCK HASH FAIL\n0x");
		uint8_t * d = genesisBlock->hash;
		int x;
		for ( x = 0; x < 32; x++) {
			printf("%.2X",d[x]);
		}
		printf("\n!=\n0x");
		d = calcHash;
		for ( x = 0; x < 32; x++) {
			printf("%.2X",d[x]);
		}
		return 1;
	}

	/* Test deserialised data */
	if (genesisBlock->version != 1) {
		printf("GENESIS BLOCK VERSION FAIL\n");
		return 1;
	}
	int x;
	for ( x = 0; x < 32; x++) {
		if(getByteFromByteArray(genesisBlock->previousBlockHash, x) != 0){
			printf("GENESIS BLOCK PREV FAIL\n");
			return 1;
		}
	}
	if (compareByteArrays(genesisBlock->merkleTreeRootHash, genesisMerkleRoot)) {
		printf("GENESIS BLOCK MERKLE ROOT FAIL\n0x");
		uint8_t * d = getByteArrayData(genesisBlock->merkleTreeRootHash);
		int x;
		for (x = 0; x < 32; x++) {
			printf("%.2X",d[x]);
		}
		printf("\n!=\n0x");
		d = getByteArrayData(genesisMerkleRoot);
		for (x = 0; x < 32; x++) {
			printf("%.2X",d[x]);
		}
		return 1;
	}
	if (genesisBlock->time != 1231006505) {
		printf("GENESIS BLOCK TIME FAIL\n0x");
		return 1;
	}
	if (genesisBlock->target != 0x1D00FFFF) {
		printf("GENESIS BLOCK DIFFICULTY FAIL\n0x");
		return 1;
	}
	if (genesisBlock->nonce != 2083236893) {
		printf("GENESIS BLOCK DIFFICULTY FAIL\n0x");
		return 1;
	}
	if (genesisBlock->numOfTransactions != 1) {
		printf("GENESIS BLOCK TRANSACTION NUM FAIL\n0x");
		return 1;
	}
	Transaction * genesisCoinBase = genesisBlock->transactions[0];
	if (genesisCoinBase->numOfTransactionInput != 1) {
		printf("GENESIS BLOCK TRANSACTION INPUT NUM FAIL\n0x");
		return 1;
	}
	if (genesisCoinBase->numOfTransactionOutput != 1) {
		printf("GENESIS BLOCK TRANSACTION OUTPUT NUM FAIL\n0x");
		return 1;
	}
	if (genesisCoinBase->version != 1) {
		printf("GENESIS BLOCK TRANSACTION VERSION FAIL\n0x");
		return 1;
	}
	if (genesisCoinBase->lockTime != 0) {
		printf("GENESIS BLOCK TRANSACTION LOCK TIME FAIL\n0x");
		return 1;
	}
	if (genesisCoinBase->inputs[0]->scriptObject->length != 0x4D) {
		printf("GENESIS BLOCK TRANSACTION INPUT SCRIPT LENGTH FAIL\n0x");
		return 1;
	}
	if (genesisCoinBase->outputs[0]->scriptObject->length != 0x43) {
		printf("GENESIS BLOCK TRANSACTION OUTPUT SCRIPT LENGTH FAIL\n0x");
		return 1;
	}

	for (x = 0; x < 32; x++) {
		if(getByteFromByteArray(genesisCoinBase->inputs[0]->prevOutput.hash, x) != 0){
			printf("GENESIS BLOCK TRANSACTION INPUT OUT POINTER HASH FAIL\n");
			return 1;
		}
	}
	if (genesisCoinBase->inputs[0]->prevOutput.index != TRANSACTION_INPUT_FINAL) {
		printf("GENESIS BLOCK TRANSACTION INPUT OUT POINTER INDEX FAIL\n0x");
		return 1;
	}
	if (genesisCoinBase->inputs[0]->sequence != TRANSACTION_INPUT_FINAL) {
		printf("GENESIS BLOCK TRANSACTION INPUT SEQUENCE FAIL\n0x");
		return 1;
	}
	if (compareByteArrays(genesisCoinBase->inputs[0]->scriptObject, genesisInScript)) {
		printf("GENESIS BLOCK IN SCRIPT FAIL\n0x");
		uint8_t * d = getByteArrayData(genesisCoinBase->inputs[0]->scriptObject);
		int j;
		for (j = 0; j < genesisCoinBase->inputs[0]->scriptObject->length; j++) {
			printf("%.2X",d[j]);
		}
		printf("\n!=\n0x");
		d = getByteArrayData(genesisInScript);

		for (j = 0; j < genesisInScript->length; j++) {
			printf("%.2X",d[j]);
		}
		return 1;
	}
	if (genesisCoinBase->outputs[0]->value != 5000000000) {
		printf("GENESIS BLOCK TRANSACTION OUTPUT VALUE FAIL\n0x");
		return 1;
	}
	if (compareByteArrays(genesisCoinBase->outputs[0]->scriptObject, genesisOutScript)) {
		printf("GENESIS BLOCK OUT SCRIPT FAIL\n0x");
		uint8_t * d = getByteArrayData(genesisCoinBase->outputs[0]->scriptObject);
		int j;
		for ( j = 0; j < genesisCoinBase->outputs[0]->scriptObject->length; j++) {
			printf("%.2X",d[j]);
		}
		printf("\n!=\n0x");
		d = getByteArrayData(genesisOutScript);
		for (j = 0; x < genesisOutScript->length; j++) {
			printf("%.2X",d[j]);
		}
		return 1;
	}
	/* Test serialisation into genesis block */
	Block * block = createNewBlock(onErrorReceived);
	block->version = 1;
	uint8_t * zeroHash = malloc(32);
	memset(zeroHash, 0, 32);
	block->previousBlockHash = createNewByteArrayFromData(zeroHash, 32, onErrorReceived);
	block->merkleTreeRootHash = genesisMerkleRoot;
	block->target = 0x1D00FFFF;
	block->time = 1231006505;
	block->nonce = 2083236893;
	block->numOfTransactions = 1;
	block->transactions = malloc(sizeof(*block->transactions));
	block->transactions[0] = createNewTransaction(0, 1, onErrorReceived);
	incrementReferenceCount(block->previousBlockHash); /* Retain for the zero hash in the input*/

	takeTransactionInput(block->transactions[0], createNewTransactionInput(genesisInScript, 0xFFFFFFFF, block->previousBlockHash, 0xFFFFFFFF, onErrorReceived));
	takeTransactionOutput(block->transactions[0], createNewTransactionOutput(5000000000, genesisOutScript, onErrorReceived));
	getMessage(block)->bytes = createNewByteArrayOfSize(getMessage(genesisBlock)->bytes->length, onErrorReceived);
	printf("Before Serialize\n");

	serializeBlock(block, TRUE);
	printf("after Serialize\n");
	if (compareByteArrays(getMessage(block)->bytes, getMessage(genesisBlock)->bytes)) {
		printf("SERIALISATION TO GENESIS BLOCK FAIL\n0x");
		uint8_t * d = getByteArrayData(getMessage(block)->bytes);
		int k;
		for (k = 0; k < getMessage(block)->bytes->length; k++) {
			printf("%.2X",d[k]);
		}
		printf("\n!=\n0x");
		d = getByteArrayData(getMessage(genesisBlock)->bytes);
		for (k = 0; k < getMessage(genesisBlock)->bytes->length; k++) {
			printf("%.2X",d[k]);
		}
		return 1;
	}
	destroyObject(genesisBlock);


	return 0;
}

