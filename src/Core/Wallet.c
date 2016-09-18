/*
 * Wallet.c
 *
 * Created on: Nov 27, 2012
 * Created by: Wen Shen
 * Copyright (c) 2012 MIBitcoin Project Team
 */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "Wallet.h"
#include "assert.h"
#include <time.h>
#include "../YAJL/src/api/yajl_tree.h"

static unsigned char fileData[65536]; /*TODO look into extending this to a larger value e.g. LONG_MAX in limits.h */

Wallet * createWallet(void (*onErrorReceived)(Error error, char *, ...)) {

	Wallet *self = malloc(sizeof(*self));
	if (!self) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,
				"Cannot allocate %i bytes of memory in createWallet\n",
				sizeof(*self));
		return NULL;
	}

	getObject(self)->destroy = destroyWallet;
	if (initWallet(self, onErrorReceived)) {
		return self;
	}
	free(self);
	return NULL;

}

boolean initWallet(Wallet *self,
		void (*onErrorReceived)(Error error, char *, ...)) {
	assert(self != NULL);
	self->onErrorReceived = onErrorReceived;
	self->records = malloc(sizeof(**self->records));
	self->numOfRecords = 0;

	if (!self->records) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,
				"Cannot allocate %i bytes of memory in initWallet for the records structure.\n",
				sizeof(**self->records));
		return FALSE;
	}

	self->keypairs = malloc(sizeof(**self->records));
	if (!self->keypairs) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,
				"Cannot allocate %i bytes of memory in initWallet for the keypairs structure.\n",
				sizeof(*self->keypairs));
		return FALSE;
	}

	return TRUE;

}
/*To be finished*/
boolean storeWallet(Wallet * self) {
	FILE *walletFile;

	walletFile = fopen("wallet", "w");
	if (walletFile == NULL) {
		fprintf(stderr, "Can not open output file\n");
		return FALSE;
	}

	/*The strings that define the JSON structure */
	char *json_opener = "{\"wallet\":[";
	char *format =
			"{       \
		            \"Date\": \"%s\",        \
		            \"Type\": %lli,      \
		            \"AddressTo\": \"%s\",   \
		            \"AddressFrom\": \"%s\", \
		            \"Amount\": %lf     \
		        }";
	char *json_closer = "]}";

	fprintf(walletFile, "%s", json_opener);

	int i = 0;
	for (; i < self->numOfRecords - 1; i++) { /*The last item will not have a comma after its JSON object so we don't iterate to the end*/

		Record *record = self->records[i];

		fprintf(walletFile, format, record->date, record->type, record->to,
				record->from, record->amount);

		fprintf(walletFile, "%s", ","); /*Add a comma after each JSON object */
	}

	Record *lastRecord = self->records[self->numOfRecords - 1];

	fprintf(walletFile, format, lastRecord->date, lastRecord->type,
			lastRecord->to, lastRecord->from, lastRecord->amount);

	fprintf(walletFile, "%s", json_closer);

	fclose(walletFile);
	return TRUE;
}
/*To be finished*/
Wallet * readWallet() {

	Wallet * wallet = NULL;
	return wallet;
}

