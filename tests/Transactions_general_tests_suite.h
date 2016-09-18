/******----------------------- File Name: Transactions_general_tests_suite.h ---------------------------******

 Created by: apopoola, Fadwa & Senka on 01/12/2012.
 Tested by: Full Name on DD/MM/YYYY.
 Modified by: Fadwa Alseiari & Senka Kojic on 01/12/2012.
 Version: 1.0
 Copyright (c) 2012, MIBitcoinc


 Brief Description:
 ------------------
 This file includes all the test cases related to Transactions. It includes functions:
        test_case_createNewTransaction
		test_case_createNewTransactionFromByteArray
		test_case_setUpTransactionData
		test_case_setUpTransactionDataUsingByteArray
		test_case_destroyTransaction
		test_case_addTransactionInput
		test_case_addTransactionOutput
		test_case_serializeTransactionI
		test_case_deserializeTransactionI
		test_case_serializeTransactionIII
		test_case_serializeTransactionII
Basic functionalities were tested and functions fulfilled them. However, test_case_destroyTransaction, test_case_addTransactionInput, test_case_addTransactionOutput
showed certain inconsistencies and need more testing
 ******--------------------------------------------------------------------******/

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "stdarg.h"
#include <time.h>
#include "openssl/ssl.h"
#include "openssl/ripemd.h"
#include "openssl/rand.h"
#include "../src/Constants.h"
#include <string.h>
#include "CUnit/CUnit.h"
#include "CUnit/Basic.h"
/*#include "CUnit/TBTest.h"*/
/*#include "CUnit/Automated.h"
 #include "CUnit/Console.h"*/
#include "../src/Object/Message/Transaction.h"
#include "../src/Object/Message/TransactionInput.h"
#include "../src/Object/Message/TransactionInput.c"
#include "../src/Object/Script.h"
#include "../src/Object/ByteArray.h"
#include "../src/Utils/VariableLengthInteger/VarLenInt.h"

/* Test Suite setup and cleanup functions: */

int init_Transactions_general_suite(void) {
	return 0;
}

int clean_Transactions_general_suite(void) {
	return 0;
}

void err4(Error a, char * format, ...);
void err4(Error a, char * format, ...) {
	va_list argptr;
	va_start(argptr, format);
	vfprintf(stderr, format, argptr);
	va_end(argptr);
	printf("\n");
}

/************* Test case functions ****************/

/* Test TransactionInput deserialisation */

void test_case_createNewTransaction(void) {
	unsigned int s = (unsigned int) time(NULL);
	s = 1337544566;
	uint8_t hash[32];
	int value = 234;
	ByteArray *byte = createNewByteArrayUsingDataCopy(hash, 32, err4);
	ByteArray *transByte = createNewByteArrayOfSize(47, err4);
	Transaction *newTransaction = createNewTransaction(s, 1, err4);
	CU_ASSERT_PTR_NOT_NULL(newTransaction);
	CU_ASSERT_EQUAL(newTransaction->lockTime, s);
	CU_ASSERT_EQUAL(newTransaction->numOfTransactionInput,0);
	CU_ASSERT_EQUAL(newTransaction->numOfTransactionOutput ,0);
	CU_ASSERT_EQUAL(newTransaction->inputs , NULL);
	CU_ASSERT_EQUAL(newTransaction->outputs , NULL);
	CU_ASSERT_EQUAL(newTransaction->version , 1);
	getMessage(newTransaction)->bytes = transByte;

	/* serializing the transaction */ /*SEGMENTATION ERROR, COME BACK ON THIS*/

}

