/*
 *  Database.h
 *
 *  Created on: 22/11/2012.
 *  Created by: Prajowal Manandhar
 *  Modified by:
 *  Copyright (c) 2012 Bitcoin Project Team

 */


#ifndef DATABASE_H_
#define DATABASE_H_

#include<stdio.h>
#include<sqlite3.h>
#include<stdlib.h>
#include "../ByteArray.h"
#include<string.h>

void dbConnectionOpen();
void dbConnectionClose();
/**
 @brief Contains Database Structure for transaction, User Details and Wallet.
 */
void dbCreateBaseTables();
void dbInsertUserWalletInfo(char *userId, char *balanceAmount, char *transactionId, char *userPublicKey, char *userPrivateKey);
void dbTransactionWalletUpdate(char *userId, char *transactionId, char *values);
void dbInsertTransInfo(char *transId,char *transHash,char *transAmt, char *transTimeStamp, char *fromUserId,char *toUserId);
int qCnt ,qSize,ind,retVal;
sqlite3_stmt *stmt;
static sqlite3 *handle;
char **queries;


#endif /* DATABASE_H_ */
