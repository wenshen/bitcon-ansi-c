/*
 * BlockValidationFunction.c
 *
 * Created on: Nov 25, 2012
 * Created by: Wen Shen
 * Copyright (c) 2012 MIBitcoin Project Team
 */

#include "BlockValidationFunction.h"
#include "../BigInt/BigInt.h"
#include "../Object/Message/TransactionInput.h"
#include "../Object/Message/Transaction.h"
#include "../Object/ByteArray.h"
#include "../Object/Script.h"

uint64_t calculateBlockReward(uint64_t blockHeight){
	return (50 * ONE_BITCOIN) >> (blockHeight / 210000);
}
bool calculateBlockWork(BigInt * work, uint32_t target){
	/* Get the base-256 exponent and the mantissa.*/
	uint8_t zeroBytes = target >> 24;
	target &= 0x00FFFFFF;
	/* Allocate BigInt data*/
	work->length = 32;
	BigIntAlloc(work, work->length);
	if (! work->data)
		return FALSE;
	/* Do base-4294967296 long division and adjust for trailing zeros in target.*/
	uint64_t temp = 0x01000000;
	uint32_t workSeg;
	uint8_t x;
	for ( x = 0;; x++) {
		workSeg = (uint32_t)(temp / target);
		uint8_t i = 31 - x * 4 - zeroBytes + 4;
		uint8_t y;
		for ( y = 0; y < 4; y++){
			work->data[i] = workSeg >> ((3 - y) * 8);
			if (! i){
				BigIntNormalise(work);
				return TRUE;
			}
			i--;
		}
		temp -= workSeg * target;
		temp <<= 32;
	}
}
void calculateMerkleRoot(uint8_t * hashes,uint32_t hashNum){
	uint8_t hash[32];
	uint32_t x;
	for ( x = 0; hashNum != 1;) {
		if (x == hashNum - 1) {
			/* Duplicate final hash */
			uint8_t dup[64];
			memcpy(dup, hashes + x * 32, 32);
			memcpy(dup + 32, hashes + x * 32, 32);
			Sha256(dup, 64, hash);
		}else{
			Sha256(hashes + x * 32, 64, hash);
		}
		/* Double SHA256*/
		Sha256(hash, 32, hashes + x * 32/2);
		x += 2;
		if (x >= hashNum) {
			if (x > hashNum){
				/* The number of hashes was odd. Increment to even*/
				hashNum++;
			}
			hashNum /= 2;
			x = 0;
		}
	}
}

