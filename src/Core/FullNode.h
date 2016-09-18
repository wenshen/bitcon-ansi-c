/*
 * FullNode.h
 *
 * Created on: Nov 24, 2012
 * Created by: Wen Shen
 * Copyright (c) 2012 MIBitcoin Project Team
 */

#ifndef FULLNODEH
#define FULLNODEH

/**
 @file
 @brief Downloads and validates the entire bitcoin block-chain.
 */

#include "../Constants.h"
#include "../Object/Network/NetworkCommunicator.h"
#include "stdio.h"

/**
 @brief Structure for FullNode objects. @see FullNode.h
 */
typedef struct {

	NetworkCommunicator base;
    FILE * addressFile;
    void (*onErrorReceived)(Error error,char *,...);

} FullNode;

/**
 @brief Creates a new FullNode object.
 @returns A new FullNode object.
 */
FullNode * newFullNode(void (*logError)(char *,...));

/**
 @brief Gets a FullNode from another object. Use this to avoid casts.
 @param self The object to obtain the FullNode from.
 @returns The FullNode object.
 */
FullNode * getFullNode(void * self);

/**
 @brief Initialises a FullNode object
 @param self The FullNode object to initialise
 @returns true on success, false on failure.
 */
bool initFullNode(FullNode * self, void (*logError)(char *,...));

/**
 @brief Frees a FullNode object.
 @param self The FullNode object to free.
 */
void destroyFullNode(void * self);

/**
 @brief Handles an onBadTime event.
 @param self The FullNode object.
 */
void fullNodeOnBadTime(void * self);

#endif
