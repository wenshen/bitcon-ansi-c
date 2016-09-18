/*
 * Transcation.c
 *
 *  Created on: 04/10/2012
 *  Created by: Abdulfatai Popoola
 *  Copyright (c) 2012 Bitcoin Project Team
 */
#include "Transaction.h"
#include "../../Constants.h"
#include <stdint.h>
#include <assert.h>
#include "../../Utils/VariableLengthInteger/VarLenInt.h"
#include "../ByteArray.h"
#include "TransactionInput.h"
#include "Message.h"

/* Constructors */
Transaction * createNewTransaction(uint32_t lockTime, uint32_t version,
		void (*onErrorReceived)(Error error, char *, ...)) {
	assert(lockTime >= 0);
	assert(version >= 0);
	assert(onErrorReceived != NULL);

	Transaction * self = malloc(sizeof(*self)); /*Create memory for Transaction */

	if (!self) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,
				"Cannot allocate %i bytes of memory for NewTransaction in createNewTransaction\n",
				sizeof(*self));
		return NULL;
	}

	getObject(self)->destroy = destroyTransaction; /*assign destructor function to object*/

	if (setUpTransactionData(self, lockTime, version, onErrorReceived)) {
		return self;
	}

	/* Initialization failed; Clean up everything and return NULL*/
	free(self);
	return NULL;
}

Transaction * createNewTransactionFromByteArray(ByteArray * bytes,
		void (*onErrorReceived)(Error error, char *, ...)) {
	Transaction * self = malloc(sizeof(*self)); /*Create memory for Transaction*/

	if (!self) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,
				"Cannot allocate %i bytes of memory for NewTransaction in createNewTransactionFromByteArray\n",
				sizeof(*self));
		return NULL;
	}

	if (setUpTransactionDataUsingByteArray(self, bytes, onErrorReceived)) {
		return self;
	}

	/* Initialization failed; Clean up everything and return NULL*/
	free(self);
	return NULL;
}

int setUpTransactionData(Transaction * self, uint32_t lockTime,
		uint32_t version, void (*onErrorReceived)(Error error, char *, ...)) {
	assert(self != NULL);
	assert(lockTime >= 0);
	assert(version >= 0);
	assert(onErrorReceived != NULL);

	self->lockTime = lockTime;
	self->numOfTransactionInput = 0;
	self->numOfTransactionOutput = 0;
	self->inputs = NULL;
	self->outputs = NULL;
	self->version = version;

	/* Check to see that the base Object class can be retrieved and then update details */
	if (getObject(self) != NULL) {
		/* retrieve the base object and use it to update the underlying object details */
		getObject(self)->referenceCount = 1;

		/* Set other Message fields */
		if (getMessage(self) != NULL) {
			getMessage(self)->bytes = NULL; /*set Message data to the ByteArray data that was passed in */

			getMessage(self)->onErrorReceived = onErrorReceived;
			getMessage(self)->expectResponse = FALSE;
			return TRUE;
		}
	}

	return FALSE;
}

int setUpTransactionDataUsingByteArray(Transaction * self, ByteArray * data,
		void (*onErrorReceived)(Error error, char *, ...)) {
	assert(self != NULL);
	assert(data != NULL);
	assert(onErrorReceived != NULL);

	self->numOfTransactionInput = 0;
	self->numOfTransactionOutput = 0;
	self->inputs = NULL;
	self->outputs = NULL;

	/* Check to see that the base Object class can be retrieved and then update details */
	if (getObject(self) != NULL) {
		/* retrieve the base object and use it to update the underlying object details */
		getObject(self)->referenceCount = 1;

		/* Set other Message fields */
		if (getMessage(self) != NULL) {
			getMessage(self)->bytes = data; /*set Message data to the ByteArray data that was passed in */
			incrementReferenceCount(data);

			getMessage(self)->onErrorReceived = onErrorReceived;
			getMessage(self)->expectResponse = FALSE;
			return TRUE;
		}
	}

	return FALSE;
}

