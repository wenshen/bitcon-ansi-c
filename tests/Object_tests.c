/*
 * Object_tests.c
 *
 *  Created on: Oct 21, 2012
 *      Author: apopoola
 */

#include "../src/Object/Object.h"
#include <assert.h>

int main(int argc, char *argv[])
{
	Object *obj = createNewObject();
	assert(obj->referenceCount == 1);

	/*test incrementReferenceCount*/
	incrementReferenceCount(obj);
	assert(obj->referenceCount == 2);

	/*test decrementReferenceCount*/
	decrementReferenceCount(obj);
	assert(obj->referenceCount==1);

	destroyObject(obj);

	return 0;
}
