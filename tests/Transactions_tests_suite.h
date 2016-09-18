/******----------------------- File Name: Transactions_tests_suite.h ---------------------------******

Created by: apopoola, Fadwa & Senka on 01/12/2012.
Tested by: Full Name on DD/MM/YYYY.
Modified by: Fadwa Alseiari & Senka Kojic on 01/12/2012.
Version: 1.0
Copyright (c) 2012, MIBitcoinc


Brief Description:
------------------
This file includes all the test cases related to Transactions. It includes functions: test_case_TransactionInput_deserialization,
test_case_TransactionInput_serialization, test_case_TransactionOutput_deserialization,
test_case_TransactionOutput_serialization, test_case_Transaction_deserialization, test_case_Signing_operations.

Signing operations were not thoroughly tested.

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
#include "../src/Object/Script.h"
#include "../src/Object/ByteArray.h"
#include "../src/Utils/VariableLengthInteger/VarLenInt.h"


/* Test Suite setup and cleanup functions: */

int init_Transactions_suite(void) {
        return 0;
}

int clean_Transactions_suite(void) {
        return 0;
}




void err2(Error a,char * format,...);
void err2(Error a,char * format,...){
	va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
	printf("\n");
}

/************* Test case functions ****************/

/* Test TransactionInput deserialisation */

void test_case_TransactionInput_deserialization(void){
	uint8_t hash[32], hash1[32];
	Sha160((uint8_t *)"Hello", 5, hash);
	Sha160((uint8_t *)"Make my day", 11, hash1);
	/*Hash test*/
	CU_ASSERT_NOT_EQUAL (hash, NULL);
	CU_ASSERT_NOT_EQUAL (hash1, NULL);
	CU_ASSERT_NOT_EQUAL (hash, hash1);

	/* Move bytes into ByteArray */
	ByteArray * bytes = createNewByteArrayOfSize(42, err2);
	setBytesInByteArray(bytes, 0, hash, 32);
	setBytesInByteArray(bytes, 32, (uint8_t[]) {0x05,0xFA,0x4B,0x51}, 4);
	encodeVarLenInt(bytes, 36, createVarLenIntFromUInt64(1));
	setByteInByteArray(bytes, 37, SCRIPT_OPCODE_TRUE);


	uint32_t sequence = rand();
	writeInt32AsLittleEndianIntoByteArray(bytes, 38, sequence);
	TransactionInput *input = createNewTransactionInputFromData(bytes, err2);
	uint32_t deserialized = deserializeTransactionInput(input);


	CU_ASSERT_EQUAL(memcmp(hash, getByteArrayData(input->prevOutput.hash), 32), 0);
	CU_ASSERT_EQUAL(input->prevOutput.index,0x514BFA05);

	ScriptStack scriptStack = createEmptyScriptStack();
	CU_ASSERT_EQUAL((executeScript(input->scriptObject, &scriptStack, NULL, NULL, 0, FALSE)), GOOD_SCRIPT);

	CU_ASSERT_EQUAL(input->sequence, sequence);

}


/* Test TransactionInput serialisation */

void test_case_TransactionInput_serialization(void){
	uint8_t hash[32], hash1[32];
	Sha160((uint8_t *)"Hello", 5, hash);
	ByteArray * bytes = createNewByteArrayOfSize(42, err2);
	setBytesInByteArray(bytes, 0, hash, 32);
	setBytesInByteArray(bytes, 32, (uint8_t[]) {0x05,0xFA,0x4B,0x51}, 4);
	encodeVarLenInt(bytes, 36, createVarLenIntFromUInt64(1));
	setByteInByteArray(bytes, 37, SCRIPT_OPCODE_TRUE);
	uint32_t sequence = rand();
	writeInt32AsLittleEndianIntoByteArray(bytes, 38, sequence);
	TransactionInput *input = createNewTransactionInputFromData(bytes, err2);
	uint32_t deserialized = deserializeTransactionInput(input);

	Script * scriptObj = createNewScriptStackUsingCopyOfData((uint8_t []) {SCRIPT_OPCODE_TRUE}, 1, err2) ;
	ByteArray * outPointerHash = createNewByteArrayUsingDataCopy(hash, 32, err2);
	input = createNewTransactionInput(scriptObj, sequence, outPointerHash, 0x514BFA05, err2);

	getMessage(input)->bytes = createNewByteArrayOfSize(42, err2);

	serializeTransactionInput(input);
	CU_ASSERT_EQUAL(compareByteArrays(bytes, getMessage(input)->bytes),0);

}

