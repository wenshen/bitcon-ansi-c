/*
 * Object.h
 *
 *  Created on: 03/10/2012
 *  Created by: Abdulfatai Popoola
 *  Modified by: Wen Shen & Dmytro Krasnoshtan
 *  Copyright (c) 2012 Bitcoin Project Team
 */

/** @file
 @brief Basic object which serves as a basic block for all other objects in the system
 @details The Object is the pseudo-object class that provides fundamental methods that are needed by all Bitcoin objects
 It handles reference handling (incrementing and decrementing the number of references to a Object) and the creation and cleaning up
 of memory
 */

#ifndef OBJECT_H_
#define OBJECT_H_

#include <stdint.h>
#include <stdlib.h>
#include "../Constants.h"

/**
 @struct Object
 @brief Base class
 */
typedef struct Object {
	void (*destroy)(void *); /**< Pointer to function for destroying the Object */
	void (*dump)(void *); /**< Pointer to function for printing out the contents the Object */
	uint32_t referenceCount; /**< Number of references to this Object */
} Object;

/**
 @fn Object createNewObject(void)
 @brief Creates a new Object
 @returns Object
 */
Object * createNewObject(void);

/**
 @fn Object * getObject(void * self)
 @brief Casts the pointer to the Object; use this to avoid explicitly casting
 @param self
 @returns Object
 */
Object * getObject(void * self);

/**
 @fn void destroyObject(void * self)
 @brief destructor
 @param self
 @returns void
 */
void destroyObject(void * self);

/**
 @fn void decrementReferenceCount(void * self)
 @brief Decrease the number in the reference counter. (It keeps track of how many objects have been created). When the number goes to 0 -
 	 	the object will be destroyed (assigned to NULL)
 @param self
 @returns void
 */
void decrementReferenceCount(void * self);

/**
 @fn void incrementReferenceCount(void * self)
 @brief Increase the number in the reference counter. (It keeps track of how many objects have been created).
 @param self
 @returns void
 */
void incrementReferenceCount(void * self);

/**
 @brief resets the reference counts to an object to one
 @param self The Object to reset its referenceCount
 @returns This should always returns TRUE.
 */
int setObjectRefCountToOne(Object * self);

/**
 @brief prints the referenceCount for this Object
 @param self The Object to reset its referenceCount
 */
void dumpObject(void * self);


#endif /* OBJECT_H_ */
