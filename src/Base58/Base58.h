/*
 * Base58.h
 *
 *  Created on: 03/10/2012
 *  Created by: Hayk Baluyan
 *  Copyright (c) 2012 Bitcoin Project Team
 */

/**
 @file
 @brief Functions for encoding and decoding in base 58. Avoids 0olI whch may look alike. This is for readability concerns.
 */

#ifndef BASE58H
#define BASE58H

#include <stdlib.h>
#include "../BigInt/BigInt.h"


static const char base58Characters[58] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

/**
 @brief Decodes base 58 string into byte data as a BigInt.
 @param str Base 58 string to decode.
 @returns Pointer to the byte data as a BigInt. The byte data will be created in this function. Remember to free the data. On error the big int will have a NULL data pointer.
 */

BigInt decodeBase58(char * str);
/**
 @brief Decodes base 58 string into byte data as a BigInt and checks a 4 byte checksum.
 @param str Base 58 string to decode.
 @returns Byte data as a BigInt. Is zero on failure. Checksum is included in returned data. On error the big int will have a NULL data pointer.
 */

BigInt decodeBase58Checked(char * str,void (*onErrorReceived)(Error error,char *,...));
/**
 @brief Encodes byte data into base 58.
 @param bytes Pointer to byte data to encode. Will almost certainly be modified. Copy data beforehand if needed.
 @param len Length of bytes to encode.
 @returns Newly allocated string with encoded data or NULL on error.
 */

char * dncodeBase58(uint8_t * bytes, uint8_t len);

uint8_t* getCheckSum(uint8_t* data);
uint8_t* verifyAndRemoveCheckSum(uint8_t* data);

#endif

