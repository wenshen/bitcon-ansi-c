/*
 * BlockValidationFunction.h
 *
 * Created on: Nov 25, 2012
 * Created by: Wen Shen
 * Copyright (c) 2012 MIBitcoin Project Team
 */

/**
 @file
 @brief Here are functions for doing parts of bitcoin validation. MIbitcoin is designed to make a broad variety of bitcoin development easier and is not designed to implement a client node. Therefore, cbitcoin does not provide provide functions for validating block chains as there are different methods for doing so. So wether you want to use SPV, block-chain pruning, full validation or whatever, you can use these functions to help. Also see Script.h and MerkleNode.h for validation. NetworkCommunicator.h has message size checks and message deserialisation functions check messages for syntax and sizes.
 */
#ifndef BLOCKVALIDATIONFUNCTION_H_
#define BLOCKVALIDATIONFUNCTION_H_
#include <stdint.h>
#include "../Constants.h"
#include "../Object/Message/Block.h"

/**
 @brief Calculates the block reward at a particular block height
 @param blockHeight The height
 @returns The reward for generating the block, not including transaction fees.
 */
uint64_t calculateBlockReward(uint64_t blockHeight);
/**
 @brief Calculates the block work which is 256^31 divided by the target
 @param work The block work to be created as a BigInt.
 @param target The target to calculate the work for.
 @returns true on success, false on failure.
 */
bool calculateBlockWork(BigInt * work,uint32_t target);
/**
 @brief Calculates the merkle root from a list of hashes.
 @param hashes The hashes stored as continuous byte data with each 32 byte hash after each other. The data pointed to by "hashes" will be modified and will result in the merkle root as the first 32 bytes.
 @param hashNum The number of hashes in the memory block
 */
void calculateMerkleRoot(uint8_t * hashes,uint32_t hashNum);
/**
 @brief Recalcultes the target for every 2016 blocks.
 @param oldTarget The old target in compact form.
 @param time The time between the newer block and older block for recalulating the target.
 @returns the new target in compact form.
 */
uint32_t calculateTarget(uint32_t oldTarget, uint32_t time);
/**
 @brief Returns the number of sigops from a transaction but does not include P2SH scripts. P2SH sigop counting requires that the previous output is known to be a P2SH output.
 @param tx The transaction.
 @returns the number of sigops for validation.
 */
uint32_t transactionGetSigOps(Transaction * tx);
/**
 @brief Validates a transaction has outputs and inputs, is below the maximum block size, has outputs that do not overflow and that there are no duplicate spent transaction outputs. This function also checks coinbase transactions have input scripts between 2 and 100 bytes and non-coinbase transactions do not have a NULL previous output. Further validation can be done by checking the transaction against input transactions. With simplified payment verification instead the validation is done through trusting miners but the basic validation can still be done for these basic checks.
 @param tx The transaction to validate. This should be deserialised.
 @param coinbase true to validate for a coinbase transaction, false to validate for a non-coinbase transaction.
 @param outputValue Pointer to the integer to hold the output value calculated by this function.
 @param err Pointer to error which is set true when a memory failure occurs.
 @returns A memory block of previous transaction ouputs spent by this transaction if validation passed or NULL. The outputs can be used for further validation that the transaction is not a double-spend or spends non-existant outputs. The block of PrevOuts needs to be freed. The hashes are not retained, so the data is good until the transaction is freed. Do not try to release the hashes, thinking they have been retained in this function.
 */
PreviousOutput * transactionValidateBasic(Transaction * tx, bool coinbase, uint64_t * outputValue, bool * err);
/**
 @brief Determines if a transaction is final and therefore can exist in a block. A transaction is final if the lockTime has been reached or if all inputs are final.
 @param tx The transaction.
 @param time The time for determining if the transaction is final.
 @param height The block height for determining if the transaction is final.
 @returns true if final and false if not final.
 */
bool transactionIsFinal(Transaction * tx,uint64_t time,uint64_t height);
/**
 @brief Validates proof of work.
 @brief hash Block hash.
 @returns true if the validation passed, false otherwise.
 */
bool validateProofOfWork(uint8_t * hash,uint32_t target);

#endif /* BLOCKVALIDATIONFUNCTION_H_ */
