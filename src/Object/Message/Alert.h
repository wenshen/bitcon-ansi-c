/*
 *  Alert.h
 *
 *  Created on: 9/12/2012.
 *  Created by: Prajowal Manandhar
 *  Modified by:
 *  Copyright (c) 2012 Bitcoin Project Team
 *  please read https://en.bitcoin.it/wiki/Protocol_Specification
 */

/**
 @file
 @brief A message containing an alert, Inherits Message
 */

#ifndef ALERTH
#define ALERTH

#include "Message.h"
#include <stdbool.h>

/**
 @brief Structure for Alert objects. @see Alert.h
 */
typedef struct {
	Message base; /**< Message base structure */
	int32_t version; /**< The version of the alert */
	int64_t relayUntil; /**< Do not relay past this date */
	int64_t expiration; /**< The date at which the alert is no longer in effect */
	int32_t ID; /**< Unique id for this alert */
	int32_t cancel; /**< Alerts under this ID should be deleted and not relayed in the future */
	uint16_t setCancelNum; /**< Number of IDs to cancel */
	int32_t * setCancel; /**< A selection of IDs to cancel */
	int32_t minVer; /**< The alert is valid for versions greater than this. */
	int32_t maxVer; /**< The alert is valid for versions less than this. */
	uint16_t userAgentNum; /**< Number of user agents. */
	ByteArray ** userAgents; /**< List of the userAgents of the effected versions. */
	int32_t priority; /**< Priority compared to other alerts */
	ByteArray * hiddenComment; /**< Comment that should be hidden from end-users */
	ByteArray * displayedComment; /**< Comment that can be displayed to end-users */
	ByteArray * reserved; /**< Reserved bytes */
	ByteArray * signature; /**< ECDSA signature for data */
} Alert;

/**
 @brief Creates a new Alert object.
 @returns A new Alert object.
 */
Alert * newAlert(int32_t version, int64_t relayUntil, int64_t expiration,
		int32_t ID, int32_t cancel, int32_t minVer, int32_t maxVer,
		int32_t priority, ByteArray * hiddenComment,
		ByteArray * displayedComment, ByteArray * reserved,
		ByteArray * signature, void (*logError)(char *, ...));
/**
 @brief Creates a new Alert object from serialised data.
 @param data Serialised Alert data.
 @returns A new Alert object.
 */
Alert * newAlertFromData(ByteArray * data, void (*logError)(char *, ...));

/**
 @brief Gets a Alert from another object. Use this to avoid casts.
 @param self The object to obtain the Alert from.
 @returns The Alert object.
 */
Alert * getAlert(void * self);

/**
 @brief Initialises a Alert object
 @param self The Alert object to initialise
 @returns true on success, false on failure.
 */
short int initAlert(Alert * self, int32_t version, int64_t relayUntil,
		int64_t expiration, int32_t ID, int32_t cancel, int32_t minVer,
		int32_t maxVer, int32_t priority, ByteArray * hiddenComment,
		ByteArray * displayedComment, ByteArray * reserved,
		ByteArray * signature, void (*logError)(char *, ...));
/**
 @brief Initialises a Alert object from serialised data
 @param self The Alert object to initialise
 @param data The serialised data.
 @returns true on success, false on failure.
 */
short int initAlertFromData(Alert * self, ByteArray * data,
		void (*logError)(char *, ...));

/**
 @brief Frees a Alert object.
 @param self The Alert object to free.
 */
void destroyAlert(void * self);

/**
 @brief Adds a ID to the cancel set
 @param self The Alert object
 @param ID The id to add.
 @returns true if the cancel ID was added successfully or false on error.
 */
short int alertAddCancelID(Alert * self, uint32_t ID);
/**
 @brief Adds a user agent to the user agent set
 @param self The Alert object
 @param userAgent The user agent to add.
 @returns true if the user agent was added successfully or false on error.
 */
short int alertAddUserAgent(Alert * self, ByteArray * userAgent);
/**
 @brief Calculates the length needed to serialise the object.
 @param self The Alert object.
 @returns The length read on success, 0 on failure.
 */
uint32_t alertCalculateLength(Alert * self);
/**
 @brief Deserialises a Alert so that it can be used as an object.
 @param self The Alert object
 @returns The length read on success, 0 on failure.
 */
uint32_t deserializeAlert(Alert * self);
/**
 @brief Gets the payload from the data. Should be serialised beforehand.
 @param self The Alert object
 @returns A ByteArray object for the payload. Can be used to check the signature. When checking the signature the payload should be hashed by SHA256 twice.
 */
ByteArray * alertGetPayload(Alert * self);
/**
 @brief Serialises the payload of a Alert and returns a ByteArray to the specifc payload bytes to aid in creating a signature.
 @param self The Alert object
 @returns The payload bytes on sucess. NULL on failure.
 */
ByteArray * alertSerialisePayload(Alert * self);
/**
 @brief Serialises the signature of a Alert.
 @param self The Alert object
 @param offset The offset to the begining of the signature which should be exactly after the payload.
 @returns The total length of the serialised Alert on sucess, else false.
 */
uint16_t alertSerialiseSignature(Alert * self, uint16_t offset);
/**
 @brief Takes a user agent for the user agent set. This does not retain the ByteArray so you can pass an ByteArray into this while releasing control from the calling function.
 @param self The Alert object
 @param userAgent The user agent to take.
 @returns true if the user agent was taken successfully or false on error.
 */
short int alertTakeUserAgent(Alert * self, ByteArray * userAgent);

#endif
