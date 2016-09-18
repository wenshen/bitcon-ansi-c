/*
* Constants.h
*
*  Created on: 26/10/2012
*  Created by: Attila - Peter Toth
*  Modified by:
*  Copyright (c) 2012 Bitcoin Project Team
*/

#include "NetworkFunctions.h"

IPType getIPType(uint8_t * IP){

	uint8_t x = 0;
	uint8_t loc_rfc[8] = {0xFE,0x80,0,0,0,0,0,0};
	/* Shifts the start address to the left by 3 and check if it violates the
	 * protocol. It is an old bitcoin protocol rule.*/
	if (! memcmp(IP, IPv4Start+3, 9))
        return IP_INVALID;

	/*check for unspecified addresses*/
	for (; x < 16; x++)
		if (IP[x])	break;

	/*The address consists only of zeros -> invalid IP address*/
	if (x == 16)
		return IP_INVALID;
	/*Check x first to see that the first byte was not zero.
	 *Just pronErrorReceived unnecessary memory comparisons.*/
	if(!x && isTor(IP))
		return IP_TOR;

	if(! x && isI2P(IP))
		return IP_I2P;

	if (isIPv4(IP)) {
		/*modifications regarding Matthew's code: he test only the first byte to be
		 * equal to 127 or 0 then he returns as a result loopback*/
		if (IP[12] == 0x7F && IP[13] == 0x0 && IP[14] == 0x0 && IP[15] == 0x1)
			/* Loopback address */
			return IP_IPv4 | IP_LOCAL;
		/* if address starts with 10.x.x.x == class A private network;
		 * or if network starts with 12.163 => don't know why or what is it, but invalid
		 * if address is between 172.16.x.x - 172.31.x.x == class B private network. */
		if(IP[12] == 0x0A || (IP[12] == 0xC0 && IP[13] == 0xA3) || (IP[12] == 0xAC && (IP[13] >= 0x10 && IP[13] <= 0x1F)))
			/*Private network*/
			return IP_INVALID;
		/* 169.254.x.x == Link-local reserved address*/
		if (IP[12] == 0xA9 && IP[13] == 0xFE)
			/*Link-local*/
			return IP_INVALID;
		/*Zero network, only valid as a source network*/
		if (IP[12] == 0x00 && IP[13] == 0x00 && IP[14] == 0x00 && IP[15] == 0x00)
			return IP_INVALID;
		/*255.255.255.255 == Broadcast address.*/
		if (IP[12] == 0xFF && IP[13] == 0xFF && IP[14] == 0xFF && IP[15] == 0xFF)
			return IP_INVALID;
		return IP_IPv4;
	}
	if (! memcmp(IP, SITTStart, 12)) {
		return IP_IPv6 | IP_SITT;
	}
	if (! memcmp(IP, RFC6052Start, 12)) {
		return IP_IPv6 | IP_RFC6052;
	}
	if (IP[0] == 0x20 && IP[1] == 0x01 && IP[2] == 0 && IP[3] == 0) {
		return IP_IPv6 | IP_TEREDO;
	}
	if (IP[0] == 0x20 && IP[1] == 0x02) {
		return IP_IPv6 | IP_6TO4;
	}
	if (IP[0] == 0x20 && IP[1] == 0x11 && IP[2] == 0x04 && IP[3] == 0x70) {
		return IP_IPv6 | IP_HENET;
	}
	/* 0:0:0:0:0:0:0:1 == localhost*/
	if (x == 15 && IP[15] == 1)
		return IP_LOCAL | IP_IPv6;
	/* Documentation*/
	if (IP[0] == 0x20 && IP[1] == 0x01 && IP[2] == 0x0D && IP[3] == 0xB8)
		return IP_INVALID;
	/*Link-Local RFC4862 reserved addresses*/
	if (! memcmp(IP, loc_rfc, 8))
		return IP_INVALID;
	/* RFC4843 addresses*/
	if (IP[0] == 0x20 && IP[1] == 0x01 && IP[2] == 0x00 && (IP[3] & 0xF0) == 0x10)
		return IP_INVALID;
	/* Reserved Unicast addresses*/
	if ((IP[0] & 0xFE) == 0xFC)

		return IP_INVALID;
	return IP_IPv6;
}

int isI2P(uint8_t * IP){
	uint8_t i2p[6] = {0xFD,0x60,0xDB,0x4D,0xDD,0xB5};
	return ! memcmp(IP, i2p, 6);
}

/*tests if the address properly implements the IPv4 to IP_IPv6 mapping*/
int isIPv4(uint8_t * IP){
	return ! memcmp(IP, IPv4Start, 12);
}
int isTor(uint8_t * IP){
	uint8_t tor[6] = {0xFD,0x87,0xD8,0x7E,0xEB,0x43};
	return  !memcmp(IP, tor, 6);
}
