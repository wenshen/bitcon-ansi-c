/*
 * AssociativeArray.h
 *
 * Created on: Nov 26, 2012
 * Created by: Wen Shen
 * Copyright (c) 2012 MIBitcoin Project Team
 */

/**
 @file
 @brief Stores keys and values with insert and search functions. The keys and values must all be of equal length. Useful for producing indexes.
 */

#ifndef ASSOCIATIVEARRAY_H_
#define ASSOCIATIVEARRAY_H_
#include "../Constants.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/**
 @brief A node for a B-Tree. After this data should come the keys and the data elements.
 */
typedef struct{
	void * parent; /**< The parent node */
	void * children[BTREE_ORDER + 1]; /**< Children nodes */
	uint8_t numElements; /**< The number of elements */
	uint8_t * elements[BTREE_ORDER]; /**< The elements cointaining key-value pairs and can be dereferenced to uint8_t for access to the keys */
} BTreeNode;

/**
 @brief Describes the result of a find operation
 */
typedef struct{
	bool found; /**< True if found, false otherwise. */
	BTreeNode * node; /**< If found, this is the node with the data, otherwise it is the node where the data should be inserted. */
	uint8_t pos; /**< If found, this is the element with the data, otherwise it is the position where the data should be inserted. */
} FindResult;

/**
 @brief @see AssociativeArray.h
 */
typedef struct{
	uint8_t keySize; /**< The size of the keys */
	BTreeNode * root; /**< The root of the B-tree */
} AssociativeArray;

/**
 @brief Deletes an element from an array.
 @param self The array object
 @param pos The result from AssociativeArrayFind which determines the position to delete data.
 @returns true on success and false on failure.
 */
bool associativeArrayDelete(AssociativeArray * self, FindResult pos);
/**
 @brief Finds data for a key in the array
 @param self The array object
 @param key The key to search for.
 @returns The result of the find.
 */
FindResult associativeArrayFind(AssociativeArray * self, uint8_t * key);
/**
 @brief Inserts an element into an array.
 @param self The array object.
 @param key The key-value pair to insert.
 @param pos The result from AssociativeArrayFind which determines the position to insert data.
 @oaram right The child to the right of the value we are inserting, which will be a new child from a split or NULL for a new value.
 @returns true on success and false on failure.
 */
bool associativeArrayInsert(AssociativeArray * self, uint8_t * keyValue, FindResult pos, BTreeNode * right);
/**
 @brief Does a binary search on a B-tree node.
 @param self The node
 @param key Key to search for.
 @param keySize The size of keys.
 @returns The position and wether or not the key exists at this position.
 */
FindResult bTreeNodeBinarySearch(BTreeNode * self, uint8_t * key, uint8_t keySize);
/**
 @brief Frees an associative array.
 @param self The array object.
 */
void freeAssociativeArray(AssociativeArray * self);
/**
 @brief Frees a B-tree node.
 @param self The node
 */
void freeBTreeNode(BTreeNode * self);
/**
 @brief Initialises an empty associative array.
 @param self The array object
 @param keySize The size of the keys for the array in bytes.
 @returns true on success and false on failure.
 */
bool initAssociativeArray(AssociativeArray * self, uint8_t keySize);

#endif /* ASSOCIATIVEARRAY_H_ */
