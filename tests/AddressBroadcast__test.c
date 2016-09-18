/*
 * testAddressBroadcast.c
 *
 *  Created on: 21/10/2012
 *  Created by: Hayk Baluyan
 *  Copyright (c) 2012 Bitcoin Project Team
 */

/** @file
 @brief A test case for Address
 @details Test the AddressBroadcast struct
 */


#include <stdio.h>
#include "../src/Object/Network/AddressBroadcast.h"
#include <time.h>


void onErrorReceived(Error a,char * format,...);
void onErrorReceived(Error a,char * format,...){

printf("\n");
}

int main(){
	unsigned int session = (unsigned int)time(NULL);
	session = 1337544566;
	printf("Session = %u\n",session);
	srand(session);
	/* Test deserialisation with timestamps*/
	uint8_t data[61] = {
	0x02, /* Two addresses*/
	0x46,0xAE,0xF4,0x4F, /* Time 1341435462*/
	0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /* _SERVICE_FULL_BLOCKS*/
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x0A,0x00,0x00,0x01, /* IP ::ffff:10.0.0.1*/
	0x20,0x8D, /* Port 8333*/
	0x7E,0xB7,0xF4,0x4F, /* Time 1341437822*/
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /* No services*/
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x24,0x60,0xA2,0x08, /* IP ::ffff:36.96.162.8*/
	0x5F,0x2E, /* Port 24366*/
	};
	ByteArray * bytes = createNewByteArrayUsingDataCopy(data, 61, onErrorReceived);

	printf("\nAddress broadcast as a sequence of 61 bytes (2 Network Addresses):\n");
	int k;
	for(k=0;k<61;k++)
		printf("%u ",bytes->sharedData->data[k]);
	AddressBroadcast * addBroadcast = createNewAddressBroadcastFromSerialisedData(bytes, true, onErrorReceived);

	uint32_t lengthOfAddressBroadcast=deserialiseAddressBroadcast(addBroadcast);
	if(lengthOfAddressBroadcast!=61){

	printf("DESERIALISATION LEN FAIL\n");
	return 1;
	}


	int j;
	for(j=0;j<addBroadcast->addrNum;j++)
	{
		printf("\nIP for Network Address %i: ",j+1);
		for(k=0;k<16;k++)
			printf("%u ",addBroadcast->addresses[j]->ip->sharedData->data[k]);
	}

	if (addBroadcast->addrNum != 2) {
	printf("DESERIALISATION NUM FAIL\n");
	return 1;
	}
	if (addBroadcast->addresses[0]->time != 1341435462) {
	printf("DESERIALISATION FIRST TIME FAIL\n");
	return 1;
	}
	if (addBroadcast->addresses[0]->services != 1) {
	printf("DESERIALISATION FIRST SERVICES FAIL\n");
	return 1;
	}

	if (memcmp(getByteArrayData(addBroadcast->addresses[0]->ip),(uint8_t []){0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x0A,0x00,0x00,0x01},16)) {
		printf("DESERIALISATION FIRST IP FAIL\n0x");
		return 1;
	}
	if (addBroadcast->addresses[0]->port != 8333) {
	printf("DESERIALISATION FIRST PORT FAIL\n");
	return 1;
	}
	if (addBroadcast->addresses[1]->time != 1341437822) {
	printf("DESERIALISATION SECOND TIME FAIL\n");
	return 1;
	}
	if (addBroadcast->addresses[1]->services != 0) {
	printf("DESERIALISATION SECOND SERVICES FAIL\n");
	return 1;
	}
	if (memcmp(getByteArrayData(addBroadcast->addresses[1]->ip),(uint8_t []){0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x24,0x60,0xA2,0x08},16)) {
		printf("DESERIALISATION SECOND IP FAIL\n0x");
		return 1;
	}

	if (addBroadcast->addresses[1]->port != 24366) {
		printf("DESERIALISATION SECOND PORT FAIL\n");
		return 1;
	}

	decrementReferenceCount(addBroadcast->addresses[0]->ip);
	decrementReferenceCount(addBroadcast->addresses[1]->ip);
	decrementReferenceCount(addBroadcast);
	decrementReferenceCount(bytes);

	/* Test deserialisation without timestamps*/
	uint8_t dataWithoutTimestamps[53] = {
	0x02, /*two addresses*/
	0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/* _SERVICE_FULL_BLOCKS*/
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x0A,0x00,0x00,0x01,/* IP ::ffff:10.0.0.1*/
	0x20,0x8D, /*Port 8333*/
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*No services*/
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x24,0x60,0xA2,0x08, /* IP ::ffff:36.96.162.8*/
	0x5F,0x2E, /* Port 24366*/
	};
	bytes = createNewByteArrayUsingDataCopy(dataWithoutTimestamps, 53, onErrorReceived);
	addBroadcast = createNewAddressBroadcastFromSerialisedData(bytes, false, onErrorReceived);

	if(deserialiseAddressBroadcast(addBroadcast) != 53){
		printf("DESERIALISATION NO TIME LEN FAIL\n");
		return 1;
	}
	if (addBroadcast->addrNum != 2) {
		printf("DESERIALISATION NO TIME NUM FAIL\n");
		return 1;
	}
	if (addBroadcast->addresses[0]->services != 1) {
		printf("DESERIALISATION NO TIME FIRST SERVICES FAIL\n");
		return 1;
	}
	if (memcmp(getByteArrayData(addBroadcast->addresses[0]->ip),(uint8_t []){0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x0A,0x00,0x00,0x01},16)) {
		printf("DESERIALISATION NO TIME FIRST IP FAIL\n0x");

		return 1;
	}
	if (addBroadcast->addresses[0]->port != 8333) {
		printf("DESERIALISATION NO TIME FIRST PORT FAIL\n");
		return 1;
	}
	if (addBroadcast->addresses[1]->services != 0) {
		printf("DESERIALISATION NO TIME SECOND SERVICES FAIL\n");
		return 1;
	}
	if (memcmp(getByteArrayData(addBroadcast->addresses[1]->ip),(uint8_t []){0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x24,0x60,0xA2,0x08},16)) {
		printf("DESERIALISATION NO TIME SECOND IP FAIL\n0x");
		uint8_t * d = getByteArrayData(addBroadcast->addresses[1]->ip);

		return 1;
	}
	if (addBroadcast->addresses[1]->port != 24366) {
		printf("DESERIALISATION NO TIME SECOND PORT FAIL\n");
		return 1;
	}

	/* Test serialisation without timestamps*/
	memset(getByteArrayData(bytes), 0, 53);
	decrementReferenceCount(addBroadcast->addresses[0]->ip);
	addBroadcast->addresses[0]->ip = createNewByteArrayUsingDataCopy((uint8_t []){0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x0A,0x00,0x00,0x01}, 16, onErrorReceived);
	decrementReferenceCount(addBroadcast->addresses[1]->ip);
	addBroadcast->addresses[1]->ip = createNewByteArrayUsingDataCopy((uint8_t []){0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x24,0x60,0xA2,0x08}, 16, onErrorReceived);
	if(serialiseAddressBroadcast(addBroadcast) != 53){
		printf("SERIALISATION NO TIME LEN FAIL\n");
		return 1;
	}
	decrementReferenceCount(addBroadcast);
	decrementReferenceCount(bytes);


	return 0;
}

