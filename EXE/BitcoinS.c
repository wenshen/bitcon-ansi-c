/*
 * Bitcoin.c
 *
 * Created on: Nov 20, 2012
 * Created by: wen shen
 * Copyright (c) 2012 MIBitcoin Project Team
 */

#include "../src/Core/NetworkHelper.h"
#include "../src/Core/Wallet.h"
#include "../src/Constants.h"
#include <pthread.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>
#include <arpa/inet.h>

#include <ctype.h>
#include <openssl/ssl.h>
#include <openssl/ripemd.h>
#include <openssl/rand.h>
#include "../src/Object/Network/Address.h"
#include "../src/Object/ByteArray.h"
#include "../src/Object/Object.h"
#include "../src/Object/VersionChecksumBytes.h"


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

void getLine1(char * ptr);

void getLine1(char * ptr) {

	int c;
	int len = 30;

	for (;;) {

		c = fgetc(stdin);

		if (c == EOF) {

			break;
		}

		if (--len == 0) {

			break;
		}

		*ptr = c;

		if (c == '\n') {

			break;
		}

		ptr++;
	}

	*ptr = 0;
}

char * getRealBitcoinAddress() {

	printf("OpenSSL version: %s\n", OPENSSL_VERSION_TEXT);
	/*printf("Enter the number of keys: ");
	fflush(stdout);
	*/
	char stringMatch[31];
	/*getLine1(stringMatch);
	unsigned long int i = strtol(stringMatch, NULL, 0);*/
	printf("Please enter a string of text for the key (30 max): ");
	fflush(stdout);
	getLine1(stringMatch);
	printf("Waiting for entropy... Move the cursor around...\n");
	fflush(stdout);
	char entropy[32];
	FILE * f = fopen("/dev/random", "r");

	if (fread(entropy, 32, 1, f) != 1) {

		printf("FAILURING GETTING ENTROPY!");
		return 1;
	}

	RAND_add(entropy, 32, 32);
	fclose(f);
	printf("Making your addresses for \"%s\"\n\n", stringMatch);
	EC_KEY * key = EC_KEY_new_by_curve_name(NID_secp256k1);
	uint8_t * pubKey = NULL;
	int pubSize = 0;
	uint8_t * privKey = NULL;
	int privSize = 0;
	uint8_t * shaHash = malloc(32);
	uint8_t * ripemdHash = malloc(20);
	unsigned int x;

	if (!EC_KEY_generate_key(key)) {

		printf("GENERATE KEY FAIL\n");
		exit(1);
	}

	int pubSizeNew = i2o_ECPublicKey(key, NULL);

	if (!pubSizeNew) {

		printf("PUB KEY TO DATA ZERO\n");
		exit(1);
	}

	if (pubSizeNew != pubSize) {

		pubSize = pubSizeNew;
		pubKey = realloc(pubKey, pubSize);
	}
	uint8_t * pubKey2 = pubKey;

	if (i2o_ECPublicKey(key, &pubKey2) != pubSize) {

		printf("PUB KEY TO DATA FAIL\n");
		exit(1);
	}

	SHA256(pubKey, pubSize, shaHash);
	RIPEMD160(shaHash, 32, ripemdHash);
	Address * address = createNewAddressFromRIPEMD160Hash(ripemdHash, 0, 0,
			err8);
	ByteArray * string = getStringForVersionChecksumBytes(
			getVersionChecksumBytes(address));
	decrementReferenceCount(address);
	uint8_t offset = 1;
	size_t matchSize = strlen(stringMatch);
	uint8_t y;
	/* Get private key*/
	const BIGNUM * privKeyNum = EC_KEY_get0_private_key(key);

	if (!privKeyNum) {
		printf("PRIV KEY TO BN FAIL\n");
	}

	int privSizeNew = BN_num_bytes(privKeyNum);

	if (privSizeNew != privSize) {
		privSize = privSizeNew;
		privKey = realloc(privKey, privSize);
	}

	int res = BN_bn2bin(privKeyNum, privKey);

	if (res != privSize) {
		printf("PRIV KEY TO DATA FAIL\n");
	}

	/* Print data to stdout*/
	printf("Private key (hex): ");
	int i;

	for (i = 0; i < privSize; i++) {
		printf(" %.2X", privKey[i]);
	}

	printf("\nPublic key (hex): ");
	int j;

	for (j = 0; j < pubSize; j++) {
		printf(" %.2X", pubKey[j]);
	}
	printf("\n");

	char *realBitcoinAddress = getByteArrayData(string);

	/*printf("\nAddress (base-58): %s\n\n", realBitcoinAddress);*/
	x++; /*Move to next*/

	decrementReferenceCount(string);


	free(shaHash);
	free(ripemdHash);
	EC_KEY_free(key);
	return realBitcoinAddress;
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
	while (amount > currentBalance || amount <= 0) {
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

	sendMessageToIPAddress(to_ip, buff);

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

int main(int argc, char **argv) {
	daemon(argc,argv);
}

