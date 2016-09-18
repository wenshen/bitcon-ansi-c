/*
 * BigInt.h
 *
 *  Created on: 04/10/2012
 *  Created by: Wen Shen
 *  Copyright (c) 2012 Bitcoin Project Team
 */

#ifndef BIGINT_H_
#define BIGINT_H_
#include "../Constants.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/**
 @brief Contains byte data with the length of this data to represent a large integer. The byte data is in little-endian which stores the smallest byte first. On an error data is set to NULL and length is 0.
 */
typedef struct {

	uint8_t * data; /**< The byte data. Should be little-endian */
	uint8_t length; /**< The length of this data in bytes */
} BigInt;

int BigIntFromPowUInt8(BigInt * bi, uint8_t a, uint8_t b);
int BigIntEqualsMultiplicationByUInt8(BigInt * a,uint8_t b, uint8_t * ans);
int BigIntEqualsAdditionByBigInt(BigInt * a, BigInt * b);

void BigIntNormalise(BigInt * a);
uint8_t BigIntModuloWith58(BigInt * a);
void BigIntEqualsSubtractionByUInt8(BigInt * a, uint8_t b);
void BigIntEqualsDivisionBy58(BigInt * a, uint8_t * ans);
Compare BigIntCompareTo58(BigInt * a);
#endif /* BIGINT_H_ */
