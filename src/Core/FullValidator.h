/*
 * FullValidator.h
 *
 * Created on: Nov 25, 2012
 * Created by: wenshen
 * Copyright (c) 2012 MIBitcoin Project Team
 */

/**
 @file
 @brief Validates blocks, finding the main chain.
 */
#ifndef FULLVALIDATOR_H_
#define FULLVALIDATOR_H_

#include "../Constants.h"
#include "../Object/Message/Block.h"
#include "../BigInt/BigInt.h"
#include "BlockValidationFunction.h"
#include "../Object/Message/TransactionInput.h"
#include <string.h>

/**
 @brief References an output in the block storage.
 */
typedef struct{
	uint8_t outputHash[32]; /** The transaction hash for the output */
	uint32_t outputIndex; /** The index for the output */
	uint32_t blockIndex; /**< The index of the block containing this output */
	bool coinbase; /**< True if a coinbase output */
	uint8_t branch; /**< The branch this output belongs to. */
	uint32_t position; /**< The positon of the output in the block */
}OutputReference;

/**
 @brief Keeps the target and time for a block in a cache.
 */
typedef struct{
	uint32_t target; /** The target for this block */
	uint32_t time; /**< The block's timestamp */
}BlockReference;

/**
 @brief Indexes the block references to the block hashes. When stored as a sorted lookup table, an interpolation search can be used to find the index at which a block reference is stored.
 */
typedef struct{
	uint8_t blockHash[32]; /**< The block hash. */
	uint32_t index; /**< The index for the block reference for this hash */
} BlockReferenceHashIndex;

/**
 @brief Represents a block branch.
 */
typedef struct{
	uint32_t numRefs; /**< The number of block references in the branch */
	BlockReference * references; /**< The block references */
	BlockReferenceHashIndex * referenceTable; /**< The lookup table for block references */
	uint32_t lastRetargetTime; /**< The block timestamp at the last retarget. */
	uint8_t parentBranch; /**< The branch this branch is connected to. */
	uint32_t parentBlockIndex; /**< The block index in the parent branch which this branch is connected to */
	uint32_t startHeight; /**< The starting height where this branch begins */
	uint32_t lastValidation; /**< The index of the last block in this branch that has been fully validated. */
	uint32_t numUnspentOutputs; /**< The number of unspent outputs for this branch upto the last validated block. */
	OutputReference * unspentOutputs; /**< A list of unspent outputs for this branch upto the last validated block. */
	BigInt work; /**< The total work for this branch. The branch with the highest work is the winner! */
	uint16_t numBlockFiles; /**< Number of open block files for this branch. */
} BlockBranch;

/**
 @brief Structure for FullValidator objects. @see FullValidator.h
 */
typedef struct{
	Object base;
	uint8_t numOrphans; /**< The number of orhpans */
	Block * orphans[MAX_ORPHAN_CACHE]; /**< The ophan block references */
	uint8_t firstOrphan; /**< The orphan added first or rather the front of the orhpan queue (for overwriting) */
	uint8_t mainBranch; /**< The index for the main branch */
	uint8_t numBranches; /**< The number of block-chain branches. Cannot exceed _MAX_BRANCH_CACHE */
	BlockBranch branches[MAX_BRANCH_CACHE]; /**< The block-chain branches. */
	void (*logError)(char *,...); /**< Pointer to error callback */
	void * storage; /**< The storage component object */
} FullValidator;

/**
 @brief Creates a new FullValidator object.
 @returns A new FullValidator object.
 */

FullValidator * newFullValidator(char * dataDir, void (*logError)(char *,...));

/**
 @brief Gets a FullValidator from another object. Use this to avoid casts.
 @param self The object to obtain the FullValidator from.
 @returns The FullValidator object.
 */
FullValidator * getFullValidator(void * self);

/**
 @brief Initialises a FullValidator object.
 @param self The FullValidator object to initialise.
 @returns true on success, false on failure.
 */
bool initFullValidator(FullValidator * self, char * homeDir, void (*logError)(char *,...));

/**
 @brief Frees a FullValidator object.
 @param self The FullValidator object to free.
 */
void freeFullValidator(void * vself);

/* Functions*/

/**
 @brief Adds a block to a branch.
 @param self The FullValidator object.
 @param branch The index of the branch to add the block to.
 @param block The block to add.
 @param work The new branch work. This is not the block work but the total work upto this block. This is taken by the function and the old work is freed.
 @returns true on success and false on failure.
 */
bool fullValidatorAddBlockToBranch(FullValidator * self, uint8_t branch, Block * block, BigInt work);
/**
 @brief Adds a block to the orphans.
 @param self The FullValidator object.
 @param block The block to add.
 @returns true on success and false on error.
 */
bool fullValidatorAddBlockToOrphans(FullValidator * self, Block * block);
/**
 @brief Does basic validation on a block
 @param self The FullValidator object.
 @param block The block to valdiate.
 @param txHashes 32 byte double Sha-256 hashes for the transactions in the block, one after the other. These will be modified by this function.
 @param networkTime The network time.
 @returns The block status.
 */
BlockStatus fullValidatorBasicBlockValidation(FullValidator * self, Block * block, uint8_t * txHashes, uint64_t networkTime);
/**
 @brief Same as FullValidatorBasicBlockValidation but copies the "txHashes" so that the original data is not modified.
 @see FullValidatorBasicBlockValidation
 */
