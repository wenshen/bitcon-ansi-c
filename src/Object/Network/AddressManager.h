/*
 *  AddressManager.h
 *
 *  Created on: 9/11/2012.
 *  Created by: Prajowal Manandhar
 *  Modified by:
 *  Copyright (c) 2012 Bitcoin Project Team
 *  please read https://en.bitcoin.it/wiki/Protocol_Specification
 */

/**
 @file
 @brief Stores addresses both unconnected and connected. It also proves serialisation and deserialisation so that addresses can be stored in binary format. Credit largely goes to the addrman.cpp code in the original client, although the code here has differences. Inherits Message. The binary format contains a 32 bit integer for the itcoin version, followed by _BUCKET_NUM buckets. Each of these buckets is written as a 16 bit integer for the number of addresses and then the serialised NetworkAddresses one after the other. After the buckets is the 64 bit integer secret.
 */

#ifndef ADDRESSMANAGERH
#define ADDRESSMANAGERH

#include <stdio.h>
#include <stdlib.h>
#include "Peer.h"
#include "AddressBroadcast.h"
#include "NetworkFunctions.h"
#include <time.h>

/**
 @brief Structure for storing a pointer to a NetworkAddress and describing it's location in the AddressManager.
 */
typedef struct {
	NetworkAddress * addr;
	uint8_t bucketIndex;
	uint16_t addrIndex;
} NetworkAddressLocator;

/**
 @brief Structure for placing IP addresses such that is is difficult to obtain IPs that fall into many buckets.
 */
typedef struct {
	NetworkAddress ** addresses;
	uint16_t numOfAddresses; /**< The number of addresses in this bucket. */
} Bucket;

/**
 @brief Structure for AddressManager objects. @see AddressManager.h
 */
typedef struct {
	Message base; /**< Message base structure */
	Bucket buckets[255]; /**< Unconnected peers stored in the buckets */
	Peer ** peers; /**< Connected peers sorted by the time offset. */
	uint32_t peersNum; /**< Number of connected peers */
	int16_t networkTimeOffset; /**< Offset to get from system time to network time. */
	IPType reachability; /**< Bitfield for reachable address types */
	uint16_t maxAddressesInBucket; /**< Maximum number of addresses that can be stored in a single bucket. */
	uint64_t secret; /**< Securely generated pseudo-random number to generate a secret used to mix-up groups into different buckets. */
	uint64_t rndGen; /**< Random number generator instance. */
	uint64_t rndGenForBucketIndexes; /**< Random number generator used for generating bucket indexes. */
	void * callbackHandler; /**< Sent to onBadTime callback */
	void (*onBadTime)(void *); /**< Called when bitcoin detects a divergence between the network time and system time which suggests the system time may be wrong, in the same way bitcoin-qt detects it. Has an argument for the callback handler. */
} AddressManager;

/**
 @brief Creates a new AddressManager object.
 @returns A new AddressManager object.
 */
AddressManager * newAddressManager(void (*logError)(char *, ...),
		void (*onBadTime)(void *));
/**
 @brief Creates a new AddressManager object from serialised data.
 @param data Serialised AddressManager data.
 @returns A new AddressManager object.
 */
AddressManager * newAddressManagerFromData(ByteArray * data,
		void (*logError)(char *, ...), void (*onBadTime)(void *));

/**
 @brief Gets a AddressManager from another object. Use this to avoid casts.
 @param self The object to obtain the AddressManager from.
 @returns The AddressManager object.
 */
AddressManager * getAddressManager(void * self);

/**
 @brief Initialises a AddressManager object
 @param self The AddressManager object to initialise
 @returns 1 on success, 0 on failure.
 */
boolean initAddressManager(AddressManager * self, void (*logError)(char *, ...),
		void (*onBadTime)(void *));
/**
 @brief Initialises a AddressManager object from serialised data
 @param self The AddressManager object to initialise
 @param data The serialised data.
 @returns 1 on success, 0 on failure.
 */
boolean initAddressManagerFromData(AddressManager * self, ByteArray * data,
		void (*logError)(char *, ...), void (*onBadTime)(void *));

/**
 @brief Frees a AddressManager object.
 @param self The AddressManager object to free.
 */
void destroyAddressManager(void * vself);

/**
 @brief Adds a Peer and places it into the AddressManager with a retain.
 @param self The AddressManager object.
 @param peer The NetworkAddress to take.
 @returns 1 if the address was taken successfully, 0 if an error occured.
 */
boolean addAddressToAddressManager(AddressManager * self, NetworkAddress * addr);
/**
 @brief Adjust the network time offset with a peer's time.
 @param self The AddressManager object.
 @param time Time to adjust network time with.
 */
