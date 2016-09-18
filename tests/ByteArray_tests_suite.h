/******----------------------- File Name: ByteArray_tests_suite.h ---------------------------******

 Created by: Issak on 05/11/2012.
 Tested by: Full Name on DD/MM/YYYY.
 Modified by: Fadwa Alseiari & Senka Kojic on 21/11/2012.
 Version: 1.0
 Copyright (c) 2012, MIBitcoinc


 Brief Description:
 ------------------
 This file includes all the test cases related to an ByteArray. It inlcudes functions:
 test_case_createNewByteArrayFromData , test_case_getByteArray , test_case_copySubsectionOfByteArrayToByteArray , test_case_destroyByteArray , test_case_readInt16AsLittleEndianFromByteArray,
 test_case_readInt32AsLittleEndianFromByteArray , test_case_readInt64AsLittleEndianFromByteArray , test_case_compareByteArrays,
 test_case_copyByteArray , test_case_copyByteArrayToByteArray , test_case_copySubsectionOfByteArrayToByteArray ,
 test_case_createNewByteArrayFromSubsection , test_case_getByteArraySubsectionReference , test_case_getByteFromByteArray ,
 test_case_getByteArrayData , test_case_getLastByteFromByteArray , test_case_isNullByteArray, test_case_setBytesInByteArray ,
 test_case_writeInt16AsLittleEndianIntoByteArray , test_case_writeInt32AsLittleEndianIntoByteArray,
 test_case_writeInt64AsLittleEndianIntoByteArray , test_case_reverseBytes , test_case_changeByteArrayDataReference

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
#include "../src/Object/Message/Message.h"
#include "../src/Constants.h"
#include <string.h>

/* Test Suite setup and cleanup functions: */

int init_ByteArray_suite(void) {
        return 0;
}

int clean_ByteArray_suite(void) {
        return 0;
}

/*  onErrorReceived functions: */
void onErrorReceived(Error a, char * format, ...);
/**
 @fn void onErrorReceived(Error a,char * format,...)
 @brief Error alert message printing
 @param a
 @param format
 @return void
 */
void onErrorReceived(Error a, char * format, ...) {
        va_list argptr;
        va_start(argptr, format);
        vfprintf(stderr, format, argptr);
        va_end(argptr);
        printf("\n");
}

/*************This is the structure ByteArray******

 *
 @brief Structure for ByteArray objects. @see ByteArray.h


 typedef struct {
 Object base; *< Object base structure
 SharedData * sharedData; *< Underlying byte data
 uint32_t offset; *< Offset from the beginning of the byte data to the beginning of this array
 uint32_t length; *< Length of byte array. Set to _BYTE_ARRAY_UNKNOWN_LENGTH if unknown.
 void (*onErrorReceived)(Error error, char *, ...);
 } ByteArray;*/

/************* Test case functions ****************/

void test_case_createNewByteArrayOfSize(void) {
        uint32_t size = 2;
        ByteArray *byteArray = createNewByteArrayOfSize(size, onErrorReceived);
        CU_ASSERT_EQUAL(byteArray->length, size);
}

void test_case_createNewByteArrayFromData(void) {
        uint8_t *data = 234;
        uint16_t size = 12;
        ByteArray *byteArray = createNewByteArrayFromData(data, size,
                        onErrorReceived);
        CU_ASSERT_PTR_NOT_NULL(byteArray);

}

void test_case_getByteArray(void)
{
        uint8_t *data = 234;
        uint16_t size = 12;
        ByteArray *byteArray = createNewByteArrayFromData(data, size, onErrorReceived);
        CU_ASSERT_PTR_NOT_NULL(getByteArray(byteArray));

}


void test_case_initNewByteArrayFromData(void) {
        uint8_t *data = 234;
        uint16_t size = 12;
        ByteArray *byteArray = createNewByteArrayFromData(data, size,
                        onErrorReceived);
        int initReturn = initializeNewByteArrayFromData(byteArray, data, size,
                        onErrorReceived);
        CU_ASSERT_EQUAL(initReturn, TRUE);
}


void test_case_destroyByteArray(void) {
}

void test_case_releaseByteArraySharedDataReference(void) {
}

void test_case_readInt16AsLittleEndianFromByteArray(void) {
}

void test_case_readInt32AsLittleEndianFromByteArray(void) {
}

void test_case_readInt64AsLittleEndianFromByteArray(void) {
}

void test_case_compareByteArrays(void) {
        uint32_t size = 2;
        ByteArray * byteArray1 = createNewByteArrayOfSize(size, onErrorReceived);
        size = 4;
        ByteArray *byteArray2 = createNewByteArrayOfSize(size, onErrorReceived);
        CU_ASSERT_NOT_EQUAL(byteArray1, byteArray2);

}

