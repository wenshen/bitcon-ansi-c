/*
 * ByteArray.c
 *
 *  Created on: 05/10/2012
 *  Created by: Abdulfatai Popoola
 *  Modified by: Wen Shen and Issak Gezehei
 *  Copyright (c) 2012 Bitcoin Project Team
 */


/** @file
 @brief 
 @details 
 */

#include "ByteArray.h"
#include <assert.h>

ByteArray * createNewByteArrayFromString(char * string, int terminator,
		void(*onErrorReceived)(Error error, char *, ...)){
	ByteArray * self = malloc(sizeof(*self));
	if (! self) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory in NewByteArrayFromString\n",sizeof(*self));
		return NULL;
	}
	getObject(self)->destroy = destroyByteArray;
	if(initByteArrayFromString(self,string,terminator,onErrorReceived))
		return self;
	free(self);
	return NULL;
}

ByteArray * createNewByteArraySubsectionReference(ByteArray * ref,uint32_t offset,uint32_t length)
{
	assert(ref != NULL);
	assert(offset >= 0);
	assert(length >= 0);

	ByteArray * self = malloc(sizeof(*self));
	if (!self) {
		ref->onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory in createNewByteArraySubReference\n",sizeof(*self));
		return NULL;
	}

	getObject(self)->destroy = destroyByteArray;
	if(initializeByteArraySubsectionReference(self, ref, offset, length)) {
		return self;
	}

	free(self);
	return NULL;
}

int initByteArrayFromString(ByteArray * self, char * string,
		int terminator, void(*onErrorReceived)(Error error, char *, ...)){
	self->onErrorReceived = onErrorReceived;

	self->length = (uint32_t)(strlen(string) + terminator);
	self->sharedData = malloc(sizeof(*self->sharedData));
	if (! self->sharedData) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory in InitByteArrayFromString for the sharedData structure.\n",sizeof(*self->sharedData));
		return FALSE;
	}
	self->sharedData->data = malloc(self->length);
	if (! self->sharedData->data) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory in InitByteArrayFromString for the shared data.\n",self->length);
		return FALSE;
	}
	self->sharedData->referenceCount = 1;
	self->offset = 0;
	memmove(self->sharedData->data, string, self->length);

	return TRUE;
}

/**
 @fn void copyByteArrayToByteArray(ByteArray * self, uint32_t writeOffset, ByteArray * source)
 @brief copy a ByteArray To another ByteArray
 @param self
 @param writeOffset
 @param source
 */

void copyByteArrayToByteArray(ByteArray * self, uint32_t writeOffset, ByteArray * source)
{
	assert(self != NULL);
	assert(writeOffset >=0); /*writeOffset is a position in the array and array indices start at 0*/
	assert(source != NULL);
	printf("we r here 7th.\n");

	printf("source lengthta: %d\n", source->length );
	memmove(self->sharedData->data + self->offset + writeOffset, source->sharedData->data + source->offset, source->length);
	printf("we r here 6th.\n");
}

/**
 @fn int isNullByteArray(ByteArray * self)
 @brief Checks if the byte array is null or not
 @param self
 @return TRUE/FALSE
 */
int isNullByteArray(ByteArray * self)
{
	uint32_t i;
	for (i=0; i < self->length; i++) {
		if (self->sharedData->data[self->offset+i] + i) {
			return FALSE;
		}
	}
	return TRUE;
}


/**
 @fn ByteArray * getByteArray(void * self))
 @brief returns a ByteArray when given a ByteArray
 @param self
 @return self
 */
ByteArray * getByteArray(void * self)
{
	assert(self != NULL);

	return self;
}

/**
 @fn uint8_t * getDataFromByteArray(ByteArray * self)
 @brief gets data from ByteArray
 @param self
 @return data
 */
uint8_t * getByteArrayData(ByteArray * self)
{
	assert(self != NULL);
	return self->sharedData->data + self->offset;
}


/**
 @fn uint8_t getByteFromByteArray(ByteArray * self, uint32_t index)
 @brief gets Byte from ByteArray
 @param self
 @param self
 @param index
 @return uint8_t
 */
uint8_t getByteFromByteArray(ByteArray * self, uint32_t index)
{
	assert(self != NULL);
	assert(index >=0); /*index is a position in the array and array indices start at 0*/

	return self->sharedData->data[self->offset + index];
}

