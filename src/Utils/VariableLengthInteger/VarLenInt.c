/*
 * VarLenInt.c
 *
 *  Created on: 06/10/2012
 *  Created by: Abdulfatai Popoola
 *  Modified by: Wen Shen
 *  Copyright (c) 2012 Bitcoin Project Team
 */

#include "VarLenInt.h"
#include <assert.h>

VarLenInt decodeVarLenInt(ByteArray * bytes, uint32_t offset) {
	assert(bytes != NULL);


	uint8_t firstByte = getByteFromByteArray(bytes, offset);

	VarLenInt varInt;

	if (firstByte < 253) {
		/* Use byte literally*/
		varInt.value = firstByte;
		varInt.storageSize = 1;
	} else if (firstByte == 253) {
		/* Read the next two bytes as a little endian 16-bit number*/
		varInt.value
				= readInt16AsLittleEndianFromByteArray(bytes, offset + 1);
		varInt.storageSize = 3;
	} else if (firstByte == 254) {
		/*read the next four bytes as a little endian 32-bit number*/
		varInt.value
				= readInt32AsLittleEndianFromByteArray(bytes, offset + 1);
		varInt.storageSize = 5;
	} else {
		/* read the next eight bytes as a little endian 64-bit number*/
		varInt.value
				= readInt64AsLittleEndianFromByteArray(bytes, offset + 1);
		varInt.storageSize = 9;
	}

	return varInt;
}

void encodeVarLenInt(ByteArray * bytes, uint32_t offset, VarLenInt varInt) {
	assert(bytes != NULL);
	assert(offset >= 0);
	assert(&varInt != NULL);

	switch (varInt.storageSize) {
	case 1:
		setByteInByteArray(bytes, offset, (uint8_t) varInt.value);
		break;
	case 3:
		setByteInByteArray(bytes, offset, 253);
		writeInt16AsLittleEndianIntoByteArray(bytes, offset + 1,
				(uint16_t) varInt.value);
		break;
	case 5:
		setByteInByteArray(bytes, offset, 254);
		writeInt32AsLittleEndianIntoByteArray(bytes, offset + 1,
				(uint32_t) varInt.value);
		break;
	case 9:
		setByteInByteArray(bytes, offset, 255);
		writeInt64AsLittleEndianIntoByteArray(bytes, offset + 1, varInt.value);
		break;
	}
}

VarLenInt createVarLenIntFromUInt64(uint64_t integer) {
	assert(integer >= 0);

	VarLenInt varInt;

	varInt.value = integer;
	varInt.storageSize = getSizeOfVarLenInt(integer);

	return varInt;
}

uint8_t getSizeOfVarLenInt(uint64_t value) {
	assert(value >= 0);

	if (value < 253) { /*TODO replace all these figures with constants*/
		return 1;
	} else if (value < 65536) {
		return 3; /* 1 marker + 2 data bytes */
	} else if (value < 4294967296) {
		return 5; /*1 marker + 4 data bytes*/
	} else {
		return 9; /* 1 marker + 8 data bytes */
	}
}
