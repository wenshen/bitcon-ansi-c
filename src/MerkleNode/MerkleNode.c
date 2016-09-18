/*
*  MerkleNode.c
*
*  Created on: 24/10/2012
*  Created by: Fadwa Alseiari
*  Modified by:
*  Copyright (c) 2012 Bitcoin Project Team
*/

#include <assert.h>
#include "MerkleNode.h"
#include "../Object/ByteArray.h"
#include "../Utils/Cryptography/Crypt.h"


MerkleNode * createMerkleTree(ByteArray ** hashes, uint32_t numHashes) {

	assert(hashes != NULL);
	assert(numHashes >= 0);

	uint32_t x;
	uint8_t hash[32];
	uint8_t cat[64];
	MerkleNode *nextLevel, *level;

	level = malloc(numHashes * sizeof(*level)); /**<  Nodes on a level of the tree for processing. */

	if (! level) {

		return NULL;
	}

	/**<  Create nodes from the ByteArray hashes */
	for (x = 0; x < numHashes; x++) {

		memcpy(level[x].hash, getByteArrayData(hashes[x]), 32);
		level[x].left = NULL;
		level[x].right = NULL;
	}

	/**<  Build each level upwards to the root */

	nextLevel = malloc((numHashes + 1)/2 * sizeof(*level));

	if (! nextLevel) {

		free(level);
		return NULL;
	}

	for (x = 0;;) {

		nextLevel[x/2].left = level + x;

		if (x == numHashes - 1) {

			nextLevel[x/2].right = level + x;
		} else {

			nextLevel[x/2].right = level + x + 1;
		}

		memcpy(cat, nextLevel[x/2].left->hash, 32);
		memcpy(cat + 32, nextLevel[x/2].right->hash, 32);
		/**<  Double SHA256 */
		Sha256(cat, 64, hash);
		Sha256(hash, 32, nextLevel[x/2].hash);
		x += 2;

		if (x >= numHashes) {

			/**<  Finished level */
			if (x > numHashes) {
				/**<  The number of hashes was odd. Increment to even */
				numHashes++;
			}
			numHashes /= 2;
			/**<  Move to next level */
			level = nextLevel;

			if (numHashes == 1) {

				/**<  Done, got the single root hash */
				break;
			}
			x = 0;
			nextLevel = malloc((numHashes + 1)/2 * sizeof(*level));

			if (! nextLevel) {

				destroyMerkleTree(level);
				return NULL;
			}
		}
	}
	/**<  Return last level which contains only the root node. */
	return level;
}

void destroyMerkleTree(MerkleNode * root) {

	assert(root != NULL);
	/**<  Free all levels along the far left */
	MerkleNode * next;
	MerkleNode * node;

	for (node = root; node != NULL; node = next) {

		next = node->left;
		free(node);
	}
}

MerkleNode * getHashListForLevel(MerkleNode * root, uint8_t level) {

	assert(root != NULL);
	assert(level >=0 );

	uint8_t x;

	for ( x = 0; x < level; x++) {

		if (root->left == NULL) {

			return root;
		}

		root = root->left;
	}

	return root;
}
