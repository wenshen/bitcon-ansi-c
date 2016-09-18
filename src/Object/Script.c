/*
 * Script.c
 *
 *  Created on: 08/10/2012
 *  Created by: Abdulfatai Popoola
 *  Copyright (c) 2012 Bitcoin Project Team
 */


#include <assert.h>
#include "Script.h"
#include "ByteArray.h"
#include "../Utils/Cryptography/Crypt.h"

/**  Constructor*/

Script * createNewScriptFromReference(ByteArray * program, uint32_t offset, uint32_t len)
{
	assert(program != NULL);
	assert(offset >= 0);
	assert(len >= 0);

	return getByteArraySubsectionReference(program, offset, len);
}

ScriptStack createEmptyScriptStack()
{
	ScriptStack stack;
	stack.elements = NULL;
	stack.length = 0;

	return stack;
}
uint32_t ScriptGetSigOpCount(Script * self, bool inP2SH){
	uint32_t sigOps = 0;
	ScriptOp lastOp = SCRIPT_OP_INVALIDOPCODE;
	uint32_t cursor = 0;
	for (;cursor < self->length;) {
		ScriptOp op = getByteFromByteArray(self, cursor);
		if (op < 76) {
			cursor += op + 1;
		}else if (op < 79){
			cursor++;
			if(self->length - cursor < 1)
				break; /* Needs at least one more byte*/
			if (op == SCRIPT_OP_PUSHDATA1)
				cursor += 1 + getByteFromByteArray(self, cursor);
			else if (op == SCRIPT_OP_PUSHDATA2){
				if (self->length - cursor < 2)
					break; /* Not enough space.*/
				cursor += 2 + readInt16AsLittleEndianFromByteArray(self, cursor);
			}else{
				if (self->length - cursor < 4)
					break; /* Not enough space.*/
				cursor += 4 + readInt32AsLittleEndianFromByteArray(self, cursor);
			}
		}else if (op == SCRIPT_OP_CHECKSIG || op == SCRIPT_OP_CHECKSIGVERIFY){
			sigOps++;
			cursor++;
		}else if (op == SCRIPT_OP_CHECKMULTISIG || op == SCRIPT_OP_CHECKMULTISIGVERIFY){
			if (inP2SH && lastOp >= SCRIPT_OP_1 && lastOp <= SCRIPT_OP_16)
				sigOps += lastOp - SCRIPT_OP_1 + 1;
			else sigOps += 20;
			cursor++;
		}else cursor++;
		lastOp = op;
	}
	return sigOps;
}

int isP2SHScript(Script * self)
{
	assert(self != NULL);

	return (self->length == 23
				&& getByteFromByteArray(self, 0) == SCRIPT_OPCODE_HASH160
				&& getByteFromByteArray(self, 1) == 0x14  /* push-20-bytes-onto-the-stack opcode */
				&& getByteFromByteArray(self, 22) == SCRIPT_OPCODE_EQUAL);
}

ScriptByteVector copyByteVectorFromScriptStack(ScriptStack * stack, uint8_t fromTop)
{
	assert(stack != NULL);
	assert(fromTop >= 0);

	ScriptByteVector oldByteVector = stack->elements[stack->length - fromTop - 1];
	ScriptByteVector newByteVector;

	if (oldByteVector.data == NULL) { /* Zero */
		newByteVector.data = NULL;
		newByteVector.length = 0;
	} else {
		newByteVector.data = malloc(oldByteVector.length);

		if (!newByteVector.data){ /* There was nothing in the old copy */
			newByteVector.data = NULL;
			newByteVector.length = 1; /* Detect not zero. */
			return newByteVector;
		}

		newByteVector.length = oldByteVector.length;
		memmove(newByteVector.data, oldByteVector.data, newByteVector.length); /* Move contents of the old ScriptByteVector */
	}

	return newByteVector;
}

int pushScriptByteVectorOntoScriptStack(ScriptStack * stack, ScriptByteVector byteVector)
{
	assert(stack != NULL);

	stack->length++;
	ScriptByteVector * temp = realloc(stack->elements, sizeof(*stack->elements)*stack->length);
	if (!temp) {
		return FALSE;
	}

	stack->elements = temp;
	stack->elements[stack->length-1] = byteVector;
	return TRUE;
}

int topElementEvaluatesToTrue(ScriptStack * stack)
{
	assert(stack != NULL);

	ScriptByteVector byteVector = stack->elements[stack->length - 1];
	if (byteVector.data == NULL) {
		return FALSE;
	}

	uint16_t i;

	for (i = 0; i < byteVector.length; i++){
		/* Iterate through every digit to ensure that they are all zeros */
		if (byteVector.data[i] != 0) { /* Check if this is the last digit and see if it evaluates to 1 which indicates -0 */
			if (i == byteVector.length - 1 && byteVector.data[i] == 0x80) {
				return FALSE;
			} else {
				return TRUE;
			}
		}
	}

	return FALSE;
}

void removeElementFromStack(ScriptStack * stack)
{
	assert(stack != NULL);

	stack->length--;
	free(stack->elements[stack->length].data);
}

ScriptByteVector popElementFromStack(ScriptStack * stack)
{
	assert(stack != NULL);

	stack->length--;
	ScriptByteVector item = stack->elements[stack->length];

	return item;
}

ScriptByteVector convertInt64ToScriptByteVector(ScriptByteVector byteVector, int64_t int64)
{
	assert(int64 >= 0);

	if (int64 == 0) {
		/* Return NULL byteVector */
		free(byteVector.data);

		byteVector.length = 0;
		byteVector.data = NULL;

		return byteVector;
	}

	uint64_t absValue = (int64 < 0) ? -int64 : int64;

	/* Get length */
	uint8_t i = 7;
	for (;; i--) {
		uint8_t b = absValue >> (8 * i);
		if (b) {
			byteVector.length = i + 1;
			/* If byte over 0x7F add extra byte */
			if (b > 0x7F) {
				byteVector.length++;
			}
			break;
		}
		if (!i) {
			break;
		}
	}

	uint8_t * temp = realloc(byteVector.data, byteVector.length);
	if (!temp) {
		free(byteVector.data);

		byteVector.data = NULL;
		byteVector.length = 1; /* Detect err and not zero. */
		return byteVector;
	}

	byteVector.data = temp;

	/* Add data */
	i = 0; /* re-initialize counter */
	for (; i < byteVector.length; i++){
		if (i == 8) {
			byteVector.data[8] = 0; /* Extra byte overflowing 8 bytes */
		}
		else {
			byteVector.data[i] = absValue >> (8 * i);
		}
	}

	/* Restore negativity if it was there initially */
	if (int64 < 0) {
		byteVector.data[byteVector.length - 1] ^= 0x80;
	}

	return byteVector;
}

int64_t convertScriptByteVectorToInt64(ScriptByteVector byteVector)
{

	if (byteVector.data == NULL) {
		return 0;
	}

	int64_t res = 0; /* Watch for 32-bit overflow */
	uint8_t i = 0;
	for (; i < byteVector.length; i++) {
		res |= byteVector.data[i] << (8 * i);
	}
	if (byteVector.data[byteVector.length - 1] & 0x80) { /* Negative */
		/* Convert sign to int64_t */
		res ^= 0x80 << (8 * (byteVector.length - 1)); /* Removed sign bit. */
		res = -res;
	}

	return res;
}

void removeSignatureFromStackScript(uint8_t * subScript, uint32_t * subScriptLen, ScriptByteVector signature)
{
	assert(subScript >= 0);
	assert(subScriptLen != NULL);

	if (signature.data == NULL) {
		return; /* Signature is zero */
	}

	uint8_t * ptr = subScript;
	uint8_t * end = subScript + *subScriptLen;
	for (; ptr < end;) {
		if (*ptr && *ptr < 79) { /* Push */
		    /* Move to data for push and record movement to next operation. No checking of push data bounds since it should have already been done. */
			uint32_t move;
			if (*ptr < 76) {
				move = *ptr;
				ptr++;
			} else if (*ptr == SCRIPT_OPCODE_PUSHDATA1) {
				move = ptr[1];
				ptr += 2;
			} else if (*ptr == SCRIPT_OPCODE_PUSHDATA2) {
				move = ptr[1];
				move += ptr[2] << 8;
				ptr += 3;
			} else { /* PUSHDATA4 */
				move = ptr[1];
				move += ptr[2] << 8;
				move += ptr[3] << 16;
				move += ptr[4] << 24;
				ptr += 5;
			}

			/* Check bounds */
			if (ptr + signature.length >= end) {
				break;
			}

			/* Check signature */
			if (memcmp(ptr, signature.data, signature.length)) {
				/* Remove signature */
				memmove(ptr, ptr + signature.length, end - (ptr + signature.length));
				end -= signature.length;
				*subScriptLen -= signature.length; /* Length adjustment. */
			}
			/* Move to next operation */
			ptr += move;
		} else {
			ptr++;
		}
	}
}

Script * createNewScriptStackFromData(uint8_t * data, uint32_t size, void (*onErrorReceived)(Error error,char *,...))
{
	assert(data >= 0);
	assert(size >= 0);
	assert(onErrorReceived!= NULL);

	return createNewByteArrayFromData(data, size, onErrorReceived);
}

Script * createNewScriptStackUsingCopyOfData(uint8_t * data, uint32_t size, void (*onErrorReceived)(Error error,char *,...))
{
	assert(data >= 0);
	assert(size >= 0);
	assert(onErrorReceived!= NULL);

	return createNewByteArrayUsingDataCopy(data, size, onErrorReceived);
}