/**
 @fn ByteArray * getByteArraySubsectionReference(ByteArray * refByteArray, uint32_t offset, uint32_t length)
 @brief gets ByteArray Subsection Reference
 @param refByteArray
 @param offset
 @param length
 @return NULL
 */
ByteArray * getByteArraySubsectionReference(ByteArray * refByteArray, uint32_t offset, uint32_t length)
{
	assert(refByteArray != NULL);
	assert(offset >=0); /*offset is a position in the array and array indices start at 0*/
	assert(length >=0); /*length should be at least 0*/

	ByteArray * self = malloc(sizeof(*self));

	if (!self) {
		refByteArray->onErrorReceived(
				ERROR_OUT_OF_MEMORY,
				"Cannot allocate %i bytes of memory in getByteArraySubsectionReference\n",
				sizeof(*self));
		return NULL;
	}
   
   /*assign destructor function to object */
	getObject(self)->destroy = destroyByteArray; 

	if (initializeByteArraySubsectionReference(self, refByteArray, offset, length)) {
		return self;
	}
   
   /* Initialization failed; Clean up everything and return NULL */ 
	free(self);
	return NULL;
}




/**
 @fn int initializeByteArraySubsectionReference(ByteArray * self, ByteArray * refByteArray, uint32_t offset, uint32_t length)
 @brief initializes ByteArray Subsection Reference
 @param self
 @param refByteArray
 @param offset
 @param length
 @return TRUE/FALSE
 */
int initializeByteArraySubsectionReference(ByteArray * self, ByteArray * refByteArray, uint32_t offset, uint32_t length)
{
	assert(self != NULL);
	assert(refByteArray != NULL);
	assert( offset >=0); /*offset is a position in the array and array indices start at 0*/
	assert(length >=0); /*length should be at least 0 */

	Object *obj = getObject(self);
	if (obj== NULL){
		return FALSE;
	}
	obj->referenceCount = 1;

	self->onErrorReceived = refByteArray->onErrorReceived;
	self->sharedData = refByteArray->sharedData;
   
   /* Increase reference count as this is a new pointer to the sharedData */
	self->sharedData->referenceCount++;
   
   /* If length is 0, set to the reference length */
	self->length = length ? length : refByteArray->length; 
   
	self->offset = refByteArray->offset + offset;

	return TRUE;
}

/**
 @fn void destroyByteArray(void * self)
 @brief destorys byte array
 @param self
 */
void destroyByteArray(void * self)
{
	assert(self != NULL);

	releaseByteArraySharedDataReference(self);
	destroyObject(getObject(self));
}


/**
 @fn void releaseByteArraySharedDataReference(ByteArray * self)
 @brief release Byte Array SharedData Reference
 @param self
 */
void releaseByteArraySharedDataReference(ByteArray * self)
{
	assert(self != NULL);

	self->sharedData->referenceCount--;

	if (self->sharedData->referenceCount < 1) {
   
   /* Shared data now owned by no one; free up the memory */
		free(self->sharedData->data);
		free(self->sharedData);
	}
}

/**
 @fn uint16_t readInt16AsLittleEndianFromByteArray(ByteArray * self,	uint32_t offset)
 @brief read an int 16 As little Endian from ByteArray
 @param self
 @param offset
 @return result
 */
uint16_t readInt16AsLittleEndianFromByteArray(ByteArray * self,	uint32_t offset)
{

/*Network order is BIG-ENDIAN so you have to convert this else you have issues such as wrong data and the NUXI problem */
	assert(self != NULL);
	assert(offset >=0); /*offset is a position in the array and array indices start at 0*/

	uint16_t result = self->sharedData->data[self->offset + offset];
   /*Shift it 8 bits to the left and do a bitwise OR with the earlier read value*/
	result |= (uint16_t) self->sharedData->data[self->offset + offset + 1] << 8; 

	return result;
}

/**
 @fn uint32_t readInt32AsLittleEndianFromByteArray(ByteArray * self, uint32_t offset)
 @brief read an int 32 As little Endian from ByteArray
 @param self
 @param offset
 @return result
 */
uint32_t readInt32AsLittleEndianFromByteArray(ByteArray * self, uint32_t offset)
{
	assert(self != NULL);
	assert(offset >=0); /*offset is a position in the array and array indices start at 0*/

	uint32_t result = self->sharedData->data[self->offset + offset];

	result |= (uint32_t) self->sharedData->data[self->offset + offset + 1] << 8;
	result |= (uint32_t) self->sharedData->data[self->offset + offset + 2] << 16;
	result |= (uint32_t) self->sharedData->data[self->offset + offset + 3] << 24;

	return result;
}