/*Destructors*/
void destroyTransaction(void * transaction) {
	Transaction * self = transaction;

	uint32_t i;
	for (i = 0; i < self->numOfTransactionInput; i++) {
		decrementReferenceCount(self->inputs[i]); /*handles cleaning up of referenced object once num of references get to zero*/
	}
	free(self->inputs);

	for (i = 0; i < self->numOfTransactionOutput; i++) {
		decrementReferenceCount(self->outputs[i]); /*handles cleaning up of referenced object once num of references get to zero*/
	}
	free(self->outputs);

	destroyMessage(getObject(self));
}

/* Getter*/
Transaction * getTransaction(void * self) {
	assert(self != NULL);

	return self;
}

/*Methods*/
int addTransactionInput(Transaction * self, TransactionInput * input) {
	assert(self != NULL);
	assert(input != NULL);

	incrementReferenceCount(input); /*Increase the number of references to this input*/

	self->numOfTransactionInput++;
	size_t size = self->numOfTransactionInput * sizeof(*self->inputs); /*Size of memory to allocate for the transactionInputs*/
	TransactionInput ** tmp = realloc(self->inputs, size);

	if (tmp != NULL) {
		self->inputs = tmp;
		self->inputs[self->numOfTransactionInput - 1] = input; /*append input to the inputs array; i.e. put it at the last position*/
		return TRUE;
	}

	return FALSE;
}

int addTransactionOutput(Transaction * self, TransactionOutput * output) {
	assert(self != NULL);
	assert(output != NULL);

	incrementReferenceCount(output); /*Increase the number of references to this output*/

	self->numOfTransactionOutput++;
	size_t size = self->numOfTransactionOutput * sizeof(*self->outputs); /*Size of memory to allocate for the transactionOutputs*/
	TransactionInput ** tmp = realloc(self->outputs, size);

	if (tmp != NULL) {
		self->outputs = tmp;
		self->outputs[self->numOfTransactionOutput - 1] = output;
		return TRUE;
	}

	return FALSE;
}

void calculateTransactionHash(Transaction * self) {
	assert(self != NULL);

	uint8_t tmpHashStore[32];
	uint8_t * data = getByteArrayData(getMessage(self)->bytes);

	Sha256(data, getMessage(self)->bytes->length, tmpHashStore);
	Sha256(tmpHashStore, 32, self->hash);
}

uint8_t * getTransactionHash(Transaction * self) {
	assert(self != NULL);

	if (self->hash == NULL) {
		calculateTransactionHash(self);
	}

	return self->hash;
}

uint32_t calculateTransactionLength(Transaction * self) {
	assert(self != NULL);

	uint32_t len = TX_VERSION_FIELD_SIZE + TX_LOCKTIME_FIELD_SIZE
			+ getSizeOfVarLenInt(self->numOfTransactionInput)
			+ (self->numOfTransactionInput
					* TX_INPUT_FIELD_SIZE_EXCLUDING_SCRIPT)
			+ getSizeOfVarLenInt(self->numOfTransactionOutput)
			+ (self->numOfTransactionOutput
					* TX_OUTPUT_FIELD_SIZE_EXCLUDING_SCRIPT);

	uint32_t i = 0;
	for (i = 0; i < self->numOfTransactionInput; i++) {
		/*Get the length of the transactionInput script object and append it to the cumulative length*/
		if (!self->inputs[i]->scriptObject) {
			return 0;
		}

		len += getSizeOfVarLenInt(self->inputs[i]->scriptObject->length)
				+ self->inputs[i]->scriptObject->length;
	}

	for (i = 0; i < self->numOfTransactionOutput; i++) {
		/*Get the length of the numOfTransactionOutput script object and append it to the cumulative length*/
		if (!self->outputs[i]->scriptObject) {
			return 0;
		}

		len += getSizeOfVarLenInt(self->outputs[i]->scriptObject->length)
				+ self->outputs[i]->scriptObject->length;
	}

	return len;
}

