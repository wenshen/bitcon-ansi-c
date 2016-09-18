/*
 *  AddressManager.c
 *
 *  Created on: 9/11/2012.
 *  Created by: Prajowal Manandhar
 *  Modified by:
 *  Copyright (c) 2012 Bitcoin Project Team
 *  please read https://en.bitcoin.it/wiki/Protocol_Specification
 */

#include "AddressManager.h"
#include "../../Utils/Cryptography/Crypt.h"
#include "assert.h"

AddressManager * newAddressManager(void (*logError)(char *, ...),
		void (*onBadTime)(void *)) {
	AddressManager * self = malloc(sizeof(*self));
	if (!self) {
		logError("Cannot allocate %i bytes of memory in NewAddressManager\n",
				sizeof(*self));
		return NULL;
	}
	getObject(self)->destroy = destroyAddressManager;
	if (initAddressManager(self, logError, onBadTime))
		return self;
	free(self);
	return NULL;
}
AddressManager * newAddressManagerFromData(ByteArray * data,
		void (*logError)(char *, ...), void (*onBadTime)(void *)) {
	AddressManager * self = malloc(sizeof(*self));
	if (!self) {
		logError(
				"Cannot allocate %i bytes of memory in NewAddressManagerFromData\n",
				sizeof(*self));
		return NULL;
	}
	getObject(self)->destroy = destroyAddressManager;
	if (initAddressManagerFromData(self, data, logError, onBadTime))
		return self;
	free(self);
	return NULL;
}

AddressManager * getAddressManager(void * self) {
	return self;
}

boolean initAddressManager(AddressManager * self, void (*logError)(char *, ...),
		void (*onBadTime)(void *)) {
	if (!addressManagerSetup(self))
		return 0;
	self->secret = secureRandomInteger(self->rndGen);
	self->onBadTime = onBadTime;
	if (!initializeMessageFromObject(getMessage(self), logError)) {
		destroySecureRandomGenerator(self->rndGen);
		return 0;
	}
	return 1;
}
boolean initAddressManagerFromData(AddressManager * self, ByteArray * data,
		void (*logError)(char *, ...), void (*onBadTime)(void *)) {
	if (!addressManagerSetup(self))
		return 0;
	self->onBadTime = onBadTime;
	if (!initializeMessageFromByteArrayData(getMessage(self), data, logError)) {
		destroySecureRandomGenerator(self->rndGen);
		return 0;
	}
	return 1;
}

void destroyAddressManager(void * vself) {
	AddressManager * self = vself;
	destroySecureRandomGenerator(self->rndGen);
	uint8_t x;
	uint16_t y;
	for (x = 0; x < 255; x++)
		for (y = 0; y < (self->buckets + x)->numOfAddresses; y++)
			decrementReferenceCount((self->buckets + x)->addresses[y]);
	destroyMessage(self);
}