/**
 @fn uint32_t readInt64AsLittleEndianFromByteArray(ByteArray * self, uint32_t offset)
 @brief read an int 64 As little Endian from ByteArray
 @param self
 @param offset
 @return result
 */

uint64_t readInt64AsLittleEndianFromByteArray(ByteArray * self, uint32_t offset)
{
	assert(self != NULL);
	assert(offset >=0); /*offset is a position in the array and array indices start at 0*/

	uint64_t result = self->sharedData->data[self->offset + offset];

	result |= (uint64_t) self->sharedData->data[self->offset + offset + 1] << 8;
	result |= (uint64_t) self->sharedData->data[self->offset + offset + 2] << 16;
	result |= (uint64_t) self->sharedData->data[self->offset + offset + 3] << 24;
	result |= (uint64_t) self->sharedData->data[self->offset + offset + 4] << 32;
	result |= (uint64_t) self->sharedData->data[self->offset + offset + 5] << 40;
	result |= (uint64_t) self->sharedData->data[self->offset + offset + 6] << 48;
	result |= (uint64_t) self->sharedData->data[self->offset + offset + 7] << 56;

	return result;
}


/**
 @fn void setByteInByteArray(ByteArray * self, uint32_t index, uint8_t byte)
 @brief sets Byte In ByteArray
 @param self
 @param index
 @param byte
 */
void setByteInByteArray(ByteArray * self, uint32_t index, uint8_t byte)
{

	assert(self != NULL);
	assert(index >=0); /*index is a position in the array and array indices start at 0*/
	/*assert(byte != NULL);*/

	self->sharedData->data[self->offset + index] = byte;
}


/**
 @fn void setByteInByteArray(ByteArray * self, uint32_t index, uint8_t byte)
 @brief sets Bytes In ByteArray
 @param self
 @param index
 @param bytes
 @param length
 */
void setBytesInByteArray(ByteArray * self, uint32_t index, uint8_t * bytes, uint32_t length)
{
	assert(self != NULL);
	assert(index >=0); /*index is a position in the array and array indices start at 0*/
	assert(bytes != NULL);
	assert(length  >=0); /*length should be at least 0*/

	memmove(self->sharedData->data + self->offset + index, bytes, length);
}

/**
 @fn void writeInt16AsLittleEndianIntoByteArray(ByteArray * self, uint32_t offset, uint16_t integer)
 @brief writes Integer_16 as Little-Endian Into ByteArray
 @param self
 @param offset
 @param integer
 */
void writeInt16AsLittleEndianIntoByteArray(ByteArray * self, uint32_t offset, uint16_t integer)
{

/*storing is done in 'BIG-ENDIAN' form */
	assert(self != NULL);
	assert(offset >=0); /*offset is a position in the array and array indices start at 0*/
	assert(integer != NULL);

	self->sharedData->data[self->offset + offset] = integer;
   
   /*Store next set of 8 bits as the next value in the array*/
	self->sharedData->data[self->offset + offset + 1] = integer >> 8; 
}

/**
 @fn void writeInt32AsLittleEndianIntoByteArray(ByteArray * self, uint32_t offset, uint32_t integer)
 @brief writes Integer_32 as Little-Endian Into ByteArray
 @param self
 @param offset
 @param integer
 */

void writeInt32AsLittleEndianIntoByteArray(ByteArray * self, uint32_t offset, uint32_t integer)
{
	assert(self != NULL);
	assert(offset >=0); /*offset is a position in the array and array indices start at 0*/
	assert(integer >= 0);

	self->sharedData->data[self->offset + offset] = integer;
	self->sharedData->data[self->offset + offset + 1] = integer >> 8;
	self->sharedData->data[self->offset + offset + 2] = integer >> 16;
	self->sharedData->data[self->offset + offset + 3] = integer >> 24;
}


/**
 @fn void writeInt64AsLittleEndianIntoByteArray(ByteArray * self, uint32_t offset, uint64_t integer)
 @brief writes Integer_64 as Little-Endian Into ByteArray
 @param self
 @param offset
 @param integer
 */