boolean * readRecordsFromWalletFile(Wallet *self, char *file) {
	FILE *walletFile;
	walletFile = fopen(file, "rb");

	size_t rd;
	yajl_val node;
	char errbuf[1024];

	/* null plug buffers */
	fileData[0] = errbuf[0] = 0;

	/* read the entire config file */
	rd = fread((void *) fileData, 1, sizeof(fileData) - 1, walletFile);

	/* file read error handling */
	if (rd == 0 && !feof(stdin)) {
		fprintf(stderr, "error encountered on wallet file read\n");
		return FALSE;
	} else if (rd >= sizeof(fileData) - 1) {
		fprintf(stderr, "wallet file too big\n");
		return TRUE;
	}

	/* we have the whole config file in memory.  let's parse it ... */
	node = yajl_tree_parse((const char *) fileData, errbuf, sizeof(errbuf));

	/* parse error handling */
	if (node == NULL) {
		fprintf(stderr, "parse_error: ");
		if (strlen(errbuf))
			fprintf(stderr, " %s", errbuf);
		else
			fprintf(stderr, "unknown error");
		fprintf(stderr, "\n");
		return FALSE;
	}

	/* ... and extract a nested value from the config file */
	{
		const char * path[] = { "wallet", (const char *) 0 };
		yajl_val v = yajl_tree_get(node, path, yajl_t_array);

		if (v) {
			yajl_val *objs = v->u.array.values;

			int i = 0;
			for (; i < v->u.array.len; i++) {
				if (objs[i]) { /*A complete JSON object*/
					char **keys = objs[i]->u.object.keys; /*keys for JSON obj*/
					yajl_val *vals = objs[i]->u.object.values; /*values for JSON obj*/

					Record * record = createRecord();

					int j = 0;
					for (; j < objs[i]->u.object.len; j++) {
						if (!strcmp(keys[j],"Date")) {
							record->date = YAJL_GET_STRING(vals[j]);
						} else if (!strcmp(keys[j],"Type")) {
							record->type = YAJL_GET_INTEGER(vals[j]);
						} else if (!strcmp(keys[j], "AddressTo")) {
							record->to = YAJL_GET_STRING(vals[j]);
						} else if (!strcmp(keys[j], "AddressFrom")) {
							record->from = YAJL_GET_STRING(vals[j]);
						} else if (!strcmp(keys[j], "Amount")) {
							record->amount =  vals[j]->u.number.d;
						}
					}
					addRecordToWallet(self, record);
				} else {
					self->onErrorReceived(ERROR_JSON_READ,
							"Cannot read a record in the wallet file\n",
							sizeof(*self));
				}
			}
		}
	}

	yajl_tree_free(node);
	fclose(walletFile);
	return TRUE;
}

Record * createRecord() {
	Record *self = malloc(sizeof(*self));
	if (!self) {
		printf("Cannot allocate %i bytes of memory in createRecord\n",
				sizeof(*self));
		return NULL;
	}

	/*getObject(self)->destroy = destroyRecord;*/
	return self;
}

boolean addKeypairToWallet(Wallet * self, Keypair * keypair) {
	assert(self != NULL);
	assert(keypair != NULL);

	incrementReferenceCount(keypair); /*Increase the number of references to this keypair*/
	self->numOfKeypairs++;
	size_t size = self->numOfKeypairs * sizeof(*self->keypairs); /*Size of memory to allocate for the keypairs*/
	Keypair ** tmp = realloc(self->keypairs, size);

	if (tmp != NULL) {
		self->keypairs = tmp;
		self->keypairs[self->numOfKeypairs - 1] = keypair; /*append keypair to the keypairs array; i.e. put it at the last position*/
		return TRUE;
	}
	return FALSE;
}
boolean addRecordToWallet(Wallet * self, Record * record) {
	assert(self != NULL);
	assert(record != NULL);
	incrementReferenceCount(record); /*Increase the number of references to this keypair*/
	self->numOfRecords++;
	size_t size = self->numOfRecords * sizeof(*self->records); /*Size of memory to allocate for the records*/
	Record ** tmp = realloc(self->records, size);

	if (tmp != NULL) {
		self->records = tmp;
		self->records[self->numOfRecords - 1] = record; /*append record to the records array; i.e. put it at the last position*/
		return TRUE;
	}
	return FALSE;
}

float getBalance(Wallet * self) {
	float balance = 0.0;
	uint32_t i;
	float from = 0.0;
	float to = 0.0;
	for (i = 0; i < self->numOfRecords; i++) {
		if (self->records[i]->type == RECORD_SENT) {
			from = from + self->records[i]->amount;

		} else if (self->records[i]->type == RECORD_RECEIVED) {
			to = to + self->records[i]->amount;
		}
	}
	balance = to - from;
	return balance;
}

void updateWallet(Wallet * self, Wallet *wallet) {
	assert(self != NULL);
	assert(wallet != NULL);

	uint32_t i;
	for (i = 0; i < self->numOfRecords; i++) {
		self->records[i]->base.referenceCount =
				wallet->records[i]->base.referenceCount;
		self->records[i]->date = wallet->records[i]->date;
		self->records[i]->amount = wallet->records[i]->amount;
		self->records[i]->type = wallet->records[i]->type;
		self->records[i]->from = wallet->records[i]->from;
		self->records[i]->to = wallet->records[i]->to;
	}
	for (i = 0; i < self->numOfKeypairs; i++) {
		self->keypairs[i]->base.referenceCount =
				wallet->keypairs[i]->base.referenceCount;
		self->keypairs[i]->privateKey = wallet->keypairs[i]->privateKey;
		self->keypairs[i]->publicKey = wallet->keypairs[i]->publicKey;
	}

}