ScriptExecutionStatus executeScript(Script * script, ScriptStack * stack, TransactionHashStatus (*getTransactionInputHashForVerification)(void *, ByteArray *, uint32_t, SignatureType, uint8_t *), void * transaction, uint32_t inputIndex, int p2sh)
{
	assert(script != NULL);
	assert(stack != NULL);
	assert(inputIndex >= 0);
	assert(p2sh == TRUE || p2sh == FALSE); /*Binary values check */

	/* ??? Adding syntax parsing to the beginning of the interpreter is maybe a good idea. */

	ScriptStack altStack = createEmptyScriptStack();
	uint16_t skipIfElseBlock = 0xffff; /* Skips all instructions on or over this if/else level. */
	uint16_t ifElseSize = 0; /* Amount of if/else block levels */
	uint32_t beginSubScript = 0;
	uint32_t cursor = 0;

	if (script->length > 10000) {
		return SCRIPT_OPCODE_INVALID; /* Script is an illegal size. */
	}

	/* Determine if P2SH https://en.bitcoin.it/wiki/BIP_0016 */
	ScriptByteVector p2shScript;
	int isP2SH;

	if (p2sh && isP2SHScript(script)) {
		p2shScript = copyByteVectorFromScriptStack(stack, 0);
		if (!p2shScript.data && p2shScript.length) {
			return SCRIPT_ERR;
		}
		isP2SH = TRUE;
	} else {
		isP2SH = FALSE;
	}

	uint8_t opCount = 0;
	for (opCount; script->length - cursor > 0;) {
		ScriptOpcode byte = getByteFromByteArray(script, cursor);
		if (byte > SCRIPT_OPCODE_16 && ++opCount > 201) {
			return BAD_SCRIPT; /* Too many op codes */
		}

		if (byte == SCRIPT_OPCODE_VERIF
				|| byte == SCRIPT_OPCODE_VERNOTIF
				|| byte == SCRIPT_OPCODE_CAT
				|| byte == SCRIPT_OPCODE_SUBSTR
				|| byte == SCRIPT_OPCODE_LEFT
				|| byte == SCRIPT_OPCODE_RIGHT
				|| byte == SCRIPT_OPCODE_INVERT
				|| byte == SCRIPT_OPCODE_AND
				|| byte == SCRIPT_OPCODE_OR
				|| byte == SCRIPT_OPCODE_XOR
				|| byte == SCRIPT_OPCODE_2MUL
				|| byte == SCRIPT_OPCODE_2DIV
				|| byte == SCRIPT_OPCODE_CAT
				|| byte == SCRIPT_OPCODE_MUL
				|| byte == SCRIPT_OPCODE_DIV
				|| byte == SCRIPT_OPCODE_MOD
				|| byte == SCRIPT_OPCODE_LSHIFT
				|| byte == SCRIPT_OPCODE_RSHIFT) {
			return BAD_SCRIPT; /* Invalid op codes independent of execution */
		}
		cursor++;

		/* Control management for branching */
		if (ifElseSize >= skipIfElseBlock) { /* Skip when "ifElseSize" level is over or at "skipIfElseBlock" */
			if (byte == SCRIPT_OPCODE_ELSE && ifElseSize == skipIfElseBlock) {
				skipIfElseBlock = 0xffff; /* No more skipping */
			} else if (byte == SCRIPT_OPCODE_ENDIF) {
				if (ifElseSize == skipIfElseBlock) {
					skipIfElseBlock = 0xffff; /* No more skipping */
				}
				ifElseSize--;
			} else if (byte == SCRIPT_OPCODE_IF) {
				ifElseSize++;
			}
		} else { /* Execute script without skipping */
			if (!byte) {
				/* Push 0 onto stack. This is NULL due to counter intuitive rubbish in the C++ client. */
				ScriptByteVector byteVector = { NULL, 0 };
				if (!pushScriptByteVectorOntoScriptStack(stack, byteVector))
				{
					return SCRIPT_ERR;
				}
			} else if (byte < 76) {
				/* Check size */
				if ((script->length - cursor) < byte) {
					return BAD_SCRIPT; /* Not enough space.*/
				}

				/* Push data the size of the value of the byte */
				ScriptByteVector byteVector;
				byteVector.data = malloc(byte);
				if (!byteVector.data) {
					script->onErrorReceived(
							ERROR_OUT_OF_MEMORY,
							"Run out of memory during a script push operation. Attempted to push %i bytes\n",
							byte);
					return SCRIPT_ERR;
				}
				byteVector.length = byte;
				memmove(byteVector.data, getByteArrayData(script) + cursor, byte); /* Copy data starting from the current cursor */
				if (!pushScriptByteVectorOntoScriptStack(stack, byteVector)) {
					return SCRIPT_ERR;
				}
				cursor += byte;
			} else if (byte < 79) {
				/* Push data with the length of bytes represented by the next bytes. The number of bytes to push is in little endian
				 unlike arithmetic operations which is weird.*/

				uint32_t amount;
				if (script->length - cursor < 1) {
					return BAD_SCRIPT; /* Needs at least one more byte */
				}
				if (byte == SCRIPT_OPCODE_PUSHDATA1) {
					amount = getByteFromByteArray(script, cursor);
					cursor++;
				} else if (byte == SCRIPT_OPCODE_PUSHDATA2) {
					if (script->length - cursor < 2) {
						return BAD_SCRIPT; /* Not enough space to read data. */
					}
					amount = readInt16AsLittleEndianFromByteArray(script, cursor);
					cursor += 2;
				} else {
					if (script->length - cursor < 4) {
						return BAD_SCRIPT; /* Not enough space to read data; expecting 4 bytes */
					}
					amount = readInt32AsLittleEndianFromByteArray(script, cursor);
					cursor += 4;
				}

				/* Check limitation */
				if (amount > 520) {
					return BAD_SCRIPT; /* pushing too much data.*/
				}

				/* Check size */
				if (script->length - cursor < amount) {
					return BAD_SCRIPT;  /* Not enough space for data. */
				}

				ScriptByteVector byteVector;
				if (amount) {
					byteVector.data = malloc(amount);
					if (!byteVector.data) {
						script->onErrorReceived(
								ERROR_OUT_OF_MEMORY,
								"Run out of memory during a script push operation. Attempted to push %i bytes\n",
								amount);
						return SCRIPT_ERR;
					}
					memmove(byteVector.data, getByteArrayData(script) + cursor, amount);
				} else {
					byteVector.data = NULL;
				}
				byteVector.length = amount;
				if (!pushScriptByteVectorOntoScriptStack(stack, byteVector)) {
					return SCRIPT_ERR;
				}
				cursor += amount;
			} else if (byte == SCRIPT_OPCODE_1NEGATE) {
				/* Push -1 onto the stack */
				ScriptByteVector byteVector;
				byteVector.data = malloc(1);
				if (!byteVector.data) {
					script->onErrorReceived(
							ERROR_OUT_OF_MEMORY,
							"Run out of memory during OP_1NEGATE\n");
					return SCRIPT_ERR;
				}
				byteVector.length = 1;
				byteVector.data[0] = 0x81; /* 10000001 the most significant bit indicates a negative number, so this is equivalent to -00000001 */
				if (!pushScriptByteVectorOntoScriptStack(stack, byteVector))
					return SCRIPT_ERR;
			} else if (byte == SCRIPT_OPCODE_RESERVED) {
				return BAD_SCRIPT;
			} else if (byte < 97) {
				/* Push a number onto the stack */
				ScriptByteVector byteVector;
				byteVector.data = malloc(1);
				if (!byteVector.data) {
					script->onErrorReceived(
							ERROR_OUT_OF_MEMORY,
							"Run out of memory during OP_%i\n",
							byte - SCRIPT_OPCODE_1 + 1);
					return SCRIPT_ERR;
				}
				byteVector.length = 1;
				byteVector.data[0] = byte - SCRIPT_OPCODE_1 + 1;
				if (!pushScriptByteVectorOntoScriptStack(stack, byteVector)){
					return SCRIPT_ERR;
				}
			} else if (byte == SCRIPT_OPCODE_NOP) {
				/* Do nothing */
			} else if (byte == SCRIPT_OPCODE_IF || byte == SCRIPT_OPCODE_NOTIF) {
				/* If condition evaluates to TRUE, continue; else jump to branch OP_ELSE or OP_ENDIF */
				ifElseSize++;
				if (!stack->length) { /* Check if stack is empty */
					return BAD_SCRIPT;
				}
				int res = topElementEvaluatesToTrue(stack);
				if ((res && byte == SCRIPT_OPCODE_IF) || (!res && byte == SCRIPT_OPCODE_NOTIF)) {
					skipIfElseBlock = 0xffff;
				} else {
					skipIfElseBlock = ifElseSize; /* jump to matching OP_ELSE or OP_ENDIF branch */
					removeElementFromStack(stack); /* Remove top stack item */
				}
			} else if (byte == SCRIPT_OPCODE_ELSE) {
				if (!ifElseSize) {
					return BAD_SCRIPT; /* OP_ELSE should not be in this stack location */
				}
				skipIfElseBlock = ifElseSize; /* Begin skipping */
			} else if (byte == SCRIPT_OPCODE_ENDIF) {
				if (! ifElseSize) {
					return BAD_SCRIPT; /* OP_ELSE should not be in this stack location */
				}
				ifElseSize--; /* Go to Lower level */
			} else if (byte == SCRIPT_OPCODE_VERIFY) {
				if (stack->length <= 0) { /*Check if stack is empty */
					return BAD_SCRIPT;
				}
				if (topElementEvaluatesToTrue(stack)) { /* Remove top stack item if it evaluates to TRUE */
					removeElementFromStack(stack);
				} else {
					return BAD_SCRIPT; /* Check failed */
				}
			} else if (byte == SCRIPT_OPCODE_RETURN) {
				return BAD_SCRIPT; /* Check failed, return */
			} else if (byte == SCRIPT_OPCODE_TOALTSTACK) {
				if (stack->length <= 0) { /*Check if stack is empty */
					return BAD_SCRIPT;
				}
				if (!pushScriptByteVectorOntoScriptStack(&altStack, popElementFromStack(stack))){
					return SCRIPT_ERR;
				}
			} else if (byte == SCRIPT_OPCODE_FROMALTSTACK) {
				if (!altStack.length) {
					return BAD_SCRIPT; /* Alternative stack should not be empty */
				}
				if (!pushScriptByteVectorOntoScriptStack(stack, popElementFromStack(&altStack))) {
					return SCRIPT_ERR;
				}
			} else if (byte == SCRIPT_OPCODE_IFDUP) {
				if (stack->length <= 0) { /*Check if stack is empty */
					return BAD_SCRIPT;
				}
				if (topElementEvaluatesToTrue(stack)) {/* Duplicated top stack item */
					ScriptByteVector byteVector = copyByteVectorFromScriptStack(stack, 0);
					if (!byteVector.data && byteVector.length) {
						return SCRIPT_ERR;
					}
					if (!pushScriptByteVectorOntoScriptStack(stack, byteVector)) {
						return SCRIPT_ERR;
					}
				}
			} else if (byte == SCRIPT_OPCODE_DEPTH) {
				ScriptByteVector tempByteVector = {NULL, 0};
				ScriptByteVector temp = convertInt64ToScriptByteVector(tempByteVector, stack->length);
				if (!temp.data && temp.length) {
					return SCRIPT_ERR;
				}
				if (!pushScriptByteVectorOntoScriptStack(stack, temp)) {
					return SCRIPT_ERR;
				}
			} else if (byte == SCRIPT_OPCODE_DROP) {
				if (!stack->length) {
					return BAD_SCRIPT; /* ScriptStack empty */
				}
				removeElementFromStack(stack);
			} else if (byte == SCRIPT_OPCODE_DUP) {
				if (!stack->length) {
					return BAD_SCRIPT; /* ScriptStack empty */
				}

				/* Carry out duplication of top stack element */
				ScriptByteVector byteVector = copyByteVectorFromScriptStack(stack, 0);
				if (!byteVector.data && byteVector.length) {
						return SCRIPT_ERR;
				}
				if (!pushScriptByteVectorOntoScriptStack(stack, byteVector)) {
						return SCRIPT_ERR;
				}
			} else if (byte == SCRIPT_OPCODE_NIP) {
				if (stack->length < 2) {
					return BAD_SCRIPT; /* Stack length should be >= 2 */
				}

				/* Remove the second element from the top of the scriptstack; */
				stack->length--;
				free(stack->elements[stack->length - 1].data); /*removed 2nd element from top */
				stack->elements[stack->length - 1] = stack->elements[stack->length]; /* Move top element to this position */

				ScriptByteVector * temp = realloc(stack->elements, sizeof(*stack->elements) * stack->length);
				if (!temp) {
					return SCRIPT_ERR;
				}
				stack->elements = temp;
			} else if (byte == SCRIPT_OPCODE_OVER) {
				if (stack->length < 2) {
					return BAD_SCRIPT; /* Stack length should be >= 2 */
				}

				ScriptByteVector byteVector = copyByteVectorFromScriptStack(stack, 1); /* This is the second element from the top of the scriptstack */
				if (!byteVector.data && byteVector.length) {
					return SCRIPT_ERR;
				}

				if (!pushScriptByteVectorOntoScriptStack(stack, byteVector)) { /* Push second element from top on top of the stack */
					return SCRIPT_ERR;
				}
			} else if (byte == SCRIPT_OPCODE_PICK || byte == SCRIPT_OPCODE_ROLL) {
				if (stack->length < 2) {
					return BAD_SCRIPT; /* Stack length should be >= 2 */
				}

				ScriptByteVector byteVector = popElementFromStack(stack);
				if (byteVector.length > 4) {
					return BAD_SCRIPT; /* Integer lengths in the BIP 0016 should not be greater than 32 bits */
				}

				int64_t int64 = convertScriptByteVectorToInt64(byteVector);
				if (int64 < 0) {
					return BAD_SCRIPT; /* number must be > 0 */
				}
				if (int64 >= stack->length) {
					return BAD_SCRIPT; /* number must be smaller than the stack length */
				}

				if (byte == SCRIPT_OPCODE_PICK) {
					/* Copy element */
					ScriptByteVector byteVector = copyByteVectorFromScriptStack(stack,int64);
					if (!byteVector.data && byteVector.length) {
						return SCRIPT_ERR;
					}
					if (!pushScriptByteVectorOntoScriptStack(stack, byteVector)){
						return SCRIPT_ERR;
					}
				} else if (byte == SCRIPT_OPCODE_ROLL)  { /* Move element. */
					ScriptByteVector temp = stack->elements[stack->length - int64 - 1]; /* Get the item to "roll" */
					uint32_t i = 0;
					for (; int64 < i; i++) { /* Move other elements down*/
						stack->elements[stack->length - int64 + i - 1] = stack->elements[stack->length - int64 + i];
					}
					stack->elements[stack->length - 1] = temp;
				}
			} else if (byte == SCRIPT_OPCODE_ROT) {
				if (stack->length < 3) {
					return BAD_SCRIPT; /*stack elements must be at least 3 in this scenario */
					}

				/* Rotate top three elements to the left. */
				ScriptByteVector temp = stack->elements[stack->length - 3];
				stack->elements[stack->length - 3] = stack->elements[stack->length - 2];
				stack->elements[stack->length - 2] = stack->elements[stack->length - 1];
				stack->elements[stack->length - 1] = temp;
			} else if (byte == SCRIPT_OPCODE_SWAP) {
				if (stack->length < 2) {
					return BAD_SCRIPT; /* Stack needs 2 or more elements. */
				}
				ScriptByteVector temp = stack->elements[stack->length - 2];
				stack->elements[stack->length - 2] = stack->elements[stack->length - 1];
				stack->elements[stack->length - 1] = temp;
			} else if (byte == SCRIPT_OPCODE_TUCK) {
				if (stack->length < 2) {
					return BAD_SCRIPT; /* Stack needs 2 or more elements. */
				}

				ScriptByteVector byteVector = copyByteVectorFromScriptStack(stack, 0);

				if (!byteVector.data && byteVector.length) {
					return SCRIPT_ERR;
				}

				/* New copy three down.*/
				stack->length++;
				ScriptByteVector * temp = realloc(stack->elements, sizeof(*stack->elements) * stack->length);
				if (!temp) {
					return SCRIPT_ERR;
				}

				stack->elements = temp;
				stack->elements[stack->length - 1] = stack->elements[stack->length - 2];
				stack->elements[stack->length - 2] = stack->elements[stack->length - 3];
				stack->elements[stack->length - 3] = byteVector;
			} else if (byte == SCRIPT_OPCODE_2DROP) {
				if (stack->length < 2) {
					return BAD_SCRIPT; /* Stack must have at least 2 elements */
				}
				removeElementFromStack(stack);
				removeElementFromStack(stack);
			} else if (byte == SCRIPT_OPCODE_2DUP || byte == SCRIPT_OPCODE_3DUP) {
				if (stack->length < byte - SCRIPT_OPCODE_2DUP + 2) {
					return BAD_SCRIPT; /* Stack needs more elements. */
				}
				uint8_t i = 0;
				for (; i < byte - SCRIPT_OPCODE_2DUP + 2; i++) {
					ScriptByteVector byteVector = copyByteVectorFromScriptStack(stack, byte - SCRIPT_OPCODE_2DUP + 1);
					if (!byteVector.data && byteVector.length) {
						return SCRIPT_ERR;
					}
					if (!pushScriptByteVectorOntoScriptStack(stack, byteVector)) {
						return SCRIPT_ERR;
					}
				}
			} else if (byte == SCRIPT_OPCODE_2OVER) {
				if (stack->length < 4) {
					return BAD_SCRIPT; /* Stack needs 4 or more elements. */
				}

				ScriptByteVector byteVector = copyByteVectorFromScriptStack(stack, 3);

				/*Check that byteVector is valid and then push it on top of the stack */
				if (!byteVector.data && byteVector.length && !pushScriptByteVectorOntoScriptStack(stack, byteVector)) {
					return SCRIPT_ERR;
				}

				byteVector = copyByteVectorFromScriptStack(stack, 3);

				if (!byteVector.data && byteVector.length && !pushScriptByteVectorOntoScriptStack(stack, byteVector)) {
					return SCRIPT_ERR;
				}
			} else if (byte == SCRIPT_OPCODE_2ROT) {
				if (stack->length < 6) {
					return BAD_SCRIPT; /* Stack needs at least 3 elements. */
				}

				/* Rotate top three pairs of elements to the left. */
				ScriptByteVector temp = stack->elements[stack->length - 6];
				ScriptByteVector temp2 = stack->elements[stack->length - 5];
				stack->elements[stack->length - 6] = stack->elements[stack->length - 4];
				stack->elements[stack->length - 5] = stack->elements[stack->length - 3];
				stack->elements[stack->length - 4] = stack->elements[stack->length - 2];

				stack->elements[stack->length - 3] = stack->elements[stack->length - 1];
				stack->elements[stack->length - 2] = temp;
				stack->elements[stack->length - 1] = temp2;
			} else if (byte == SCRIPT_OPCODE_2SWAP) {
				if (stack->length < 4) {
					return BAD_SCRIPT; /* Stack needs 4 or more elements. */
				}

				ScriptByteVector temp = stack->elements[stack->length - 4];
				ScriptByteVector temp2 = stack->elements[stack->length - 3];
				stack->elements[stack->length - 4] = stack->elements[stack->length - 2];
				stack->elements[stack->length - 3] = stack->elements[stack->length - 1];
				stack->elements[stack->length - 2] = temp;
				stack->elements[stack->length - 1] = temp2;
			} else if (byte == SCRIPT_OPCODE_SIZE) {
				if (!stack->length) {
					return BAD_SCRIPT; /* Stack empty */
				}

				ScriptByteVector byteVector = convertInt64ToScriptByteVector((ScriptByteVector) {NULL,0}, stack->elements[stack->length-1].length);
				if (!byteVector.data && byteVector.length && !pushScriptByteVectorOntoScriptStack(stack, byteVector)) {
					return SCRIPT_ERR;
				}

			} else if (byte == SCRIPT_OPCODE_EQUAL
					|| byte == SCRIPT_OPCODE_EQUALVERIFY) {
				if (stack->length < 2) {
					return BAD_SCRIPT; /* Stack needs 2 or more elements. */
				}

				ScriptByteVector topmostElement = popElementFromStack(stack);
				ScriptByteVector secondTopmostElement = popElementFromStack(stack);
				int ok = TRUE;
				if (topmostElement.length != secondTopmostElement.length) {
					ok = FALSE;
				} else {
					uint16_t i = 0;
					for (; i < topmostElement.length; i++)
						if (topmostElement.data[i] != secondTopmostElement.data[i]) {
							ok = FALSE;
							break;
						}
				}
				if (byte == SCRIPT_OPCODE_EQUALVERIFY && !ok) {
						return BAD_SCRIPT; /* Not equal */
				} else {
					/* Push result onto stack */
					ScriptByteVector byteVector;
					if (ok) {
						byteVector.data = malloc(1);
						if (!byteVector.data) {
							script->onErrorReceived(
									ERROR_OUT_OF_MEMORY,
									"Run out of memory during OP_EQUAL\n");
							return SCRIPT_ERR;
						}
						byteVector.length = 1;
						byteVector.data[0] = 1;
					} else {
						byteVector.data = NULL;
						byteVector.length = 0;
					}
					if (!pushScriptByteVectorOntoScriptStack(stack, byteVector)) {
						return SCRIPT_ERR;
					}
				}
			} else if (byte == SCRIPT_OPCODE_1ADD || byte == SCRIPT_OPCODE_1SUB) {
				if (!stack->length) {
					return BAD_SCRIPT; /* Stack empty */
				}

				ScriptByteVector byteVector = stack->elements[stack->length - 1];
				if (byteVector.length > 4) {
					return BAD_SCRIPT; /* Protocol does not except integers more than 32 bits. */
				}

				/* Convert to 64 bit integer. */
				int64_t int64 = convertScriptByteVectorToInt64(byteVector);
				if (byte == SCRIPT_OPCODE_1ADD){
					int64++;
				}
				else {
					int64--;
				}

				/* Convert back to bitcoin format. Re-assign item as length may have changed. */
				stack->elements[stack->length - 1] = convertInt64ToScriptByteVector(byteVector, int64);
				if (!stack->elements[stack->length - 1].data && stack->elements[stack->length - 1].length) {
					/* Detected error. */
					return SCRIPT_ERR;
				}
			} else if (byte == SCRIPT_OPCODE_NEGATE) {
				if (!stack->length) {
					return BAD_SCRIPT; /* Stack empty */
				}

				ScriptByteVector byteVector = stack->elements[stack->length - 1];
				if (byteVector.length > 4) {
					return BAD_SCRIPT; /* Protocol does not except integers more than 32 bits. */
				}

				if (byteVector.data == NULL) { /* Zero */
					/* Zero becomes 0x80; bug in C++ client */
					byteVector.data = malloc(1);
					if (!byteVector.data) {
						script->onErrorReceived(ERROR_OUT_OF_MEMORY,
								"Run out of memory during OP_NEGATE\n");
						return SCRIPT_ERR;
					}
					byteVector.length = 1;
					byteVector.data[0] = 0x80; /* -0 */
				} else if (byteVector.data[0] != 0x80) {
					byteVector.data[byteVector.length - 1] ^= 0x80; /* Toggle most significant bit. */
				} else {
					/* Negative zero becomes NULL. Positive zero is NULL to support weirdness with the C++ client. Arghh. ??? Needs checking over for inevitable inconsistencies with C++. */
					free(byteVector.data);
					byteVector.data = NULL;
					byteVector.length = 0;
				}
			} else if (byte == SCRIPT_OPCODE_ABS) {
				if (!stack->length) {
					return BAD_SCRIPT; /* Stack empty */
				}
				ScriptByteVector byteVector = stack->elements[stack->length - 1];
				if (byteVector.length > 4) {
					return BAD_SCRIPT; /* Protocol does not except integers more than 32 bits. */
				}
				if (byteVector.data != NULL) { /* If not zero */
					byteVector.data[byteVector.length - 1] &= 0x7F; /* Unset most significant bit. */
				}
			} else if (byte == SCRIPT_OPCODE_NOT|| byte == SCRIPT_OPCODE_0NOTEQUAL) {
				if (!stack->length) {
					return BAD_SCRIPT; /* Stack empty */
				}

				ScriptByteVector byteVector = stack->elements[stack->length - 1];
				if (byteVector.length > 4) {
					return BAD_SCRIPT; /* Protocol does not except integers more than 32 bits. */
				}

				int result = topElementEvaluatesToTrue(stack);
				if ((!result && byte == SCRIPT_OPCODE_NOT) || (result && byte == SCRIPT_OPCODE_0NOTEQUAL)) {
					byteVector.length = 1;
					uint8_t * temp = realloc(byteVector.data, 1);
					if (!temp) {
						return SCRIPT_ERR;
					}
					byteVector.data = temp;
					byteVector.data[0] = 1;
				} else {
					/* Should be zero as NULL. Remember the C++ represents zero as an empty vector. Here NULL is used. This can be slightly annoying.*/
					free(byteVector.data);
					byteVector.data = NULL;
					byteVector.length = 0;
				}
				stack->elements[stack->length - 1] = byteVector;
			} else if (byte == SCRIPT_OPCODE_ADD
					|| byte == SCRIPT_OPCODE_SUB
					|| byte == SCRIPT_OPCODE_NUMEQUAL
					|| byte == SCRIPT_OPCODE_NUMNOTEQUAL
					|| byte == SCRIPT_OPCODE_NUMEQUALVERIFY
					|| byte == SCRIPT_OPCODE_LESSTHAN
					|| byte == SCRIPT_OPCODE_LESSTHANOREQUAL
					|| byte == SCRIPT_OPCODE_GREATERTHAN
					|| byte == SCRIPT_OPCODE_GREATERTHANOREQUAL
					|| byte == SCRIPT_OPCODE_MIN || byte == SCRIPT_OPCODE_MAX) {
				if (stack->length < 2) {
					return BAD_SCRIPT; /* Stack needs 2 or more elements. */
				}

				/* Take top two items, removing the top one. First one which is two down will be assigned the result. */
				ScriptByteVector secondTopmostElement = stack->elements[stack->length - 2];
				ScriptByteVector topmostElement = popElementFromStack(stack);
				if (secondTopmostElement.length > 4 || topmostElement.length > 4) {
					return BAD_SCRIPT; /* Protocol does not except integers more than 32 bits. */
				}

				int64_t result = convertScriptByteVectorToInt64(secondTopmostElement);
				int64_t second = convertScriptByteVectorToInt64(topmostElement);

				free(topmostElement.data); /* No longer need topmostElement */
				switch (byte) {
				case SCRIPT_OPCODE_ADD:
					result += second;
					break;
				case SCRIPT_OPCODE_SUB:
					result -= second;
					break;
				case SCRIPT_OPCODE_NUMEQUALVERIFY:
				case SCRIPT_OPCODE_NUMEQUAL:
					result = (result == second);
					break;
				case SCRIPT_OPCODE_NUMNOTEQUAL:
					result = (result != second);
					break;
				case SCRIPT_OPCODE_LESSTHAN:
					result = (result < second);
					break;
				case SCRIPT_OPCODE_LESSTHANOREQUAL:
					result = (result <= second);
					break;
				case SCRIPT_OPCODE_GREATERTHAN:
					result = (result > second);
					break;
				case SCRIPT_OPCODE_GREATERTHANOREQUAL:
					result = (result >= second);
					break;
				case SCRIPT_OPCODE_MIN:
					result = (result > second) ? second : result;
					break;
				default:
					result = (result < second) ? second : result;
					break;
				}
				if (byte == SCRIPT_OPCODE_NUMEQUALVERIFY) {
					if (!result) {
						return BAD_SCRIPT;
					}
					removeElementFromStack(stack); /* Remove top item that will not hold the rest as this is OP_NUMEQUALVERIFY */
				} else {
					/* Convert back to bitcoin format. Re-assign item as length may have changed. secondTopmostElement now goes to top of stack. */
					stack->elements[stack->length - 1] = convertInt64ToScriptByteVector(secondTopmostElement, result);
					if (!stack->elements[stack->length - 1].data && stack->elements[stack->length - 1].length) {
						return SCRIPT_ERR;
					}
				}
			} else if (byte == SCRIPT_OPCODE_BOOLAND || byte == SCRIPT_OPCODE_BOOLOR) {
				if (stack->length < 2) {
					return BAD_SCRIPT; /* Stack needs 2 or more elements */
				}

				/* Take top two items, removing the top one. First on which is two down will be assigned the result. */
				ScriptByteVector secondTopmostElement = stack->elements[stack->length - 2];
				int topmostElementTrue = topElementEvaluatesToTrue(stack);

				ScriptByteVector topmostElement = popElementFromStack(stack);
				int secondTopmostElementTrue = topElementEvaluatesToTrue(stack);
				if (secondTopmostElement.length > 4 || topmostElement.length > 4) {
					return BAD_SCRIPT; /* Protocol does not except integers more than 32 bits. */
				}

				free(topmostElement.data); /* Top of stack no longer needed */
				secondTopmostElement.length = 1;
				uint8_t * temp = realloc(secondTopmostElement.data, 1);
				if (!temp) {
					return SCRIPT_ERR;
				}
				secondTopmostElement.data = temp;
				secondTopmostElement.data[0] = (byte == SCRIPT_OPCODE_BOOLAND) ? secondTopmostElementTrue && topmostElementTrue : secondTopmostElementTrue || topmostElementTrue;
				stack->elements[stack->length - 1] = secondTopmostElement;
			} else if (byte == SCRIPT_OPCODE_WITHIN) {
				if (stack->length < 3) {
					return BAD_SCRIPT; /* Stack needs 3 or more elements */
				}

				ScriptByteVector thirdByteVector = stack->elements[stack->length - 3];
				ScriptByteVector top = popElementFromStack(stack);
				ScriptByteVector bottom = popElementFromStack(stack);
				if (thirdByteVector.length > 4 || top.length > 4 || bottom.length > 4) {
					return BAD_SCRIPT; /* Protocol does not except integers more than 32 bits. */
				}

				int64_t res = convertScriptByteVectorToInt64(thirdByteVector);
				int64_t topInt = convertScriptByteVectorToInt64(top);
				int64_t bottomInt = convertScriptByteVectorToInt64(bottom);

				/* Free top and bottom as they are no longer needed */
				free(top.data);
				free(bottom.data);
				thirdByteVector.length = 1;
				uint8_t * temp = realloc(thirdByteVector.data, 1);
				if (!temp) {
					return SCRIPT_ERR;
				}
				thirdByteVector.data = temp;
				thirdByteVector.data[0] = bottomInt <= res && res < topInt;
				stack->elements[stack->length - 1] = thirdByteVector;
			} else if (byte == SCRIPT_OPCODE_RIPEMD160
					|| byte == SCRIPT_OPCODE_SHA1
					|| byte == SCRIPT_OPCODE_HASH160
					|| byte == SCRIPT_OPCODE_SHA256
					|| byte == SCRIPT_OPCODE_HASH256) {
				if (!stack->length) {
					return BAD_SCRIPT; /* Stack cannot be empty */
				}

				ScriptByteVector byteVector = stack->elements[stack->length - 1];
				uint8_t * data;
				uint8_t tempData[32];
				switch (byte) {
				case SCRIPT_OPCODE_RIPEMD160:
					data = malloc(20);
					if (!data) {
						script->onErrorReceived(ERROR_OUT_OF_MEMORY,
								"Run out of memory during OP_RIPEMD160\n");
						return SCRIPT_ERR;
					}
					Ripemd160(byteVector.data, byteVector.length, data);
					break;
				case SCRIPT_OPCODE_SHA1:
					data = malloc(20);
					if (!data) {
						script->onErrorReceived(ERROR_OUT_OF_MEMORY,
								"Run out of memory during OP_SHA1\n");
						return SCRIPT_ERR;
					}
					Sha160(byteVector.data, byteVector.length, data);
					break;
				case SCRIPT_OPCODE_HASH160:
					Sha256(byteVector.data, byteVector.length, tempData);
					data = malloc(20);
					if (!data) {
						script->onErrorReceived(ERROR_OUT_OF_MEMORY,
								"Run out of memory during OP_HASH160\n");
						return SCRIPT_ERR;
					}
					Ripemd160(tempData, 32, data);
					break;
				case SCRIPT_OPCODE_SHA256:
					data = malloc(32);
					if (!data) {
						script->onErrorReceived(ERROR_OUT_OF_MEMORY,
								"Run out of memory during OP_SHA256\n");
						return SCRIPT_ERR;
					}
					Sha256(byteVector.data, byteVector.length, data);
					break;
				default:
					Sha256(byteVector.data, byteVector.length, tempData);
					data = malloc(32);
					if (!data) {
						script->onErrorReceived(ERROR_OUT_OF_MEMORY,
								"Run out of memory during OP_HASH256\n");
						return SCRIPT_ERR;
					}
					Sha256(tempData, 32, data);
					break;
				}
				free(byteVector.data);
				byteVector.data = data;
				byteVector.length = (byte == SCRIPT_OPCODE_SHA256 || byte == SCRIPT_OPCODE_HASH256) ? 32 : 20;
				stack->elements[stack->length - 1] = byteVector;
			} else if (byte == SCRIPT_OPCODE_CODESEPARATOR) {
				beginSubScript = cursor;
			} else if (byte == SCRIPT_OPCODE_CHECKSIG
					|| byte == SCRIPT_OPCODE_CHECKSIGVERIFY
					|| byte == SCRIPT_OPCODE_CHECKMULTISIG
					|| byte == SCRIPT_OPCODE_CHECKMULTISIGVERIFY) {
				/* Get sub script and remove OP_CODESEPARATORs */
				uint32_t subScriptLen = script->length - beginSubScript;
				uint8_t * subScript = malloc(subScriptLen);
				if (!subScript) {
					script->onErrorReceived(ERROR_OUT_OF_MEMORY,
							"Ran out of memory during CHECKSIG operation\n");
					return SCRIPT_ERR;
				}

				uint8_t * sourceScript = getByteArrayData(script);
				uint8_t * subScriptCopyPointer = subScript;
				uint8_t * lastSeparator = sourceScript + beginSubScript;
				uint8_t * end = sourceScript + script->length;
				uint8_t * ptr = lastSeparator;

				/* Remove code separators for subScript. */
				int fail = FALSE; /* Checks for push failures.*/
				for (;;) {
					if (ptr >= end) {
						memmove(subScriptCopyPointer, lastSeparator, end - lastSeparator);
						break;
					} else if (*ptr == SCRIPT_OPCODE_CODESEPARATOR) {
						uint32_t len = (uint32_t) (ptr - lastSeparator); /* Do not include code separator. */
						memmove(subScriptCopyPointer, lastSeparator, len);
						lastSeparator = ptr + 1; /* Point past code seperator for copying next time. */
						subScriptCopyPointer += len;
						subScriptLen--; /* One less element. */
					}

					/* Move to next operation */
					if (*ptr < 78 && *ptr) {
						/* If pushing bytes, skip these bytes. */
						if (*ptr < 78) {
							ptr += *ptr;
						} else {
							uint32_t move;
							if (*ptr == SCRIPT_OPCODE_PUSHDATA1) {
								move = 2;
								if (ptr + 1 >= end) { /*Failure */
									fail = TRUE;
									break;
								}
								move += ptr[1];
							} else if (*ptr == SCRIPT_OPCODE_PUSHDATA2) {
								move = 3;
								if (ptr + 2 >= end) { /*Failure */
									fail = TRUE;
									break;
								}
								move += ptr[1];
								move += ptr[2] << 8;
							} else { /* PUSHDATA4 */
								move = 5;
								if (ptr + 4 >= end) { /*Failure */
									fail = TRUE;
									break;
								}
								move += ptr[1];
								move += ptr[2] << 8;
								move += ptr[3] << 16;
								move += ptr[4] << 24;
							}
							ptr += move;
						}
					} else {
						ptr++; /* Not a push operation, move along one. */
					}
				}
				if (fail) { /* Push failure. */
					free(subScript);
					return BAD_SCRIPT;
				}

				int result;
				uint8_t hash[32];
				if (byte == SCRIPT_OPCODE_CHECKSIG || byte == SCRIPT_OPCODE_CHECKSIGVERIFY) {
					if (stack->length < 2) {
						free(subScript);
						return BAD_SCRIPT; /* Stack needs 2 or more elements */
					}
					ScriptByteVector publicKey = popElementFromStack(stack);
					ScriptByteVector signature = popElementFromStack(stack);
					if (signature.data == NULL || publicKey.data == NULL) {
						/* Is zero. Not valid so return FALSE result */
						result = FALSE;
					} else { /* Can check signature */
						SignatureType signType = signature.data[signature.length - 1];
						removeSignatureFromStackScript(subScript, &subScriptLen, signature); /* Delete any instances of the signature */

						/* Complete verification */
						ByteArray * subScriptByteArray = createNewByteArrayFromData(subScript, subScriptLen, script->onErrorReceived);
						if (!subScriptByteArray) {
							free(publicKey.data);
							free(signature.data);
							return SCRIPT_ERR;
						}

						TransactionHashStatus hashRes = getTransactionInputHashForVerification(transaction, subScriptByteArray, inputIndex, signType, hash);
						if (hashRes == TX_HASH_FAIL) {
							free(publicKey.data);
							free(signature.data);
							destroyObject(subScriptByteArray);
							return SCRIPT_ERR;
						} else if (hashRes == TX_HASH_GOOD) {
							/* Use minus one on the signature length because the hash type */
							result = ECDSAVerify(signature.data,
									signature.length - 1, hash, publicKey.data,
									publicKey.length);
						} else {
							result = FALSE;
						}
						destroyObject(subScriptByteArray);
					}
					free(publicKey.data);
					free(signature.data);
				} else {
					if (stack->length < 5) {
						free(subScript);
						return BAD_SCRIPT; /* Stack needs 5 or more elements. At least for numSig, numKeys, one key and signature and the dummy value due to an issue with the protocol.*/
					}

					int64_t numKeys = convertScriptByteVectorToInt64(stack->elements[stack->length - 1]);
					uint8_t sigPointer = 3 + numKeys; /* To first signature. */
					uint8_t key = 2; /* To first key. */
					if (numKeys < 0 || numKeys > 20) {
						free(subScript);
						return BAD_SCRIPT;
					}
					opCount += numKeys;
					if (opCount > 201) {
						free(subScript);
						return BAD_SCRIPT;
					}
					if (stack->length < numKeys + 4) {
						free(subScript);
						return BAD_SCRIPT; /* Not enough space on stack for keys */
					}

					int64_t numSigs = convertScriptByteVectorToInt64(stack->elements[stack->length - 2 - numKeys]); /* Go back the number of keys to find the number of signatures.*/
					if (numSigs < 0 || numSigs > numKeys) {
						free(subScript);
						return BAD_SCRIPT; /* The number of signatures must be positive and blow or equal to the number of keys. */
					}
					if (stack->length < 3 + numKeys + numSigs) {
						free(subScript);
						return BAD_SCRIPT; /* Not enough space for keys, signatures, numSig, numKeys and the dummy value. */
					}

					/* Remove signatures from subScript */
					uint8_t i = 0;
					for (; i < numSigs; i++) {
						ScriptByteVector sigItem = stack->elements[stack->length - sigPointer - i];
						removeSignatureFromStackScript(subScript, &subScriptLen, sigItem);
					}
					ByteArray * subScriptByteArray = createNewByteArrayFromData( subScript, subScriptLen, script->onErrorReceived);
					if (!subScriptByteArray) {
						return SCRIPT_ERR;
					}
					result = TRUE;
					uint8_t removeItemsNum = 3 + numKeys + numSigs;
					while (result && numSigs > 0) {
						ScriptByteVector * signature = &stack->elements[stack->length - sigPointer]; /* Reference for changed data */
						ScriptByteVector publicKey = stack->elements[stack->length - key];
						if (publicKey.data == NULL) {
							/* signature is zero. Can't be validated so break from here */
							result = FALSE;
							break;
						}
						if (publicKey.data != NULL) { /* If not zero can check signature. */
							/* Get sign type */
							SignatureType signatureType = signature->data[signature->length - 1];
							/* Check signature */
							TransactionHashStatus hashRes = getTransactionInputHashForVerification(transaction, subScriptByteArray, inputIndex,	signatureType, hash);
							if (hashRes == TX_HASH_FAIL) {
								destroyObject(subScriptByteArray);
								return SCRIPT_ERR;
							} else if (hashRes == TX_HASH_GOOD) {
								/* Use minus one on the signature length because the hash type */
								if (ECDSAVerify(signature->data, signature->length - 1, hash, publicKey.data, publicKey.length)) {
									sigPointer++;
									numSigs--;
								}
							}
						}
						key++;
						numKeys--;
						if (numSigs > numKeys) {
							result = FALSE; /* More signatures than keys. Cannot verify all signatures. */
						}
					}
					destroyObject(subScriptByteArray);
					/* Remove the items from the stack including an additional dummy value because of a problem in the bitcoin protocol. */
					i = 0;
					for (; i < removeItemsNum; i++) {
						removeElementFromStack(stack);
					}
				}
				if (byte == SCRIPT_OPCODE_CHECKSIG || byte == SCRIPT_OPCODE_CHECKMULTISIG) {
					ScriptByteVector byteVector;
					if (result) {
						byteVector.data = malloc(1);
						if (!byteVector.data) {
							script->onErrorReceived(ERROR_OUT_OF_MEMORY,
									"Run out of memory during CHECKSIG operation push.\n");
							return SCRIPT_ERR;
						}
						byteVector.length = 1;
						byteVector.data[0] = 1;
					} else {
						byteVector.data = NULL;
						byteVector.length = 0;
					}
					if (!pushScriptByteVectorOntoScriptStack(stack, byteVector))
						return SCRIPT_ERR;
				} else if (!result) {
					return BAD_SCRIPT;
				}
			} else if (byte > 175 && byte < 186) {
				/* NOP */
			} else {
				return BAD_SCRIPT;
			}
			if (stack->length + altStack.length > 1000)
				return BAD_SCRIPT; /* Stack size over the limit */
		}
	}
	if (ifElseSize) {
		return BAD_SCRIPT; /* If/Else Block(s) not terminated. */
	}
	if (!stack->length) {
		return BAD_SCRIPT; /* Stack empty. */
	}
	if (topElementEvaluatesToTrue(stack)) {
		if (isP2SH) {
			Script * p2shScriptObj = createNewScriptStackFromData(p2shScript.data, p2shScript.length, script->onErrorReceived);
			if (!p2shScriptObj) {
				return SCRIPT_ERR;
			}
			removeElementFromStack(stack); /* Remove OP_TRUE */
			int res = executeScript(p2shScriptObj, stack, getTransactionInputHashForVerification, transaction, inputIndex, FALSE);
			destroyObject(p2shScriptObj);
			return res;
		}
		return GOOD_SCRIPT;
	} else {
		return BAD_SCRIPT;
	}
}