uint32_t calculateTarget(uint32_t oldTarget, uint32_t time){
	/* Limitations on the factor for retargeting.*/
	if (time < TARGET_INTERVAL / 4){
		time = TARGET_INTERVAL / 4;
	}
	if (time > TARGET_INTERVAL * 4){
		time = TARGET_INTERVAL * 4;
	}

	/* Get number of trailing 0 bytes (Base 256 exponent)*/
	uint8_t zeroBytes = oldTarget >> 24;
	/* Get the three bytes for the target mantissa. Use 8 bytes for overflow. Shift along once for division.*/
	uint64_t num = (oldTarget & 0x00FFFFFF) << 8;
	/* Modify the three bytes*/
	num *= time;
	num /= TARGET_INTERVAL;
	/* Find first byte with data. Check for flow into more or less significant byte.*/
	if (num & 0xF00000000) {
		/* Mantissa overflowed into more significant byte.*/
		num >>= 16;
		num &= 0x00FFFFFF; /* Clear the forth byte for the number of zero bytes.*/
		zeroBytes++;
	}else if (num & 0xFF000000){
		/* Data within same three bytes*/
		num >>= 8;
	}else {
		/* Data down a byte*/
		zeroBytes--;
	}
	/* Modify in the case of the first byte in the mantissa being over 0x7F... What did you say?... Oh yes, the bitcoin protocol can be dumb in places.*/
	if (num > 0x007FFFFF) {
		/* Add trailing zero by shifting right...*/
		num >>= 8;
		/* Compensate with additional trailing zero byte.*/
		zeroBytes++;
	}
	/* Apply the number of trailing zero bytes into the compact target representation.*/
	num |= zeroBytes << 24;
	/* Check if the target is too high and if it is, make it equal the maximum target.*/
	if (num > MAX_TARGET){
		num = MAX_TARGET;
	}
	/* Return the new target*/
	return (uint32_t) num;
}
uint32_t transactionGetSigOps(Transaction * tx){
	uint32_t sigOps = 0;
	uint32_t x;
	for ( x = 0; x < tx->numOfTransactionInput; x++){
		sigOps += ScriptGetSigOpCount(tx->inputs[x]->scriptObject, FALSE);
	}
	for ( x = 0; x < tx->numOfTransactionOutput; x++){
		sigOps += ScriptGetSigOpCount(tx->outputs[x]->scriptObject, FALSE);
	}
	return sigOps;
}
bool transactionIsFinal(Transaction * tx, uint64_t time, uint64_t height){
	if (tx->lockTime) {
		if (tx->lockTime < (tx->lockTime < LOCKTIME_THRESHOLD ? (int64_t)height : (int64_t)time)){
			return TRUE;
		}
		uint32_t x;
		for ( x = 0; x < tx->numOfTransactionInput; x++){
			if (tx->inputs[x]->sequence != TRANSACTION_INPUT_FINAL){
				return FALSE;
			}
		}
	}
	return TRUE;
}
PreviousOutput * transactionValidateBasic(Transaction * tx, bool coinbase, uint64_t * outputValue, bool * err){
	*err = FALSE;
	if (! tx->numOfTransactionInput || ! tx->numOfTransactionOutput){
		return NULL;
	}
	uint32_t length;
	if (getMessage(tx)->bytes) /* Already have length */{
		length = getMessage(tx)->bytes->length;
	}else{
		/* Calculate length. Worthwhile having a cache */
		length = calculateTransactionLength(tx);
	}
	if (length > BLOCK_MAX_SIZE){
		return NULL;
	}
	/* Check that outputs do not overflow by ensuring they do not go over 21 million bitcoins. There was once an vulnerability in the C++ client on this where an attacker could overflow very large outputs to equal small inputs.*/
	*outputValue = 0;
	uint32_t x;
	for ( x = 0; x < tx->numOfTransactionOutput; x++) {
		if (tx->outputs[x]->value > MAX_MONEY){
			return NULL;
		}
		*outputValue += tx->outputs[x]->value;
		if (*outputValue > MAX_MONEY){
			return NULL;
		}
	}
	if (coinbase){
		/* Validate input script for coinbase*/
		if (tx->inputs[0]->scriptObject->length < 2
			|| tx->inputs[0]->scriptObject->length > 100){
			return NULL;
		}
	}else for ( x = 0; x < tx->numOfTransactionInput; x++)
		/* Check each input for null previous output hashes.*/
		if (isNullByteArray(tx->inputs[x]->prevOutput.hash)){
			return NULL;
		}
	/* Check for duplicate transaction output spends and add them to a list of PreviousOutput structures.*/
	PreviousOutput * prevOutputs = malloc(sizeof(*prevOutputs) * tx->numOfTransactionInput);
	if (!prevOutputs){
		*err = TRUE;
		return NULL;
	}
	for ( x = 0; x < tx->numOfTransactionInput; x++) {
		uint32_t y;
		for ( y = 0; y < x; y++)
			if (compareByteArrays(prevOutputs[y].hash, tx->inputs[x]->prevOutput.hash) == COMPARE_EQUAL
				&& prevOutputs[y].index == tx->inputs[x]->prevOutput.index) {
				/* Duplicate previous output*/
				free(prevOutputs);
				return NULL;
			}
		prevOutputs[x] = tx->inputs[x]->prevOutput;
	}
	return prevOutputs;
}
bool validateProofOfWork(uint8_t * hash, uint32_t target){
	/* Get trailing zero bytes*/
	uint8_t zeroBytes = target >> 24;
	/* Check target is less than or equal to maximum.*/
	if (target > MAX_TARGET){
		return FALSE;
	}
	/* Modify the target to the mantissa (significand).*/
	target &= 0x00FFFFFF;
	/* Check mantissa is below 0x800000.*/
	if (target > 0x7FFFFF){
		return FALSE;
	}
	/* Fail if hash is above target. First check leading bytes to significant part. As the hash is seen as little-endian, do this backwards.*/
	uint8_t x;
	for ( x = 0; x < 32 - zeroBytes; x++)
		if (hash[31 - x]){
			/* A byte leading to the significant part is not zero*/
			return FALSE;
		}
	/* Check significant part*/
	uint32_t significantPart = hash[zeroBytes - 1] << 16;
	significantPart |= hash[zeroBytes - 2] << 8;
	significantPart |= hash[zeroBytes - 3];
	if (significantPart > target){
		return FALSE;
	}
	return TRUE;
}
