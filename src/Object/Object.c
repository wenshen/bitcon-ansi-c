/*
 * Object.c
 *
 *  Created on: 04/10/2012
 *  Created by: Abdulfatai Popoola
 *  Modified by: Wen Shen & Dmytro Krasnoshtan
 *  Copyright (c) 2012 Bitcoin Project Team
 */

/* See header file for documentation */

#include "Object.h"
#include <assert.h>

/*
new object constructor
allocate the memory for the object
checks that it was allocated
links the destructor function to the object
initialize reference to 1
*/
Object * createNewObject(void)
{
	/* initialize the memory of the size of the object */
	Object * self = malloc(sizeof(*self));
	
	/* checking that memory allocation was successful */
	if (!self){
		/* not successful. Go out of the function */
		return NULL;
	}
	
	self->destroy = destroyObject; /*assign destructor function to object*/
	self->dump = dumpObject; /*assign pretty printer function to object*/

	/* initialize reference to 1 */
	self->referenceCount = 1;
	return self;
}

/* retrieves the object; useful for avoiding explicit casts */
Object * getObject(void * self)
{
	assert(self != NULL);

	return self;
}

int setObjectRefCountToOne(Object * self)
{
	assert(self != NULL);

	self->referenceCount = 1;
	return TRUE;
}


/* Removes the object */
void destroyObject(void * self)
{
  /*No need to check for NULL, because it is safe if NULL pointer is passed*/
	free(self);
}

void decrementReferenceCount(void * self)
{
	assert(self != NULL);
  
  /*Cast memory location to Object*/
	Object * object = self;

	/* Decrement reference counter. Destroy if reference count is zero*/
	if(object->referenceCount != NULL){
		object->referenceCount--;
		if (object->referenceCount < 1){
			object->destroy(object);
		}
	}
}

void incrementReferenceCount(void * self)
{
  assert(self != NULL);
  
  /*Cast memory location to Object*/
  Object * object = self;
  
  /* Increment reference counter.*/
  if (object->referenceCount >= 0){
    object->referenceCount++;
  }
}

void dumpObject(void * self)
{
	assert(self != NULL);

	Object * obj = self;

	printf("Structure: OBJECT \n");
	printf("ReferenceCount = %u", obj->referenceCount);
}
