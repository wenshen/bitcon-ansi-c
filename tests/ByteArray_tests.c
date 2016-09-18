/*
 * ByteArray_tests.c
 *
 *  Created on: Oct 21, 2012
 *      Author: Issak Gezehei
 */


#include "../src/Object/Message/Message.h"
#include "../src/Object/ByteArray.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include<time.h>
#include<string.h>


void onErrorReceived(Error a, char * format,...);
void onErrorReceived(Error a, char * format,...){
	va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
	printf("\n");
}

int main(int argc, char *argv[])
{

	    unsigned int s = (unsigned int)time(NULL);
		printf("Session = %ui\n",s);
		srand(s);

		char * string = "Hello World!";
		ByteArray * ba = createNewByteArrayFromData((uint8_t *)string, (uint32_t)strlen(string), onErrorReceived);

		char * a = (char *)getByteArrayData(ba);


		if (strcmp(string,a))
		{
			printf("STRING COPY FAIL\n");
			return 1;
		}
		return 0;


}
