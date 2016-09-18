
/*
 * Script.h
 *
 *  Created on: 08/10/2012
 *  Created by: Abdulfatai Popoola
 *  Copyright (c) 2012 Bitcoin Project Team
 */


#ifndef SCRIPT_H_
#define SCRIPT_H_

/*  Includes*/

#include "ByteArray.h"
#include <stdint.h>
#include <stdbool.h>

/**
 @brief Structure for a byte vector;
        Byte vectors are interpreted as little-endian variable-length integers
        with the most significant bit determining the sign of the integer.
 */
typedef struct ScriptByteVector{
	uint8_t * data; /**< Data for this byte vector */
	uint16_t length; /**< Length of this byte vector */
} ScriptByteVector;

/**
 @brief Structure that holds byte vector data in a stack.
 */
typedef struct{
	struct ScriptByteVector * elements; /**< Elements in the stack */
	uint16_t length; /**< Length of the stack */
} ScriptStack;

typedef ByteArray Script; /** A script is a array of bytes*/

/**
 @brief Returns a new empty scriptstack.
 @returns The new empty stack.
 */
ScriptStack createEmptyScriptStack(void);

/**
 @brief Creates a new Script object.
 @returns A new Script object.
 */
Script * createNewScriptFromReference(Script * program, uint32_t offset, uint32_t len);
/**
 @brief Executes a bitcoin script.
 @param self The Script object with the program
 @param stack A pointer to the input stack for the program.
 @param getHashForSig A pointer to the function to get the hash for checking the signature. Should take a Transaction object, input index and the SignatureType and the Dependencies object.
 @param transaction Transaction for checking the signatures.
 @param inputIndex The index of the input for the signature.
 @param p2sh If FALSE, do not allow any P2SH matches.
 @returns GOOD_SCRIPT if the program ended with true, BAD_SCRIPT on script failure or SCRIPT_ERR if an error occurred with the interpreter such as running of of memory.
 */
ScriptExecutionStatus executeScript(Script * self, ScriptStack * stack, TransactionHashStatus (*getHashForSignature)(void *, ByteArray *, uint32_t, SignatureType, uint8_t *), void * transaction, uint32_t inputIndex, int p2sh);
/**
 @brief Returns the number of sigops.
 @param self The CBScript object.
 @param inP2SH true when getting sigops for a P2SH script.
 @retuns the number of sigops as used for validation.
 */
uint32_t scriptGetSigOpCount(Script * self, bool inP2SH);

/**
 @brief Determines if a script object matches the P2SH template; i.e. BIP 0016 see https://en.bitcoin.it/wiki/BIP_0016
 @param self The Script object.
 @returns one if the script matches the P2SH template, zero otherwise.
 */
int isP2SHScript(Script * self);

/**
 @brief Returns a copy of a stack item, "fromTop" from the top.
 @param stack A pointer to the stack.
 @param fromTop Number of items from the top to copy.
 @returns A copy of the stack item which should be freed.
 */
ScriptByteVector copyByteVectorFromScriptStack(ScriptStack * stack, uint8_t fromTop);

/**
 @brief Push a ScriptByteVector item onto the stack which is freed by the stack.
 @param stack A pointer to the ScriptStack to push data onto.
 @param data The item to push on the stack.
 */
int pushScriptByteVectorOntoScriptStack(ScriptStack * stack, ScriptByteVector item);

/**
 @brief Evaluates the top ScriptByteVector on a ScriptStack as a boolean. False if 0 or -0.
 @param stack The ScriptStack.
 @returns The boolean result.
 */
int topElementEvaluatesToTrue(ScriptStack * stack);

/**
 @brief Removes top ScriptByteVector from the stack.
 @param stack A pointer to the stack to remove the data.
 */
void removeElementFromStack(ScriptStack * stack);

/**
 @brief Pops the top ScriptByteVector off the ScriptStack
 @param stack A pointer to the stack to remove the data.
 @returns ScriptByteVector the top ScriptByteVector on the stack
 */
ScriptByteVector popElementFromStack(ScriptStack * stack);

/**
 @brief Converts a int64_t into a ScriptByteVector
 @param byteVector Pass in a ScriptByteVector for reallocating data.
 @param int64 The 64 bit signed integer.
 @returns A ScriptByteVector.
 */
ScriptByteVector convertInt64ToScriptByteVector(ScriptByteVector byteVector, int64_t int64);

/**
 @brief Converts a ScriptByteVector to a 64 bit signed integer
 @param item The ScriptByteVector to convert
 @returns A 64 bit signed integer.
 */
int64_t convertScriptByteVectorToInt64(ScriptByteVector byteVector);

/**
 @brief Removes occurrences of a signature from script data
 @param subScript The sub script to remove signatures from.
 @param subScriptLen A pointer to the length of the sub script. The length will be modified to the new length.
 @param signature The signature to be found and removed.
 */
