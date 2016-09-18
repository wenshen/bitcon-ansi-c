/*
 * NetworkHelper.h
 *
 * Created on: Dec 9, 2012
 * Created by: Wen Shen
 * Copyright (c) 2012 MIBitcoin Project Team
 */

#ifndef NETWORKHELPER_H_
#define NETWORKHELPER_H_

#include <stdio.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/time.h>
#include <syslog.h>

#define BUFLEN 2048

int appendRecordToWallet(char *jsonString);
int appendRecordToWallet2(char *jsonString);

int daemon(int argc, char **argv);
char * getGlobalIPAddress();
int sendMessageToIPAddress(char *destination, char *msg);
int initiateServer(int argc, char *argv);

#endif /* NETWORKHELPER_H_ */
