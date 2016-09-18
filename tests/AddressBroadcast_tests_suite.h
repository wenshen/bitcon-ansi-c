/******----------------------- File Name: AddressBroadcast_tests_suite.h ---------------------------******

 Created by: Senka Kojic on 07/12/2012.
 Tested by: Full Name on DD/MM/YYYY.
 Modified by:
 Version: 1.0
 Copyright (c) 2012, MIBitcoinc


 Brief Description:
 ------------------
 This file includes all the test cases related to AddressBroadcast. It includes functions:
 test_case_createNewAddressBroadcast", test_case_createNewAddressBroadcast,
 test_case_createNewAddressBroadcastFromSerialisedData", test_case_createNewAddressBroadcastFromSerialisedData,
 test_case_initializeAddressBroadcast", test_case_initializeAddressBroadcast
 test_case_initializeAddressBroadcastFromSerialisedData", test_case_initializeAddressBroadcastFromSerialisedData
 test_case_destroyAddressBroadcast", test_case_destroyAddressBroadcast
 test_case_addNetworkAddressToAddressBroadcast", test_case_addNetworkAddressToAddressBroadcast
 test_case_deserialiseAddressBroadcast", test_case_deserialiseAddressBroadcast
 test_case_lengthOfAddressBroadcast", test_case_lengthOfAddressBroadcast
 test_case_serialiseAddressBroadcast", test_case_serialiseAddressBroadcast
 test_case_addressBroadcastTakeNetworkAddress", test_case_addressBroadcastTakeNetworkAddress
 test_case_Deserialization_with_timestamps", test_case_Deserialization_with_timestamps
 test_case_Deserialization_without_timestamps", test_case_Deserialization_without_timestamps
 test_case_Serialization_with_timestamps", test_case_Serialization_with_timestamps

 ******--------------------------------------------------------------------******/
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "CUnit/CUnit.h"
#include "CUnit/Basic.h"
/*#include "CUnit/TBTest.h"*/
/*#include "CUnit/Automated.h"
 #include "CUnit/Console.h"*/
#include "../src/Object/ByteArray.h"
#include "../src/Constants.h"
#include <string.h>
#include "../src/Object/Network/AddressBroadcast.h"
#include "../src/Object/Network/AddressBroadcast.c"
#include "../src/Object/Network/NetworkAddress.h"
#include <time.h>

/* Test Suite setup and cleanup functions: */

int init_AddressBroadcast_suite(void) {
        return 0;
}

int clean_AddressBroadcast_suite(void) {
        return 0;
}

/*  onErrorReceived functions: */
void err5(Error a, char * format, ...);
/**
 @fn void onErrorReceived(Error a,char * format,...)
 @brief Error alert message printing
 @param a
 @param format
 @return void
 */
void err5(Error a, char * format, ...) {
        va_list argptr;
        va_start(argptr, format);
        vfprintf(stderr, format, argptr);
        va_end(argptr);
        printf("\n");
}

/*brief Structure for AddressBroadcast objects. see AddressBroadcast.h
typedef struct{
	Message base; /**< Message base structure */
	/*uint8_t timeStamps; /**< If TRUE, timestamps are included with the NetworkAddresses */
	/*uint8_t addrNum; /**< Number of addresses. Maximum is 30. */
	/*NetworkAddress ** addresses; /**< List of NetworkAddresses shared so that peers can find new peers. */
/*} AddressBroadcast;*/


/************* Test case functions ****************/
void test_case_createNewAddressBroadcast(void) {

	AddressBroadcast *self = createNewAddressBroadcast(FALSE,err5);
	CU_ASSERT(self);
	CU_ASSERT(!self->addrNum);
	CU_ASSERT(!self->addresses);
	CU_ASSERT(!self->timeStamps);
	/*sets all to 0 in case without timestamps*/

	AddressBroadcast *self_timestamp = createNewAddressBroadcast(TRUE,err5);
	CU_ASSERT(self_timestamp);
	CU_ASSERT(!self_timestamp->addrNum);
	CU_ASSERT(!self_timestamp->addresses);
	CU_ASSERT( self_timestamp->timeStamps);
	/*sets all to 0, except timestamps: self_timestamp->timeStamps=1 in case with timestamps*/
}