void test_case_createNewTransactionFromByteArray(void) {
	ByteArray * bytes = createNewByteArrayOfSize(42, err4);
	Transaction *self = createNewTransactionFromByteArray(bytes, err4);
	CU_ASSERT_PTR_NOT_NULL(self);
	CU_ASSERT_EQUAL(sizeof(bytes), 8);
	CU_ASSERT_EQUAL(sizeof(bytes), sizeof(self));
	CU_ASSERT_EQUAL(self->version, 0);
	CU_ASSERT_EQUAL(self->numOfTransactionInput, 0);
	CU_ASSERT_EQUAL(self->numOfTransactionOutput, 0);
	CU_ASSERT_EQUAL(self->inputs, NULL);
	CU_ASSERT_EQUAL(self->outputs, NULL);
}

void test_case_setUpTransactionData(void) {

	ByteArray * bytes = createNewByteArrayOfSize(42, err4);
	Transaction *self = createNewTransactionFromByteArray(bytes, err4);
	CU_ASSERT_EQUAL(setUpTransactionData(self, 122, 2, err4), 1);
	/*printf("reference count: %d\n", getObject(self)->referenceCount);*/
	CU_ASSERT_EQUAL(self->lockTime, 122);
	CU_ASSERT_EQUAL(self->version, 2);
	CU_ASSERT_EQUAL(self->numOfTransactionInput, 0);
	CU_ASSERT_EQUAL(self->numOfTransactionOutput, 0);
	CU_ASSERT_EQUAL(self->inputs, NULL);
	CU_ASSERT_EQUAL(self->outputs, NULL);
	CU_ASSERT_EQUAL(getObject(self)->referenceCount, 1);
	CU_ASSERT_NOT_EQUAL(getObject(self), NULL);
	CU_ASSERT_EQUAL(getMessage(self)->bytes, NULL);
	CU_ASSERT_EQUAL(getMessage(self)->expectResponse, FALSE);
}

void test_case_setUpTransactionDataUsingByteArray(void) {
	ByteArray * bytes = createNewByteArrayOfSize(42, err4);
	Transaction *self = createNewTransactionFromByteArray(bytes, err4);
	CU_ASSERT_EQUAL(setUpTransactionDataUsingByteArray(self, bytes, err4), 1);
	CU_ASSERT_EQUAL(getMessage(self)->bytes, bytes);

}

/*Return to this after finishing with TransactionInput and Output*/

void test_case_destroyTransaction(void) {

}