/* Test TransactionOutput deserialisation */
void test_case_TransactionOutput_deserialization(void){

	uint8_t hash[32];
	Sha160((uint8_t *)"hello", 5, hash);
	ByteArray * bytes = createNewByteArrayOfSize(42, err2);
	bytes = createNewByteArrayOfSize(10, err2);

	/* Move bytes into ByteArray */
	setBytesInByteArray(bytes, 0, hash, 32);
	setBytesInByteArray(bytes, 32, (uint8_t[]) {0x05,0xFA,0x4B,0x51}, 4);
	encodeVarLenInt(bytes, 36, createVarLenIntFromUInt64(1));
	setByteInByteArray(bytes, 37, SCRIPT_OPCODE_TRUE);


	uint64_t value = rand(); /* Use a random value */
	writeInt64AsLittleEndianIntoByteArray(bytes, 0, value);
	encodeVarLenInt(bytes, 8, createVarLenIntFromUInt64(1));

	setByteInByteArray(bytes, 9, SCRIPT_OPCODE_TRUE);
	TransactionOutput * output = createNewTransactionOutputFromData(bytes, err2);
	deserializeTransactionOutput(output);

	CU_ASSERT_EQUAL(output->value,value);

	ScriptStack scriptStack = createEmptyScriptStack();
	CU_ASSERT_EQUAL((executeScript(output->scriptObject, &scriptStack, NULL, NULL, 0, FALSE)), GOOD_SCRIPT);

}

/* Test TransactionOutput serialization */
void test_case_TransactionOutput_serialization(void){
	uint8_t hash[32];
	Sha160((uint8_t *)"hello", 5, hash);
	ByteArray * bytes = createNewByteArrayOfSize(42, err2);
	bytes = createNewByteArrayOfSize(10, err2);

	/* Move bytes into ByteArray */
	setBytesInByteArray(bytes, 0, hash, 32);
	setBytesInByteArray(bytes, 32, (uint8_t[]) {0x05,0xFA,0x4B,0x51}, 4);
	encodeVarLenInt(bytes, 36, createVarLenIntFromUInt64(1));
	setByteInByteArray(bytes, 37, SCRIPT_OPCODE_TRUE);


	uint64_t value = rand(); /* Use a random value */
	writeInt64AsLittleEndianIntoByteArray(bytes, 0, value);
	encodeVarLenInt(bytes, 8, createVarLenIntFromUInt64(1));

	setByteInByteArray(bytes, 9, SCRIPT_OPCODE_TRUE);
	TransactionOutput * output = createNewTransactionOutputFromData(bytes, err2);
	Script * scriptObj = createNewScriptStackUsingCopyOfData((uint8_t []) {SCRIPT_OPCODE_TRUE}, 1, err2) ;

	output = createNewTransactionOutput(value, scriptObj, onErrorReceived);
	getMessage(output)->bytes = createNewByteArrayOfSize(10, onErrorReceived);

	serializeTransactionOutput(output);
	ScriptStack scriptStack = createEmptyScriptStack();
	CU_ASSERT_EQUAL(compareByteArrays(bytes, getMessage(output)->bytes),0);


}

/*Deserialization of transaction version, numOfTransactionInput,numOfTransactionOutput, lockTime and prevOutput.hash */