void test_case_createNewAddressBroadcastFromSerialisedData(void) {
	uint8_t data[53] = {
			0x02, /*two addresses*/
			0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/* _SERVICE_FULL_BLOCKS*/
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x0A,0x00,0x00,0x01,/* IP ::ffff:10.0.0.1*/
			0x20,0x8D, /*Port 8333*/
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*No services*/
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x24,0x60,0xA2,0x08, /* IP ::ffff:36.96.162.8*/
			0x5F,0x2E, /* Port 24366*/
			};
	ByteArray * bytes = createNewByteArrayUsingDataCopy(data, 53, err5);
	AddressBroadcast *self = createNewAddressBroadcastFromSerialisedData(bytes,FALSE,err5);
	CU_ASSERT(self);
	CU_ASSERT(!self->addrNum);
	CU_ASSERT(!self->addresses);
	CU_ASSERT(!self->timeStamps);
	/*sets all to 0 in case without timestamps*/

	uint8_t data_timestamp[61] = {
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
	ByteArray * bytes_timestamp = createNewByteArrayUsingDataCopy(data_timestamp, 61, err5);
	AddressBroadcast *self_timestamp = createNewAddressBroadcastFromSerialisedData(bytes,TRUE,err5);
	CU_ASSERT(self_timestamp);
	CU_ASSERT(!self_timestamp->addrNum);
	CU_ASSERT(!self_timestamp->addresses);
	CU_ASSERT( self_timestamp->timeStamps);
	/*sets all to 0, except timestamps: self_timestamp->timeStamps=1 in case with timestamps*/


}

void test_case_initializeAddressBroadcast(void) {


	AddressBroadcast *self = createNewAddressBroadcast(FALSE,err5);
	CU_ASSERT(initializeAddressBroadcast(self,FALSE,err5));
	CU_ASSERT(!self->addrNum);
	CU_ASSERT(!self->addresses);
	CU_ASSERT(!self->timeStamps);

	AddressBroadcast *self_timestamp = createNewAddressBroadcast(TRUE,err5);
	CU_ASSERT(initializeAddressBroadcast(self_timestamp,TRUE,err5));
	CU_ASSERT(!self_timestamp->addrNum);
	CU_ASSERT(!self_timestamp->addresses);
	CU_ASSERT(self_timestamp->timeStamps);
	/* it again initializes all values to 0, except timestamps which are 1 in case with timestamps*/
}

void test_case_initializeAddressBroadcastFromSerialisedData(void) {
	uint8_t data[53] = {
				0x02, /*two addresses*/
				0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/* _SERVICE_FULL_BLOCKS*/
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x0A,0x00,0x00,0x01,/* IP ::ffff:10.0.0.1*/
				0x20,0x8D, /*Port 8333*/
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*No services*/
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x24,0x60,0xA2,0x08, /* IP ::ffff:36.96.162.8*/
				0x5F,0x2E, /* Port 24366*/
				};
    ByteArray * bytes = createNewByteArrayUsingDataCopy(data, 53, err5);
	AddressBroadcast *self = createNewAddressBroadcast(FALSE,err5);
	CU_ASSERT(initializeAddressBroadcastFromSerialisedData(self,FALSE, bytes,err5));
    CU_ASSERT(!self->addrNum);
	CU_ASSERT(!self->addresses);
	CU_ASSERT(!self->timeStamps);
/*not completely clear why function initializes them to 0*/
	uint8_t data_timestamp[61] = {
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
	    ByteArray * bytes_timestamp = createNewByteArrayUsingDataCopy(data_timestamp, 61, err5);
		AddressBroadcast *self_timestamp = createNewAddressBroadcast(TRUE,err5);
		CU_ASSERT(initializeAddressBroadcastFromSerialisedData(self_timestamp,TRUE, bytes_timestamp,err5));
	    CU_ASSERT(!self_timestamp->addrNum);
		CU_ASSERT(!self_timestamp->addresses);
		CU_ASSERT(self_timestamp->timeStamps);
}


void test_case_destroyAddressBroadcast(void) {
	uint8_t a, b;
		int d, e, f,g,l;
		uint8_t data[53] = {
							0x02, /*two addresses*/
							0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/* _SERVICE_FULL_BLOCKS*/
							0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x0A,0x00,0x00,0x01,/* IP ::ffff:10.0.0.1*/
							0x20,0x8D, /*Port 8333*/
							0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*No services*/
							0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x24,0x60,0xA2,0x08, /* IP ::ffff:36.96.162.8*/
							0x5F,0x2E, /* Port 24366*/
							};
		ByteArray * bytes = createNewByteArrayUsingDataCopy(data, 53, err5);
		AddressBroadcast *self = createNewAddressBroadcast(FALSE,err5);
		NetworkAddress *newAddress = createNewNetworkAddressFromSerialisedData(bytes, err5);
		f=getObject(newAddress)->referenceCount;
		a=addNetworkAddressToAddressBroadcast(self,newAddress);

		l=getObject(newAddress)->referenceCount;
		destroyAddressBroadcast(self);
		g=getObject(newAddress)->referenceCount;
		CU_ASSERT_EQUAL(l-1,g);

/*destroybroadcast(self) decremented ref.count of newAddress for 1, numAddr is 1 so this seems correct*/

}

void test_case_addNetworkAddressToAddressBroadcast(void) {
	uint8_t a, b;
	int d, e, f,g;
	uint8_t data[53] = {
						0x02, /*two addresses*/
						0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/* _SERVICE_FULL_BLOCKS*/
						0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x0A,0x00,0x00,0x01,/* IP ::ffff:10.0.0.1*/
						0x20,0x8D, /*Port 8333*/
						0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*No services*/
						0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x24,0x60,0xA2,0x08, /* IP ::ffff:36.96.162.8*/
						0x5F,0x2E, /* Port 24366*/
						};
	ByteArray * bytes = createNewByteArrayUsingDataCopy(data, 53, err5);
	AddressBroadcast *self = createNewAddressBroadcast(FALSE,err5);
	NetworkAddress *newAddress = createNewNetworkAddressFromSerialisedData(bytes, err5);
	f=getObject(newAddress)->referenceCount;
	d=self->addrNum;
	a=addNetworkAddressToAddressBroadcast(self,newAddress);
	e=self->addrNum;
	CU_ASSERT_EQUAL(d, e-1);

	g=getObject(newAddress)->referenceCount;
	CU_ASSERT_EQUAL(f, g-1); /*why this ref. count increments?*/

}

void test_case_deserialiseAddressBroadcast(void) {

	uint8_t data[53] = {
				0x02, /*two addresses*/
				0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/* _SERVICE_FULL_BLOCKS*/
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x0A,0x00,0x00,0x01,/* IP ::ffff:10.0.0.1*/
				0x20,0x8D, /*Port 8333*/
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*No services*/
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x24,0x60,0xA2,0x08, /* IP ::ffff:36.96.162.8*/
				0x5F,0x2E, /* Port 24366*/
				};
		ByteArray * Bytes = createNewByteArrayUsingDataCopy(data, 53, err5);
		AddressBroadcast *self = createNewAddressBroadcastFromSerialisedData(Bytes,FALSE,err5);
        CU_ASSERT_EQUAL(deserialiseAddressBroadcast(self), 53);

        /* Checking function itself*/
		ByteArray * bytes = getMessage(self)->bytes;
		CU_ASSERT(bytes);
		CU_ASSERT(bytes->length > 26 + self->timeStamps * 4);
		VarLenInt num = decodeVarLenInt(bytes, 0);
		CU_ASSERT(num.value < 30);
		self->addresses = malloc(sizeof(*self->addresses) * (size_t)num.value);
		CU_ASSERT_NOT_EQUAL(self->addresses, NULL);

		self->addrNum = num.value;
		uint16_t cursor = num.storageSize;

		uint8_t x ;
			for (x = 0; x < num.value; x++) {
				/* Make new NetworkAddress from the rest of the data.*/
				uint8_t len;

		ByteArray * data = createNewByteArrayFromSubsection(bytes, cursor, bytes->length-cursor);
		CU_ASSERT(data);
		self->addresses[x] = createNewNetworkAddressFromSerialisedData(data, getMessage(self)->onErrorReceived);
		CU_ASSERT(self->addresses[x]);
		len = deserialiseNetworkAddress(self->addresses[x], self->timeStamps);
		CU_ASSERT_NOT_EQUAL(len, NULL);
		data->length = len;
		decrementReferenceCount(data);
		cursor += len;
			}
		CU_ASSERT_EQUAL(cursor,53);

}

void test_case_lengthOfAddressBroadcast(void) {
uint8_t b;
uint8_t data[53] = {
				0x02, /*two addresses*/
				0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/* _SERVICE_FULL_BLOCKS*/
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x0A,0x00,0x00,0x01,/* IP ::ffff:10.0.0.1*/
				0x20,0x8D, /*Port 8333*/
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*No services*/
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x24,0x60,0xA2,0x08, /* IP ::ffff:36.96.162.8*/
				0x5F,0x2E, /* Port 24366*/
				};
ByteArray * Bytes = createNewByteArrayUsingDataCopy(data, 53, err5);
AddressBroadcast *self = createNewAddressBroadcastFromSerialisedData(Bytes,FALSE,err5);

lengthOfAddressBroadcast(self);
CU_ASSERT(lengthOfAddressBroadcast(self));

b=getSizeOfVarLenInt((self->addrNum) + self->addrNum * (self->timeStamps ? 30 : 26));/*am not clear about this size, b=1, i added brackets*/
CU_ASSERT(b);
}

void test_case_serialiseAddressBroadcast(void) {
	uint32_t a;
	uint8_t data[53] = {
					0x02, /*two addresses*/
					0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/* _SERVICE_FULL_BLOCKS*/
					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x0A,0x00,0x00,0x01,/* IP ::ffff:10.0.0.1*/
					0x20,0x8D, /*Port 8333*/
					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*No services*/
					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x24,0x60,0xA2,0x08, /* IP ::ffff:36.96.162.8*/
					0x5F,0x2E, /* Port 24366*/
					};
	ByteArray * Bytes = createNewByteArrayUsingDataCopy(data, 53, err5);
	AddressBroadcast *self = createNewAddressBroadcastFromSerialisedData(Bytes,FALSE,err5);

	a= deserialiseAddressBroadcast(self);/*This incremented values addrNum, num and num.value for 1 each, why? Without is, it does not go into the loop below*/
	ByteArray * bytes = getMessage(self)->bytes;
	CU_ASSERT_EQUAL(serialiseAddressBroadcast(self), 53);
	CU_ASSERT(bytes);
	/*Checking function itself*/
	CU_ASSERT(bytes->length > (26 + self->timeStamps * 4) * self->addrNum);
	VarLenInt num = createVarLenIntFromUInt64(self->addrNum);
	encodeVarLenInt(bytes, 0, num);
	uint16_t cursor = num.storageSize;
	uint8_t x;
		for (x = 0; x < num.value; x++) {
			getMessage(self->addresses[x])->bytes = createNewByteArrayFromSubsection(bytes, cursor, bytes->length-cursor);
			uint32_t len = serialiseNetworkAddress(self->addresses[x],self->timeStamps);
			CU_ASSERT(len);
			getMessage(self->addresses[x])->bytes->length = len;
			cursor += len;
		}

  CU_ASSERT_EQUAL(cursor, 53);

}


void test_case_addressBroadcastTakeNetworkAddress(void) {
uint8_t a, b;
int d, e, f,g, h;
uint8_t data[53] = {
						0x02, /*two addresses*/
						0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/* _SERVICE_FULL_BLOCKS*/
						0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x0A,0x00,0x00,0x01,/* IP ::ffff:10.0.0.1*/
						0x20,0x8D, /*Port 8333*/
						0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*No services*/
						0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x24,0x60,0xA2,0x08, /* IP ::ffff:36.96.162.8*/
						0x5F,0x2E, /* Port 24366*/
						};
ByteArray * bytes = createNewByteArrayUsingDataCopy(data, 53, err5);
AddressBroadcast *self = createNewAddressBroadcast(FALSE,err5);
NetworkAddress *newAddress = createNewNetworkAddressFromSerialisedData(bytes, err5);

f=getObject(newAddress)->referenceCount;
d=self->addrNum;
a=addNetworkAddressToAddressBroadcast(self,newAddress);
e=self->addrNum;
g=getObject(newAddress)->referenceCount;
CU_ASSERT_EQUAL(addressBroadcastTakeNetworkAddress(self,newAddress), TRUE);
h=self->addrNum;
CU_ASSERT_EQUAL(e, h-1);
NetworkAddress ** temp = realloc(self->addresses, sizeof(*self->addresses) * self->addrNum);
CU_ASSERT(temp);
CU_ASSERT_EQUAL(self->addresses,temp);
CU_ASSERT_EQUAL(self->addresses[self->addrNum-1],newAddress);

}




void test_case_Deserialization_with_timestamps(void) {

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
		ByteArray * Bytes = createNewByteArrayUsingDataCopy(data, 61, err5);
		AddressBroadcast *self = createNewAddressBroadcastFromSerialisedData(Bytes,TRUE,err5);
		CU_ASSERT_EQUAL(deserialiseAddressBroadcast(self), 61);


		/* Testing function itself*/
		ByteArray * bytes = createNewByteArrayUsingDataCopy(data, 61, err5);
		AddressBroadcast * addBroadcast = createNewAddressBroadcastFromSerialisedData(bytes, TRUE, err5);
		uint32_t lengthOfAddressBroadcast=deserialiseAddressBroadcast(addBroadcast);
		CU_ASSERT_EQUAL(lengthOfAddressBroadcast, 61);

		CU_ASSERT_EQUAL(addBroadcast->addrNum, 2);
		CU_ASSERT_EQUAL(addBroadcast->addresses[0]->time, 1341435462);
		CU_ASSERT_EQUAL(addBroadcast->addresses[0]->services, 1);
		CU_ASSERT_EQUAL(memcmp(getByteArrayData(addBroadcast->addresses[0]->ip),(uint8_t []){0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x0A,0x00,0x00,0x01},16), 0);
		CU_ASSERT_EQUAL(addBroadcast->addresses[0]->port, 8333);
		CU_ASSERT_EQUAL(addBroadcast->addresses[1]->time, 1341437822);
		CU_ASSERT_EQUAL(addBroadcast->addresses[1]->services, 0);
		CU_ASSERT_EQUAL(memcmp(getByteArrayData(addBroadcast->addresses[1]->ip),(uint8_t []){0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x24,0x60,0xA2,0x08},16), 0);
        CU_ASSERT_EQUAL(addBroadcast->addresses[1]->port, 24366);

        uint32_t referenceCountb,referenceCounta;

        referenceCountb=getObject(addBroadcast->addresses[0]->ip)->referenceCount;
        decrementReferenceCount(addBroadcast->addresses[0]->ip);
        referenceCounta=getObject(addBroadcast->addresses[0]->ip)->referenceCount;
        CU_ASSERT_EQUAL(referenceCountb,referenceCounta+1);

        referenceCountb=getObject(addBroadcast->addresses[1]->ip)->referenceCount;
        decrementReferenceCount(addBroadcast->addresses[1]->ip);
        referenceCounta=getObject(addBroadcast->addresses[1]->ip)->referenceCount;
        CU_ASSERT_EQUAL(referenceCountb,referenceCounta+1);

        referenceCountb=getObject(addBroadcast)->referenceCount;
        decrementReferenceCount(addBroadcast);
        referenceCounta=getObject(addBroadcast)->referenceCount;
        CU_ASSERT_EQUAL(referenceCountb,referenceCounta+1);


        referenceCountb=getObject(bytes)->referenceCount;
        decrementReferenceCount(bytes);
        referenceCounta=getObject(bytes)->referenceCount;
        CU_ASSERT_EQUAL(referenceCountb,referenceCounta+1);
}

void test_case_Deserialization_without_timestamps(void) {
	uint8_t data[53] = {
		0x02, /*two addresses*/
		0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/* _SERVICE_FULL_BLOCKS*/
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x0A,0x00,0x00,0x01,/* IP ::ffff:10.0.0.1*/
		0x20,0x8D, /*Port 8333*/
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /*No services*/
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x24,0x60,0xA2,0x08, /* IP ::ffff:36.96.162.8*/
		0x5F,0x2E, /* Port 24366*/
		};
	ByteArray * Bytes = createNewByteArrayUsingDataCopy(data, 53, err5);
	AddressBroadcast *self = createNewAddressBroadcastFromSerialisedData(Bytes,FALSE,err5);
	CU_ASSERT_EQUAL(deserialiseAddressBroadcast(self), 53);

	/*Testing function itself*/
	ByteArray * bytes = createNewByteArrayUsingDataCopy(data, 53, err5);
	bytes = createNewByteArrayUsingDataCopy(data, 53, err5);
	AddressBroadcast * addBroadcast = createNewAddressBroadcastFromSerialisedData(bytes, FALSE, err5);
	addBroadcast = createNewAddressBroadcastFromSerialisedData(bytes, FALSE, err5);
	CU_ASSERT_EQUAL(deserialiseAddressBroadcast(addBroadcast), 53);
	CU_ASSERT_EQUAL(addBroadcast->addrNum, 2);
	CU_ASSERT_EQUAL(addBroadcast->addresses[0]->services, 1);
	CU_ASSERT_EQUAL(memcmp(getByteArrayData(addBroadcast->addresses[0]->ip),(uint8_t []){0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x0A,0x00,0x00,0x01},16), 0);
    CU_ASSERT_EQUAL(addBroadcast->addresses[0]->port, 8333);
    CU_ASSERT_EQUAL(addBroadcast->addresses[1]->services, 0);
    CU_ASSERT_EQUAL(memcmp(getByteArrayData(addBroadcast->addresses[1]->ip),(uint8_t []){0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x24,0x60,0xA2,0x08},16), 0);
    CU_ASSERT_EQUAL(addBroadcast->addresses[1]->port, 24366);
}

void test_case_Serialization_with_timestamps(void) {
uint32_t a;
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
ByteArray * Bytes = createNewByteArrayUsingDataCopy(data, 61, err5);
AddressBroadcast *self = createNewAddressBroadcastFromSerialisedData(Bytes,TRUE,err5);
a= deserialiseAddressBroadcast(self);/*This incremented values addrNum, num and num.value for 1 each, why? Without is, it does not go into the loop below*/
ByteArray * bytes = getMessage(self)->bytes;
CU_ASSERT_EQUAL(serialiseAddressBroadcast(self), 61);

}


/* Grouping all test cases into an array that will be registered later with a single test suite. */

CU_TestInfo addressbroadcast_test_array[] = {


		        { "test_case_createNewAddressBroadcast", test_case_createNewAddressBroadcast},
		        { "test_case_createNewAddressBroadcastFromSerialisedData", test_case_createNewAddressBroadcastFromSerialisedData},
		        { "test_case_initializeAddressBroadcast", test_case_initializeAddressBroadcast},
		        { "test_case_initializeAddressBroadcastFromSerialisedData", test_case_initializeAddressBroadcastFromSerialisedData},
		        { "test_case_destroyAddressBroadcast", test_case_destroyAddressBroadcast},
		        { "test_case_addNetworkAddressToAddressBroadcast", test_case_addNetworkAddressToAddressBroadcast},
		        { "test_case_deserialiseAddressBroadcast", test_case_deserialiseAddressBroadcast},
		        { "test_case_lengthOfAddressBroadcast", test_case_lengthOfAddressBroadcast},
		        { "test_case_serialiseAddressBroadcast", test_case_serialiseAddressBroadcast},
		        { "test_case_addressBroadcastTakeNetworkAddress", test_case_addressBroadcastTakeNetworkAddress},
		        { "test_case_Deserialization_with_timestamps", test_case_Deserialization_with_timestamps },
                { "test_case_Deserialization_without_timestamps", test_case_Deserialization_without_timestamps},
                { "test_case_Serialization_with_timestamps", test_case_Serialization_with_timestamps},
                CU_TEST_INFO_NULL, };
