/*
 *  Alert.c
 *
 *  Created on: 9/12/2012.
 *  Created by: Prajowal Manandhar
 *  Modified by:
 *  Copyright (c) 2012 Bitcoin Project Team
 *  please read https://en.bitcoin.it/wiki/Protocol_Specification
 */

#include "Alert.h"
#include "../../Utils/VariableLengthInteger/VarLenInt.h"
#include <assert.h>

Alert * newAlert(int32_t version, int64_t relayUntil, int64_t expiration,
		int32_t ID, int32_t cancel, int32_t minVer, int32_t maxVer,
		int32_t priority, ByteArray * hiddenComment,
		ByteArray * displayedComment, ByteArray * reserved,
		ByteArray * signature, void (*logError)(char *, ...)) {
	Alert * self = malloc(sizeof(*self));
	if (!self) {
		logError("Cannot allocate %i bytes of memory in NewAlert\n",
				sizeof(*self));
		return NULL;
	}
	getObject(self)->destroy = destroyAlert;
	if (initAlert(self, version, relayUntil, expiration, ID, cancel, minVer,
			maxVer, priority, hiddenComment, displayedComment, reserved,
			signature, logError)){
		return self;
	}
	free(self);
	return NULL;
}
Alert * newAlertFromData(ByteArray * data, void (*logError)(char *, ...)) {
	Alert * self = malloc(sizeof(*self));
	if (!self) {
		logError("Cannot allocate %i bytes of memory in NewAlertFromData\n",
				sizeof(*self));
		return NULL;
	}
	getObject(self)->destroy = destroyAlert;
	if (initAlertFromData(self, data, logError)){
		return self;
	}
	free(self);
	return NULL;
}

Alert * getAlert(void * self) {
	return self;
}

short int initAlert(Alert * self, int32_t version, int64_t relayUntil,
		int64_t expiration, int32_t ID, int32_t cancel, int32_t minVer,
		int32_t maxVer, int32_t priority, ByteArray * hiddenComment,
		ByteArray * displayedComment, ByteArray * reserved,
		ByteArray * signature, void (*logError)(char *, ...)) {
	self->version = version;
	self->relayUntil = relayUntil;
	self->expiration = expiration;
	self->ID = ID;
	self->cancel = cancel;
	self->minVer = minVer;
	self->maxVer = maxVer;
	self->priority = priority;
	self->hiddenComment = hiddenComment;
	if (hiddenComment){
		incrementReferenceCount(hiddenComment);
	}
	self->displayedComment = displayedComment;
	if (displayedComment){
		incrementReferenceCount(displayedComment);
	}
	self->reserved = reserved;
	if (reserved){
		incrementReferenceCount(reserved);
	}
	self->signature = signature;
	incrementReferenceCount(signature);
	if (!initializeMessageFromObject(getMessage(self), logError)){
		return 0;
	}
	return 1;
}
short int initAlertFromData(Alert * self, ByteArray * data,
		void (*logError)(char *, ...)) {
	self->setCancel = NULL;
	self->userAgents = NULL;
	self->hiddenComment = NULL;
	self->displayedComment = NULL;
	self->reserved = NULL;
	self->signature = NULL;
	if (!initializeMessageFromByteArrayData(getMessage(self), data, logError)){
		return 0;
	}
	return 1;
}

void destroyAlert(void * vself) {
	Alert * self = vself;
	free(self->setCancel);
	uint16_t x;
	for (x = 0; x < self->userAgentNum; x++) {
		decrementReferenceCount(self->userAgents[x]);
	}
	free(self->userAgents);
	if (self->hiddenComment){
		decrementReferenceCount(self->hiddenComment);
	}
	if (self->displayedComment){
		decrementReferenceCount(self->displayedComment);
	}
	if (self->reserved){
		decrementReferenceCount(self->reserved);
	}
	if (self->signature){
		decrementReferenceCount(self->signature);
	}
	free(self);
}