void test_case_Transaction_deserialization(void){
	uint8_t * scripts[6];
		scripts[0] =
				(uint8_t[]) {SCRIPT_OPCODE_4,SCRIPT_OPCODE_2,SCRIPT_OPCODE_SUB,SCRIPT_OPCODE_3,SCRIPT_OPCODE_MAX,SCRIPT_OPCODE_3,SCRIPT_OPCODE_EQUAL};
		scripts[1] =
				(uint8_t[]) {SCRIPT_OPCODE_1,SCRIPT_OPCODE_3,SCRIPT_OPCODE_7,SCRIPT_OPCODE_ROT,SCRIPT_OPCODE_SUB,SCRIPT_OPCODE_ADD,SCRIPT_OPCODE_9,SCRIPT_OPCODE_EQUAL};
		scripts[2] =
				(uint8_t[]) {SCRIPT_OPCODE_TRUE,SCRIPT_OPCODE_IF,SCRIPT_OPCODE_3,SCRIPT_OPCODE_ELSE,SCRIPT_OPCODE_2,SCRIPT_OPCODE_ENDIF,SCRIPT_OPCODE_3,SCRIPT_OPCODE_EQUAL};
		scripts[3] =
				(uint8_t[]) {SCRIPT_OPCODE_3,SCRIPT_OPCODE_DUP,SCRIPT_OPCODE_DUP,SCRIPT_OPCODE_ADD,SCRIPT_OPCODE_ADD,SCRIPT_OPCODE_9,SCRIPT_OPCODE_EQUAL};
		scripts[4] =
				(uint8_t[]) {SCRIPT_OPCODE_3,SCRIPT_OPCODE_3,SCRIPT_OPCODE_SUB,SCRIPT_OPCODE_NOT,SCRIPT_OPCODE_TRUE,SCRIPT_OPCODE_BOOLAND};
		uint8_t hash[32];
		uint8_t i = 0;
		for (; i < 32; i++) {
			hash[i] = rand();
		}
		uint32_t randInt = rand();
		uint64_t randInt64 = rand();
		ByteArray *bytes = createNewByteArrayOfSize(187, err2);

		/* set Protocol version */
		writeInt32AsLittleEndianIntoByteArray(bytes, 0, 2);
		/* Input number */
		encodeVarLenInt(bytes, 4, createVarLenIntFromUInt64(3));

		/* First input test */
		setBytesInByteArray(bytes, 5, hash, 32);
		writeInt32AsLittleEndianIntoByteArray(bytes, 37, randInt);
		encodeVarLenInt(bytes, 41, createVarLenIntFromUInt64(7));
		setBytesInByteArray(bytes, 42, scripts[0], 7);
		writeInt32AsLittleEndianIntoByteArray(bytes, 49, randInt);

		/* Second input test */
		setBytesInByteArray(bytes, 53, hash, 32);
		writeInt32AsLittleEndianIntoByteArray(bytes, 85, randInt);
		encodeVarLenInt(bytes, 89, createVarLenIntFromUInt64(8));
		setBytesInByteArray(bytes, 90, scripts[1], 8);
		writeInt32AsLittleEndianIntoByteArray(bytes, 98, randInt);

		/* Third input test */
		setBytesInByteArray(bytes, 102, hash, 32);
		writeInt32AsLittleEndianIntoByteArray(bytes, 134, randInt);
		encodeVarLenInt(bytes, 138, createVarLenIntFromUInt64(8));
		setBytesInByteArray(bytes, 139, scripts[2], 8);
		writeInt32AsLittleEndianIntoByteArray(bytes, 147, randInt);

		/* Output number */
		encodeVarLenInt(bytes, 151, createVarLenIntFromUInt64(2));

		/* First output test */
		writeInt64AsLittleEndianIntoByteArray(bytes, 152, randInt64);
		encodeVarLenInt(bytes, 160, createVarLenIntFromUInt64(7));
		setBytesInByteArray(bytes, 161, scripts[3], 7);

		/* Second output test */
		writeInt64AsLittleEndianIntoByteArray(bytes, 168, randInt64);
		encodeVarLenInt(bytes, 176, createVarLenIntFromUInt64(6));
		setBytesInByteArray(bytes, 177, scripts[4], 7);

		/* Lock time */
	writeInt32AsLittleEndianIntoByteArray(bytes, 183, randInt);
	Transaction * transaction = createNewTransactionFromByteArray(bytes, err2);

	deserializeTransaction(transaction);
	CU_ASSERT_EQUAL(transaction->version, 2);
	CU_ASSERT_EQUAL(transaction->numOfTransactionInput, 3);
	CU_ASSERT_EQUAL(transaction->numOfTransactionOutput, 2);
	CU_ASSERT_EQUAL(transaction->lockTime, randInt);

	CU_ASSERT_EQUAL(memcmp(hash, getByteArrayData(transaction->inputs[0]->prevOutput.hash), 32),0);
	CU_ASSERT_EQUAL(memcmp(hash, getByteArrayData(transaction->inputs[1]->prevOutput.hash), 32),0);
	CU_ASSERT_EQUAL(memcmp(hash, getByteArrayData(transaction->inputs[2]->prevOutput.hash), 32),0);
    CU_ASSERT_EQUAL (transaction->inputs[0]->prevOutput.index, randInt);
    CU_ASSERT_EQUAL (transaction->inputs[1]->prevOutput.index, randInt);
    CU_ASSERT_EQUAL (transaction->inputs[2]->prevOutput.index, randInt);


/*here input  stack test */

  CU_ASSERT_EQUAL(transaction->inputs[0]->sequence, randInt);
  CU_ASSERT_EQUAL(transaction->inputs[1]->sequence, randInt);
  CU_ASSERT_EQUAL(transaction->inputs[2]->sequence, randInt);



  CU_ASSERT_EQUAL(transaction->outputs[0]->value,randInt64);
  CU_ASSERT_EQUAL(transaction->outputs[1]->value,randInt64);



  Script * scriptObj = createNewScriptStackUsingCopyOfData((uint8_t []) {SCRIPT_OPCODE_TRUE}, 1, err2) ;
  transaction = createNewTransaction(randInt, 2, err2);
  TransactionInput *input = createNewTransactionInputFromData(bytes, err2);
  TransactionOutput * output = createNewTransactionOutputFromData(bytes, err2);
  ByteArray * outPointerHash = createNewByteArrayUsingDataCopy(hash, 32, err2);
  outPointerHash = createNewByteArrayUsingDataCopy(hash, 32, err2);
  i = 0; /*re-init counter */
  	for (; i < 3; i++) {
  		scriptObj = createNewScriptStackUsingCopyOfData(scripts[i], (!i) ? 7 : 8, err2);
  		input = createNewTransactionInput(scriptObj, randInt, outPointerHash, randInt, err2);
  		addTransactionInput(transaction, input);
  		decrementReferenceCount(scriptObj);
  	}

  	decrementReferenceCount(outPointerHash);
  	i = 3; /*re-init counter */
  	for (; i < 5; i++) {
  		scriptObj = createNewScriptStackUsingCopyOfData(scripts[i], (i == 3) ? 7 : 6, err2);
  		output = createNewTransactionOutput(randInt64, scriptObj, err2);
  		addTransactionOutput(transaction, output);
  		decrementReferenceCount(scriptObj);
  	}
  	getMessage(transaction)->bytes = createNewByteArrayOfSize(187, err2); /*Create space to put bytes in */
  	serializeTransaction(transaction);
  	CU_ASSERT_EQUAL(compareByteArrays(bytes, getMessage(transaction)->bytes),0);

}

