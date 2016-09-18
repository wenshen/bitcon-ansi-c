/*
 * Bitcoin.c
 *
 * Created on: Nov 20, 2012
 * Created by: wen shen
 * Copyright (c) 2012 MIBitcoin Project Team
 */

#include "Core/NetworkHelper.h"
#include "Core/Wallet.h"
#include "Constants.h"
#include <pthread.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>
#include <arpa/inet.h>

#define SIZEOFBUFFER 102400
#define BUFFER1SIZE 10240
#define SIZEOFIPADDR 1024
#define SIZEOFAMOUNT 1024

pthread_t runBitcoinThread;
pthread_t runClientThread;

void err8(Error a, char * format, ...);

void err8(Error a, char * format, ...) {

	va_list argptr;
	va_start(argptr, format);
	vfprintf(stderr, format, argptr);
	va_end(argptr);
	printf("\n");
}

char * trim(char * s) {
	int l = strlen(s);

	while (isspace(s[l - 1]))
		--l;
	while (*s && isspace(* s))
		++s, --l;

	return strndup(s, l);
}

int isValidIpAddress(char *ipAddress) {
	struct sockaddr_in sa;
	int result = inet_pton(AF_INET, trim(ipAddress), &(sa.sin_addr));
	return result != 0;
}

void getbalance() {
	printf("Begin to calculate your balance \n");
	Wallet *wallet = createWallet(err8);
	readRecordsFromWalletFile(wallet, "wallet");
	float l = getBalance(wallet);
	printf("Your Balance is: %f\n", l);
}

float getOnlyBalance() {
	Wallet *wallet = createWallet(err8);
	readRecordsFromWalletFile(wallet, "wallet");
	float l = getBalance(wallet);

	return l;
}

char * createJSONRecordString(float amount, char * from, char * to, int type) {

	char
			*format =
					"{\"Date\": \"%s\",\"Type\": %i, \"AddressTo\": \"%s\",\"AddressFrom\": \"%s\",\"Amount\": %f}";
	time_t mytime;
	mytime = time(NULL);

	char *dateString = ctime(&mytime);
	int record_type = type;

	/*strip off new line characters i.e \n */
	dateString[strcspn(dateString, "\n")] = '\0';
	to[strcspn(to, "\n")] = '\0';

	char buff[500];
	sprintf(buff, format, dateString, type, to, from, amount);
	printf("This is buff %s\n", buff);
	return buff;
}

void saveCoinsToWalletFile(char * record) {
	int ch;
	FILE *f = fopen("wallet", "r+");
	int i = 0;
	while ((ch = fgetc(f)) != ']') {
		i++;
	}
	printf("Saver\n");
	printf(record);
	fseek(f, i, SEEK_SET);
	char footer[] = "]}";
	int len = fprintf(f, ",");
	len = fprintf(f, record);
	printf("Length of xters written : %i\n", len);
	len = fprintf(f, footer);
	printf("Length of xters written : %i\n", len);
	fclose(f);
}

void sendBitcoin() {
	char *ip_addr = getGlobalIPAddress();
	char
			*format =
					"{\"Date\": \"%s\",\"Type\": %i, \"AddressTo\": \"%s\",\"AddressFrom\": \"%s\",\"Amount\": %f}";

	printf("Begin to send your bitcoins\n");
	float amount;
	char to_ip[SIZEOFIPADDR];
	char *from_ip = getGlobalIPAddress();
	printf("Please input your destination(IP Address): ");
	fgets(to_ip, SIZEOFIPADDR, stdin);
	while (!isValidIpAddress(to_ip)) {
		printf("Please input valid IP Address(IPv4): ");
		fgets(to_ip, SIZEOFIPADDR, stdin);
	}
	while(strcmp(trim(to_ip), from_ip) == 0){
			printf("Can not send MIBitcoins to yourself! Please input another address:");
			fgets(to_ip, SIZEOFIPADDR, stdin);
	}

	printf("Please input the amount: ");
	scanf("%f", &amount);
	float currentBalance = getOnlyBalance();
	while (amount > currentBalance) {
		printf("Please input an valid amount(your balance is %f):",
				currentBalance);
		scanf("%f", &amount);
	}

	time_t mytime;
	mytime = time(NULL);

	char *dateString = ctime(&mytime);
	dateString[strcspn(dateString, "\n")] = '\0';
	to_ip[strcspn(to_ip, "\n")] = '\0';

	char buff[500];
	char buff1[500];
	sprintf(buff, format, dateString, RECORD_RECEIVED, to_ip, from_ip, amount);

	printf("Record string before \n %s", buff);
	sendMessageToIPAddress(to_ip, buff);

	printf("Record string \n %s", buff);

	sprintf(buff1, format, dateString, RECORD_SENT, to_ip, from_ip, amount);

	saveCoinsToWalletFile(buff1);
	/*sprintf(buff, format, record->date, record->type, record->to, record->from, record->amount);
	 */
}

void quit() {
	printf("You'll exit from MIBitcoin! We deserve an A!\n");
	exit(1);

}
void doCommands() {
	char buf1[BUFFER1SIZE];
	printf("Please input your command(sendBitcoins  getBalance  quit): ");
	while (1) {
		fgets(buf1, BUFFER1SIZE, stdin);
		if (!strncasecmp(buf1, "sendBitcoins", 12)) {
			sendBitcoin();
		} else if (!strncasecmp(buf1, "getBalance", 10)) {
			getbalance();
		} else if (!strncasecmp(buf1, "quit", 4)) {
			quit();
		} else {

		}
	}

}

void *runBitcoin(void *lparam) {
	printf("Hello runBitcoin!\n");
	char *myIP = getGlobalIPAddress();
	initiateServer(1, myIP);
}
void *runClient(void *lpara) {
	while (TRUE) {
		printf("true\n");
		sleep(5);
	}

}