short int alertAddCancelID(Alert * self, uint32_t ID) {
	self->setCancelNum++;
	int32_t * temp = realloc(self->setCancel,
			sizeof(*self->setCancel) * self->setCancelNum);
	if (!temp){
		return 0;
	}
	self->setCancel = temp;
	self->setCancel[self->setCancelNum - 1] = ID;
	return 1;
}

short int alertAddUserAgent(Alert * self, ByteArray * userAgent) {
	incrementReferenceCount(userAgent);
	return alertTakeUserAgent(self, userAgent);
}

uint32_t alertCalculateLength(Alert * self) {
	uint32_t len = 40 + getSizeOfVarLenInt(self->userAgentNum)
			+ getSizeOfVarLenInt(self->setCancelNum) + self->setCancelNum * 4;
	if (self->displayedComment){
		len += self->displayedComment->length;
	} else {
		len++;
	}
	if (self->hiddenComment){
		len += self->hiddenComment->length;
	} else {
		len++;
	}
	if (self->reserved){
		len += self->reserved->length;
	} else {
		len++;
	}
	uint32_t x1;
	for (x1 = 0; x1 < self->userAgentNum; x1++)
		len += getSizeOfVarLenInt(self->userAgents[x1]->length)
				+ self->userAgents[x1]->length;
	len += getSizeOfVarLenInt(len);
	return len + getSizeOfVarLenInt(self->signature->length)
			+ self->signature->length;
}

