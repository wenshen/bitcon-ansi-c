/*
 * BigInt.h
 *
 *  Created on: 04/10/2012
 *  Created by: Wen Shen
 *  Copyright (c) 2012 Bitcoin Project Team
 */
#include "BigInt.h"
#include <assert.h>
#include <stdbool.h>

int BigIntAlloc(BigInt * bi, uint8_t length){

	bi -> length = length;
	bi -> data = malloc(length);

	return bi -> data != NULL;
}

int BigIntRealloc(BigInt * bi, uint8_t length){

	if (bi -> length < length) {

		uint8_t * temp = realloc(bi->data, length);
		if (! temp) {

			return false;
		}

		bi -> data = temp;
		bi -> length = length;
	}

	return true;
}

Compare BigIntCompareTo58(BigInt * a){

	if (a -> length > 1) {

		return COMPARE_MORE_THAN;
	}
	if (a -> data[0] > 58) {

		return COMPARE_MORE_THAN;
	}
	else if (a -> data[0] < 58) {

		return COMPARE_LESS_THAN;
	}
	return COMPARE_EQUAL;
}

/*
Compare BigIntCompareToBigInt(BigInt * a,BigInt * b){
	if (a->length > b->length){
		return COMPARE_MORE_THAN;
		}
	else if (a->length < b->length){
		return COMPARE_LESS_THAN;
		}
	uint8_t x;
	for (x = a->length - 1;; x--) {
		if (a->data[x] < b->data[x]){
			return COMPARE_LESS_THAN;
			}
		else if (a->data[x] > b->data[x]){
			return COMPARE_MORE_THAN;
			}
		if (!x){
			break;
			}
	}
	return COMPARE_EQUAL;
}*/

int BigIntEqualsAdditionByBigInt(BigInt * a,BigInt * b){

	if (a -> length < b -> length) {

		/* Make certain expansion of data is empty*/
		uint8_t * temp = realloc(a->data, b->length);
			if (!temp) {
				free(a->data);
				a->data = NULL;
				a->length = 0;
				return false;
			}
			a->data = temp;

			uint8_t diff = b->length - a->length;
			memset(a->data + (b->length - diff), 0, diff);

		a->length = b->length;
	}

	/* a->length >= b->length */
	bool overflow = 0;

	uint8_t x = 0;
	for (; x < b -> length; x++) {

		a -> data[x] += b -> data[x] + overflow;
		/* a->data[x] now equals the result of the addition.*/
		/* The overflow will never go beyond 1. Imagine a->data[x] == 0xff, b->data[x] == 0xff and the overflow is 1, the new overflow is still 1 and a->data[x] is 0xff. Therefore it does work.*/
		overflow = (a -> data[x] < (b -> data[x] + overflow))? 1 : 0;
	}
	/* Propagate overflow up the whole length of a if necessary*/

	while (overflow && x < a -> length) {

		overflow = ! ++a->data[x++]; /* Index at x, increment x, increment data, test new value for overflow.*/
	}

	if (overflow) {

		a -> length++;

		if (! BigIntRealloc(a, a -> length)) {

			return false;
		}
		a->data[a->length - 1] = 1;
	}
	return true;
}

void BigIntEqualsDivisionBy58(BigInt * a,uint8_t * ans) {

	if (a -> length == 1 && ! a -> data[0]) {

		return;
	}

	uint16_t temp = 0;
	uint8_t x;
	for (x = a -> length-1;; x--) {

		temp <<= 8;
		temp |= a->data[x];
		ans[x] = temp / 58;
		temp -= ans[x] * 58;

		if (! x) {

			break;
		}
	}
	if (! ans[a->length-1]) {

		a -> length--;
	}

	memmove(a -> data, ans, a -> length);
}
int BigIntEqualsMultiplicationByUInt8(BigInt * a,uint8_t b,uint8_t * ans) {

	if (! b) {

		/* Mutliplication by zero. "a" becomes zero */
		a->length = 1;
		a->data[0] = 0;
		return true;
	}

	if (a->length == 1 && ! a->data[0]) { /* "a" is zero */

		return true;
	}
	/* Multiply b by each byte and then add to answer*/
	uint8_t x;
	for (x = 0; x < a->length; x++) {

		uint16_t mult = ans[x] + a->data[x] * b; /*Allow for overflow onto next byte.*/
		ans[x] = mult;
		ans[x+1] = (mult >> 8);
	}

	if (ans[a->length]) { /*If last byte is ! zero, adjust length.*/

		a->length++;

		if (! BigIntRealloc(a, a->length)) {

			return false;
		}
	}

	memmove(a->data, ans, a->length); /*Done calculation. Move ans to "a".*/

	return true;
}
/*void BigIntEqualsSubtractionByBigInt(BigInt * a,BigInt * b){

	uint8_t x;
	for (x = 0; x < b->length; x++) {
		uint8_t sub = b->data[x];
		uint8_t y;
		for (y = x; y < a->length; y++) {
			if (a->data[y] >= sub) {
				a->data[y] -= sub;
				break;
			}else{
				a->data[y] = 255 - (sub - a->data[y] - 1);
				sub = 1;
			}
		}
	}
	BigIntNormalise(a);
}*/
void BigIntEqualsSubtractionByUInt8(BigInt * a,uint8_t b) {

	uint8_t sub = b;
	uint8_t x;
	for (x = 0;x < a->length; x++) {

		if (a->data[x] >= sub) {
			a->data[x] -= sub;
			break;
		} else {
			a->data[x] = 255 - (sub - a->data[x] - 1);
			sub = 1;
		}
	}
	BigIntNormalise(a);
}


int BigIntFromPowUInt8(BigInt * bi,uint8_t a,uint8_t b) {

	bi -> length = 1;
	bi -> data[0] = 1;
	uint8_t * temp = malloc(b);

	if (! temp) {

		return false;
	}

	uint8_t x;

	for (x = 0; x < b; x++) {
		memset(temp, 0, bi->length);

		if (! BigIntEqualsMultiplicationByUInt8(bi, a, temp)){
			return false;
		}
	}
	free(temp);
	return true;
}

uint8_t BigIntModuloWith58(BigInt * a) {

	uint16_t result = 0;
	uint8_t x;

	for(x = a -> length - 1;; x--) {

		result *= (256 % 58);
		result %= 58;
		result += a->data[x] % 58;
		result %= 58;

		if (! x) {

			break;
		}
	}
	return result;
}
void BigIntNormalise(BigInt * a) {

	uint8_t x;
	for (x = a->length - 1;; x--) {

		if (a->data[x]) {

			if (x == a->length - 1) {

				break;
			}

			a -> length = x + 1;
			break;

		} else if (! x) {

			a -> length = 1;

			break;
		}
	}
}