void writeInt64AsLittleEndianIntoByteArray(ByteArray * self, uint32_t offset, uint64_t integer)
{
	assert(self != NULL);
	assert(offset >=0); /*offset is a position in the array and array indices start at 0*/


	self->sharedData->data[self->offset + offset] = integer;
	self->sharedData->data[self->offset + offset + 1] = integer >> 8;
	self->sharedData->data[self->offset + offset + 2] = integer >> 16;
	self->sharedData->data[self->offset + offset + 3] = integer >> 24;
	self->sharedData->data[self->offset + offset + 4] = integer >> 32;
	self->sharedData->data[self->offset + offset + 5] = integer >> 40;
	self->sharedData->data[self->offset + offset + 6] = integer >> 48;
	self->sharedData->data[self->offset + offset + 7] = integer >> 56;
}


/**
 @fn void changeByteArrayDataReference(ByteArray * self, ByteArray * ref, uint32_t offset)
 @brief changes ByteArray Data's Reference
 @param self
 @param ref
 @param offset
 */
void changeByteArrayDataReference(ByteArray * self, ByteArray * ref, uint32_t offset)
{
/* Release last shared data */
	releaseByteArraySharedDataReference(self); 
	self->sharedData = ref->sharedData;
   /* Since a new reference to the shared data is being made, an increase in the reference count must be made.*/
	self->sharedData->referenceCount++;
   /* New offset for shared data */ 
	self->offset = ref->offset + offset; 
}


/**
 @fn ByteArray * createNewByteArrayOfSize(uint32_t size,void (*onErrorReceived)(Error error,char *,...))
 @brief createNewByteArrayOfSize
 @param self
 @param ref
 @param offset
 */
ByteArray * createNewByteArrayOfSize(uint32_t size,void (*onErrorReceived)(Error error,char *,...)){
	ByteArray * self = malloc(sizeof(*self));
	if (! self) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory in createNewByteArrayOfSize\n",sizeof(*self));
		return NULL;
	}
	getObject(self)->destroy = destroyByteArray;
	if(initByteArrayOfSize(self,size,onErrorReceived))
		return self;
	free(self);
	return NULL;

}


/**
 @fn Compare compareByteArrays(ByteArray * self, ByteArray * second)
 @brief initByteArrayOfSize
 @param self
 @param second
 @return memcmp
  */
Compare compareByteArrays(ByteArray * first, ByteArray * second)
{
	assert(first != NULL);
	assert(second != NULL);

	if (first->length > second->length) {
		return COMPARE_MORE_THAN;
	}
	else if (first->length < second->length) {
		return COMPARE_LESS_THAN;
	}
	return memcmp(getByteArrayData(first), getByteArrayData(second), first->length);
}

/**
 @brief Copies a ByteArray
 @param self The ByteArray object to copy
 @returns The new ByteArray.
 */
ByteArray * copyByteArray(ByteArray * self)
{
	ByteArray * new = createNewByteArrayOfSize(self->length,self->onErrorReceived);
	if (! new)
		return NULL;
	memmove(new->sharedData->data,self->sharedData->data + self->offset,self->length);
	return new;
}

/**
 @fn int initByteArrayOfSize(ByteArray * self, uint32_t size, void(*onErrorReceived)(Error error, char *, ...))
 @brief initByteArrayOfSize
 @param self
 @param ref
 @param offset
 */
int initByteArrayOfSize(ByteArray * self, uint32_t size, void(*onErrorReceived)(Error error, char *, ...)){
	Object *obj = getObject(self);
	if (obj== NULL){
		return FALSE;
	}
	obj->referenceCount = 1;
	self->onErrorReceived = onErrorReceived;
	self->length = size;
	self->sharedData = malloc(sizeof(*self->sharedData));
	if (!self->sharedData) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory in initByteArrayOfSize for the sharedData structure.\n",sizeof(*self->sharedData));
		return FALSE;
	}
	self->sharedData->data = malloc(size);
	if (!self->sharedData->data) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory in initByteArrayOfSize for the shared data.\n",size);
		return FALSE;
	}
	self->sharedData->referenceCount = 1;
	self->offset = 0;
	return TRUE;
}

/**
 @fn ByteArray * createNewByteArrayFromData(uint8_t * data, uint32_t size, void(*onErrorReceived)(Error error, char *, ...))
 @brief createNewByteArrayFromData
 @param self
 @param ref
 @param offset
 @return NULL
 */
