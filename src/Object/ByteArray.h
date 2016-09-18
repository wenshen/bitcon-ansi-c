/*
 * ByteArray.h
 *
 *  Created on: 03/10/2012
 *  Created by: Abdulfatai Popoola
 *  Modified by: Wen Shen
 *  Copyright (c) 2012 Bitcoin Project Team
 */

#ifndef BYTEARRAY_H_
#define BYTEARRAY_H_

#include "Object.h"
#include <stdint.h>
#include <string.h>
#include "../Constants.h"
#include <stdbool.h>

/**
 @brief Stores byte data that can be shared amongst many ByteArrays
 */
typedef struct {
	uint8_t * data; /**< Pointer to byte data */
	uint32_t referenceCount; /**< References to this data */
} SharedData;

/**
 @brief Structure for ByteArray objects. @see ByteArray.h
 */
typedef struct {
	Object base; /**< Object base structure */
	SharedData * sharedData; /**< Underlying byte data */
	uint32_t offset; /**< Offset from the beginning of the byte data to the beginning of this array */
	uint32_t length; /**< Length of byte array. Set to _BYTE_ARRAY_UNKNOWN_LENGTH if unknown. */
	void (*onErrorReceived)(Error error, char *, ...);
} ByteArray;

/**
 @brief Creates a ByteArray object from a C string. The termination character is not included in the new ByteArray.
 @param string The string to put into a ByteArray.
 @param terminator If true, include the termination character.
 @param onErrorReceived Engine for errors.
 @returns A new ByteArray object.
 */
ByteArray * createNewByteArrayFromString(char * string, int terminator,
		void(*onErrorReceived)(Error error, char *, ...));
/**
 @brief Creates an empty ByteArray object.
 @param size Size in bytes for the new array.
 @param onErrorReceived Engine for errors.
 @returns An empty ByteArray object.
 */
ByteArray * createNewByteArrayOfSize(uint32_t size,
		void(*onErrorReceived)(Error error, char *, ...));
/**
 @brief Creates a new ByteArray using data.
 @param data The data. This should be dynamically allocated. The new ByteArray object will take care of it's memory management so do not free this data once passed into this constructor.
 @param size Size in bytes for the new array.
 @param onErrorReceived Engine for errors.
 @returns The new ByteArray object.
 */
ByteArray * createNewByteArrayFromData(uint8_t * data, uint32_t size,
		void(*onErrorReceived)(Error error, char *, ...));
/**
 @brief Creates a new ByteArray using data which is copied.
 @param data The data. This data is copied.
 @param size Size in bytes for the new array.
 @param onErrorReceived Engine for errors.
 @returns The new ByteArray object.
 */
ByteArray * createNewByteArrayUsingDataCopy(uint8_t * data, uint32_t size,
		void(*onErrorReceived)(Error error, char *, ...));
/**
 @brief Creates a ByteArray object referencing another ByteArrayObject
 @param ref The ByteArray to reference.
 @param offset The offset to the start of the reference in the reference ByteArray.
 @param length The length of the new ByteArray. If 0 the length is set to be the same as the reference ByteArray.
 @returns An empty ByteArray object.
 */
ByteArray * createNewByteArraySubsectionReference(ByteArray * ref,uint32_t offset,uint32_t length);

/**
 @brief Gets a ByteArray from another object. Use this to avoid casts.
 @param self The object to obtain the ByteArray from.
 @returns The ByteArray object.
 */
ByteArray * getByteArray(void * self);

/**
 @brief Initialises a ByteArray object from a C string. The termination character is not included in the new ByteArray.
 @param self The ByteArray object to initialise
 @param string The string to put into a ByteArray.
 @param terminator If tru, include the termination character.
 @param onErrorReceived Engine for errors.
 @returns true on success, false on failure.
 */
int initByteArrayFromString(ByteArray * self, char * string,
		int terminator, void(*onErrorReceived)(Error error, char *, ...));
/**
 @brief Initialises an empty ByteArray object
 @param self The ByteArray object to initialise
 @param size Size in bytes for the new array.
 @param onErrorReceived Engine for errors.
 @returns true on success, false on failure.
 */
int initByteArrayOfSize(ByteArray * self, uint32_t size,
		void(*onErrorReceived)(Error error, char *, ...));