int isCoinBase(Transaction * self) {
	assert(self != NULL);

	return (self->numOfTransactionInput == 1
			&& self->inputs[0]->prevOutput.index == 0xFFFFFFFF
			&& isNullByteArray(self->inputs[0]->prevOutput.hash));
}

uint32_t serializeTransaction(Transaction * self) {
	assert(self != NULL);

	ByteArray * bytes = getMessage(self)->bytes;

	if (!bytes) {
		getMessage(self)->onErrorReceived(
				ERROR_MESSAGE_SERIALISATION_NULL_BYTES,
				"Attempting to serialise a Transaction with no bytes");
		return 0;
	}

	VarLenInt numTransactionInputs = createVarLenIntFromUInt64(
			self->numOfTransactionInput);
	VarLenInt numTransactionOutputs = createVarLenIntFromUInt64(
			self->numOfTransactionOutput);

	uint32_t cursor = 4 + numTransactionInputs.storageSize;

	if (bytes->length < cursor + 5) {
		getMessage(self)->onErrorReceived(ERROR_MESSAGE_SERIALISATION_BAD_BYTES,
				"Transaction has fewer bytes than expected. Transaction byte-length: %i < required: %i\n",
				bytes->length, cursor + 5);
		return 0;
	}

	/* All error checks done; serialize data into a ByteArray*/
	writeInt32AsLittleEndianIntoByteArray(bytes, 0, self->version); /*Write transaction version information first*/
	encodeVarLenInt(bytes, 4, numTransactionInputs);

	uint32_t i;
	for (i = 0; i < self->numOfTransactionInput; i++) { /*Serialize all transaction inputs stored in this transaction */
		/* Retrieve byte */
		getMessage(self->inputs[i])->bytes = getByteArraySubsectionReference(
				bytes, cursor, bytes->length - cursor);
		if (!getMessage(self->inputs[i])->bytes) { /*Get the bytes of the transactionInputs (All messages have bytes and a transactionInput is a Message)*/
			getMessage(self)->onErrorReceived(ERROR_INIT_FAIL,
					"Cannot create a new subreference to a ByteArray in serializeTransaction for input number %u",
					i);

			return 0;
		}
		uint32_t len = serializeTransactionInput(self->inputs[i]); /*Serialize this transaction input*/
		printf("we r here.\n");

		if (!len) { /*TransactionInptest_case_createNewTransactionFromByteArrayut serialization failed*/
			getMessage(self)->onErrorReceived(
					ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,
					"Transaction cannot be serialised because of an error with the input number %u.",
					i);

			/*Release ByteArray objects to avoid problems overwriting pointer without release, if serialization is tried again.*/
			uint32_t j;
			for (j = 0; j < i + 1; j++) {
				destroyObject(getMessage(self->inputs[j])->bytes);
			}
			return 0;
		}
		getMessage(self->inputs[i])->bytes->length = len;
		cursor += len;
	}

	if (bytes->length < cursor + 5) { /* Check room for output number and lockTime.*/
		getMessage(self)->onErrorReceived(
				ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,
				"Attempting to serialise a Transaction with less bytes than required for output number and the lockTime. %i < %i\n",
				bytes->length, cursor + 5);
		return 0;
	}

	encodeVarLenInt(bytes, cursor, numTransactionOutputs);
	cursor += numTransactionOutputs.storageSize;

	for (i = 0; i < self->numOfTransactionOutput; i++) {
		getMessage(self->outputs[i])->bytes = getByteArraySubsectionReference(
				bytes, cursor, bytes->length - cursor);

		if (!getMessage(self->outputs[i])->bytes) { /*Get the bytes of the transactionOutputs (All Messages have bytes and a transactionOutput is a Message)*/
			getMessage(self)->onErrorReceived(ERROR_INIT_FAIL,
					"Cannot create a new subreference to a ByteArray in serializeTransaction for input number %u",
					i);

			return 0;
		}

		uint32_t len = serializeTransactionOutput(self->outputs[i]);

		if (!len) { /*TransactionOutput serialization failed*/

			getMessage(self)->onErrorReceived(
					ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,
					"Transaction cannot be serialized because of an error with the output number %u.",
					i);

			/*Release ByteArray objects to avoid problems overwriting pointer without release, if serialization is tried again.*/
			uint32_t j;
			for (j = 0; j < i + 1; j++) {
				destroyObject(getMessage(self->inputs[j])->bytes); /*Clean up TransactionInput that have just been set earlier in this method*/
			}
			for (j = 0; j < i + 1; j++) {
				destroyObject(getMessage(self->outputs[j])->bytes); /*Clean up TransactionOutputs that are being set*/
			}
			return 0;
		}

		getMessage(self->outputs[i])->bytes->length = len;
		cursor += len;
	}

	if (bytes->length < cursor + 4) { /* Check room for lockTime.*/
		getMessage(self)->onErrorReceived(
				ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,
				"Attempting to serialise a Transaction with less bytes than required for the lockTime. %i < %i\n",
				bytes->length, cursor + 4);

		/* Release ByteArray objects to avoid problems overwritting pointer without release, if serialiZation is tried again.*/
		uint32_t i;

		for (i = 0; i < self->numOfTransactionInput; i++) {

			destroyObject(getMessage(self->inputs[i])->bytes);
		}
		for (i = 0; i < self->numOfTransactionInput; i++) {
			decrementReferenceCount(getMessage(self->outputs[i])->bytes);
		}
		return 0;
	}
	writeInt32AsLittleEndianIntoByteArray(bytes, cursor, self->lockTime);
	return cursor + 4;
}

