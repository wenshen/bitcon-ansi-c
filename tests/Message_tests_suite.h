/******----------------------- File Name: Message_tests_suite.h ---------------------------******

Created by: Issak on 05/11/2012.
Tested by: Full Name on DD/MM/YYYY.
Modified by: Fadwa Alseiari & Senka Kojic on 15/11/2012.
Version: 1.0
Copyright (c) 2012, MIBitcoinc


Brief Description:
------------------
This file includes all the test cases related to an Message. It inlcudes functions:
test_case_createNewMessage, test_case_getMessage, test_case_setByteArrayInMessage,
test_case_destroyMessage
******--------------------------------------------------------------------******/

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "CUnit/CUnit.h"
#include "CUnit/Basic.h"
/*#include "CUnit/TBTest.h"*/
/*#include "CUnit/Automated.h"
 #include "CUnit/Console.h"*/
#include "../src/Object/Message/Message.h"
#include "../src/Object/Object.h"


/* Test Suite setup and cleanup functions: */

int init_Msg_suite(void) {
        return 0;
}

int clean_Msg_suite(void) {
        return 0;
}


/**
 @struct Message
 @brief Structure for Message objects. @see Message.h

typedef struct Message{
        Object base; *< Object base structure
        MessageType type; *< The type of the message
        uint8_t * altText; *< For an alternative message: This is the type text.
        ByteArray * bytes; *< Raw message data minus the message header. When serialising this should be assigned to a ByteArray large enough to hold the serialised data.
        uint8_t checksum[4]; *< The message checksum. When sending messages using a NetworkCommunicator, this is calculated for you.
        void (*onErrorReceived)(Error error,char *,...); *< Pointer to error callback
        MessageType expectResponse; *< Set to zero if no message expected or the type of message expected as a response.
        short int serialized;
} Message;*/

/************* Test case functions ****************/

void test_case_createNewMessage(void){
        Message *message = createNewMessage(onErrorReceived);
        CU_ASSERT_PTR_NOT_NULL(message);
        CU_ASSERT_EQUAL(message->onErrorReceived, onErrorReceived);
        CU_ASSERT_EQUAL(message->expectResponse, FALSE);
        CU_ASSERT_EQUAL(getObject(message)->referenceCount, 1);

}

void test_case_getMessage(void)
{
        Message *message = createNewMessage(onErrorReceived);
        CU_ASSERT_PTR_NOT_NULL(getMessage(message));

}


void test_case_setByteArrayInMessage(void){
        Message *message = createNewMessage(onErrorReceived);
        uint32_t size = 2;
        ByteArray *data = createNewByteArrayOfSize(size, onErrorReceived);
        setByteArrayInMessage(message, data, onErrorReceived);
        CU_ASSERT_EQUAL(message->bytes->length, data->length);

}


void test_case_destroyMessage(void){
        Message *message = createNewMessage(onErrorReceived);
        uint32_t size = 2;
        ByteArray *data = createNewByteArrayOfSize(size, onErrorReceived);
        setByteArrayInMessage(message, data, onErrorReceived);
        int refCount = getObject(message->bytes)->referenceCount;
        destroyMessage(message);
        CU_ASSERT_EQUAL(getObject(data)->referenceCount, refCount-1);

}
/* Grouping all test cases into an array that will be registered later with a single test suite. */
                CU_TestInfo message_test_array[] = {

                { "test_case_createNewMessage", test_case_createNewMessage },
                { "test_case_getMessage", test_case_getMessage },
                { "test_case_setByteArrayInMessage", test_case_setByteArrayInMessage },
                { "test_case_destroyMessage", test_case_destroyMessage },
                CU_TEST_INFO_NULL,
                };
