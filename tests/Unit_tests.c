/******----------------------- File Name: Unit_tests.c ---------------------------******

Created by: Fadwa Alseiari on 05/11/2012.
Tested by: Full Name on DD/MM/YYYY.
Modified by: Fadwa Alseiari, Senka Kojic, Abdulfatai Popoola on 21/11/2012.
Version: 1.0
Copyright (c) 2012, MIBitcoinc


Brief Description:
------------------
This file used to run all the testing functionalities for all testing suites using
Cunit testing framework.

******--------------------------------------------------------------------******/


#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "CUnit/CUnit.h"
#include "CUnit/Basic.h"
/*#include "CUnit/Automated.h"*/
/* #include "CUnit/Console.h"*/
#include "../src/Object/Object.h"
#include "../src/Object/ByteArray.h"
#include "../src/Object/Message/Message.h"
#include "Object_tests_suite.h"
#include "ByteArray_tests_suite.h"
#include "Message_tests_suite.h"
#include "Block_tests_suite.h"
#include "Transactions_tests_suite.h"
#include "LibevSockets_test_suite.h"
#include "Transactions_general_tests_suite.h"
#include "Address_tests_suite.h"
#include "AddressBroadcast_tests_suite.h"
#include "Alert_tests_suite.h"




/* importing the arrays of test cases created on each test suite*/
extern CU_TestInfo object_test_array[];
extern CU_TestInfo byteArray_test_array[];
extern CU_TestInfo message_test_array[];
extern CU_TestInfo block_test_array[];
extern CU_TestInfo address_test_array[];
extern CU_TestInfo addressbroadcast_test_array[];
extern CU_TestInfo alert_test_array[];
extern CU_TestInfo libevsocket_test_array[];
extern CU_TestInfo transactions_test_array[];
extern CU_TestInfo transactions_general_test_array[];

/* Grouping all the suites into one array */
CU_SuiteInfo suites[] = {
  { "Object_tests_suite", init_suite, clean_suite, object_test_array},
  { "ByteArray_tests_suite", init_suite, clean_suite, byteArray_test_array},
  { "Message_tests_suite", init_suite, clean_suite, message_test_array},
  { "Block_tests_suite", init_Block_suite, clean_Block_suite, block_test_array},
  { "Address_tests_suite", init_address_suite, clean_address_suite, address_test_array},
  { "AddressBroadcast_tests_suite", init_AddressBroadcast_suite, clean_AddressBroadcast_suite, addressbroadcast_test_array},
  { "Alert_tests_suite", init_Alert_suite, clean_Alert_suite, alert_test_array},
  { "LibevSockets_test_suite", init_socket_suite, clean_socket_suite, libevsocket_test_array},
  { "Transactions_tests_suite", init_Transactions_suite, clean_Transactions_suite, transactions_test_array},
  { "Transactions_general_tests_suite", init_Transactions_general_suite, clean_Transactions_general_suite, transactions_general_test_array},
  CU_SUITE_INFO_NULL,
};


/* create the registry*/
void AddTests(void)
{
	assert(NULL != CU_get_registry());
	assert(!CU_is_test_running());

	if(CUE_SUCCESS != CU_register_suites(suites))
	{
		printf(stderr, "Register suites failed - %s", CU_get_error_msg());
		exit(EXIT_FAILURE);
	}
}


/************* Test Runner Code goes here **************/
int main(int argc, char *argv[])
{
	if(CU_initialize_registry())
	{
		printf(stderr, "\nInitialization of Test Registry failed.\n");
		exit(EXIT_FAILURE);
	}
	else
	{
		AddTests();

		/*CU_set_output_filename("Unit_tests_try");*/
		/*CU_list_tests_to_file();*/

		/* Run all tests using the basic interface */
		CU_basic_set_mode(CU_BRM_VERBOSE);
		CU_basic_run_tests();
		printf("\n");
		CU_basic_show_failures(CU_get_failure_list());
		printf("\n\n");

		/*CU_automated_run_tests();*/
		CU_cleanup_registry();
	}

	return 0;

}