uint32_t deserializeAlert(Alert * self)
{
	ByteArray * bytes = getMessage(self)->bytes;
	if (!bytes) {
		getMessage(self)->onErrorReceived(ERROR_OUT_OF_MEMORY,
				"Attempting to deserialize a Alert with no bytes.");
		return 0;
	}
	if (bytes->length < 47) {
		getMessage(self)->onErrorReceived(ERROR_OUT_OF_MEMORY,
				"Attempting to deserialize a Alert with less than 47 bytes minimally required.");
		return 0;
	}

	VarLenInt payloadLen = decodeVarLenInt(bytes, 0);
	if (bytes->length < payloadLen.storageSize + payloadLen.value + 1) {
		getMessage(self)->onErrorReceived(ERROR_OUT_OF_MEMORY,
				"Attempting to deserialize a Alert with less bytes than required for payload.");
		return 0;
	}
	if (payloadLen.value > 2000) {
		getMessage(self)->onErrorReceived(ERROR_OUT_OF_MEMORY,
				"Attempting to deserialize a Alert with a payload var int larger than 2000 bytes.");
		return 0;
	}
	if (payloadLen.value < 45) {
		getMessage(self)->onErrorReceived(ERROR_OUT_OF_MEMORY,
				"Attempting to deserialize a Alert with a payload var int smaller than 45.");
		return 0;
	}
	uint16_t cursor = payloadLen.storageSize;
	self->version = readInt32AsLittleEndianFromByteArray(bytes, cursor);
	cursor += 4;
	self->relayUntil = readInt64AsLittleEndianFromByteArray(bytes, cursor);
	cursor += 8;
	self->expiration = readInt64AsLittleEndianFromByteArray(bytes, cursor);
	cursor += 8;
	self->ID = readInt32AsLittleEndianFromByteArray(bytes, cursor);
	cursor += 4;
	self->cancel = readInt32AsLittleEndianFromByteArray(bytes, cursor);
	cursor += 4;

	VarLenInt setCancelLen = decodeVarLenInt(bytes, cursor);
	if (payloadLen.value
			< 44 + setCancelLen.storageSize + setCancelLen.value * 4) {
		getMessage(self)->onErrorReceived(ERROR_OUT_OF_MEMORY,
				"Attempting to deserialize a Alert with a payload var int smaller than required to cover the cancel set.");
		return 0;
	}
	self->setCancelNum = setCancelLen.value;
	cursor += setCancelLen.storageSize;
	if (self->setCancelNum) {
		self->setCancel = malloc(sizeof(*self->setCancel) * self->setCancelNum);
		if (!self->setCancel) {
			getMessage(self)->onErrorReceived(ERROR_OUT_OF_MEMORY,
					"Cannot allocate %i bytes of memory in AlertDeserialise for the cancel set.",
					sizeof(*self->setCancel) * self->setCancelNum);
			return 0;
		}
		uint16_t x;
		for (x = 0; x < self->setCancelNum; x++) {
			readInt32AsLittleEndianFromByteArray(bytes, cursor);
			cursor += 4;
		}
	}
	self->minVer = readInt32AsLittleEndianFromByteArray(bytes, cursor);
	cursor += 4;
	self->maxVer = readInt32AsLittleEndianFromByteArray(bytes, cursor);
	cursor += 4;

	VarLenInt userAgentsLen = decodeVarLenInt(bytes, cursor);
	if (payloadLen.value
			< 7 + cursor + userAgentsLen.storageSize + userAgentsLen.value
					- payloadLen.storageSize) {
		getMessage(self)->onErrorReceived(ERROR_OUT_OF_MEMORY,
				"Attempting to deserialize a Alert with a payload var int smaller than required to cover the cancel set and the user agent set assuming empty strings.");
		return 0;
	}
	self->userAgentNum = userAgentsLen.value;
	cursor += userAgentsLen.storageSize;
	if (self->userAgentNum) {
		self->userAgents = malloc(
				sizeof(*self->userAgents) * self->userAgentNum);
		if (!self->userAgents) {
			getMessage(self)->onErrorReceived(ERROR_OUT_OF_MEMORY,
					"Cannot allocate %i bytes of memory in AlertDeserialise for the user agent set.",
					sizeof(*self->userAgents) * self->userAgentNum);
			return 0;
		}
		uint16_t x;
		for (x = 0; x < self->userAgentNum; x++) {

			VarLenInt userAgentLen = decodeVarLenInt(bytes, cursor);
			cursor += userAgentLen.storageSize;
			if (payloadLen.value
					< 7 + cursor + userAgentLen.value + self->userAgentNum - x
							- payloadLen.storageSize) {
				getMessage(self)->onErrorReceived(ERROR_OUT_OF_MEMORY,
						"Attempting to deserialize a Alert with a payload var int smaller than required to cover the cancel set and the user agent set up to user agent %u.",
						x);
				return 0;
			}

			if (userAgentLen.value) {
				self->userAgents[x] = getByteArraySubsectionReference(bytes,
						cursor, (uint32_t) userAgentLen.value);
				if (!self->userAgents[x]) {
					getMessage(self)->onErrorReceived(ERROR_OUT_OF_MEMORY,
							"Cannot create new ByteArray in AlertDeserialise for userAgent %i.\n",
							x);
					return 0;
				}
			} else {
				self->userAgents[x] = NULL;
			}
			cursor += userAgentLen.value;
		}
	}
	self->priority = readInt32AsLittleEndianFromByteArray(bytes, cursor);
	cursor += 4;

	uint8_t size;
	size = getByteFromByteArray(bytes, cursor);
	if (size < 253)
		size = 1;
	else if (size == 253)
		size = 2;
	else if (size == 254)
		size = 4;
	else if (size == 255)
		size = 8;
	if (payloadLen.value >= cursor + size + 2 - payloadLen.storageSize) {
		VarLenInt hiddenCommentLen = decodeVarLenInt(bytes, cursor);
		cursor += size;
		if (payloadLen.value
				>= cursor + hiddenCommentLen.value + 2
						- payloadLen.storageSize) {
			if (hiddenCommentLen.value) {
				self->hiddenComment = getByteArraySubsectionReference(bytes,
						cursor, (uint32_t) hiddenCommentLen.value);
				if (!self->hiddenComment) {
					getMessage(self)->onErrorReceived(ERROR_OUT_OF_MEMORY,
							"Cannot create new ByteArray in AlertDeserialise for the hidden comment.\n");
					return 0;
				}
			} else {
				self->hiddenComment = NULL;
			}
			cursor += hiddenCommentLen.value;

			size = getByteFromByteArray(bytes, cursor);
			if (size < 253)
				size = 1;
			else if (size == 253)
				size = 2;
			else if (size == 254)
				size = 4;
			else if (size == 255)
				size = 8;
			if (payloadLen.value
					>= cursor + size + 1 - payloadLen.storageSize) {
				VarLenInt displayedCommentLen = decodeVarLenInt(bytes, cursor);
				cursor += size;
				if (payloadLen.value
						>= cursor + displayedCommentLen.value + 1
								- payloadLen.storageSize) {
					if (displayedCommentLen.value) {
						self->displayedComment =
								getByteArraySubsectionReference(bytes, cursor,
										(uint32_t) displayedCommentLen.value);
						if (!self->displayedComment) {
							getMessage(self)->onErrorReceived(ERROR_OUT_OF_MEMORY,
									"Cannot create new ByteArray in AlertDeserialise for the displayed comment.\n");
							return 0;
						}
					} else {
						self->displayedComment = NULL;
					}
					cursor += displayedCommentLen.value;

					size = getByteFromByteArray(bytes, cursor);
					if (size < 253)
						size = 1;
					else if (size == 253)
						size = 2;
					else if (size == 254)
						size = 4;
					else if (size == 255)
						size = 8;
					if (payloadLen.value
							>= cursor + size - payloadLen.storageSize) {
						VarLenInt reservedLen = decodeVarLenInt(bytes, cursor);
						cursor += size;
						if (payloadLen.value
								== cursor + reservedLen.value
										- payloadLen.storageSize) {
							if (reservedLen.value) {
								self->reserved =
										getByteArraySubsectionReference(bytes,
												cursor,
												(uint32_t) reservedLen.value);
								if (!self->reserved) {
									getMessage(self)->onErrorReceived(ERROR_OUT_OF_MEMORY,
											"Cannot create new ByteArray in AlertDeserialise for the reserved section\n");
									return 0;
								}
							} else {
								self->reserved = NULL;
							}
							cursor += reservedLen.value;

							size = getByteFromByteArray(bytes, cursor);
							if (size < 253)
								size = 1;
							else if (size == 253)
								size = 2;
							else if (size == 254)
								size = 4;
							else if (size == 255)
								size = 8;
							if (bytes->length >= cursor + size) {
								VarLenInt sigLen = decodeVarLenInt(bytes,
										cursor);
								cursor += size;
								if (bytes->length >= cursor + sigLen.value) {
									self->signature =
											getByteArraySubsectionReference(
													bytes, cursor,
													(uint32_t) sigLen.value);
									if (self->signature) {

										return cursor += sigLen.value;
									} else
										getMessage(self)->onErrorReceived(ERROR_OUT_OF_MEMORY,
												"Cannot create new ByteArray in AlertDeserialise for the signature\n");
								} else {
									getMessage(self)->onErrorReceived(ERROR_OUT_OF_MEMORY,
											"Attempting to deserialize a Alert with a byte array length smaller than required to cover the signature.");
								}
							} else {
								getMessage(self)->onErrorReceived(ERROR_OUT_OF_MEMORY,
										"Attempting to deserialize a Alert with a byte array length smaller than required to cover the signature var int.");
							}

							if (self->reserved)
								decrementReferenceCount(self->reserved);
						} else {
							getMessage(self)->onErrorReceived(ERROR_OUT_OF_MEMORY,
									"Attempting to deserialize a Alert with a payload var int smaller than required to cover the reserved string.");
						}
					} else {
						getMessage(self)->onErrorReceived(ERROR_OUT_OF_MEMORY,
								"Attempting to deserialize a Alert with a payload var int smaller than required to cover the reserved string var int.");
					}

					if (self->displayedComment)
						decrementReferenceCount(self->displayedComment);
				} else {
					getMessage(self)->onErrorReceived(ERROR_OUT_OF_MEMORY,
							"Attempting to deserialize a Alert with a payload var int smaller than required to cover the displayed string. %u < %u",
							payloadLen.value,
							cursor + displayedCommentLen.value + 1);
				}
			} else {
				getMessage(self)->onErrorReceived(ERROR_OUT_OF_MEMORY,
						"Attempting to deserialize a Alert with a payload var int smaller than required to cover the displayed string var int.");
			}

			if (self->hiddenComment)
				decrementReferenceCount(self->hiddenComment);
		} else {
			getMessage(self)->onErrorReceived(ERROR_OUT_OF_MEMORY,
					"Attempting to deserialize a Alert with a payload var int smaller than required to cover the hidden string.");
		}
	} else {
		getMessage(self)->onErrorReceived(ERROR_OUT_OF_MEMORY,
				"Attempting to deserialize a Alert with a payload var int smaller than required to cover the hidden string var int.");
	}

	return 0;
}