/*Return to this after finishing with TransactionInput and Output*/
void test_case_addTransactionInput(void) {
	ByteArray *bytes = createNewByteArrayOfSize(42, err4);
	Transaction *newTransaction = createNewTransactionFromByteArray(bytes,
			err4);

	int i, j;
	i = newTransaction->numOfTransactionInput;

	ByteArray * newbytes = createNewByteArrayOfSize(42, err4);
	uint8_t hash[32];
	Sha160((uint8_t *) "hello", 5, hash);
	/* Move bytes into ByteArray */
	setBytesInByteArray(newbytes, 0, hash, 32);
	setBytesInByteArray(newbytes, 32, (uint8_t[]) {0x05,0xFA,0x4B,0x51}, 4);
	encodeVarLenInt(newbytes, 36, createVarLenIntFromUInt64(1));
	setByteInByteArray(newbytes, 37, SCRIPT_OPCODE_TRUE);

	uint32_t sequence = rand();
	writeInt32AsLittleEndianIntoByteArray(bytes, 38, sequence);

	TransactionInput *input = createNewTransactionInputFromData(newbytes, err4);
	addTransactionInput(newTransaction, input);
	j = newTransaction->numOfTransactionInput;
	CU_ASSERT_EQUAL(newTransaction->numOfTransactionInput, j);
	CU_ASSERT_EQUAL(getObject(newTransaction)->referenceCount, 1);
	CU_ASSERT_EQUAL(getObject(newTransaction)->referenceCount, 1);
	CU_ASSERT_EQUAL(getObject(input)->referenceCount, 2);
	CU_ASSERT_EQUAL(newTransaction->inputs[0], getObject(input));
	/*CU_ASSERT_EQUAL(compareByteArrays(getMessage(newTransaction)->bytes, getObject(input)), 0);*//*We need to compare between them*/
}
/*Return to this after finishing with TransactionInput and Output*/
void test_case_addTransactionOutput(void) {
	ByteArray *bytes = createNewByteArrayOfSize(42, err4);
	Transaction *newTransaction = createNewTransactionFromByteArray(bytes,
			err4);
	int i, j;
	i = newTransaction->numOfTransactionOutput;
	ByteArray * newbytes = createNewByteArrayOfSize(42, err4);
	uint8_t hash[32];
	Sha160((uint8_t *) "hello", 5, hash);
	/* Move bytes into ByteArray */
	setBytesInByteArray(newbytes, 0, hash, 32);
	setBytesInByteArray(newbytes, 32, (uint8_t[]) {0x05,0xFA,0x4B,0x51}, 4);
	encodeVarLenInt(newbytes, 36, createVarLenIntFromUInt64(1));
	setByteInByteArray(newbytes, 37, SCRIPT_OPCODE_TRUE);

	uint32_t sequence = rand();
	writeInt32AsLittleEndianIntoByteArray(bytes, 38, sequence);

	TransactionOutput *Output = createNewTransactionOutputFromData(newbytes,
			err4);

	addTransactionOutput(newTransaction, Output);
	j = newTransaction->numOfTransactionOutput;
	CU_ASSERT_EQUAL(newTransaction->numOfTransactionOutput, j);
	CU_ASSERT_EQUAL(getObject(newTransaction)->referenceCount, 1);
	CU_ASSERT_EQUAL(getObject(newTransaction)->referenceCount, 1);
	CU_ASSERT_EQUAL(getObject(Output)->referenceCount, 2);
	CU_ASSERT_EQUAL(newTransaction->outputs[0], getObject(Output));
	/*CU_ASSERT_EQUAL(compareByteArrays(getMessage(newTransaction)->bytes, getObject(output)), 0);*//*We need to compare between them*/
}

/*Return to this after finishing with TransactionInput and Output*/