Script * createNewScriptFromString(char * string, void (*onErrorReceived)(Error error,char *,...))
{
	Script * self = malloc(sizeof(*self));
	if (!self) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory in createNewScriptFromString\n", sizeof(*self));
		return NULL;
	}
	getObject(self)->destroy = destroyByteArray;
	if (initScriptFromString(self,string,onErrorReceived)){
		return self;
	}

	free(self);
	return NULL;
}

boolean initScriptFromString(Script * self, char * string, void (*onErrorReceived)(Error error,char *,...))
{
	uint8_t * data = NULL;
	uint32_t dataLast = 0;
	boolean space = FALSE;
	boolean line = TRUE;

	char * cursor = string;
	for (; *cursor != '\0';) {
		switch (*cursor) {
			case 'O':
				if (!(space ^ line)){
					free(data);
					return FALSE; /* Must be lines or a space before operation and cannot be both. */
				}
				space = FALSE;
				line = FALSE;
				uint8_t * temp = realloc(data, dataLast + 1);
				if (!temp) {
					free(data);
					return FALSE;
				}
				data = temp;
				if(!strncmp(cursor, "OP_FALSE",8)){
					data[dataLast] = SCRIPT_OPCODE_0;
					cursor += 8;
				}else if(!strncmp(cursor, "OP_1NEGATE",10)){
					data[dataLast] = SCRIPT_OPCODE_1NEGATE;
					cursor += 10;
				}else if(!strncmp(cursor, "OP_VERNOTIF",11)){
					data[dataLast] = SCRIPT_OPCODE_VERNOTIF;
					cursor += 11;
				}else if(!strncmp(cursor, "OP_VERIFY",9)){ /* OP_VERIFY before OP_VERIF and OP_VER*/
					data[dataLast] = SCRIPT_OPCODE_VERIFY;
					cursor += 9;
				}else if(!strncmp(cursor, "OP_VERIF",8)){
					data[dataLast] = SCRIPT_OPCODE_VERIF;
					cursor += 8;
				}else if(!strncmp(cursor, "OP_VER",6)){
					data[dataLast] = SCRIPT_OPCODE_VER;
					cursor += 6;
				}else if(!strncmp(cursor, "OP_IFDUP",8)){ /* OP_IFDUP before OP_IF */
					data[dataLast] = SCRIPT_OPCODE_IFDUP;
					cursor += 8;
				}else if(!strncmp(cursor, "OP_IF",5)){
					data[dataLast] = SCRIPT_OPCODE_IF;
					cursor += 5;
				}else if(!strncmp(cursor, "OP_NOTIF",8)){
					data[dataLast] = SCRIPT_OPCODE_NOTIF;
					cursor += 8;
				}else if(!strncmp(cursor, "OP_ELSE",7)){
					data[dataLast] = SCRIPT_OPCODE_ELSE;
					cursor += 7;
				}else if(!strncmp(cursor, "OP_ENDIF",8)){
					data[dataLast] = SCRIPT_OPCODE_ENDIF;
					cursor += 8;
				}else if(!strncmp(cursor, "OP_RETURN",9)){
					data[dataLast] = SCRIPT_OPCODE_RETURN;
					cursor += 9;
				}else if(!strncmp(cursor, "OP_TOALTSTACK",13)){
					data[dataLast] = SCRIPT_OPCODE_TOALTSTACK;
					cursor += 13;
				}else if(!strncmp(cursor, "OP_FROMALTSTACK",15)){
					data[dataLast] = SCRIPT_OPCODE_FROMALTSTACK;
					cursor += 15;
				}else if(!strncmp(cursor, "OP_2DROP",8)){
					data[dataLast] = SCRIPT_OPCODE_2DROP;
					cursor += 8;
				}else if(!strncmp(cursor, "OP_2DUP",7)){
					data[dataLast] = SCRIPT_OPCODE_2DUP;
					cursor += 7;
				}else if(!strncmp(cursor, "OP_3DUP",7)){
					data[dataLast] = SCRIPT_OPCODE_3DUP;
					cursor += 7;
				}else if(!strncmp(cursor, "OP_2OVER",8)){
					data[dataLast] = SCRIPT_OPCODE_2OVER;
					cursor += 8;
				}else if(!strncmp(cursor, "OP_2ROT",7)){
					data[dataLast] = SCRIPT_OPCODE_2ROT;
					cursor += 7;
				}else if(!strncmp(cursor, "OP_2SWAP",8)){
					data[dataLast] = SCRIPT_OPCODE_2SWAP;
					cursor += 8;
				}else if(!strncmp(cursor, "OP_DEPTH",8)){
					data[dataLast] = SCRIPT_OPCODE_DEPTH;
					cursor += 8;
				}else if(!strncmp(cursor, "OP_DROP",7)){
					data[dataLast] = SCRIPT_OPCODE_DROP;
					cursor += 7;
				}else if(!strncmp(cursor, "OP_DUP",6)){
					data[dataLast] = SCRIPT_OPCODE_DUP;
					cursor += 6;
				}else if(!strncmp(cursor, "OP_NIP",6)){
					data[dataLast] = SCRIPT_OPCODE_NIP;
					cursor += 6;
				}else if(!strncmp(cursor, "OP_OVER",7)){
					data[dataLast] = SCRIPT_OPCODE_OVER;
					cursor += 7;
				}else if(!strncmp(cursor, "OP_PICK",7)){
					data[dataLast] = SCRIPT_OPCODE_PICK;
					cursor += 7;
				}else if(!strncmp(cursor, "OP_ROLL",7)){
					data[dataLast] = SCRIPT_OPCODE_ROLL;
					cursor += 7;
				}else if(!strncmp(cursor, "OP_ROT",6)){
					data[dataLast] = SCRIPT_OPCODE_ROT;
					cursor += 6;
				}else if(!strncmp(cursor, "OP_SWAP",7)){
					data[dataLast] = SCRIPT_OPCODE_SWAP;
					cursor += 7;
				}else if(!strncmp(cursor, "OP_TUCK",7)){
					data[dataLast] = SCRIPT_OPCODE_TUCK;
					cursor += 7;
				}else if(!strncmp(cursor, "OP_CAT",6)){
					data[dataLast] = SCRIPT_OPCODE_CAT;
					cursor += 6;
				}else if(!strncmp(cursor, "OP_SUBSTR",9)){
					data[dataLast] = SCRIPT_OPCODE_SUBSTR;
					cursor += 9;
				}else if(!strncmp(cursor, "OP_LEFT",7)){
					data[dataLast] = SCRIPT_OPCODE_LEFT;
					cursor += 7;
				}else if(!strncmp(cursor, "OP_RIGHT",8)){
					data[dataLast] = SCRIPT_OPCODE_RIGHT;
					cursor += 8;
				}else if(!strncmp(cursor, "OP_SIZE",7)){
					data[dataLast] = SCRIPT_OPCODE_SIZE;
					cursor += 7;
				}else if(!strncmp(cursor, "OP_INVERT",9)){
					data[dataLast] = SCRIPT_OPCODE_INVERT;
					cursor += 9;
				}else if(!strncmp(cursor, "OP_AND",6)){
					data[dataLast] = SCRIPT_OPCODE_AND;
					cursor += 6;
				}else if(!strncmp(cursor, "OP_OR",5)){
					data[dataLast] = SCRIPT_OPCODE_OR;
					cursor += 5;
				}else if(!strncmp(cursor, "OP_XOR",6)){
					data[dataLast] = SCRIPT_OPCODE_XOR;
					cursor += 6;
				}else if(!strncmp(cursor, "OP_EQUALVERIFY",14)){
					data[dataLast] = SCRIPT_OPCODE_EQUALVERIFY;
					cursor += 14;
				}else if(!strncmp(cursor, "OP_EQUAL",8)){
					data[dataLast] = SCRIPT_OPCODE_EQUAL;
					cursor += 8;
				}else if(!strncmp(cursor, "OP_RESERVED1",12)){
					data[dataLast] = SCRIPT_OPCODE_RESERVED1;
					cursor += 12;
				}else if(!strncmp(cursor, "OP_RESERVED2",12)){
					data[dataLast] = SCRIPT_OPCODE_RESERVED2;
					cursor += 12;
				}else if(!strncmp(cursor, "OP_RESERVED",11)){
					data[dataLast] = SCRIPT_OPCODE_RESERVED;
					cursor += 11;
				}else if(!strncmp(cursor, "OP_1ADD",7)){
					data[dataLast] = SCRIPT_OPCODE_1ADD;
					cursor += 7;
				}else if(!strncmp(cursor, "OP_1SUB",7)){
					data[dataLast] = SCRIPT_OPCODE_1SUB;
					cursor += 7;
				}else if(!strncmp(cursor, "OP_2MUL",7)){
					data[dataLast] = SCRIPT_OPCODE_2MUL;
					cursor += 7;
				}else if(!strncmp(cursor, "OP_2DIV",7)){
					data[dataLast] = SCRIPT_OPCODE_2DIV;
					cursor += 7;
				}else if(!strncmp(cursor, "OP_NEGATE",9)){
					data[dataLast] = SCRIPT_OPCODE_NEGATE;
					cursor += 9;
				}else if(!strncmp(cursor, "OP_ABS",6)){
					data[dataLast] = SCRIPT_OPCODE_ABS;
					cursor += 6;
				}else if(!strncmp(cursor, "OP_NOT",6)){
					data[dataLast] = SCRIPT_OPCODE_NOT;
					cursor += 6;
				}else if(!strncmp(cursor, "OP_0NOTEQUAL",12)){
					data[dataLast] = SCRIPT_OPCODE_0NOTEQUAL;
					cursor += 12;
				}else if(!strncmp(cursor, "OP_ADD",6)){
					data[dataLast] = SCRIPT_OPCODE_ADD;
					cursor += 6;
				}else if(!strncmp(cursor, "OP_SUB",6)){
					data[dataLast] = SCRIPT_OPCODE_SUB;
					cursor += 6;
				}else if(!strncmp(cursor, "OP_MUL",6)){
					data[dataLast] = SCRIPT_OPCODE_MUL;
					cursor += 6;
				}else if(!strncmp(cursor, "OP_DIV",6)){
					data[dataLast] = SCRIPT_OPCODE_DIV;
					cursor += 6;
				}else if(!strncmp(cursor, "OP_MOD",6)){
					data[dataLast] = SCRIPT_OPCODE_MOD;
					cursor += 6;
				}else if(!strncmp(cursor, "OP_LSHIFT",9)){
					data[dataLast] = SCRIPT_OPCODE_LSHIFT;
					cursor += 9;
				}else if(!strncmp(cursor, "OP_RSHIFT",9)){
					data[dataLast] = SCRIPT_OPCODE_RSHIFT;
					cursor += 9;
				}else if(!strncmp(cursor, "OP_BOOLAND",10)){
					data[dataLast] = SCRIPT_OPCODE_BOOLAND;
					cursor += 10;
				}else if(!strncmp(cursor, "OP_BOOLOR",9)){
					data[dataLast] = SCRIPT_OPCODE_BOOLOR;
					cursor += 9;
				}else if(!strncmp(cursor, "OP_NUMEQUALVERIFY",17)){
					data[dataLast] = SCRIPT_OPCODE_NUMEQUALVERIFY;
					cursor += 17;
				}else if(!strncmp(cursor, "OP_NUMEQUAL",11)){
					data[dataLast] = SCRIPT_OPCODE_NUMEQUAL;
					cursor += 11;
				}else if(!strncmp(cursor, "OP_NUMNOTEQUAL",14)){
					data[dataLast] = SCRIPT_OPCODE_NUMNOTEQUAL;
					cursor += 14;
				}else if(!strncmp(cursor, "OP_LESSTHANOREQUAL",18)){
					data[dataLast] = SCRIPT_OPCODE_LESSTHANOREQUAL;
					cursor += 18;
				}else if(!strncmp(cursor, "OP_LESSTHAN",11)){
					data[dataLast] = SCRIPT_OPCODE_LESSTHAN;
					cursor += 11;
				}else if(!strncmp(cursor, "OP_GREATERTHANOREQUAL",21)){
					data[dataLast] = SCRIPT_OPCODE_GREATERTHANOREQUAL;
					cursor += 21;
				}else if(!strncmp(cursor, "OP_GREATERTHAN",14)){
					data[dataLast] = SCRIPT_OPCODE_GREATERTHAN;
					cursor += 14;
				}else if(!strncmp(cursor, "OP_MIN",6)){
					data[dataLast] = SCRIPT_OPCODE_MIN;
					cursor += 6;
				}else if(!strncmp(cursor, "OP_MAX",6)){
					data[dataLast] = SCRIPT_OPCODE_MAX;
					cursor += 6;
				}else if(!strncmp(cursor, "OP_WITHIN",9)){
					data[dataLast] = SCRIPT_OPCODE_WITHIN;
					cursor += 9;
				}else if(!strncmp(cursor, "OP_RIPEMD160",12)){
					data[dataLast] = SCRIPT_OPCODE_RIPEMD160;
					cursor += 12;
				}else if(!strncmp(cursor, "OP_SHA1",7)){
					data[dataLast] = SCRIPT_OPCODE_SHA1;
					cursor += 7;
				}else if(!strncmp(cursor, "OP_SHA256",9)){
					data[dataLast] = SCRIPT_OPCODE_SHA256;
					cursor += 9;
				}else if(!strncmp(cursor, "OP_HASH160",10)){
					data[dataLast] = SCRIPT_OPCODE_HASH160;
					cursor += 10;
				}else if(!strncmp(cursor, "OP_HASH256",10)){
					data[dataLast] = SCRIPT_OPCODE_HASH256;
					cursor += 10;
				}else if(!strncmp(cursor, "OP_CODESEPARATOR",16)){
					data[dataLast] = SCRIPT_OPCODE_CODESEPARATOR;
					cursor += 16;
				}else if(!strncmp(cursor, "OP_CHECKSIGVERIFY",17)){
					data[dataLast] = SCRIPT_OPCODE_CHECKSIGVERIFY;
					cursor += 17;
				}else if(!strncmp(cursor, "OP_CHECKSIG",11)){
					data[dataLast] = SCRIPT_OPCODE_CHECKSIG;
					cursor += 11;
				}else if(!strncmp(cursor, "OP_CHECKMULTISIGVERIFY",22)){
					data[dataLast] = SCRIPT_OPCODE_CHECKMULTISIGVERIFY;
					cursor += 22;
				}else if(!strncmp(cursor, "OP_CHECKMULTISIG",16)){
					data[dataLast] = SCRIPT_OPCODE_CHECKMULTISIG;
					cursor += 16;
				}else if(!strncmp(cursor, "OP_NOP10",8)){
					data[dataLast] = SCRIPT_OPCODE_NOP10;
					cursor += 8;
				}else if(!strncmp(cursor, "OP_NOP1",7)){
					data[dataLast] = SCRIPT_OPCODE_NOP1;
					cursor += 7;
				}else if(!strncmp(cursor, "OP_NOP2",7)){
					data[dataLast] = SCRIPT_OPCODE_NOP2;
					cursor += 7;
				}else if(!strncmp(cursor, "OP_NOP3",7)){
					data[dataLast] = SCRIPT_OPCODE_NOP3;
					cursor += 7;
				}else if(!strncmp(cursor, "OP_NOP4",7)){
					data[dataLast] = SCRIPT_OPCODE_NOP4;
					cursor += 7;
				}else if(!strncmp(cursor, "OP_NOP5",7)){
					data[dataLast] = SCRIPT_OPCODE_NOP5;
					cursor += 7;
				}else if(!strncmp(cursor, "OP_NOP6",7)){
					data[dataLast] = SCRIPT_OPCODE_NOP6;
					cursor += 7;
				}else if(!strncmp(cursor, "OP_NOP7",7)){
					data[dataLast] = SCRIPT_OPCODE_NOP7;
					cursor += 7;
				}else if(!strncmp(cursor, "OP_NOP8",7)){
					data[dataLast] = SCRIPT_OPCODE_NOP8;
					cursor += 7;
				}else if(!strncmp(cursor, "OP_NOP9",7)){
					data[dataLast] = SCRIPT_OPCODE_NOP9;
					cursor += 7;
				}else if(!strncmp(cursor, "OP_NOP",6)){
					data[dataLast] = SCRIPT_OPCODE_NOP;
					cursor += 6;
				}else if(!strncmp(cursor, "OP_0",4)){
					data[dataLast] = SCRIPT_OPCODE_0;
					cursor += 4;
				}else if(!strncmp(cursor, "OP_10",5)){
					data[dataLast] = SCRIPT_OPCODE_10;
					cursor += 5;
				}else if(!strncmp(cursor, "OP_11",5)){
					data[dataLast] = SCRIPT_OPCODE_11;
					cursor += 5;
				}else if(!strncmp(cursor, "OP_12",5)){
					data[dataLast] = SCRIPT_OPCODE_12;
					cursor += 5;
				}else if(!strncmp(cursor, "OP_13",5)){
					data[dataLast] = SCRIPT_OPCODE_13;
					cursor += 5;
				}else if(!strncmp(cursor, "OP_14",5)){
					data[dataLast] = SCRIPT_OPCODE_14;
					cursor += 5;
				}else if(!strncmp(cursor, "OP_15",5)){
					data[dataLast] = SCRIPT_OPCODE_15;
					cursor += 5;
				}else if(!strncmp(cursor, "OP_16",5)){
					data[dataLast] = SCRIPT_OPCODE_16;
					cursor += 5;
				}else if(!strncmp(cursor, "OP_1",4)){
					data[dataLast] = SCRIPT_OPCODE_1;
					cursor += 4;
				}else if(!strncmp(cursor, "OP_TRUE",7)){
					data[dataLast] = SCRIPT_OPCODE_1;
					cursor += 7;
				}else if(!strncmp(cursor, "OP_2",4)){
					data[dataLast] = SCRIPT_OPCODE_2;
					cursor += 4;
				}else if(!strncmp(cursor, "OP_3",4)){
					data[dataLast] = SCRIPT_OPCODE_3;
					cursor += 4;
				}else if(!strncmp(cursor, "OP_4",4)){
					data[dataLast] = SCRIPT_OPCODE_4;
					cursor += 4;
				}else if(!strncmp(cursor, "OP_5",4)){
					data[dataLast] = SCRIPT_OPCODE_5;
					cursor += 4;
				}else if(!strncmp(cursor, "OP_6",4)){
					data[dataLast] = SCRIPT_OPCODE_6;
					cursor += 4;
				}else if(!strncmp(cursor, "OP_7",4)){
					data[dataLast] = SCRIPT_OPCODE_7;
					cursor += 4;
				}else if(!strncmp(cursor, "OP_8",4)){
					data[dataLast] = SCRIPT_OPCODE_8;
					cursor += 4;
				}else if(!strncmp(cursor, "OP_9",4)){
					data[dataLast] = SCRIPT_OPCODE_9;
					cursor += 4;
				}else{
					free(data);
					return FALSE;
				}
				dataLast++;
				break;
			case ' ':
				if (space){
					free(data);
					return FALSE; /* Only one space */
				}
				space = TRUE;
				cursor++;
				break;
			case '\n':
				line = TRUE;
				cursor++;
				break;
			case '0':
				if (!(space ^ line)){
					free(data);
					return FALSE; /* Must be lines or a space before hex and cannot be both. */
				}
				space = FALSE;
				line = FALSE;
				if (cursor[1] == 'x') {
					/* Is hex, interpret hex digits. */
					char interpret[3];
					char * spaceFind = strchr(cursor, ' ');
					char * lineFind = strchr(cursor, '\n');
					uint32_t diff;
					if (spaceFind == NULL)
						if (lineFind == NULL)
							diff = (uint32_t)strlen(cursor);
						else
							diff = (uint32_t)(lineFind - cursor);
					else
						if (lineFind == NULL)
							diff = (uint32_t)(spaceFind - cursor);
						else if (lineFind < spaceFind)
							diff = (uint32_t)(lineFind - cursor);
						else
							diff = (uint32_t)(spaceFind - cursor);
					diff -= 2; /* For the "0x" */
					if (diff & 1) {
						/* Is odd */
						cursor[1] = '0';
						cursor--;
						diff++;
					}
					uint32_t num = diff/2;
					/* Create push operations. The number of bytes to push is represented in little endian. */
					if (num < 76) {
						uint8_t * temp = realloc(data, dataLast + 1);
						if (!temp) {
							free(data);
							return FALSE;
						}
						data = temp;
						data[dataLast] = num;
						dataLast++;
					}else if (num < 256){
						uint8_t * temp = realloc(data, dataLast + 2);
						if (!temp) {
							free(data);
							return FALSE;
						}
						data = temp;
						data[dataLast] = SCRIPT_OPCODE_PUSHDATA1;
						dataLast++;
						data[dataLast] = num;
						dataLast++;
					}else if (num < 65536){
						uint8_t * temp = realloc(data, dataLast + 3);
						if (!temp) {
							free(data);
							return FALSE;
						}
						data = temp;
						data[dataLast] = SCRIPT_OPCODE_PUSHDATA2;
						dataLast++;
						data[dataLast] = num;
						dataLast++;
						data[dataLast] = num >> 8;
						dataLast++;
					}else{
						uint8_t * temp = realloc(data, dataLast + 5);
						if (!temp) {
							free(data);
							return FALSE;
						}
						data = temp;
						data[dataLast] = SCRIPT_OPCODE_PUSHDATA4;
						dataLast++;
						data[dataLast] = num;
						dataLast++;
						data[dataLast] = num >> 8;
						dataLast++;
						data[dataLast] = num >> 16;
						dataLast++;
						data[dataLast] = num >> 24;
						dataLast++;
					}
					while (TRUE) {
						cursor += 2;
						interpret[0] = cursor[0];
						if ((interpret[0] >= '0' && interpret[0] <= '9')
							|| (interpret[0] >= 'a' && interpret[0] <= 'f')
							|| (interpret[0] >= 'A' && interpret[0] <= 'F')) {
							interpret[1] = cursor[1];
							interpret[2] = '\0';
							if ((interpret[1] >= '0' && interpret[1] <= '9')
								|| (interpret[1] >= 'a' && interpret[1] <= 'f')
								|| (interpret[1] >= 'A' && interpret[1] <= 'F')) {
								/* Both */
								uint8_t * temp = realloc(data, dataLast + 1);
								if (!temp) {
									free(data);
									return FALSE;
								}
								data = temp;
								data[dataLast] = strtoul(interpret, NULL, 16);
								dataLast++;
							}else{
								free(data);
								return FALSE; /* In pairs */
							}
						}else if(interpret[0] == ' '
								 || interpret[0] == '\n'
								 || interpret[0] == '\0'){
							/* End */
							break;
						}else{
							free(data);
							return FALSE; /* Needs to be space, line or end. */
						}
					}
					break;
				}else{
					free(data);
					return FALSE; /* Not x for hex. */
				}
			default:
				free(data);
				return FALSE;
		}
	}
	/* Got data. Now create script byte array for data */
	if (!initializeNewByteArrayFromData(self, data, dataLast, onErrorReceived)) {
		return FALSE;
	}
	return TRUE;
}