ByteArray * alertGetPayload(Alert * self)
{
	assert(self!= NULL);

	ByteArray * bytes = getMessage(self)->bytes;
	VarLenInt payloadLen = decodeVarLenInt(bytes, 0);
	return getByteArraySubsectionReference(bytes, payloadLen.storageSize,(uint32_t) payloadLen.value);
}

ByteArray * alertSerialisePayload(Alert * self) {
	ByteArray * bytes = getMessage(self)->bytes;
	if (!bytes) {
		getMessage(self)->onErrorReceived(
				ERROR_OUT_OF_MEMORY,
				"Attempting to serialise a Alert with no bytes.");
		return NULL;
	}

	VarLenInt setCancelLen = createVarLenIntFromUInt64(self->setCancelNum);
	VarLenInt userAgentLen = createVarLenIntFromUInt64(self->userAgentNum);
	VarLenInt hiddenCommentLen;
	if (self->hiddenComment) {
		hiddenCommentLen = createVarLenIntFromUInt64(
				self->hiddenComment->length);
	} else {
		hiddenCommentLen = createVarLenIntFromUInt64(0);
	}
	VarLenInt displayedCommentLen;
	if (self->displayedComment) {
		displayedCommentLen = createVarLenIntFromUInt64(
				self->displayedComment->length);
	} else {
		displayedCommentLen = createVarLenIntFromUInt64(0);
	}
	VarLenInt reservedLen;
	if (self->reserved) {
		reservedLen = createVarLenIntFromUInt64(self->reserved->length);
	} else {
		reservedLen = createVarLenIntFromUInt64(0);
	}
	uint16_t length = 40 + setCancelLen.storageSize + setCancelLen.value * 4
			+ userAgentLen.storageSize + hiddenCommentLen.storageSize
			+ hiddenCommentLen.value + displayedCommentLen.storageSize
			+ displayedCommentLen.value + reservedLen.storageSize
			+ reservedLen.value;
	uint16_t x;
	for (x = 0; x < self->userAgentNum; x++) {
		length += self->userAgents[x]->length
				+ getSizeOfVarLenInt(self->userAgents[x]->length);
	}
	VarLenInt payloadLen = createVarLenIntFromUInt64(length);
	length += payloadLen.storageSize;
	if (bytes->length < length + 1) {
		getMessage(self)->onErrorReceived(ERROR_OUT_OF_MEMORY,
				"Attempting to serialise a Alert with less bytes than required for the payload and a signature var int.");
		return NULL;
	}

	encodeVarLenInt(bytes, 0, payloadLen);
	uint16_t cursor = payloadLen.storageSize;
	writeInt32AsLittleEndianIntoByteArray(bytes, cursor, self->version);
	cursor += 4;
	writeInt64AsLittleEndianIntoByteArray(bytes, cursor, self->relayUntil);
	cursor += 8;
	writeInt64AsLittleEndianIntoByteArray(bytes, cursor, self->expiration);
	cursor += 8;
	writeInt32AsLittleEndianIntoByteArray(bytes, cursor, self->ID);
	cursor += 4;
	writeInt32AsLittleEndianIntoByteArray(bytes, cursor, self->cancel);
	cursor += 4;

	encodeVarLenInt(bytes, cursor, setCancelLen);
	cursor += setCancelLen.storageSize;

	for (x = 0; x < self->setCancelNum; x++) {
		writeInt32AsLittleEndianIntoByteArray(bytes, cursor,
				self->setCancel[x]);
		cursor += 4;
	}
	writeInt32AsLittleEndianIntoByteArray(bytes, cursor, self->minVer);
	cursor += 4;
	writeInt32AsLittleEndianIntoByteArray(bytes, cursor, self->maxVer);
	cursor += 4;

	encodeVarLenInt(bytes, cursor, userAgentLen);
	cursor += userAgentLen.storageSize;

	for (x = 0; x < self->userAgentNum; x++) {
		VarLenInt aUserAgentLen = createVarLenIntFromUInt64(
				self->userAgents[x]->length);
		encodeVarLenInt(bytes, cursor, aUserAgentLen);
		cursor += aUserAgentLen.storageSize;
		copyByteArrayToByteArray(bytes, cursor, self->userAgents[x]);
		changeByteArrayDataReference(self->userAgents[x], bytes, cursor);
		cursor += aUserAgentLen.value;
	}
	writeInt32AsLittleEndianIntoByteArray(bytes, cursor, self->priority);
	cursor += 4;

	encodeVarLenInt(bytes, cursor, hiddenCommentLen);
	cursor += hiddenCommentLen.storageSize;
	if (self->hiddenComment) {
		copyByteArrayToByteArray(bytes, cursor, self->hiddenComment);
		changeByteArrayDataReference(self->hiddenComment, bytes, cursor);
		cursor += hiddenCommentLen.value;
	}

	encodeVarLenInt(bytes, cursor, displayedCommentLen);
	cursor += displayedCommentLen.storageSize;
	if (self->displayedComment) {
		copyByteArrayToByteArray(bytes, cursor, self->displayedComment);
		changeByteArrayDataReference(self->displayedComment, bytes, cursor);
		cursor += displayedCommentLen.value;
	}

	encodeVarLenInt(bytes, cursor, reservedLen);
	cursor += reservedLen.storageSize;
	if (self->reserved) {
		copyByteArrayToByteArray(bytes, cursor, self->reserved);
		changeByteArrayDataReference(self->reserved, bytes, cursor);
		cursor += reservedLen.value;
	}
	return getByteArraySubsectionReference(bytes, payloadLen.storageSize, cursor - payloadLen.storageSize);
}

