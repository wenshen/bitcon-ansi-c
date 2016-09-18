/*
* Constants.h
*
*  Created on: 26/10/2012
*  Created by: Attila - Peter Toth
*  Modified by:
*  Copyright (c) 2012 Bitcoin Project Team
*/

/*
 * @file
 * @brief Currently we are focusing only on IPv4 and IPv6 addresses, the hidden services
 * are on the waiting list.
 */

#ifndef NETWORKFUNCTIONS_H_
#define NETWORKFUNCTIONS_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "../../Constants.h"

/*
 * brief Everything is in network byte order. IPv4 addresses are represented in IPv4 mapped IPv6 format. The IPv4Start
 * is part of the mapping. Uses arrays of unsigned integer of length of 8 bits.
 */
static const uint8_t IPv4Start[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xFF, 0xFF};
static const uint8_t SITTStart[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0xFF, 0xFF, 0, 0};
static const uint8_t RFC6052Start[12] = {0, 0x64, 0xFF, 0x9B, 0, 0, 0, 0, 0, 0, 0, 0};

/**
 @brief Determines the type of IP, including validity.
 @param IP The IP address to check.
 @returns The IP type.
 */
IPType getIPType(uint8_t * IP);
/**
 @brief Determines if an IP address is an GarliCat I2P hidden service representation
 @param IP The IP address to check.
 @returns true if the IP address is an I2P address, false otherwise.
 */
int isI2P(uint8_t * IP);
/**
 @brief Determines if an IP address is an IPv4 address
 @param IP The IP address to check.
 @returns true if the IP address is IPv4, false otherwise.
 */
int isIPv4(uint8_t * IP);
/**
 @brief Determines if an IP address is an OnionCat Tor hidden service representation
 @param IP The IP address to check.
 @returns true if the IP address is a Tor address, false otherwise.
 */
int isTor(uint8_t * IP);

#endif
