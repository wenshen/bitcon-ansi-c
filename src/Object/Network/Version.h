/*
 * Version.h
 *
 * Created on: Nov 9, 2012
 * Created by: Wen Shen
 * Copyright (c) 2012 MIBitcoin Project Team
 */

/**
 @file
 @brief Contains infromation relating to the version of a peer, the timestamp, features, IPs, user agent and last block recieved. Inherits Message
*/


#ifndef VERSION_H_
#define VERSION_H_

#include "../Message/Message.h"
#include "NetworkAddress.h"
/**
 @brief Structure for Version objects. @see Version.h
*/
typedef struct{
	Message base; /**< Object base structure */
	int32_t version; /**< The protocol version. There appears to be no good reason why this is signed. */
	VersionServices services; /**< The services which a peer is offering. */
	int64_t time; /**< The timestamp of this peer. This assumes time(NULL) returns a correct 64 bit timestamp which it should to avoid massive problems in the future. */
	NetworkAddress * addRecv; /**< Socket information for the receiving peer. */
	NetworkAddress * addSource; /**< The socket information for the source address. */
	uint64_t nounce; /**< Nounce used to detect self. */
	ByteArray * userAgent; /**< Used to identify bitcoin software. Should be a string no more than 400 characters in length. */
	int32_t blockHeight; /**< The latest block height for the peer. Should probably be unsigned but the protocol specifies a signed integer. */
} Version;

/**
 @brief Creates a new Version object.
 @returns A new Version object.
 */
Version * createNewVersion(int32_t version,VersionServices services,int64_t time,NetworkAddress * addRecv,NetworkAddress * addSource,uint64_t nounce,ByteArray * userAgent,int32_t blockHeight,void (*onErrorReceived)(Error error,char *,...));
/**
 @brief Creates a new Version object from serialized data.
 @param data A ByteArray holding the serialized data.
 @returns A new Version object.
 */
Version * createNewVersionFromData(ByteArray * data,void (*onErrorReceived)(Error error,char *,...));

/**
 @brief Gets a Version from another object. Use this to avoid casts.
 @param self The object to obtain the Version from.
 @returns The Version object.
 */
Version * getVersion(void * self);

/**
 @brief Initializes a Version object
 @param self The Version object to initialize
 @returns true on success, false on failure.
 */
int initVersion(Version * self,int32_t version,VersionServices services,int64_t time,NetworkAddress * addRecv,NetworkAddress * addSource,uint64_t nounce,ByteArray * userAgent,int32_t blockHeight,void (*onErrorReceived)(Error error,char *,...));
/**
 @brief Initializes a new Version object from serialized data.
 @param self The Version object to initialize
 @param data A ByteArray holding the serialized data.
 @returns true on success, false on failure.
 */
int initVersionFromData(Version * self,ByteArray * data,void (*onErrorReceived)(Error error,char *,...));

/**
 @brief Frees a Version object.
 @param self The Version object to free.
 */
void destroyVersion(void * self);


/**
 @brief Deserializes a Version so that it can be used as an object.
 @param self The Version object
 @returns The length read on success, 0 on failure.
 */
uint32_t deserializeVersion(Version * self);
/**
 @brief Calculates the length needed to serialize the object
 @param self The Version object
 @returns The length read on success, 0 on failure.
 */
uint32_t calculateVersionLength(Version * self);
/**
 @brief Serializes a Version to the byte data.
 @param self The Version object
 @returns The length written on success, 0 on failure.
 */
uint32_t serializeVersion(Version * self);

#endif /* VERSION_H_ */