void test_case_serializeTransactionI(void) {

	Script *newScript = createNewScriptStackFromData((uint8_t[]){SCRIPT_OPCODE_TRUE},1,err4);



	ByteArray * data = createNewByteArrayOfSize(42, err4);
	Transaction *self = createNewTransactionFromByteArray(data, err4);
	ByteArray * bytes = getMessage(self)->bytes;

	CU_ASSERT_NOT_EQUAL(bytes, NULL);

	/*Creating first set of input/output*/
	ByteArray * newbytes = createNewByteArrayOfSize(42, err4);
	uint8_t hash[32];
	Sha160((uint8_t *) "hello", 5, hash);
	/* Move bytes into ByteArray */
	setBytesInByteArray(newbytes, 0, hash, 32);
	setBytesInByteArray(newbytes, 32, (uint8_t[]) {0x05,0xFA,0x4B,0x51}, 4);
	encodeVarLenInt(newbytes, 36, createVarLenIntFromUInt64(1));
	setByteInByteArray(newbytes, 37, SCRIPT_OPCODE_TRUE);
	uint32_t sequence = rand();
	writeInt32AsLittleEndianIntoByteArray(bytes, 38, sequence);

	TransactionInput *input = createNewTransactionInputFromData(newbytes, err4);
	TransactionOutput *Output = createNewTransactionOutputFromData(newbytes,
			err4);

	addTransactionInput(self, input);
	addTransactionOutput(self, Output);

	/*Creating second set of input/output*/
	ByteArray * newbytesI = createNewByteArrayOfSize(42, err4);

	Sha160((uint8_t *) "hello", 5, hash);
	/* Move bytes into ByteArray */
	setBytesInByteArray(newbytesI, 0, hash, 32);
	setBytesInByteArray(newbytesI, 32, (uint8_t[]) {0x05,0xFA,0x4B,0x51}, 4);
	encodeVarLenInt(newbytesI, 36, createVarLenIntFromUInt64(1));
	setByteInByteArray(newbytesI, 37, SCRIPT_OPCODE_TRUE);

	writeInt32AsLittleEndianIntoByteArray(bytes, 38, sequence);

	TransactionInput *inputI = createNewTransactionInputFromData(newbytesI,
			err4);
	TransactionOutput *OutputI = createNewTransactionOutputFromData(newbytesI,
			err4);

	addTransactionInput(self, inputI);
	addTransactionOutput(self, OutputI);

	/*Creating third set of input/output*/

	ByteArray * newbytesII = createNewByteArrayOfSize(42, err4);

	Sha160((uint8_t *) "hello", 5, hash);
	/* Move bytes into ByteArray */
	setBytesInByteArray(newbytesII, 0, hash, 32);
	setBytesInByteArray(newbytesII, 32, (uint8_t[]) {0x05,0xFA,0x4B,0x51}, 4);
	encodeVarLenInt(newbytesII, 36, createVarLenIntFromUInt64(1));
	setByteInByteArray(newbytesII, 37, SCRIPT_OPCODE_TRUE);

	writeInt32AsLittleEndianIntoByteArray(bytes, 38, sequence);

	TransactionInput *inputII = createNewTransactionInputFromData(newbytesII,
			err4);
	TransactionOutput *OutputII = createNewTransactionOutputFromData(newbytesII,
			err4);

	addTransactionInput(self, inputII);
	addTransactionOutput(self, OutputII);
	printf("self->numOfTransactionOutput: %d\n", self->numOfTransactionOutput);

	VarLenInt numTransactionInputs = createVarLenIntFromUInt64(
			self->numOfTransactionInput);
	VarLenInt numTransactionOutputs = createVarLenIntFromUInt64(
			self->numOfTransactionOutput);
	uint32_t cursor = 4 + numTransactionInputs.storageSize;
	CU_ASSERT_NOT_EQUAL(bytes->length, cursor + 5);/*come back if have time*/

	writeInt32AsLittleEndianIntoByteArray(bytes, 0, self->version);/*version is 0, try setting it differently*/
	printf("self->version: %d\n", self->version);
	encodeVarLenInt(bytes, 4, numTransactionInputs);

	getMessage(self->inputs[0])->bytes = getByteArraySubsectionReference(bytes,
			cursor, bytes->length - cursor);
	CU_ASSERT_NOT_EQUAL(getMessage(self->inputs[0])->bytes, NULL);

	getMessage(self->inputs[1])->bytes = getByteArraySubsectionReference(bytes,
			cursor, bytes->length - cursor);
	CU_ASSERT_NOT_EQUAL(getMessage(self->inputs[1])->bytes, NULL);
	getMessage(self->inputs[2])->bytes = getByteArraySubsectionReference(bytes,
			cursor, bytes->length - cursor);
	CU_ASSERT_NOT_EQUAL(getMessage(self->inputs[2])->bytes, NULL);
}