/**
 @brief Creates a new ByteArray using data.
 @param self The ByteArray object to initialize
 @param data The data. This should be dynamically allocated. The new ByteArray object will take care of it's memory management so do not free this data once passed into this constructor.
 @param size Size in bytes for the new array.
 @param onErrorReceived Engine for errors.
 @returns true on success, false on failure.
 */
int initializeNewByteArrayFromData(ByteArray * self, uint8_t * data,
		uint32_t size, void(*onErrorReceived)(Error error, char *, ...));
/**
 @brief Creates a new ByteArray using data which is copied.
 @param self The ByteArray object to initialize
 @param data The data. This data is copied.
 @param size Size in bytes for the new array.
 @param onErrorReceived Engine for errors.
 @returns true on success, false on failure.
 */
int initNewByteArrayUsingDataCopy(ByteArray * self, uint8_t * data,
		uint32_t size, void(*onErrorReceived)(Error error, char *, ...));
/**
 @brief Initializes a reference ByteArray to a subsection of an ByteArray.
 @param self The ByteArray object to initialize.
 @param refByteArray The ByteArray object to reference.
 @param offset The offset to the start of the reference.
 @param length The length of the reference. If 0 the length is set to be the same as the reference ByteArray.
 @returns true on success, false on failure.
 */
int initializeByteArraySubsectionReference(ByteArray * self,
		ByteArray * refByteArray, uint32_t offset, uint32_t length);

/**
 @brief Destroys a ByteArray object.
 @param self The ByteArray object to destroy.
 */
void destroyByteArray(void * self);/*destroyByteArray was renamed to destroyByteArray */

/**
 @brief Releases a reference to shared byte data and frees the data if necessary.
 @param self The ByteArray object with the SharedData
 */
void releaseByteArraySharedDataReference(ByteArray * self);

/**
 @brief Reads a 16 bit integer from a ByteArray as little-endian
 @param self The ByteArray object
 @param offset Offset to where to start the read
 @returns A 16 bit integer. This can be cast to a signed integer if reading integer as a signed value.
 */
uint16_t readInt16AsLittleEndianFromByteArray(ByteArray * self,
		uint32_t offset);
/**
 @brief Reads a 32 bit integer from a ByteArray as little-endian
 @param self The ByteArray object
 @param offset Offset to where to start the read
 @returns A 32 bit integer. This can be cast to a signed integer if reading integer as a signed value
 */
uint32_t readInt32AsLittleEndianFromByteArray(ByteArray * self,
		uint32_t offset);
/**
 @brief Reads a 64 bit integer from the ByteArray as little-endian
 @param self The ByteArray object
 @param offset Offset to where to start the read
 @returns A 64 bit integer. This can be cast to a signed integer if reading integer as a signed value
 */
uint64_t readInt64AsLittleEndianFromByteArray(ByteArray * self,
		uint32_t offset);
/**
 @brief Compares a ByteArray to another ByteArray and returns with a Compare value.
 @param self The ByteArray object to compare
 @param second Another ByteArray object to compare with
 @returns If the lengths are different, _COMPARE_MORE_THAN if "self" if longer, else _COMPARE_LESS_THAN. If the bytes are equal _COMPARE_EQUAL, else _COMPARE_MORE_THAN if the first different byte if higher in "self", otherwise _COMPARE_LESS_THAN. The return value can be treated like the return value to memcmp.
 */
Compare compareByteArrays(ByteArray * self, ByteArray * second);
/**
 @brief Copies a ByteArray
 @param self The ByteArray object to copy
 @returns The new ByteArray.
 */
ByteArray * copyByteArray(ByteArray * self);
/**
 @brief Copies another byte array to this byte array.
 @param self The ByteArray object to copy to.
 @param writeOffset The offset to begin writing to in self.
 @param source The ByteArray to copy from.
 */
void copyByteArrayToByteArray(ByteArray * self, uint32_t writeOffset,
		ByteArray * source);
/**
 @brief Copies a section of another byte array to this byte array.
 @param self The ByteArray object.
 @param writeOffset The offset to begin writing
 @param source The ByteArray to copy from.
 @param readOffset The offset of the source array to begin reading.
 @param length The length to copy.
 */