uint16_t alertSerialiseSignature(Alert * self, uint16_t offset)
{
	ByteArray * bytes = getMessage(self)->bytes;
	if (!bytes) {
		getMessage(self)->onErrorReceived(ERROR_OUT_OF_MEMORY,
				"Attempting to deserialize a Alert with no bytes.");
		return 0;
	}
	VarLenInt sigLen = createVarLenIntFromUInt64(self->signature->length);
	if (bytes->length < offset + sigLen.storageSize + sigLen.value) {
		getMessage(self)->onErrorReceived(ERROR_OUT_OF_MEMORY,
				"Attempting to serialise a Alert with less bytes than required for the signature.");
		return 0;
	}
	encodeVarLenInt(bytes, offset, sigLen);
	offset += sigLen.storageSize;
	copyByteArrayToByteArray(bytes, offset, self->signature);
	changeByteArrayDataReference(self->signature, bytes, offset);
	/*getMessage(self)->serialised = 1;*/
	return offset + sigLen.value;
}

short int alertTakeUserAgent(Alert * self, ByteArray * userAgent) {
	self->userAgentNum++;
	ByteArray ** temp = realloc(self->userAgents,
			sizeof(*self->userAgents) * self->userAgentNum);
	if (!temp)
		return 0;
	self->userAgents = temp;
	self->userAgents[self->userAgentNum - 1] = userAgent;
	return 1;
}