void removeSignatureFromStackScript(uint8_t * subScript, uint32_t * subScriptLen, ScriptByteVector signature);

/**
 @brief Creates a new Script using data.
 @param data The data. This should be dynamically allocated. The new ByteArray object will take care of it's memory management so do not free this data once passed into this constructor.
 @param size Size in bytes for the new array.
 @param onErrorReceived Struct for handling errors.
 @returns The new Script object.
 */
Script * createNewScriptStackFromData(uint8_t * data, uint32_t size, void (*onErrorReceived)(Error error,char *,...));

/**
 @brief Creates a new Script using a copy of data.
 @param data The data is copied
 @param size Size in bytes for the new array.
 @param onErrorReceived Struct for handling errors.
 @returns The new Script object.
 */
Script * createNewScriptStackUsingCopyOfData(uint8_t * data, uint32_t size, void (*onErrorReceived)(Error error,char *,...));

/**
 @brief Creates a new Script object from a string. The script text should follow the following Backus–Naur Form:
 /code
 <digit> = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" | "a" | "b" | "c" | "d" | "e" | "f" | "A" | "B" | "C" | "D" | "E" | "F"
 <hex_digits> ::= <digit> | <digit> <hex_digits>
 <hex> ::= "0x" <hex_digits>
 <operation_name> = "0" | "FALSE" | "1NEGATE" | "RESERVED" | "1" | "TRUE" | "2" | "3" | "4" |  "5" | "6" | "7" | "8" | "9" | "10" | "11" | "12" | "13" | "14" | "15" | "16" | "NOP" | "VER" | "IF" | "NOTIF" | "VERIF" | "VERNOTIF" | "ELSE" | "ENDIF" | "VERIFY" | "RETURN" | "TOALTSTACK" | "FROMALTSTACK" | "2DROP" | "2DUP" | "3DUP" | "2OVER" | "2ROT" | "2SWAP" | "IFDUP" | "DEPTH" | "DROP" | "DUP" | "NIP" | "OVER" | "PICK" | "ROLL" | "ROT" | "SWAP" | "TUCK" | "CAT" | "SUBSTR" | "LEFT" | "RIGHT = 0x81" | "SIZE" | "INVERT" | "AND" | "OR" | "XOR" | "EQUAL" | "EQUALVERIFY" | "RESERVED1" | "RESERVED2" | "1ADD" | "1SUB" | "2MUL" | "2DIV" | "NEGATE" | "ABS" | "NOT" | "0NOTEQUAL" | "ADD" | "SUB" | "MUL" | "DIV" | "MOD" | "LSHIFT " | "RSHIFT" | "BOOLAND" | "BOOLOR" | "NUMEQUAL" | "NUMEQUALVERIFY" | "NUMNOTEQUAL" | "LESSTHAN" | "GREATERTHAN" | "LESSTHANOREQUAL" | "GREATERTHANOREQUAL" | "MIN" | "MAX" | "WITHIN" | "RIPEMD160" | "SHA1" | "SHA256" | "HASH160" | "HASH256" | "CODESEPARATOR" | "CHECKSIG" | "CHECKSIGVERIFY" | "CHECKMULTISIG" | "CHECKMULTISIGVERIFY" | "NOP1" | "NOP2" | "NOP3" | "NOP4" | "NOP5" | "NOP6" | "NOP7" | "NOP8" | "NOP9" | "NOP10"
 <operation> ::= "OP_" <operation_name>
 <lines> ::= "\n" | "\n" <lines>
 <seperator> ::= <lines> | " "
 <operation_or_hex> ::= <operation> | <hex>
 <seperated_operation_or_hex> ::= <operation_or_hex> <seperator>
 <script_body> ::= <seperated_operation_or_hex> | <seperated_operation_or_hex> <script_body> | ""
 <lines_or_none> ::= <lines> | ""
 <operation_or_hex_or_none> ::= <operation_or_hex> | ""
 <script> ::= <lines_or_none> <script_body> <operation_or_hex_or_none>
 /endcode
 Be warned that the scripts do not use two's compliment for hex values. To represent a negative number the far left digit should be 8 or higher. Adding 8 to the far left digit give the number a negative sign such that 0x81 = -1. Negative numbers should have an even number of digits. 0x8FF will not give -255, instead 0x80FF is needed.
 @param params The network parameters.
 @param string The string to compile into a Script object.
 @param onErrorReceived The Events structure.
 @returns A new Script object or NULL on failure.
 */
Script * createNewScriptFromString(char * string,void (*onErrorReceived)(Error error,char *,...));

/**
 @brief Initialises a Script object from a string.
 @param self The Script object to initialise
 @param params The network parameters.
 @param string The string to compile into a Script object.
 @param onErrorReceived The Events structure.
 @returns true on success, false on failure.
 */
boolean initScriptFromString(Script * self, char * string, void (*onErrorReceived)(Error error,char *,...));
#endif /* SCRIPT_H_ */