void test_case_copyByteArray(void){
 /*uint32_t size =2;
 ByteArray *byteArray = createNewByteArrayOfSize(size, onErrorReceived);
 ByteArray *copyByteArr = copyByteArray(byteArray);
 CU_ASSERT_EQUAL(byteArray, copyByteArr);*/

 uint32_t size =2;
                ByteArray *byteArray = createNewByteArrayOfSize(size, onErrorReceived);
                ByteArray *copyByteArr = copyByteArray(byteArray);
                int x;
                for (x = 0; x < byteArray->length; x++ )
                {
                        printf("original byte: %d\n", byteArray->sharedData->data);
                }

                for (x = 0; x < copyByteArr->length; x++)
                {
                printf("copy byte: %d\n", copyByteArr->sharedData->data);
                }

                CU_ASSERT_EQUAL(byteArray->sharedData->referenceCount, copyByteArr->sharedData->referenceCount);
                CU_ASSERT_EQUAL(byteArray->length, copyByteArr->length);

 }

void test_case_copyByteArrayToByteArray(void) {
        uint32_t writeOffset = 12;
        uint32_t size = 32;
        ByteArray *source = createNewByteArrayOfSize(size, onErrorReceived);
}

void test_case_copySubsectionOfByteArrayToByteArray(void) {

}

void test_case_createNewByteArrayFromSubsection(void) {

}

void test_case_getByteArraySubsectionReference(void) {

}

void test_case_getByteFromByteArray(void) {

}

void test_case_getByteArrayData(void) {

}

void test_case_getLastByteFromByteArray(void) {

}

void test_case_isNullByteArray(void) {

}

void test_case_setBytesInByteArray(void) {

}

void test_case_writeInt16AsLittleEndianIntoByteArray(void) {

}

void test_case_writeInt32AsLittleEndianIntoByteArray(void) {

}

void test_case_writeInt64AsLittleEndianIntoByteArray(void) {

}

void test_case_reverseBytes(void) {

}

void test_case_changeByteArrayDataReference(void) {

}

/* Grouping all test cases into an array that will be registered later with a single test suite. */
CU_TestInfo byteArray_test_array[] = {
/*------------------------------- Already Implemented---------------*/

                { "test_case_createNewByteArrayOfSize", test_case_createNewByteArrayOfSize },
                {"test_case_createNewByteArrayFromData", test_case_createNewByteArrayFromData },
                { "test_case_getByteArray",test_case_getByteArray },
                { "test_case_initNewByteArrayFromData", test_case_initNewByteArrayFromData },

                /*------------------------------- Not Implemented---------------*/
                /* { "test_case_copySubsectionOfByteArrayToByteArray", test_case_copySubsectionOfByteArrayToByteArray },
                 { "test_case_destroyByteArray", test_case_destroyByteArray },
                 { "test_case_readInt16AsLittleEndianFromByteArray", test_case_readInt16AsLittleEndianFromByteArray },
                 { "test_case_readInt32AsLittleEndianFromByteArray", test_case_readInt32AsLittleEndianFromByteArray },
                 { "test_case_readInt64AsLittleEndianFromByteArray", test_case_readInt64AsLittleEndianFromByteArray },
                 */

                /*------------------------------- Already Implemented---------------*/
                { "test_case_compareByteArrays", test_case_compareByteArrays },
                { "test_case_copyByteArray", test_case_copyByteArray },
                { "test_case_copyByteArrayToByteArray", test_case_copyByteArrayToByteArray },

                /*------------------------------- Not Implemented---------------*/
                /*{ "test_case_copySubsectionOfByteArrayToByteArray", test_case_copySubsectionOfByteArrayToByteArray },
                 { "test_case_createNewByteArrayFromSubsection", test_case_createNewByteArrayFromSubsection },
                 { "test_case_getByteArraySubsectionReference", test_case_getByteArraySubsectionReference },
                 { "test_case_getByteFromByteArray", test_case_getByteFromByteArray },
                 { "test_case_getByteArrayData", test_case_getByteArrayData },
                 { "test_case_getLastByteFromByteArray", test_case_getLastByteFromByteArray },
                 { "test_case_isNullByteArray", test_case_isNullByteArray },
                 { "test_case_setBytesInByteArray", test_case_setBytesInByteArray },
                 { "test_case_writeInt16AsLittleEndianIntoByteArray", test_case_writeInt16AsLittleEndianIntoByteArray },
                 { "test_case_writeInt32AsLittleEndianIntoByteArray", test_case_writeInt32AsLittleEndianIntoByteArray },
                 { "test_case_writeInt64AsLittleEndianIntoByteArray", test_case_writeInt64AsLittleEndianIntoByteArray },
                 { "test_case_reverseBytes", test_case_reverseBytes },
                 { "test_case_changeByteArrayDataReference", test_case_changeByteArrayDataReference },
                 */CU_TEST_INFO_NULL, };