boolean addAddressToAddressManager(AddressManager * self, NetworkAddress * addr) {
	incrementReferenceCount(addr);
	return addNetworkAddressToAddressManager(self, addr);
}
void addressManagerAdjustTime(AddressManager * self) {
	uint32_t index = (self->peersNum - 1) / 2;
	int16_t median = self->peers[index]->timeOffset;
	if (!self->peersNum % 2)
		median = (self->peers[index + 1]->timeOffset + median) / 2;
	if (median > 4200) {

		self->networkTimeOffset = 0;
		boolean found = 0;
		uint16_t x;
		for (x = 0; x < self->peersNum; x++)
			if (self->peers[x]->timeOffset < 300 && self->peers[x]->timeOffset)
				found = 1;
		if (!found)
			self->onBadTime(self->callbackHandler);
	} else
		self->networkTimeOffset = median;
}
uint32_t addressManagerDeserialise(AddressManager * self,
		void (*onErrorReceived)(Error error, char *, ...)) {
	ByteArray * bytes = getMessage(self)->bytes;
	if (!bytes) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,
				"Attempting to deserialise a AddressManager with no bytes.");
		return 0;
	}
	if (bytes->length < 255 * 2 + 12) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,
				"Attempting to deserialise a AddressManager with too few bytes");

		return 0;
	}
	uint32_t cursor = 4;
	uint8_t x;
	for (x = 0; x < 255; x++) {
		Bucket * bucket = self->buckets + x;
		bucket->numOfAddresses = readInt16AsLittleEndianFromByteArray(bytes, cursor);
		cursor += 2;
		if (bytes->length
				< cursor + 30 * bucket->numOfAddresses + (255 - x - 1) * 2 + 8) {
			onErrorReceived(ERROR_OUT_OF_MEMORY,
					"Attempting to deserialise a AddressManager with too few bytes at bucket %u: %u < %u",
					x, bytes->length,
					cursor + 30 * bucket->numOfAddresses + (255 - x - 1) * 2 + 8);
			return 0;
		}
		bucket->addresses = malloc(
				sizeof(*bucket->addresses) * bucket->numOfAddresses);
		if (!bucket->addresses) {
			onErrorReceived(ERROR_OUT_OF_MEMORY,
					"Cannot allocate %i bytes of memory in AddressManagerDeserialise",
					sizeof(*bucket->addresses) * bucket->numOfAddresses);
			uint8_t y;
			for (y = 0; y < x; y++) {
				Bucket * freeBucket = self->buckets + y;
				uint16_t z;
				for (z = 0; z < freeBucket->numOfAddresses; x++)
					decrementReferenceCount(freeBucket->addresses[z]);
				free(freeBucket->addresses);
			}
			return 0;
		}
		uint16_t y1;
		for (y1 = 0; y1 < bucket->numOfAddresses; y1++) {
			ByteArray * data = getByteArraySubsectionReference(bytes, cursor,
					30);
			if (!data) {
				onErrorReceived(ERROR_OUT_OF_MEMORY,
						"Could not create ByteArray in AddressManager for the network address number %u.",
						x);
				return 0;
			}
			bucket->addresses[y1] = createNewNetworkAddressFromSerialisedData(data, onErrorReceived);
			if (!bucket->addresses[y1]) {
				onErrorReceived(ERROR_OUT_OF_MEMORY,
						"Could not create NetworkAddress in AddressManager for the network address number %u.",
						x);
				decrementReferenceCount(data);
				return 0;
			}
			decrementReferenceCount(data);
			if (!deserialiseNetworkAddress(bucket->addresses[y1], 1)) {
				onErrorReceived(ERROR_OUT_OF_MEMORY,
						"Cannot deserialise a AddressManager due to an error with the network address number %u",
						x);
				return 0;
			}
			cursor += 30;
		}
	}
	self->secret = readInt64AsLittleEndianFromByteArray(bytes, cursor);
	return cursor + 8;
}
NetworkAddressLocator * getAddressesfromAddressManager(AddressManager * self, uint32_t num)
{
	uint8_t start = secureRandomInteger(self->rndGen) % 255;
	uint8_t bucketIndex = start;
	uint16_t index = 0;
	int16_t firstEmpty = -1;

	NetworkAddressLocator * addrs = malloc(sizeof(*addrs) * (num + 1));
	if (!addrs) {
		getMessage(self)->onErrorReceived(
				ERROR_OUT_OF_MEMORY,
				"Cannot allocate %i bytes of memory in AddressManagerGetAddresses\n",
				sizeof(*addrs) * (num + 1));
		return NULL;
	}
	uint8_t x = 0;
	while (x < num) {
		if (bucketIndex == 255)
			bucketIndex = 0;
		if (bucketIndex == firstEmpty)
			break;
		Bucket * bucket = self->buckets + bucketIndex;
		if (bucket->numOfAddresses > index) {

			(addrs + x)->addr = bucket->addresses[bucket->numOfAddresses - index - 1];
			(addrs + x)->bucketIndex = bucketIndex;
			(addrs + x)->addrIndex = bucket->numOfAddresses - index - 1;
			incrementReferenceCount((addrs + x)->addr);
			x++;
			firstEmpty = -1;
		} else if (firstEmpty == -1)

			firstEmpty = bucketIndex;

		bucketIndex++;

		if (bucketIndex == start)
			index++;
	}
	(addrs + x)->addr = NULL;
	return addrs;
}
uint8_t addressManagerGetBucketIndex(AddressManager * self,
		NetworkAddress * addr) {
	uint64_t group = addressManagerGetGroup(self, addr);
	randomSeed(self->rndGenForBucketIndexes, group + self->secret);
	return secureRandomInteger(self->rndGenForBucketIndexes) % 255;
}
uint64_t addressManagerGetGroup(AddressManager * self, NetworkAddress * addr) {
	uint8_t start = 0;
	int8_t bits = 16;
	uint64_t group;
	switch (addr->type) {
	case 16:
	case 8:
		group = addr->type;
		start = 6;
		bits = 4;
		break;
	case 32:
	case 64:
		group = 1;
		start = 12;
		break;
	case 128:
		group = 1;
		start = 2;
		break;
	case 256:
		return 1 | ((getByteFromByteArray(addr->ip, 12) ^ 0xFF) << 8)
				| ((getByteFromByteArray(addr->ip, 13) ^ 0xFF) << 16);
	case 512:
		group = 2;
		bits = 36;
		break;
	case 2:
		group = 2;
		bits = 32;
		break;
	case 1:
		group = 1;
		start = 12;
		break;
	default:
		group = addr->type;
		bits = 0;
		break;
	}
	uint8_t x = 8;
	for (; bits >= 8; bits -= 8, x += 8, start++)
		group |= getByteFromByteArray(addr->ip, start) << x;
	if (bits > 0)
		group |= (getByteFromByteArray(addr->ip, start) | ((1 << bits) - 1))
				<< x;
	return group;
}
uint64_t numOfStoredAddresses(AddressManager * self)
{
	assert(self!= NULL);

	uint64_t total = self->peersNum;
	uint8_t x;
	for (x = 0; x < 255; x++) {
		total += (self->buckets + x)->numOfAddresses;
	}
	return total;
}
NetworkAddress * getNetworkAddressFromAddressManager(AddressManager * self,
		NetworkAddress * addr) {

	Bucket * bucket = self->buckets + addressManagerGetBucketIndex(self, addr);
	uint16_t x1;
	for (x1 = 0; x1 < bucket->numOfAddresses; x1++)
		if (compareNetworkAddresses(bucket->addresses[x1], addr))

			return bucket->addresses[x1];
	return NULL;
}
Peer * getPeerNodeFromAddressManager(AddressManager * self, NetworkAddress * addr) {
	uint16_t x;
	for (x = 0; x < self->peersNum; x++)
		if (compareNetworkAddresses(getNetworkAddress(self->peers[x]), addr))
			return self->peers[x];
	return NULL;
}