uint32_t deserializeTransaction(Transaction * self) {
	assert(self != NULL);

	ByteArray * bytes = getMessage(self)->bytes;

	if (!bytes) {
		getMessage(self)->onErrorReceived(
				ERROR_MESSAGE_DESERIALISATION_NULL_BYTES,
				"Attempting to deserialize a Transaction with no bytes");
		return 0;
	}

	if (bytes->length < 10) {
		getMessage(self)->onErrorReceived(
				ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,
				"Attempting to deserialise a Transaction with less than 10 bytes.");
		return 0;
	}

	self->version = readInt32AsLittleEndianFromByteArray(bytes, 0);

	VarLenInt inputOutputLen = decodeVarLenInt(bytes, 4);

	if (inputOutputLen.value * 41 > bytes->length - 10) {
		getMessage(self)->onErrorReceived(
				ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,
				"Attempting to deserialize a Transaction with a bad varInt for the number of inputs.");
		return 0;
	}

	uint32_t cursor = 4 + inputOutputLen.storageSize;
	self->numOfTransactionInput = (uint32_t) inputOutputLen.value;
	self->inputs = malloc(sizeof(*self->inputs) * self->numOfTransactionInput);

	if (!self->inputs) {
		getMessage(self)->onErrorReceived(ERROR_OUT_OF_MEMORY,
				"Cannot allocate %i bytes of memory in deserializeTransaction for inputs.\n",
				sizeof(*self->inputs) * self->numOfTransactionInput);
		return 0;
	}

	uint32_t i;
	for (i = 0; i < self->numOfTransactionInput; i++) {
		ByteArray * data = getByteArraySubsectionReference(bytes, cursor,
				bytes->length - cursor);
		if (!data) {
			getMessage(self)->onErrorReceived(ERROR_INIT_FAIL,
					"Cannot create a new ByteArray in deserializeTransaction for the input number %u.",
					i);
			return 0;
		}

		TransactionInput * input = createNewTransactionInputFromData(data,
				getMessage(self)->onErrorReceived);

		if (!input) {
			getMessage(self)->onErrorReceived(ERROR_INIT_FAIL,
					"Cannot create a new TransactionInput in deserializeTransaction for the input number %u.",
					i);
			decrementReferenceCount(data);
			return 0;
		}

		uint32_t len = deserializeTransactionInput(input);

		if (!len) {
			getMessage(self)->onErrorReceived(
					ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,
					"Transaction cannot be deserialised because of an error with the input number %u.",
					i);
			decrementReferenceCount(data);
			return 0;
		}
		/* The input was deserialized correctly. Now adjust the length and add it to the transaction.*/
		data->length = len;
		decrementReferenceCount(data);
		self->inputs[i] = input;
		cursor += len; /* Move along to next input*/
	}

	if (bytes->length < cursor + 5) { /* Needs at least 5 more for the output VarLenInt and the lockTime*/
		getMessage(self)->onErrorReceived(
				ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,
				"Attempting to deserialise a Transaction with not enough bytes for the outputs and lockTime.");
		return 0;
	}

	inputOutputLen = decodeVarLenInt(bytes, cursor);

	if (inputOutputLen.value * 9 > bytes->length - 10) {
		getMessage(self)->onErrorReceived(
				ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,
				"Attempting to deserialise a Transaction with a bad varLenInt for the number of outputs.");
		return 0;
	}
	cursor += inputOutputLen.storageSize; /* Move past output VarLenInt*/

	self->numOfTransactionOutput = (uint32_t) inputOutputLen.value;
	self->outputs = malloc(
			sizeof(*self->outputs) * self->numOfTransactionOutput);

	if (!self->outputs) {
		getMessage(self)->onErrorReceived(ERROR_OUT_OF_MEMORY,
				"Cannot allocate %i bytes of memory in deserializeTransaction for outputs.\n",
				sizeof(sizeof(*self->outputs) * self->numOfTransactionOutput));
		return 0;
	}

	for (i = 0; i < self->numOfTransactionOutput; i++) {
		ByteArray * data = getByteArraySubsectionReference(bytes, cursor,
				bytes->length - cursor);

		if (!data) {
			getMessage(self)->onErrorReceived(ERROR_INIT_FAIL,
					"Cannot create a new ByteArray in deserializeTransaction for the output number %u.",
					i);
			return 0;
		}

		TransactionOutput * output = createNewTransactionOutputFromData(data,
				getMessage(self)->onErrorReceived);

		if (!output) {
			getMessage(self)->onErrorReceived(ERROR_INIT_FAIL,
					"Cannot create a new TransactionOutput in deserializeTransaction for the output number %u.",
					i);
			decrementReferenceCount(data);
			return 0;
		}

		uint32_t len = deserializeTransactionOutput(output);

		if (!len) {
			getMessage(self)->onErrorReceived(
					ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,
					"Transaction cannot be deserialised because of an error with the output number %u.",
					i);
			decrementReferenceCount(data);
			return 0;
		}
		/* The output was deserialised correctly. Now adjust the length and add it to the transaction.*/
		data->length = len;
		decrementReferenceCount(data);
		self->outputs[i] = output;
		cursor += len; /* Move along to next output*/
	}

	if (bytes->length < cursor + 4) { /* Ensure 4 bytes are available for lockTime*/
		getMessage(self)->onErrorReceived(
				ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,
				"Attempting to deserialise a Transaction with not enough bytes for the lockTime.");
		return 0;
	}
	self->lockTime = readInt32AsLittleEndianFromByteArray(bytes, cursor);
	return cursor + 4;
}

