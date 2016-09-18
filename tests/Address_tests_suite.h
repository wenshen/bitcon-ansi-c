/******----------------------- File Name: Address_tests_suite.h ---------------------------******

Created by: Fadwa Alseiari on 05/12/2012.
Tested by: Full Name on DD/MM/YYYY.
Modified by: Full Name on 21/11/2012.
Version: 1.0
Copyright (c) 2012, MIBitcoinc


Brief Description:
------------------
This file includes all the test cases related to Address. It inlcudes functions:
test_case_createNewAddressFromString, test_case_createNewAddressFromRIPEMD160Hash,
test_case_getAddress, test_case_destroyAddress


******--------------------------------------------------------------------******/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "CUnit/CUnit.h"
#include "CUnit/Basic.h"
/*#include "CUnit/TBTest.h"*/
/*#include "CUnit/Automated.h"
 #include "CUnit/Console.h"*/
#include "../src/Object/Network/Address.h"
#include "../src/Constants.h"


/* Test Suite setup and cleanup functions: */

int init_address_suite(void) {
	return 0;
}

int clean_address_suite(void) {
	return 0;
}

/*
void onErrorReceived(Error a, char * format, ...);
void onErrorReceived(Error a, char * format, ...) {
	va_list argptr;
	va_start(argptr, format);
	vfprintf(stderr, format, argptr);
	va_end(argptr);
	printf("\n");
}*/


/************* Test case functions ****************/
void test_case_createNewAddressFromString(void)
{
	/* creating Address of Bitcoin pubkey hash type(starting with 1) */
	ByteArray * addressString = createNewByteArrayFromString("1D5A1q5d192j5gYuWiP3CSE5fcaaZxe6E9", TRUE, onErrorReceived);

	/*int index;
		printf("\nFirst Address String consists of the following %i  bytes:\n",addressString->length);
		for(index=0;index<addressString->length;index++)
			printf("%u ",addressString->sharedData->data[index]);*/



	Address * address = createNewAddressFromString(addressString, FALSE, onErrorReceived);
	CU_ASSERT_PTR_NOT_NULL(address);
	CU_ASSERT_PTR_NULL(getVersionChecksumBytes(address)->cachedString);


		/*printf("\nFirst Address consists of the following %i bytes:\n",address->base.base.length);
		for(index=0;index<address->base.base.length;index++)
			printf("%u ",address->base.base.sharedData->data[index]);*/


	Address * address2 = createNewAddressFromString(addressString, FALSE, onErrorReceived);

	decrementReferenceCount(addressString);
	/*printf("\nSecond Address consists of the following %i bytes:\n",address2->base.base.length);
	for(index=0;index<address2->base.base.length;index++)
		printf("%u ",address2->base.base.sharedData->data[index]);*/


	uint8_t netVersionByte = getNetVersionByteForVersionChecksumBytes(getVersionChecksumBytes(address));
		/*printf("\n\nVersion Byte is the first byte %u",netVersionByte);*/
		CU_ASSERT_EQUAL(netVersionByte, ADDRESS_PUBKEYHASH);


	ByteArray * addressStringFromVersionChecksum = getStringForVersionChecksumBytes(getVersionChecksumBytes(address));
	CU_ASSERT_STRING_EQUAL((char *) getByteArrayData(addressStringFromVersionChecksum), "1D5A1q5d192j5gYuWiP3CSE5fcaaZxe6E9");
	decrementReferenceCount(addressStringFromVersionChecksum);
	decrementReferenceCount(address);
	/*printf("\n\nAddress as a String is: %s",(char *) getByteArrayData(addressStringFromVersionChecksum));
	if (strcmp((char *) getByteArrayData(addressStringFromVersionChecksum), "1D5A1q5d192j5gYuWiP3CSE5fcaaZxe6E9")) {
		printf(	"\nNOT CACHED STRING WRONG %s != 1D5A1q5d192j5gYuWiP3CSE5fcaaZxe6E9\n",
				(char *) getByteArrayData(addressStringFromVersionChecksum));
		return 1;
	}
	decrementReferenceCount(addressStringFromVersionChecksum);
	decrementReferenceCount(address);*/


	/* creating Address of Bitcoin testnet pubkey hash type(starting with 111) */

	ByteArray* addressString3 = createNewByteArrayFromString("mzCk9JXXF9we7MB2Gdt59tcfj6Lr2rSzpu", TRUE, onErrorReceived);
	/*printf("\nThird Address String consists of the following %i  bytes:\n",addressString3->length);
			for(index=0;index<addressString3->length;index++)
				printf("%u ",addressString3->sharedData->data[index]);*/

	Address* address3 = createNewAddressFromString(addressString3, FALSE, onErrorReceived);
	decrementReferenceCount(addressString3);

	/*printf("\nThird Address consists of the following %i bytes:\n",address3->base.base.length);
		for(index=0;index<address3->base.base.length;index++)
			printf("%u ",address3->base.base.sharedData->data[index]);*/


	netVersionByte = getNetVersionByteForVersionChecksumBytes(getVersionChecksumBytes(address3));

	CU_ASSERT_EQUAL(netVersionByte, ADDRESS_TESTNETPUBKEYHASH);
}

