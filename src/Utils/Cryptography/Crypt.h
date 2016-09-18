/*
 * Crypt.h
 *
 *  Created on: 05/10/2012
 *  Created by: Abdulfatai Popoola
 *  Modified by: Wen Shen
 *  Copyright (c) 2012 Bitcoin Project Team
 */
#ifndef CRYPT_H_
#define CRYPT_H_

#include <stdint.h>
#include <stdbool.h>
#include "../../Constants.h"

/**
 @brief SHA-256 cryptographic hash function.
 @param data A pointer to the byte data to hash.
 @param length The length of the data to hash.
 @param output A pointer to hold a 32-byte hash.
 */
void Sha256(uint8_t * data,uint16_t length,uint8_t * output);
/**
 @brief RIPEMD-160 cryptographic hash function.
 @param data A pointer to the byte data to hash.
 @param length The length of the data to hash.
 @param output A pointer to hold a 20-byte hash.
 */
void Ripemd160(uint8_t * data,uint16_t length,uint8_t * output);
/**
 @brief SHA-1 cryptographic hash function.
 @param data A pointer to the byte data to hash.
 @param length The length of the data to hash.
 @param output A pointer to hold a 10-byte hash.
 */
void Sha160(uint8_t * data,uint16_t length,uint8_t * output);
/**
 @brief Verifies an ECDSA signature. This function must stick to the cryptography requirements in OpenSSL version 1.0.0 or any other compatible version. There may be compatibility problems when using libraries or code other than OpenSSL since OpenSSL does not adhere fully to the SEC1 ECDSA standards. This could cause security problems in your code. If in doubt, stick to OpenSSL.
 @param signature BER encoded signature bytes.
 @param signatureLen The length of the signature bytes.
 @param hash A 32 byte hash for checking the signature against.
 @param pubKey Public key bytes to check this signature with.
 @param keyLen The length of the public key bytes.
 @returns true if the signature is valid and false if invalid.
 */
int ECDSAVerify(uint8_t * signature, uint8_t signatureLen, uint8_t * hash, const uint8_t * pubKey, uint8_t keyLen);

#endif /* CRYPT_H_ */