void test_case_serializeTransactionII(void) {
	ByteArray * forTransactionCreation = createNewByteArrayOfSize(42, err4); /*42*/
	Transaction *transaction = createNewTransactionFromByteArray(
			forTransactionCreation, err4);

	ByteArray * newbytes = createNewByteArrayOfSize(42, err4); /*42*/
	uint8_t hash[32];/*32*/
	Sha160((uint8_t *) "hello", 5, hash);
	/* Move bytes into ByteArray */
	setBytesInByteArray(newbytes, 0, hash, 32);/*32*/
	setBytesInByteArray(newbytes, 32, (uint8_t[]) {0x05,0xFA,0x4B,0x51}, 4);/*32*/
	encodeVarLenInt(newbytes, 36, createVarLenIntFromUInt64(1));
	setByteInByteArray(newbytes, 37, SCRIPT_OPCODE_TRUE);
	uint32_t sequence = rand();
	writeInt32AsLittleEndianIntoByteArray(newbytes, 38, sequence);

	TransactionInput *input = createNewTransactionInputFromData(newbytes, err4);
	Script * scriptObj = createNewScriptStackUsingCopyOfData(
			(uint8_t[]) {SCRIPT_OPCODE_TRUE}, 1, err4);
	ByteArray * outPointerHash = createNewByteArrayUsingDataCopy(hash, 32,
			err4); /*32*/
	input = createNewTransactionInput(scriptObj, sequence, outPointerHash,
			0x514BFA05, err4);
	addTransactionInput(transaction, input);

	getMessage(input)->bytes = createNewByteArrayOfSize(42, err4); /*42*/
	ByteArray * oldbytes = getMessage(input)->bytes;
	serializeTransactionInput(input);
	CU_ASSERT_EQUAL(compareByteArrays(oldbytes, getMessage(input)->bytes), 0);
	uint32_t len = (transaction->inputs[0]);
	CU_ASSERT_NOT_EQUAL(len, NULL);
}

void test_case_serializeTransactionIII(void) {
	ByteArray *bytes = createNewByteArrayOfSize(42, err4);
	Transaction *newTransaction = createNewTransactionFromByteArray(bytes,
			err4);
	ByteArray * newbytes = createNewByteArrayOfSize(42, err4);
	uint8_t hash[32];
	Sha160((uint8_t *) "hello", 5, hash);
	/* Move bytes into ByteArray */
	setBytesInByteArray(newbytes, 0, hash, 32);
	setBytesInByteArray(newbytes, 32, (uint8_t[]) {0x05,0xFA,0x4B,0x51}, 4);
	encodeVarLenInt(newbytes, 36, createVarLenIntFromUInt64(1));
	setByteInByteArray(newbytes, 37, SCRIPT_OPCODE_TRUE);

	uint32_t sequence = rand();
	writeInt32AsLittleEndianIntoByteArray(bytes, 38, sequence);
	TransactionInput *input = createNewTransactionInputFromData(newbytes, err4);
	addTransactionInput(newTransaction, input);
	ByteArray * boring = getMessage(newTransaction)->bytes;
	CU_ASSERT_NOT_EQUAL(getMessage(newTransaction)->bytes, NULL);
	VarLenInt numTransactionInputs = createVarLenIntFromUInt64(
			newTransaction->numOfTransactionInput);
	uint32_t cursor = 4 + numTransactionInputs.storageSize;
}

void test_case_deserializeTransactionI(void) {
	ByteArray * data = createNewByteArrayOfSize(42, err4);
	Transaction *self = createNewTransactionFromByteArray(data, err4);
	ByteArray * bytes = getMessage(self)->bytes;
	CU_ASSERT_NOT_EQUAL(bytes, NULL);
	CU_ASSERT_NOT_EQUAL(bytes->length, 9);
}

/* Grouping all test cases into an array that will be registered later with a single test suite. */
CU_TestInfo transactions_general_test_array[] ={
		{ "test_case_createNewTransaction", test_case_createNewTransaction },
		{ "test_case_createNewTransactionFromByteArray",test_case_createNewTransactionFromByteArray },
		{ "test_case_setUpTransactionData",test_case_setUpTransactionData },
		{ "test_case_setUpTransactionDataUsingByteArray",test_case_setUpTransactionDataUsingByteArray },
		{ "test_case_destroyTransaction",test_case_destroyTransaction },
		{ "test_case_addTransactionInput",test_case_addTransactionInput },
		{ "test_case_addTransactionOutput",test_case_addTransactionOutput },
		{ "test_case_serializeTransactionI",test_case_serializeTransactionI },
		{ "test_case_deserializeTransactionI",test_case_deserializeTransactionI },
		{ "test_case_serializeTransactionIII",test_case_serializeTransactionIII },
		{ "test_case_serializeTransactionII",test_case_serializeTransactionII },
		CU_TEST_INFO_NULL, };
