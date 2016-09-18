/*
*  MerkleNode.h
*
*  Created on: 24/10/2012
*  Created by: Fadwa Alseiari
*  Modified by:
*  Copyright (c) 2012 Bitcoin Project Team
*/

/**
 @file
 @brief A structure for a node in a merkle tree.
 */

#ifndef MERKLENODEH
#define MERKLENODEH

#include "../Object/ByteArray.h"

typedef struct MerkleNode MerkleNode;

/**
 @brief @see MerkleNode.h
 */
struct MerkleNode {

	uint8_t hash[32]; /**< The hash for this node. */
	MerkleNode * left; /**< The left child. NULL if leaf. */
	MerkleNode * right; /**< The right child. NULL if leaf. */
};

/**
 @brief Builds a Merkle tree from a list of hashes. In cases of duplication, "left" and "right" may refer to the same node.
 @param hashes A list of hashes as ByteArrays to build the tree from.
 @param numHashes The number of hashes.
 @returns The root MerkleNode for the tree.
 */
MerkleNode * createMerkleTree(ByteArray ** hashes, uint32_t numHashes);
/**
 @brief Frees a merkle tree from a given root.
 @param root The merkle tree root node.
 */

void destroyMerkleTree(MerkleNode * root);
/**
 @brief Gets a list of hashes for a level in a merkle tree. If the merkle tree's deepest level is smaller than specified by "level", the lowest level in the tree is returned.
 @param root The merkle tree root node.
 @param level The level to retrieve. Pass in a high number (use 255) to get the deepest level. Level 0 corresponds to the root which was passed in.
 @returns With nodes left to right, the memory block for this level of the merkle tree.
 */
MerkleNode * getHashListForLevel(MerkleNode * root, uint8_t level);

#endif
