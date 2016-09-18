/*
 * Message_tests.c
 *
 *  Created on: Oct 21, 2012
 *      Author: apopoola
 */

#include "../src/Object/Message/Message.h"
#include "../src/Object/ByteArray.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

void onErrorReceived(Error a, char * format, ...);
void onErrorReceived(Error a, char * format, ...) {
	va_list argptr;
	va_start(argptr, format);
	vfprintf(stderr, format, argptr);
	va_end(argptr);
	printf("\n");
}

int main(int argc, char *argv[]) {
	/*TESTS for object with NULL ByteArray data */
	Message *msg = createNewMessage(onErrorReceived);

	assert(msg->bytes == NULL);
	assert(msg->onErrorReceived == onErrorReceived);
	assert(msg->expectResponse == FALSE);

	assert(getObject(msg)->referenceCount == 1);

	destroyMessage(msg);

	/*TESTS for object containing byte Array data */
	ByteArray *byteArray = createNewByteArrayOfSize(32, onErrorReceived);
	Message *msg2 = createNewMessage(onErrorReceived);
	assert(setByteArrayInMessage(msg2, byteArray, onErrorReceived) == TRUE);
	assert(msg2->bytes != NULL);
	assert(msg2->onErrorReceived == onErrorReceived);
	assert(msg2->expectResponse == FALSE);

	assert(getObject(msg2)->referenceCount == 1);

	destroyObject(msg2);
	destroyObject(byteArray);

	return 0;
}