void addressManagerAdjustTime(AddressManager * self);
/**
 @brief Deserialises a AddressManager so that it can be used as an object.
 @param self The AddressManager object
 @returns The length read on success, 0 on failure.
 */
uint32_t addressManagerDeserialise(AddressManager * self,
		void (*onErrorReceived)(Error error, char *, ...));
/**
 @brief Gets a number of addresses.
 @param self The AddressManager object.
 @param num The number of addresses to get.
 @returns The addresses in a newly allocated memory block. This block is NULL terminated. The number of addresses returned may be less than "num". NetworkAddressLocator is used to locate addresses if needed. This will return NULL if malloc() fails.
 */
NetworkAddressLocator * getAddressesfromAddressManager(AddressManager * self, uint32_t num);
/**
 @brief Gets the bucket index for an address.
 @param self The AddressManager object.
 @param addr The address.
 @returns The bucket index.
 */
uint8_t addressManagerGetBucketIndex(AddressManager * self,
		NetworkAddress * addr);
/**
 @brief Gets the group number for an address. It is seperated such as to make it difficult to use an IP in many different groups. This is done such as the original bitcoin client.
 @param self The AddressManager object.
 @param addr The address.
 @returns The address group number.
 */
uint64_t addressManagerGetGroup(AddressManager * self, NetworkAddress * addr);
/**
 @brief Gets the total amount of addresses, including connected ones. Not thread safe.
 @param self The AddressManager object.
 @returns The total number of addresses.
 */
uint64_t numOfStoredAddresses(AddressManager * self);
/**
 @brief Determines if a NetworkAddress already exists in the AddressManager. Compares the IP address and port.
 @param self The AddressManager object.
 @param addr The address.
 @returns If the address already exists, returns the existing object. Else returns NULL.
 */
NetworkAddress * getNetworkAddressFromAddressManager(AddressManager * self,
		NetworkAddress * addr);
/**
 @brief Determines if a NetworkAddress is in the "peers" list. Compares the IP address and port.
 @param self The AddressManager object.
 @param addr The address.
 @returns If the address already exists as a connected peer, returns the existing object. Else returns NULL.
 */
Peer * getPeerNodeFromAddressManager(AddressManager * self, NetworkAddress * addr);
/**
 @brief Determines if an IP type is reachable.
 @param self The AddressManager object.
 @param type The type.
 @returns 1 if reachable, 0 if not reachable.
 */
boolean isReachableIPType(AddressManager * self, IPType type);
/**
 @brief Removes a NetworkAddress if the AddressManager has it.
 @param self The AddressManager object.
 @param addr The NetworkAddress to remove
 */
void removeNetworkAddressFromAddressManager(AddressManager * self, NetworkAddress * addr);
/**
 @brief Remove a Peer from the peers list.
 @param self The AddressManager object.
 @param peer The Peer to remove
 */
void removePeerNodeFromAddressManager(AddressManager * self, Peer * peer);
/**
 @brief Serialises a AddressManager to the byte data.
 @param self The AddressManager object
 @param force Serialises everything, replacing any previous serialisation of children objects.
 @returns The length written on success, 0 on failure.
 */
uint32_t addressManagerSerialise(AddressManager * self, boolean force,
		void (*onErrorReceived)(Error error, char *, ...));
/**
 @brief Sets the reachability of an IP type.
 @param self The AddressManager object
 @param type The mask to set the types.
 @param reachable 1 for setting as reachable, 0 if not.
 */
void setIPTypeReachability(AddressManager * self, IPType type, boolean reachable);
/**
 @brief Setups the AddressMenager for both InitAddressManager and InitAddressManagerFromData.
 @param self The AddressManager object
 @returns 1 if OK, 0 if not.
 */
boolean addressManagerSetup(AddressManager * self);
/**
 @brief Takes a NetworkAddress and places it into the AddressManager
 @param self The AddressManager object.
 @param peer The NetworkAddress to take.
 @returns 1 if the address was taken successfully, 0 if an error is occurred.
 */
boolean addNetworkAddressToAddressManager(AddressManager * self, NetworkAddress * addr);
/**
 @brief Takes a Peer an places it into the peers list.
 @param self The AddressManager object.
 @param peer The Peer to take.
 */
boolean addPeerToAddressManager(AddressManager * self, Peer * peer);

boolean newSecureRandomGenerator(uint64_t * gen);
void secureRandomSeed(uint64_t gen);
void randomSeed(uint64_t gen, uint64_t seed);
uint64_t secureRandomInteger(uint64_t gen);
void destroySecureRandomGenerator(uint64_t gen);
#endif

