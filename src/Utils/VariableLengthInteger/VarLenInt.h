
/*
 * VarLenInt.h
 *
 *  Created on: 06/10/2012
 *  Created by: Abdulfatai Popoola
 *  Modified by: Wen Shen
 *  Copyright (c) 2012 Bitcoin Project Team
 */

/**
 @brief Variable length integer specified in Bitcoin protocol
 */

#ifndef VARLENINT_H_
#define VARLENINT_H_

#include "../../Object/ByteArray.h"
#include "../../Constants.h"

/**
 @brief Contains decoded variable size integer information. @see VarLenInt.h
 */


typedef struct{
	uint64_t value; /**< Integer value */
	uint8_t storageSize; /**< Size of the integer when encoded in bytes */
}VarLenInt;

/**
 @brief Decodes variable size integer from bytes into a VarLenInt structure.
 @param bytes The byte array to decode a variable size integer from.
 @param offset Offset to start decoding from.
 @returns The VarLenInt information
 */
VarLenInt decodeVarLenInt(ByteArray * bytes, uint32_t offset);
/**
 @brief Encodes variable size integer into bytes.
 @param bytes The byte array to encode a variable size integer into.
 @param offset Offset to start decoding from.
 @param varInt Variable integer structure to encode.
 */
void encodeVarLenInt(ByteArray * bytes,uint32_t offset, VarLenInt varInt);
/**
 @brief Returns a variable integer from an unsigned 64 bit integer.
 @param integer The 64 bit integer
 @returns A VarLenInt.
 */
VarLenInt createVarLenIntFromUInt64(uint64_t integer);
/**
 @brief Returns the variable integer byte size of a 64 bit integer
 @param value The 64 bit integer
 @returns The size of a variable length integer for this integer.
 */
uint8_t getSizeOfVarLenInt(uint64_t value);


#endif /* VARIABLELENGTHINTEGER_H_ */