void test_case_createNewAddressFromRIPEMD160Hash(void)
{
	uint8_t * hash = malloc(20);
	int x;
	for (x = 0; x < 20; x++)
		hash[x] = rand();
	Address *address = createNewAddressFromRIPEMD160Hash(hash, ADDRESS_PUBKEYHASH, false ,onErrorReceived);
	free(hash);
	uint8_t netVersionByte = getNetVersionByteForVersionChecksumBytes(getVersionChecksumBytes(address));

	CU_ASSERT_EQUAL(netVersionByte, ADDRESS_PUBKEYHASH );

	decrementReferenceCount(address);
	hash = malloc(20);
	for (x = 0; x < 20; x++)
		hash[x] = rand();
	address = createNewAddressFromRIPEMD160Hash(hash, ADDRESS_TESTNETPUBKEYHASH, false, onErrorReceived);
	free(hash);
	netVersionByte = getNetVersionByteForVersionChecksumBytes(getVersionChecksumBytes(address));
	CU_ASSERT_EQUAL(netVersionByte, ADDRESS_TESTNETPUBKEYHASH);

	decrementReferenceCount(address);

	/* Test createNewAddressFromRIPEMD160Hash for pre-known address*/
	hash = malloc(20);

	for (x = 0; x < 20; x++)
		hash[x] = x;
	address = createNewAddressFromRIPEMD160Hash(hash, ADDRESS_PUBKEYHASH, false, onErrorReceived);
	free(hash);
	ByteArray *addressString = getStringForVersionChecksumBytes(getVersionChecksumBytes(address));
	CU_ASSERT_STRING_EQUAL(((char *)getByteArrayData(addressString)), "112D2adLM3UKy4Z4giRbReR6gjWuvHUqB");

	decrementReferenceCount(addressString);
	decrementReferenceCount(address);
}

void test_case_getAddress(void)
{
	ByteArray * addressString = createNewByteArrayFromString("1D5A1q5d192j5gYuWiP3CSE5fcaaZxe6E9", TRUE, onErrorReceived);
	Address * address = createNewAddressFromString(addressString, FALSE, onErrorReceived);
	CU_ASSERT_EQUAL(getAddress(address), address);
}

void test_case_destroyAddress(void)
{
	ByteArray * addressString = createNewByteArrayFromString("1D5A1q5d192j5gYuWiP3CSE5fcaaZxe6E9", TRUE, onErrorReceived);
	Address * address = createNewAddressFromString(addressString, FALSE, onErrorReceived);
	CU_ASSERT_PTR_NOT_NULL(address->base.base.sharedData->data);
	destroyAddress(address);
	CU_ASSERT_PTR_NULL(address->base.base.sharedData->data);

}




/* Grouping all test cases into an array that will be registered later with a single test suite. */
CU_TestInfo address_test_array[] = {
	{ "test_case_createNewAddressFromString", test_case_createNewAddressFromString },
	{ "test_case_createNewAddressFromRIPEMD160Hash", test_case_createNewAddressFromRIPEMD160Hash },
	{"test_case_getAddress", test_case_getAddress},
	{"test_case_destroyAddress", test_case_destroyAddress},
  CU_TEST_INFO_NULL,
};