ByteArray * createNewByteArrayFromData(uint8_t * data, uint32_t size,
		void(*onErrorReceived)(Error error, char *, ...)) {
	ByteArray * self = malloc(sizeof(*self));
	if (! self) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory in createNewByteArrayFromData\n",sizeof(*self));
		return NULL;
	}
	getObject(self)->destroy = destroyByteArray;
	if(initializeNewByteArrayFromData(self, data, size, onErrorReceived))
		return self;
	free(self);
	return NULL;

}

/**
 @fn ByteArray * createNewByteArrayUsingDataCopy(uint8_t * data, uint32_t size, void(*onErrorReceived)(Error error, char *,...))
 @brief createNewByteArrayUsingDataCopy
 @param self
 @param ref
 @param offset
 @return NULL
 */
ByteArray * createNewByteArrayUsingDataCopy(uint8_t * data,uint32_t size,void (*onErrorReceived)(Error error,char *,...)){
	assert(data != NULL && size !=NULL );
	ByteArray * self = malloc(sizeof(*self));
	if (!self) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory in createNewByteArrayUsingDataCopy\n",sizeof(*self));
		return NULL;
	}
	getObject(self)->destroy = destroyByteArray;
	if(initNewByteArrayUsingDataCopy(self, data, size, onErrorReceived))
		return self;
	free(self);
	return NULL;
}

/**
 @fn int initNewByteArrayUsingDataCopy(ByteArray * self, uint8_t * data, uint32_t size, void(*onErrorReceived)(Error error, char *, ...))
 @brief initNewByteArrayUsingDataCopy
 @param self
 @param data
 @param size
 @return TRUE/FALSE
 */
int initNewByteArrayUsingDataCopy(ByteArray * self, uint8_t * data,
		uint32_t size, void(*onErrorReceived)(Error error, char *, ...)){
	Object *obj = getObject(self);
	if (obj== NULL){
		return FALSE;
	}
	obj->referenceCount = 1;

	self->onErrorReceived = onErrorReceived;
	self->sharedData = malloc(sizeof(*self->sharedData));
	if (! self->sharedData) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory in initNewByteArrayUsingDataCopy for the sharedData structure.\n",sizeof(*self->sharedData));
		return FALSE;
	}
	self->sharedData->data = malloc(size);
	if (! self->sharedData->data) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory in initNewByteArrayUsingDataCopy for the shared data.\n",size);
		return FALSE;
	}
	memmove(self->sharedData->data,data,size);
	self->sharedData->referenceCount = 1;
	self->length = size;
	self->offset = 0;
	return TRUE;
}


/**
 @fn int initNewByteArrayFromData(ByteArray * self,uint8_t * data, uint32_t size, void (*onErrorReceived)(Error error,char * ,...))
 @brief createNewByteArrayUsingDataCopy
 @param self
 @param ref
 @param offset
 @return TRUE/FALSE
 */
int initializeNewByteArrayFromData(ByteArray * self,uint8_t * data, uint32_t size, void (*onErrorReceived)(Error error,char *,...)){
	assert(self != NULL && data !=NULL && size != NULL && onErrorReceived != NULL);

	Object *obj = getObject(self);
	if (obj== NULL){
		return FALSE;
	}
	obj->referenceCount = 1;
	self->onErrorReceived = onErrorReceived;
	self->sharedData = malloc(sizeof(*self->sharedData));
	if (! self->sharedData) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,"Cannot allocate %i bytes of memory in initNewByteArrayFromData for the sharedData structure.\n",sizeof(*self->sharedData));
		return TRUE;
	}
	self->sharedData->data = data;
	self->sharedData->referenceCount = 1;
	self->length = size;
	self->offset = 0;
	return TRUE;
}

/* added by Hayk Baluyan */
void reverseBytes(ByteArray * self){
	int x;
	for (x = 0; x < self->length / 2; x++) {
		uint8_t temp = self->sharedData->data[self->offset+x];
		self->sharedData->data[self->offset+x] = self->sharedData->data[self->offset+self->length-x-1];
		self->sharedData->data[self->offset+self->length-x-1] = temp;
	}
}

ByteArray * createNewByteArrayFromSubsection(ByteArray * self,uint32_t offset,uint32_t length){
	ByteArray * new = createNewByteArrayOfSize(length,self->onErrorReceived);
	if (new)
	{

		memcpy(new->sharedData->data,self->sharedData->data + self->offset + offset,length);

	}


	return new;
}