boolean isReachableIPType(AddressManager * self, IPType type)
{
	assert(self != NULL);
	assert(type != NULL);

	if (type == IP_INVALID) {
		return FALSE;
	}

	return self->reachability & type;
}
void removeNetworkAddressFromAddressManager(AddressManager * self, NetworkAddress * addr) {
	Bucket * bucket = (self->buckets + addressManagerGetBucketIndex(self, addr));
	uint16_t x;
	for (x = 0; x < bucket->numOfAddresses; x++) {
		if (compareNetworkAddresses(addr, bucket->addresses[x])) {
			bucket->numOfAddresses--;
			if (bucket->numOfAddresses) {

				if (bucket->numOfAddresses - x)
					memmove(bucket->addresses + x, bucket->addresses + x + 1,
							(bucket->numOfAddresses - x) * sizeof(*bucket->addresses));

				NetworkAddress ** temp = realloc(bucket->addresses,
						sizeof(*bucket->addresses) * bucket->numOfAddresses);
				if (temp)
					bucket->addresses = temp;
			}
			decrementReferenceCount(bucket->addresses[x]->ip);
			break;
		}
	}
}
void removePeerNodeFromAddressManager(AddressManager * self, Peer * peer) {
	uint16_t peerPos = 0;
	for (;; peerPos++)
		if (self->peers[peerPos] == peer)
			break;

	memmove(self->peers + peerPos, self->peers + peerPos + 1,
			(self->peersNum - peerPos - 1) * sizeof(*self->peers));
	self->peersNum--;
	if (1) {
		NetworkAddress * temp = realloc(peer, sizeof(NetworkAddress));
		if (temp)
			addNetworkAddressToAddressManager(self, temp);
		else
			addNetworkAddressToAddressManager(self, (NetworkAddress *) peer);
	} else
		decrementReferenceCount(peer);
	if (self->peersNum)
		addressManagerAdjustTime(self);
}
uint32_t addressManagerSerialise(AddressManager * self, boolean force,
		void (*onErrorReceived)(Error error, char *, ...)) {
	ByteArray * bytes = getMessage(self)->bytes;
	if (!bytes) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,
				"Attempting to serialise a AddressManager with no bytes.");
		return 0;
	}
	if (bytes->length < 255 * 2 + 12) {
		onErrorReceived(ERROR_OUT_OF_MEMORY,
				"Attempting to serialise a AddressManager with too few bytes.");
		return 0;
	}
	writeInt32AsLittleEndianIntoByteArray(bytes, 0, 3);
	uint32_t cursor = 4;
	uint8_t x;
	for (x = 0; x < 255; x++) {
		Bucket * bucket = self->buckets + x;
		writeInt16AsLittleEndianIntoByteArray(bytes, cursor, bucket->numOfAddresses);
		cursor += 2;
		if (bytes->length
				< cursor + 30 * bucket->numOfAddresses + (255 - x - 1) * 2 + 8) {
			onErrorReceived(ERROR_OUT_OF_MEMORY,
					"Attempting to serialise a AddressManager with too few bytes at the bucket %u.",
					x);
			bucket->numOfAddresses = 0;
			return 0;
		}
		uint16_t y;
		for (y = 0; y < bucket->numOfAddresses; y++) {
			if (force && getMessage(bucket->addresses[y])->serialized)
				decrementReferenceCount(
						getMessage(bucket->addresses[y])->bytes);
			if (!getMessage(bucket->addresses[y])->serialized || force) {
				getMessage(bucket->addresses[y])->bytes =
						getByteArraySubsectionReference(bytes, cursor, 30);
				if (!getMessage(bucket->addresses[y])->bytes) {
					onErrorReceived(ERROR_OUT_OF_MEMORY,
							"Cannot create a new ByteArray sub reference in AddressManagerSerialise for the network address %u - %u.",
							x, y);
					bucket->numOfAddresses = y;
					return 0;
				}
				serialiseNetworkAddress(bucket->addresses[y], 1);
			} else if (getMessage(bucket->addresses[y])->bytes->sharedData
					!= bytes->sharedData) {

				copyByteArrayToByteArray(bytes, cursor,
						getMessage(bucket->addresses[y])->bytes);
				changeByteArrayDataReference(
						getMessage(bucket->addresses[y])->bytes, bytes, cursor);
			}
			cursor += 30;
		}
	}
	writeInt64AsLittleEndianIntoByteArray(bytes, cursor, self->secret);
	getMessage(self)->serialized = 1;
	return cursor + 8;
}
void setIPTypeReachability(AddressManager * self, IPType type, boolean reachable)
{
	assert(self != NULL);
	assert(reachable == TRUE || reachable == FALSE);

	if (reachable) {
		self->reachability |= type;
	}
	else {
		self->reachability &= ~type;
	}
}
boolean addressManagerSetup(AddressManager * self) {
	self->peers = NULL;
	self->peersNum = 0;
	self->reachability = 0;

	memset(self->buckets, 0, sizeof(*self->buckets) * 255);

	if (newSecureRandomGenerator(&self->rndGen)) {
		secureRandomSeed(self->rndGen);
		if (newSecureRandomGenerator(&self->rndGenForBucketIndexes))
			return 1;
		destroySecureRandomGenerator(self->rndGen);
	}
	return 0;
}
boolean addNetworkAddressToAddressManager(AddressManager * self, NetworkAddress * addr) {

	Bucket * bucket = self->buckets + addressManagerGetBucketIndex(self, addr);

	uint16_t insert = 0;
	for (; insert < bucket->numOfAddresses; insert++)
		if (bucket->addresses[insert]->time > addr->time)

			break;
	if (bucket->numOfAddresses == self->maxAddressesInBucket) {

		uint16_t remove = (secureRandomInteger(self->rndGen) % bucket->numOfAddresses);
		remove *= remove;
		remove /= bucket->numOfAddresses;
		decrementReferenceCount(bucket->addresses[remove]);
		if (insert < remove)

			memmove(bucket->addresses + insert + 1, bucket->addresses + insert,
					(remove - insert) * sizeof(*bucket->addresses));
		else if (insert > remove) {
			memmove(bucket->addresses + remove, bucket->addresses + remove + 1,
					(insert - remove) * sizeof(*bucket->addresses));
			insert--;
		}
	} else {
		bucket->numOfAddresses++;
		NetworkAddress ** temp = realloc(bucket->addresses,
				sizeof(*bucket->addresses) * bucket->numOfAddresses);
		if (!temp)
			return 0;
		bucket->addresses = temp;

		if (bucket->numOfAddresses - insert - 1)
			memmove(bucket->addresses + insert + 1, bucket->addresses + insert,
					(bucket->numOfAddresses - insert - 1)
							* sizeof(*bucket->addresses));
	}
	bucket->addresses[insert] = addr;
	return 1;
}
boolean addPeerToAddressManager(AddressManager * self, Peer * peer)
{
	assert(self!= NULL);
	assert(peer != NULL);

	uint16_t insert = 0;
	for (; insert < self->peersNum; insert++)
		if (self->peers[insert]->timeOffset > peer->timeOffset)

			break;

	self->peersNum++;
	Peer ** temp = realloc(self->peers, sizeof(*self->peers) * self->peersNum);
	if (!temp)
		return 0;
	self->peers = temp;

	memmove(self->peers + insert + 1, self->peers + insert,
			(self->peersNum - insert - 1) * sizeof(*self->peers));

	self->peers[insert] = peer;

	addressManagerAdjustTime(self);
	return 1;
}

/*Random*/
boolean newSecureRandomGenerator(uint64_t * gen) {
	*gen = (uint64_t) malloc(32);
	return *gen;
}
void secureRandomSeed(uint64_t gen) {
	FILE * f = fopen("/dev/urandom", "r");
	fread((void *) gen, 32, 1, f);
}
void randomSeed(uint64_t gen, uint64_t seed) {
	memcpy((void *) gen, &seed, 8);
	memset(((uint8_t *) gen) + 8, 0, 24);
}
uint64_t secureRandomInteger(uint64_t gen) {
	Sha256((uint8_t *) gen, 32, (void *) gen);
	uint64_t i;
	memcpy(&i, (void *) gen, 8);
	return i;
}
void destroySecureRandomGenerator(uint64_t gen) {
	free((void *) gen);
}
