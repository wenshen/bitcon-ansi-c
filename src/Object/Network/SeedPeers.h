/*
 * SeedPeers.h
 *
 *  Created on: 23/11/2012
 *  Created by: Hayk Baluyan
 *  Copyright (c) 2012 Bitcoin Project Team
 */

/**
 @SeedPeers stores a pre-determined list of Bitcoin node addresses. These nodes are selected based on being
 @active on the network for a long period of time. The intention is to be a last resort way of finding a connection
 @to the network, in case IRC and DNS fail. The list comes from the Bitcoin C++ source code.
 */


#ifndef SEEDPEERSH
#define SEEDPEERSH

#include "NetworkAddress.h"

typedef struct {
	Object base; /**< Object base structure */
    int pnseedIndex; /**< index to currnt seed peer */
    uint32_t* seedAddrs; /**< array of seed peer addresses */

} SeedPeers;

/**
 @brief Creates a new SeedPeers object
@param onErrorReceived Callback for errors.
 @returns A new SeedPeers object.
 */
SeedPeers* createNewSeedPeers(void (*onErrorReceived)(Error error,char *,...));

/**
 @brief Initializes a SeedPeers object
 @param self The SeedPeers object to initialize.
 @param onErrorReceived Events for errors.
 @returns true on success, false on failure.
 */
uint8_t initializeSeedPeers(SeedPeers * self,void (*onErrorReceived)(Error error,char *,...));

/**
 @brief Converts a Peer Address into NetworkAddress object
 @param seed The Peer Address.
 @param onErrorReceived Events for errors.
 @returns NetworkAddress object.
 */
NetworkAddress* convertAddress(uint32_t seed,void (*onErrorReceived)(Error error,char *,...));

/**
 @brief Gets next Peer Address converted into NetworkAddress object
 @param self The SeedPeers object.
 @param onErrorReceived Events for errors.
 @returns NetworkAddress object.
 */
NetworkAddress* nextPeerAddress(SeedPeers * self,void (*onErrorReceived)(Error error,char *,...));
/**
    * Returns an array containing all the Bitcoin nodes within the list.
 */
/**
 @brief Gets array containing all the Bitcoin nodes converted into NetworkAddress object
 @param self The SeedPeers object.
 @param onErrorReceived Events for errors.
 @returns array of NetworkAddress objects.
 */
NetworkAddress** getAllPeers(SeedPeers * self,void (*onErrorReceived)(Error error,char *,...));

/**
 @brief Destroys a SeedPeers object.
 @param self The SeedPeers object to destroy.
 */
void destroySeedPeers(void * vself);

#endif