TransactionHashStatus getTransactionInputHashForVerification(void * txIn,
		ByteArray * prevOutSubScript, uint32_t input, SignatureType signType,
		uint8_t * hash) {
	assert(txIn != NULL);
	assert(prevOutSubScript != NULL);
	assert(input >= 0);
	assert(signType != NULL);
	assert(hash != NULL);

	Transaction * self = txIn;

	if (self->numOfTransactionInput < input + 1) {
		getMessage(self)->onErrorReceived(ERROR_TRANSACTION_FEW_INPUTS,
				"Receiving transaction hash to sign cannot be done for because the input index goes past the number of inputs.");
		return TX_HASH_BAD;
	}

	uint8_t last5Bits = (signType & 0x1f); /* For some reason this is what the C++ client does.*/
	uint32_t sizeOfData = 12 + prevOutSubScript->length; /*Version, lock time and the sign type make up 12 bytes.*/

	if (signType & SIGHASH_ANYONECANPAY) {
		sizeOfData += 41; /* Just this one input. 32 bytes for outPointHash, 4 for outPointIndex, 1 for VarLenInt, 4 for sequence*/
	} else {
		sizeOfData += getSizeOfVarLenInt(self->numOfTransactionInput)
				+ self->numOfTransactionInput * 40; /* All inputs*/
	}

	if (last5Bits == SIGHASH_NONE) {
		sizeOfData++; /* Just for the VarLenInt and no outputs.*/
	} else if ((signType & 0x1f) == SIGHASH_SINGLE) {
		if (self->numOfTransactionOutput < input + 1) {
			getMessage(self)->onErrorReceived(ERROR_TRANSACTION_FEW_OUTPUTS,
					"Receiving transaction hash to sign cannot be done for _SIGHASH_SINGLE because there are not enough outputs.");
			return TX_HASH_BAD;
		}
		sizeOfData += getSizeOfVarLenInt(input + 1) + input * 9; /* For outputs up to the input index*/
		/* The size for the output at the input index.*/
		uint32_t len = getByteArray(self->outputs[input]->scriptObject)->length;
		sizeOfData += 8 + getSizeOfVarLenInt(len) + len;
	} else { /* All outputs. Default to SIGHASH_ALL*/
		sizeOfData += getSizeOfVarLenInt(self->numOfTransactionOutput);
		uint32_t i;
		for (i = 0; i < self->numOfTransactionOutput; i++) {
			uint32_t len = getByteArray(self->outputs[i]->scriptObject)->length;
			sizeOfData += 8 + getSizeOfVarLenInt(len) + len;
		}
	}

	ByteArray * data = createNewByteArrayOfSize(sizeOfData,
			getMessage(self)->onErrorReceived);

	if (!data) {
		return TX_HASH_FAIL;
	}

	writeInt32AsLittleEndianIntoByteArray(data, 0, self->version);

	/* Copy input data. Scripts are not copied for the inputs.*/
	uint32_t cursor;

	if (signType & SIGHASH_ANYONECANPAY) {
		encodeVarLenInt(data, 4, createVarLenIntFromUInt64(1)); /* Only the input the signature is for.*/
		copyByteArrayToByteArray(data, 5, self->inputs[input]->prevOutput.hash);
		writeInt32AsLittleEndianIntoByteArray(data, 37,
				self->inputs[input]->prevOutput.index);
		/*Add prevOutSubScript*/
		copyByteArrayToByteArray(data, 41, prevOutSubScript);
		cursor = 41 + prevOutSubScript->length;
		writeInt32AsLittleEndianIntoByteArray(data, cursor,
				self->inputs[input]->sequence);
		cursor += 4;
	} else {
		VarLenInt inputNum = createVarLenIntFromUInt64(
				self->numOfTransactionInput);
		encodeVarLenInt(data, 4, inputNum);
		cursor = 4 + inputNum.storageSize;
		uint32_t i;
		for (i = 0; i < self->numOfTransactionInput; i++) {
			copyByteArrayToByteArray(data, cursor,
					self->inputs[i]->prevOutput.hash);
			cursor += 32;
			writeInt32AsLittleEndianIntoByteArray(data, cursor,
					self->inputs[i]->prevOutput.index);
			cursor += 4;
			/* Add prevOutSubScript if the input is for the signature.*/
			if (i == input) {
				copyByteArrayToByteArray(data, cursor, prevOutSubScript);
				cursor += prevOutSubScript->length;
			}
			if ((signType == SIGHASH_NONE || signType == SIGHASH_SINGLE)
					&& i != input)
				writeInt32AsLittleEndianIntoByteArray(data, cursor, 0);
			else
				/* SIGHASH_ALL or input index for signing sequence*/
				writeInt32AsLittleEndianIntoByteArray(data, cursor,
						self->inputs[i]->sequence);
			cursor += 4;
		}
	}
	/* Copy output data*/
	if (last5Bits == SIGHASH_NONE) {
		VarLenInt varInt = createVarLenIntFromUInt64(0);
		encodeVarLenInt(data, cursor, varInt);
		cursor++;
	} else if (last5Bits == SIGHASH_SINGLE) {
		VarLenInt varInt = createVarLenIntFromUInt64(input + 1);
		encodeVarLenInt(data, cursor, varInt);
		cursor += varInt.storageSize;

		uint32_t j;
		for (j = 0; j < input; j++) {
			writeInt64AsLittleEndianIntoByteArray(data, cursor,
					0xFFFFFFFFFFFFFFFF); /*_OUTPUT_VALUE_NUS_ONE = 0xFFFFFFFFFFFFFFFF*/
			cursor += 8;
			encodeVarLenInt(data, cursor, createVarLenIntFromUInt64(0));
			cursor++;
		}
		writeInt64AsLittleEndianIntoByteArray(data, cursor,
				self->outputs[input]->value);
		cursor += 8;
		varInt = createVarLenIntFromUInt64(
				getByteArray(self->outputs[input]->scriptObject)->length);
		encodeVarLenInt(data, cursor, varInt);
		cursor += varInt.storageSize;

		copyByteArrayToByteArray(data, cursor,
				getByteArray(self->outputs[input]->scriptObject));
		cursor += varInt.value;
	} else { /* SIGHASH_ALL*/
		VarLenInt varInt = createVarLenIntFromUInt64(
				self->numOfTransactionOutput);
		encodeVarLenInt(data, cursor, varInt);
		cursor += varInt.storageSize;
		uint32_t x;
		for (x = 0; x < self->numOfTransactionOutput; x++) {
			writeInt64AsLittleEndianIntoByteArray(data, cursor,
					self->outputs[x]->value);
			cursor += 8;
			varInt = createVarLenIntFromUInt64(
					getByteArray(self->outputs[x]->scriptObject)->length);
			encodeVarLenInt(data, cursor, varInt);
			cursor += varInt.storageSize;
			copyByteArrayToByteArray(data, cursor,
					getByteArray(self->outputs[x]->scriptObject));
			cursor += varInt.value;
		}
	}
	/* Set lockTime*/
	writeInt32AsLittleEndianIntoByteArray(data, cursor, self->lockTime);
	writeInt32AsLittleEndianIntoByteArray(data, cursor + 4, signType);

	assert(sizeOfData == cursor + 8); /* Must always be like this*/

	uint8_t firstHash[32];
	Sha256(getByteArrayData(data), sizeOfData, firstHash);
	Sha256(firstHash, 32, hash);

	return TX_HASH_GOOD;
}

int takeTransactionInput(Transaction * self, TransactionInput * input) {
	self->numOfTransactionInput++;
	TransactionInput ** temp = realloc(self->inputs,
			sizeof(*self->inputs) * self->numOfTransactionInput);
	if (!temp)
		return FALSE;
	self->inputs = temp;
	self->inputs[self->numOfTransactionInput - 1] = input;
	return TRUE;
}

int takeTransactionOutput(Transaction * self, TransactionOutput * output) {
	self->numOfTransactionOutput++;
	TransactionOutput ** temp = realloc(self->outputs,
			sizeof(*self->outputs) * self->numOfTransactionOutput);
	if (!temp)
		return FALSE;
	self->outputs = temp;
	self->outputs[self->numOfTransactionOutput - 1] = output;
	return TRUE;
}