BlockStatus fullValidatorBasicBlockValidationCopy(FullValidator * self, Block * block, uint8_t * txHashes, uint64_t networkTime);
/**
 @brief Completes the validation for a block during main branch extention or reorganisation.
 @param self The FullValidator object.
 @param branch The branch being validated
 @param block The block to complete validation for.
 @param txHashes 32 byte double Sha-256 hashes for the transactions in the block, one after the other.
 @param height The height of the block.
 @returns _BLOCK_VALIDATION_OK if the block passed validation, _BLOCK_VALIDATION_BAD if the block failed validation and _BLOCK_VALIDATION_ERR on an error.
 */
BlockValidationResult fullValidatorCompleteBlockValidation(FullValidator * self, uint8_t branch, Block * block, uint8_t * txHashes,uint32_t height);
/**
 @brief Ensures a file can be opened.
 @param self The FullValidator object.
 */
void fullValidatorEnsureCanOpen(FullValidator * self);
/**
 @brief Gets the mimimum time minus one allowed for a new block onto a branch.
 @param self The FullValidator object.
 @param branch The id of the branch.
 @param prevIndex The index of the last block to determine the minimum time minus one for when adding onto this block.
 */
uint32_t fullValidatorGetMedianTime(FullValidator * self, uint8_t branch, uint32_t prevIndex);
/**
 @brief Validates a transaction input.
 @param self The FullValidator object.
 @param branch The branch being validated.
 @param block The block begin validated.
 @param blockHeight The height of the block being validated
 @param transactionIndex The index of the transaction to validate.
 @param inputIndex The index of the input to validate.
 @param allSpentOutputs The previous outputs returned from TransactionValidateBasic
 @param txHashes 32 byte double Sha-256 hashes for the transactions in the block, one after the other.
 @param value Pointer to the total value of the transaction. This will be incremented by this function with the input value.
 @param sigOps Pointer to the total number of signature operations. This is increased by the signature operations for the input and verified to be less that the maximum allowed signature operations.
 @returns _BLOCK_VALIDATION_OK if the transaction passed validation, _BLOCK_VALIDATION_BAD if the transaction failed validation and _BLOCK_VALIDATION_ERR on an error.
 */
BlockValidationResult fullValidatorInputValidation(FullValidator * self, uint8_t branch, Block * block, uint32_t blockHeight, uint32_t transactionIndex,uint32_t inputIndex, PreviousOutput ** allSpentOutputs, uint8_t * txHashes, uint64_t * value, uint32_t * sigOps);
/**
 @brief Finds a block reference ad returns the index or finds the insertion point if the reference was no found.
 @param lookupTable The table of references to search.
 @param refNum The number of references to search.
 @param hash The hash of the block to search for.
 @param found This is set to true if the reference was found or false otherwise.
 @returns The position of the matching reference in the lookup table or the index of where the reference index should go in the case the reference was not found.
 */
uint32_t fullValidatorFindBlockReference(BlockReferenceHashIndex * lookupTable, uint32_t refNum, uint8_t * hash, bool * found);
/**
 @brief Finds an output reference ad returns the index or finds the insertion point if the reference was no found.
 @param refs The output references to search.
 @param refNum The number of references to search.
 @param hash The transaction hash of the output to search for.
 @param index The index of the output.
 @param found This is set to true if the reference was found or false otherwise.
 @returns The index of the matching reference or the index of where the reference should go in the case the reference was not found.
 */
uint32_t fullValidatorFindOutputReference(OutputReference * refs, uint32_t refNum, uint8_t * hash, uint32_t index, bool * found);
/**
 @brief Loads a block from storage.
 @param self The FullValidator object.
 @param blockRef A reference to the block in storage.
 @param branch The branch the block belongs to.
 @returns A new BlockObject with serailised block data which has not been deserialised or NULL on failure.
 */
Block * fullValidatorLoadBlock(FullValidator * self, BlockReference blockRef, uint32_t branch);
/**
 @brief Loads the validation data for a block-chain branch. This only loads data if the validator file for the branch has not been opened already.
 @param self The FullValidator object.
 @param branch The index of the branch to load the data for.
 @returns @see ValidatorLoadResult
 */
ValidatorLoadResult fullValidatorLoadBranchValidator(FullValidator * self, uint8_t branch);
/**
 @brief Loads the general validation data. This only loads data if the validator file has not been opened already.
 @param self The FullValidator object.
 @returns @see ValidatorLoadResult
 */
ValidatorLoadResult fullValidatorLoadValidator(FullValidator * self);
/**
 @brief Processes a block. Block headers are validated, ensuring the integrity of the transaction data is OK, checking the block's proof of work and calculating the total branch work to the genesis block. If the block extends the main branch complete validation is done. If the block extends a branch to become the new main branch because it has the most work, a re-organisation of the block-chain is done.
 @param self The FullValidator object.
 @param block The block to process.
 @param networkTime The network time.
 @return The status of the block.
 */
BlockStatus fullValidatorProcessBlock(FullValidator * self, Block * block, uint64_t networkTime);
/**
 @brief Processes a block into a branch. This is used once basic validation is done on a block and it is determined what branch it needs to go into and when this branch is ready to receive the block.
 @param self The FullValidator object.
 @param block The block to process.
 @param networkTime The network time.
 @param branch The branch to add to.
 @param prevBranch The branch of the previous block.
 @param prevBlockIndex The index of the previous block.
 @param txHashes The transaction hashes for the block.
 @return The status of the block.
 */
BlockStatus fullValidatorProcessIntoBranch(FullValidator * self, Block * block, uint64_t networkTime, uint8_t branch, uint8_t prevBranch, uint32_t prevBlockIndex, uint8_t * txHashes);

#endif /* FULLVALIDATOR_H_ */