void test_case_Signing_operations(void){
	EC_KEY * keys[21];
		uint8_t * publicKeys[21];
		uint8_t pubSizes[21];
		unsigned int sigSizes[21];
		unsigned char testKey[279] = { 0x30, 0x82, 0x01, 0x13, 0x02, 0x01, 0x01,
				0x04, 0x20, 0x87, 0x5f, 0xc0, 0x45, 0x40, 0xf0, 0x2b, 0x8a, 0x6c,
				0x21, 0x18, 0xb7, 0x87, 0xe3, 0xe6, 0x6b, 0x3d, 0xd4, 0x77, 0x32,
				0xf5, 0x62, 0x7d, 0x12, 0x30, 0xac, 0x08, 0x61, 0x1e, 0x78, 0xd6,
				0xbe, 0xa0, 0x81, 0xa5, 0x30, 0x81, 0xa2, 0x02, 0x01, 0x01, 0x30,
				0x2c, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x01, 0x01, 0x02,
				0x21, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xfc,
				0x2f, 0x30, 0x06, 0x04, 0x01, 0x00, 0x04, 0x01, 0x07, 0x04, 0x41,
				0x04, 0x79, 0xbe, 0x66, 0x7e, 0xf9, 0xdc, 0xbb, 0xac, 0x55, 0xa0,
				0x62, 0x95, 0xce, 0x87, 0x0b, 0x07, 0x02, 0x9b, 0xfc, 0xdb, 0x2d,
				0xce, 0x28, 0xd9, 0x59, 0xf2, 0x81, 0x5b, 0x16, 0xf8, 0x17, 0x98,
				0x48, 0x3a, 0xda, 0x77, 0x26, 0xa3, 0xc4, 0x65, 0x5d, 0xa4, 0xfb,
				0xfc, 0x0e, 0x11, 0x08, 0xa8, 0xfd, 0x17, 0xb4, 0x48, 0xa6, 0x85,
				0x54, 0x19, 0x9c, 0x47, 0xd0, 0x8f, 0xfb, 0x10, 0xd4, 0xb8, 0x02,
				0x21, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xba, 0xae, 0xdc, 0xe6,
				0xaf, 0x48, 0xa0, 0x3b, 0xbf, 0xd2, 0x5e, 0x8c, 0xd0, 0x36, 0x41,
				0x41, 0x02, 0x01, 0x01, 0xa1, 0x44, 0x03, 0x42, 0x00, 0x04, 0x86,
				0x6e, 0xb3, 0xaf, 0x67, 0x49, 0x74, 0x0c, 0xc1, 0x42, 0xde, 0xe7,
				0x52, 0x28, 0x20, 0x81, 0x34, 0xd8, 0xe4, 0x07, 0x94, 0x0d, 0xfe,
				0x86, 0xdc, 0xb5, 0x50, 0x5a, 0xab, 0xb0, 0x4f, 0xdc, 0xe5, 0x37,
				0x34, 0x63, 0x6a, 0xda, 0x8f, 0x28, 0x7b, 0x70, 0xf0, 0x7f, 0xda,
				0x11, 0xc6, 0xa1, 0x29, 0x1f, 0xe6, 0x01, 0xd3, 0x13, 0xad, 0x52,
				0x3a, 0x72, 0x28, 0xf3, 0x65, 0x2c, 0x71, 0x83 };

Transaction*transaction = createNewTransaction(0, 2, err2);
			/* Make outputs */
int i;
			i = 0;
			for (; i < 4; i++) {
				Script * outputsScript = createNewScriptStackUsingCopyOfData((uint8_t[]) {SCRIPT_OPCODE_TRUE}, 1, err2);
				addTransactionOutput(transaction, createNewTransactionOutput(1000, outputsScript, err2));
				decrementReferenceCount(outputsScript);
			}

			/* Make inputs */
			uint8_t hash[32];
			ByteArray * bytes = createNewByteArrayOfSize(42, err2);
			Sha256((uint8_t[]) {0x56,0x21,0xF2}, 3, hash);
			i = 0;
			for (; i < 4; i++) {
				bytes = createNewByteArrayUsingDataCopy(hash, 32, err2);
				addTransactionInput(transaction, createNewTransactionInput(NULL, 0, bytes, 0, err2));
			}
			decrementReferenceCount(bytes);

			/* Make standard output script. */
			uint8_t * subScript = malloc(25);
			subScript[0] = SCRIPT_OPCODE_DUP;
			subScript[1] = SCRIPT_OPCODE_HASH160;
			subScript[2] = 20;
			uint8_t keyHash[32];
			Sha256(publicKeys[0], pubSizes[0], keyHash);
			Ripemd160(keyHash, 32, hash);
			memmove(subScript + 3, hash, 20);
			subScript[23] = SCRIPT_OPCODE_EQUALVERIFY;
			subScript[24] = SCRIPT_OPCODE_CHECKSIG;
			ByteArray * subScriptByteArray = createNewByteArrayFromData(subScript, 25, err2);
			CU_ASSERT(subScriptByteArray);
			/*Didn't quite get the point*/
}



/* Grouping all test cases into an array that will be registered later with a single test suite. */
                CU_TestInfo transactions_test_array[] = {

                { "test_case_TransactionInput_deserialization", test_case_TransactionInput_deserialization },
                { "test_case_TransactionInput_serialization", test_case_TransactionInput_serialization},
                { "test_case_TransactionOutput_deserialization", test_case_TransactionOutput_deserialization},
                { "test_case_TransactionOutput_serialization", test_case_TransactionOutput_serialization},
                { "test_case_Transaction_deserialization", test_case_Transaction_deserialization},
                { "test_case_Signing_operations", test_case_Signing_operations},
                CU_TEST_INFO_NULL,
                };
