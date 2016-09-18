/******----------------------- File Name: Object_tests_suite.c ---------------------------******

Created by: Fadwa Alseiari on 05/11/2012.
Tested by: Full Name on DD/MM/YYYY.
Modified by: Full Name on DD/MM/YYYY.
Version: 1.0
Copyright (c) 2012, MIBitcoinc


Brief Description:
------------------
This file includes all the test cases related to an Object. It inlcudes functions: test_case_CreateNewObject(),
test_case_getObject(), test_case_destroyObject(), test_case_decrementReferenceCount(), test_case_incrementReferenceCount().


******--------------------------------------------------------------------******/

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "CUnit/CUnit.h"
#include "CUnit/Basic.h"
/*#include "CUnit/Automated.h"*/
/* #include "CUnit/Console.h"*/
#include "../src/Object/Object.h"


/* Test Suite setup and cleanup functions: */

int init_suite(void) {
	return 0;
}

int clean_suite(void) {
	return 0;
}

/************* Test case functions ****************/

void test_case_CreateNewObject(void) {
	Object *obj = createNewObject();
	CU_ASSERT_EQUAL(obj->referenceCount, 1);
}


void test_case_getObject(void){
	Object *obj = createNewObject();
	CU_ASSERT_PTR_NOT_NULL(obj);

}

void test_case_destroyObject(void){
	Object *obj = createNewObject();
	destroyObject(obj);
	CU_ASSERT_PTR_NOT_NULL(obj);

}

void test_case_decrementReferenceCount(void) {
	Object *obj = createNewObject();
	decrementReferenceCount(obj);
	CU_ASSERT_EQUAL(obj->referenceCount,0);
}

void test_case_incrementReferenceCount(void) {
	Object *obj = createNewObject();
	incrementReferenceCount(obj);
	CU_ASSERT_EQUAL(obj->referenceCount,2);
}

/* Grouping all test cases into an array that will be registered later with a single test suite. */
CU_TestInfo object_test_array[] = {
  { "test_case_CreateNewObject", test_case_CreateNewObject },
  { "test_case_getObject", test_case_getObject },
  { "test_case_destroyObject", test_case_destroyObject },
  { "test_case_decrementReferenceCount", test_case_decrementReferenceCount },
  { "test_case_incrementReferenceCount", test_case_incrementReferenceCount },
  CU_TEST_INFO_NULL,
};