void copySubsectionOfByteArrayToByteArray(ByteArray * self,
		uint32_t writeOffset, ByteArray * source, uint32_t readOffset,
		uint32_t length);
/**
 @brief Copies a subsection of a ByteArray.
 @param self The ByteArray object to copy from.
 @param offset The offset to the start of the copy.
 @param length The length of the copy.
 @returns The new ByteArray.
 */
ByteArray * createNewByteArrayFromSubsection(ByteArray * self,
		uint32_t offset, uint32_t length);

/**
 @brief References a subsection of a ByteArray.
 @param self The ByteArray object to reference.
 @param offset The offset to the start of the reference.
 @param length The length of the reference.
 @returns The new ByteArray.
 */
ByteArray * getByteArraySubsectionReference(ByteArray * self,
		uint32_t offset, uint32_t length);

/**
 @brief Get a byte from the ByteArray object. A byte will be returned from self->offset+index in the underlying data.
 @param self The ByteArray object.
 @param index The index in the array to get the byte from
 @returns The byte
 */
uint8_t getByteFromByteArray(ByteArray * self, uint32_t index);
/**
 @brief Get a pointer to the underlying data starting at self->offset.
 @param self The ByteArray object.
 @returns The pointer
 */
uint8_t * getByteArrayData(ByteArray * self);
/**
 @brief Get the last byte from the ByteArray object. A byte will be returned from self->offset+self->length in the underlying data.
 @param self The ByteArray object.
 @returns The last byte
 */
uint8_t getLastByteFromByteArray(ByteArray * self);
/**
 @brief Determines if a ByteArray is null.
 @param self The ByteArray object.
 @returns true if all bytes are zero, else false.
 */
int isNullByteArray(ByteArray * self);
/**
 @brief Set a byte into the array. This will be set at self->offset+index in the underlying data.
 @param self The ByteArray object.
 @param index The index in the array to set the byte
 @param byte The byte to be set.
 */
void setByteInByteArray(ByteArray * self, uint32_t index, uint8_t byte);
/**
 @brief Copies a length of bytes into the array. This will be set at self->offset+index in the underlying data.
 @param self The ByteArray object.
 @param index The index in the array to start writing.
 @param bytes The pointer to the bytes to be copied.
 @param length The number of bytes to copy.
 */
void setBytesInByteArray(ByteArray * self, uint32_t index, uint8_t * bytes,
		uint32_t length);
/**
 @brief Writes a 16 bit integer to a ByteArray as little-endian
 @param self The ByteArray object
 @param offset Offset to where to start the write
 @param integer The 16 bit integer to set. The argument is an unsigned integer but signed or unsigned integers are OK to pass.
 */
void writeInt16AsLittleEndianIntoByteArray(ByteArray * self,
		uint32_t offset, uint16_t integer);
/**
 @brief Writes a 32 bit integer to a ByteArray as little-endian
 @param self The ByteArray object
 @param offset Offset to where to start the write
 @param integer The 32 bit integer to set. The argument is an unsigned integer but signed or unsigned integers are OK to pass.
 */
void writeInt32AsLittleEndianIntoByteArray(ByteArray * self,
		uint32_t offset, uint32_t integer);
/**
 @brief Writes a 64 bit integer to a ByteArray as little-endian
 @param self The ByteArray object
 @param offset Offset to where to start the write
 @param integer The 64 bit integer to set. The argument is an unsigned integer but signed or unsigned integers are OK to pass.
 */
void writeInt64AsLittleEndianIntoByteArray(ByteArray * self,
		uint32_t offset, uint64_t integer);
/**
 @brief Reverses the bytes.
 @param self The ByteArray object to reverse
 */
void reverseBytes(ByteArray * self);
/**
 @brief Changes the reference of this ByteArray object to reference the underlying data of another ByteArray. Useful for moving byte data into single underlying data by copying the data into a larger ByteArray and then changing the reference to this new larger ByteArray.
 @param self The ByteArray object to change the reference for.
 @param ref The ByteArray object to get the reference from.
 @param offset The offset to start the reference.
 @returns The new ByteArray.
 */
void changeByteArrayDataReference(ByteArray * self, ByteArray * ref,
		uint32_t offset);

#endif /* BYTEARRAY_H_ */
