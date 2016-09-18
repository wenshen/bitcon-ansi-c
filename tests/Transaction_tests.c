/*
 * Transaction_tests.c
 *
 *  Created on: Oct 30, 2012
 *      Author: apopoola
 */

#include <stdio.h>
#include "stdarg.h"
#include <time.h>
#include "openssl/ssl.h"
#include "openssl/ripemd.h"
#include "openssl/rand.h"
#include "../src/Object/Message/Transaction.h"
#include "../src/Object/Message/TransactionInput.h"
#include "../src/Object/Script.h"

void onErrorReceived(Error a,char * format,...);
void onErrorReceived(Error a,char * format,...){
	va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
	printf("\n");
}

int main(int argc, char *argv[]){
	unsigned int seed = (unsigned int) time(NULL);
	seed = 1337544566;
	printf("Session = %ui \n", seed);
	srand(seed);

	/* TransactionInput Tests */

	/* Test TransactionInput deserialisation */
	uint8_t hash[32];
	Sha160((uint8_t *)"hello", 5, hash);
	ByteArray * bytes = createNewByteArrayOfSize(42, onErrorReceived);

	/* Move bytes into ByteArray */
	setBytesInByteArray(bytes, 0, hash, 32);
	setBytesInByteArray(bytes, 32, (uint8_t[]) {0x05,0xFA,0x4B,0x51}, 4);
	encodeVarLenInt(bytes, 36, createVarLenIntFromUInt64(1));
	setByteInByteArray(bytes, 37, SCRIPT_OPCODE_TRUE);

	uint32_t sequence = rand();
	writeInt32AsLittleEndianIntoByteArray(bytes, 38, sequence);
	TransactionInput *input = createNewTransactionInputFromData(bytes, onErrorReceived);
	uint32_t deserialized = deserializeTransactionInput(input);
	printf("Deserialized = %u \n", deserialized);

	if(memcmp(hash, getByteArrayData(input->prevOutput.hash), 32) != 0){
		printf("TransactionInput DESERIALISED but previous output hash doesn't match \n");
		printf("0x");
		int i = 0;

		for (i = 0; i < 32; i++) {
			printf("%.2x",hash[i]);
		}

		printf(" != 0x");
		uint8_t * previousOutputHash = getByteArrayData(input->prevOutput.hash);
		for (i = 0; i < 32; i++) {
			printf("%.2x",previousOutputHash[i]);
		}
		return 1;
	}

	if (input->prevOutput.index != 0x514BFA05) {
		printf("TransactionInput DESERIALISED previous Output index INCORRECT: 0x%X != 0x514BFA05\n",input->prevOutput.index);
		return 1;
	}

	ScriptStack scriptStack = createEmptyScriptStack();
	if(executeScript(input->scriptObject, &scriptStack, NULL, NULL, 0, FALSE) != GOOD_SCRIPT){
		printf("Deserialization of TransactionInput Script failed\n");
		return 1;
	}

	if (input->sequence != sequence) {
		printf("Deserialization of TransactionInput sequence INCORRECT: %i != %i\n",input->sequence,sequence);
		return 1;
	}
	destroyObject(input);

	/* Test TransactionInput serialisation */
	Script * scriptObj = createNewScriptStackUsingCopyOfData((uint8_t []) {SCRIPT_OPCODE_TRUE}, 1, onErrorReceived) ;
	ByteArray * outPointerHash = createNewByteArrayUsingDataCopy(hash, 32, onErrorReceived);
	input = createNewTransactionInput(scriptObj, sequence, outPointerHash, 0x514BFA05, onErrorReceived);

	getMessage(input)->bytes = createNewByteArrayOfSize(42, onErrorReceived);

	serializeTransactionInput(input);

	if (compareByteArrays(bytes, getMessage(input)->bytes) != 0) {
		printf("Serialization of TransactionInput failed\n0x");

		uint8_t * dump = getByteArrayData(getMessage(input)->bytes);

		int i = 0;
		for (; i < 42; i++) {
			printf("%.2x", dump[i]);
		}
		printf("\n!=\n0x");

		dump = getByteArrayData(bytes);

		i = 0; /*re-initialize counter */
		for (; i < 42; i++) {
			printf("%.2x", dump[i]);
		}
		return 1;
	}

	destroyObject(input);
	destroyObject(bytes);

	/* TransactionOutput Tests */

	/* Test TransactionOutput deserialisation */
	bytes = createNewByteArrayOfSize(10, onErrorReceived);

	uint64_t value = rand(); /* Use a random value */
	writeInt64AsLittleEndianIntoByteArray(bytes, 0, value);
	encodeVarLenInt(bytes, 8, createVarLenIntFromUInt64(1));

	setByteInByteArray(bytes, 9, SCRIPT_OPCODE_TRUE);
	TransactionOutput * output = createNewTransactionOutputFromData(bytes, onErrorReceived);
	deserializeTransactionOutput(output);
	if (output->value != value) {
		printf(
				"Deserialized TransactionOutput not equal to original value passed in: %llu != %llu\n",
				output->value, value);
		return 1;
	}

	scriptStack = createEmptyScriptStack();
	if (executeScript(output->scriptObject, &scriptStack, NULL, NULL, 0, FALSE)	!= GOOD_SCRIPT) {
		printf("Deserialization of TransactionOutput failed\n");
		return 1;
	}
	destroyObject(output);

	/* Test TransactionOutput serialisation */
	output = createNewTransactionOutput(value, scriptObj, onErrorReceived);
	getMessage(output)->bytes = createNewByteArrayOfSize(10, onErrorReceived);

	serializeTransactionOutput(output);

	if (compareByteArrays(bytes, getMessage(output)->bytes)) {
		printf("Serialization of TransactionOutput failed\n0x");

		uint8_t * dump = getByteArrayData(getMessage(output)->bytes);
		int i = 0;
		for (; i < 14; i++) {
			printf("%.2x", dump[i]);
		}
		printf("\n!=\n0x");

		dump = getByteArrayData(bytes);
		i = 0;
		for (; i < 14; i++) {
			printf("%.2x", dump[i]);
		}
		return 1;
	}
	destroyObject(output);
	destroyObject(bytes);

	/* Transaction Tests */
	/* Test deserialisation */
	/* Set script data */

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

	uint8_t i = 0;
	for (; i < 32; i++) {
		hash[i] = rand();
	}
	uint32_t randInt = rand();
	uint64_t randInt64 = rand();
	bytes = createNewByteArrayOfSize(187, onErrorReceived);

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

	Transaction * transaction = createNewTransactionFromByteArray(bytes, onErrorReceived);
	deserializeTransaction(transaction);
	if (transaction->version != 2) {
		printf("Deserialization of transaction version failed. %u != 2\n",
				transaction->version);
		return 1;
	}
	if (transaction->numOfTransactionInput != 3) {
		printf("Deserialization of transaction numOfTransactionInput failed. %u != 3\n",
				transaction->numOfTransactionInput);
		return 1;
	}
	if (transaction->numOfTransactionOutput != 2) {
		printf("Deserialization of transaction numOfTransactionOutput failed. %u != 2\n",
				transaction->numOfTransactionOutput);
		return 1;
	}
	if (transaction->lockTime != randInt) {
		printf("TRANSACTION DESERIALISATION LOCK TIME FAIL. %u != %u\n",
				transaction->lockTime, randInt);
		return 1;
	}

	i = 0; /*re-init counter */
	for (; i < 3; i++) {
		if (memcmp(hash, getByteArrayData(transaction->inputs[i]->prevOutput.hash), 32)) {
			printf("TRANSACTION TransactionInput %i DESERIALISED previousOutputHash incorrect: 0x\n",
					i);

			int j = 0;
			for (; j < 32; j++) {
				printf("%.2x", hash[j]);
			}
			printf(" != 0x");
			uint8_t * dump = getByteArrayData(transaction->inputs[i]->prevOutput.hash);

			j = 0;
			for (; j < 32; j++) {
				printf("%.2x", dump[j]);
			}
			return 1;
		}

		if (transaction->inputs[i]->prevOutput.index != randInt) {
			printf(
					"TRANSACTION TransactionInput %i DESERIALISED previousOutputIndex INCORRECT: %u != %u\n",
					i, transaction->inputs[i]->prevOutput.index, randInt);
			return 1;
		}

		ScriptStack stack = createEmptyScriptStack();
		if (executeScript(transaction->inputs[i]->scriptObject, &stack, NULL, NULL, 0, FALSE) != GOOD_SCRIPT) {
			printf("TRANSACTION TransactionInput %i DESERIALISED SCRIPT FAILURE\n", i);
			return 1;
		}
		if (transaction->inputs[i]->sequence != randInt) {
			printf("TRANSACTION TransactionInput %i DESERIALISED sequence INCORRECT: %i != %i\n",
					i, transaction->inputs[i]->sequence, randInt);
			return 1;
		}
	}

	i = 0; /*re-init counter */
	for (; i < 2; i++) {
		if (transaction->outputs[i]->value != randInt64) {
			printf(
					"TRANSACTION TransactionOutput %i DESERIALISED value INCORRECT: %llu != %llu\n",
					i, transaction->outputs[i]->value, randInt64);
			return 1;
		}

		ScriptStack stack = createEmptyScriptStack();
		if (executeScript(transaction->outputs[i]->scriptObject, &stack, NULL, NULL, 0, FALSE) != GOOD_SCRIPT) {
			printf(
					"TRANSACTION TransactionOutput %i DESERIALISED SCRIPT FAILURE\n",
					i);
			return 1;
		}
	}
	destroyObject(transaction);

	/* Test serialisation */

	transaction = createNewTransaction(randInt, 2, onErrorReceived);
	outPointerHash = createNewByteArrayUsingDataCopy(hash, 32, onErrorReceived);

	i = 0; /*re-init counter */
	for (; i < 3; i++) {
		scriptObj = createNewScriptStackUsingCopyOfData(scripts[i], (!i) ? 7 : 8, onErrorReceived);
		input = createNewTransactionInput(scriptObj, randInt, outPointerHash, randInt, onErrorReceived);
		addTransactionInput(transaction, input);
		decrementReferenceCount(scriptObj);
	}

	decrementReferenceCount(outPointerHash);
	i = 3; /*re-init counter */
	for (; i < 5; i++) {
		scriptObj = createNewScriptStackUsingCopyOfData(scripts[i], (i == 3) ? 7 : 6, onErrorReceived);
		output = createNewTransactionOutput(randInt64, scriptObj, onErrorReceived);
		addTransactionOutput(transaction, output);
		decrementReferenceCount(scriptObj);
	}
	getMessage(transaction)->bytes = createNewByteArrayOfSize(187, onErrorReceived); /*Create space to put bytes in */
	serializeTransaction(transaction);

	if (compareByteArrays(bytes, getMessage(transaction)->bytes) != 0) {
		printf("Transaction SERIALISATION FAILURE\n0x");
		uint8_t * dump = getByteArrayData(getMessage(transaction)->bytes);

		int j = 0;
	    for (; j < 187; j++) {
			printf("%.2x", dump[j]);
		}
		printf("\n!=\n0x");

		dump = getByteArrayData(bytes);
		j = 0; /*re-init counter */
		for (; j < 187; j++) {
			printf("%.2x", dump[j]);
		}
		return 1;
	}
	destroyObject(transaction);
	destroyObject(bytes);

	/* Keys for testing signing operations */
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

	i = 0; /*re-init counter */
	for (; i < 21; i++) {
		keys[i] = EC_KEY_new_by_curve_name(NID_secp256k1);
		if (i == 3) {
			/* Test this key because it hashes into values that may be mistaken for code separators if the script interpreter is incorrect. */
			const unsigned char * ptestKey = testKey;
			d2i_ECPrivateKey(&keys[i], &ptestKey, 279); /* Decode private key */
		} else {
			if (!EC_KEY_generate_key(keys[i])) {
				printf("KEY GENERATION FAILED\n");
				return 1;
			}
		}
		pubSizes[i] = i2o_ECPublicKey(keys[i], NULL);

		if (! pubSizes[i]) {
			printf("PUB KEY TO DATA ZERO\n");
			return 1;
		}

		publicKeys[i] = malloc(pubSizes[i]);
		uint8_t * pubKey2 = publicKeys[i];
		if (i2o_ECPublicKey(keys[i], &pubKey2) != pubSizes[i]) { /*Check if they match*/
			printf("PUB KEY TO DATA FAIL\n");
			return 1;
		}
		sigSizes[i] = ECDSA_size(keys[i]);
	}

	/* Test signing transactions and OP_CHECKSIG */
	transaction = createNewTransaction(0, 2, onErrorReceived);
	/* Make outputs */
	i = 0;
	for (; i < 4; i++) {
		Script * outputsScript = createNewScriptStackUsingCopyOfData((uint8_t[]) {SCRIPT_OPCODE_TRUE}, 1, onErrorReceived);
		addTransactionOutput(transaction, createNewTransactionOutput(1000, outputsScript, onErrorReceived));
		decrementReferenceCount(outputsScript);
	}

	/* Make inputs */
	Sha256((uint8_t[]) {0x56,0x21,0xF2}, 3, hash);
	i = 0;
	for (; i < 4; i++) {
		bytes = createNewByteArrayUsingDataCopy(hash, 32, onErrorReceived);
		addTransactionInput(transaction, createNewTransactionInput(NULL, 0, bytes, 0, onErrorReceived));
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
	ByteArray * subScriptByteArray = createNewByteArrayFromData(subScript, 25, onErrorReceived);

	return 0;
}