boolean receiveCoins(Wallet * self, Record *jsonRecord)
{
	assert(self != NULL);
	char errbuf[1024];
	yajl_val node;

	node = yajl_tree_parse((const char *) jsonRecord, errbuf, sizeof(errbuf));

	/* parse error handling */
	if (node == NULL) {
		fprintf(stderr, "parse_error: ");
		if (strlen(errbuf))
			fprintf(stderr, " %s", errbuf);
		else
			fprintf(stderr, "unknown error");
		fprintf(stderr, "\n");
		return FALSE;
	}

	/* ... and extract a nested value from the config file */
	{
		const char * path[] = { "record", (const char *) 0 };
		yajl_val v = yajl_tree_get(node, path, yajl_t_array);

		if (v) {
			yajl_val *objs = v->u.array.values;

			int i = 0;
			for (; i < v->u.array.len; i++) {
				if (objs[i]) { /*A complete JSON object*/
					char **keys = objs[i]->u.object.keys; /*keys for JSON obj*/
					yajl_val *vals = objs[i]->u.object.values; /*values for JSON obj*/

					Record * record = createRecord();

					int j = 0;
					for (; j < objs[i]->u.object.len; j++) {
						if (keys[j] == "Date") {
							record->date = YAJL_GET_STRING(vals[j]);
						} else if (keys[j] == "Type") {
							record->type = RECORD_RECEIVED; /* This is a received transaction; set the flag appropriately */
						} else if (keys[j] == "AddressTo") {
							record->to = YAJL_GET_STRING(vals[j]);
						} else if (keys[j] == "AddressFrom") {
							record->from = YAJL_GET_STRING(vals[j]);
						} else if (keys[j] == "Amount") {
							record->amount = YAJL_GET_DOUBLE(vals[j]);
						}
					}
					addRecordToWallet(self, record);
				} else {
					self->onErrorReceived(ERROR_JSON_READ,
							"Cannot read a record in the wallet file\n",
							sizeof(*self));
				}
			}
		}
	}

	yajl_tree_free(node);
	return TRUE;
}
Address * getAddressByKeyPair(Wallet * self, Keypair * keypair, int addressType) {
	assert(self != NULL);
	assert(keypair != NULL);
	assert(self->numOfKeypairs > 0);
	assert(self->numOfKeypairs == self->numOfRecords);
	Address * return_from = NULL;
	Address * return_to = NULL;
	uint32_t i;
	for (i = 0; i < self->numOfKeypairs; i++) {
		if (keypair->privateKey == self->keypairs[i]->privateKey
				&& keypair->publicKey == self->keypairs[i]->publicKey) {
			return_from = self->records[i]->from;
			return_to = self->records[i]->to;
		}

	}
	if (addressType == Address_FROM) {
		return return_from;
	} else {
		return return_to;
	}

}

boolean isMyPublicKey(Wallet * self, Address * address, int addressType) {
	assert(self != NULL);
	assert(address != NULL);
	assert(self->numOfRecords > 0);
	assert(self->numOfKeypairs == self->numOfRecords);
	uint32_t i;
	ByteArray * string = getStringForVersionChecksumBytes(
			getVersionChecksumBytes(address));
	uint8_t * addr = getByteArrayData(string);
	uint8_t * addr_temp;
	for (i = 0; i < self->numOfRecords; i++) {
		if (addressType == Address_TO) {
			addr_temp = getByteArrayData(
					getStringForVersionChecksumBytes(
							getVersionChecksumBytes(self->records[i]->to)));
			if (addr_temp == addr) {
				return TRUE;
			}

		}
		if (addressType == Address_FROM) {
			addr_temp = getByteArrayData(
					getStringForVersionChecksumBytes(
							getVersionChecksumBytes(self->records[i]->from)));
			if (addr_temp == addr) {
				return TRUE;
			}
		}
	}

	return FALSE;

}
void destroyWallet(Wallet * self) {
	assert(self != NULL);
	uint32_t i;
	for (i = 0; i < self->numOfRecords; i++) {
		decrementReferenceCount(self->records[i]); /*handles cleaning up of referenced object once num of references get to zero*/
	}

	free(self->records);
	for (i = 0; i < self->numOfKeypairs; i++) {
		decrementReferenceCount(self->keypairs[i]); /*handles cleaning up of referenced object once num of references get to zero*/
	}
	free(self->keypairs);

	destroyObject(getObject(self));
}

void destroyRecord(Record *self) {
	destroyObject(getObject(self));
}
