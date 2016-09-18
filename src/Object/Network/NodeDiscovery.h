/*
*  NodeDiscovery.h
*
*  Created on: 20/10/2012.
*  Created by: Attila Peter Toth.
*  Modified by: 
*  Copyright (c) 2012 Bitcoin Project Team
*/
/**
 @file
 @brief This class is responsible for the node discovery functionality described in the Bitcoin protocol - apart from the fact that the methods which involve IRC procedures are not included, since they are deprecated. Inherits Object
*/

#ifndef NODEDISCOVERY_H_
#define NODEDISCOVERY_H_


#include "../Object.h"
#include "../../Constants.h"
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include "../ByteArray.h"

extern int errno;

/**
 @brief Structure for NodeDiscovery objects. @see NodeDiscovery.h
*/
typedef struct{
	Object base; /**< Object base structure */
} NodeDiscovery;

/**
 @brief Creates a new NodeDiscovery object.
 @returns A new NodeDiscovery object.
 */
NodeDiscovery * newNodeDiscovery(void);

/**
 @brief Initializes a NodeDiscovery object
 @param self The NodeDiscovery object to initialize
 @returns TRUE on success, FALSE on failure.
 */
int initNodeDiscovery(NodeDiscovery * self);

/**
 @brief Gets a NodeDiscovery from an object. Use this to avoid casts.
 @param self The object to obtain the NodeDiscovery from.
 @returns The NodeDiscovery object.
 */
NodeDiscovery * getNodeDiscovery(void * self);

/**
 @brief Destroys an NodeDiscovery object.
 @param self The NodeDiscovery object to free.
 */
void destroyNodeDiscovery(void * self);

/**
 * @brief connects to specified sites and extracts from the response the IP
 * of the local machine. It uses the extractIp(char* httpresp) function to parse
 * the response and getExtIP(char*) to connect to multiple sites.
 */
char* getExtIPFromSites();

/**@author Attila-Peter Toth
 * @brief Connects to a specified host. IPv4 compatible only!!!
 * @returns the IP address in a structure or NULL if any error is encountered.
 */
char* getExtIP(char* dest);

/**
 * @author Hayk Baluyan
 * @brief Extracts a valid IPv4 or IPv6 address from a char*
 * @returns char* for the valid IP address
 */
char* extractIP(char* dest);

/**
 * @author Attila-Peter Toth
 * @brief uses the function getIPInByteArray and getExtIP to find out
 * the IP address of the local computer
 */
ByteArray* getIPByteArray();
/**
 * @author Attila-Peter Toth
 */
ByteArray *getIPInByteArray(char *ip);


#endif /* NODEDISCOVERY_H_ */
