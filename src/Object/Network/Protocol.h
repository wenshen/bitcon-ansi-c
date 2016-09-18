/*
*  Protocol.h
*
*  Created on: 23/10/2012.
*  Created by: Prajowal Manandhar
*  Modified by:
*  Copyright (c) 2012 Bitcoin Project Team
*/
/**
 @file
 @brief This class is responsible for the Protocol functionality described in the Bitcoin.
*/

#ifndef PROTOCOL_H_
#define PROTOCOL_H_



#pragma pack(push, 1)
struct IrcAddr
{
	int ip;
	short port;
};
#pragma pack(pop)


#pragma pack(push, 1)
struct NetAddr
{
	unsigned int time;
	long int services;
	unsigned char ip[16]; /* IPv4/6 */
	unsigned short port;
};
#pragma pack(pop)



#pragma pack(push, 1)
struct CmdHeader {
	unsigned int magic;
	char command[12];
	unsigned int length;
	unsigned char CheckSum[4];
	};
#pragma pack(pop)

#pragma pack(push, 1)
struct CmdHeaderCheckSum {
	unsigned int magic;
	char command[12];
	unsigned int length;
	};
#pragma pack(pop)

/*4 	version 	int32_t 	Identifies protocol version being used by the node
8 	services 	uint64_t 	bitfield of features to be enabled for this connection
8 	timestamp 	int64_t 	standard UNIX timestamp in seconds
26 	addrRecv 	NetAddr 	The network address of the node receiving this message
version >= 106
26 	addrFrom 	NetAddr 	The network address of the node emitting this message
8 	nonce 	uint64_t 	Node random nonce, randomly generated every time a version packet is sent. This nonce is used to detect connections to self.
? 	sub_version_num 	var_str 	Secondary Version information (0x00 if string is 0 bytes long)
version >= 209
4 	start_height 	int32_t 	The last block received by the emitting node */
#pragma pack(push, 1)
struct CmdVersion
{
	unsigned int version;
	long int services;
	long int timestamp;
	struct NetAddr addrRecv;


	struct NetAddr addrFrom;
	long int nonce;

};
#pragma pack(pop)



#pragma pack(push, 1)
struct BlockHeader
{
	unsigned int version;
	char prevBlock[32];
	unsigned int timestamp;
	unsigned int bits;
	unsigned int nonce;
	char transxCount;
	};
#pragma pack(pop)


#pragma pack(push, 1)
struct CmdAddr {

	/* Number of address entries */
	unsigned int count;
	/* Divided addr_list to two arrays */
	/* Corresponding Timestamp list */
	unsigned int *tslist;
	/* Network addr list */
	struct NetAddr *naddrlist;
};
#pragma pack(pop)


#pragma pack(push, 1)
struct InventoryVector {
	/*0 for ERROR, 1 for MSG_TX and 2 for MSG_BLOCK */
	unsigned int type;
	char hash[32];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct CmdInv {
	unsigned int count;
	struct InventoryVector *InventoryVectors;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BlockSpck {
	unsigned int version;
	unsigned int startCount;
	char blockLocatorHashes[32];
	char hashStop[32];

};
#pragma pack(pop)

#pragma pack(push, 1)
struct HeaderSpck {
	int startCount;
	char hashStart[32];
	char hashStop[32];

};
#pragma pack(pop)

#pragma pack(push, 1)
struct Outpoint {
	char hash[32];
	unsigned int index;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct TranxIn {
	struct Outpoint previousOut;
	unsigned int scriptLength;
	unsigned char *signatureScript;
	unsigned int sequence;
};
#pragma pack(pop)


#pragma pack(push, 1)
struct TranxOut {
	unsigned int value;
	unsigned int pkScriptLength;
	unsigned char *pkScript;
};
#pragma pack(pop)


#pragma pack(push, 1)
struct CmdTranx {
	unsigned int version;
	unsigned int tranxInCount;
	struct TranxIn *TranxInput;
	unsigned int tranxOutCount;
	struct TranxOut *TranxOutput;
	unsigned int lockTime;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct CmdBlock {
	unsigned int version;
	char prevBlock[32];
	unsigned int timestamp;
	unsigned int bits;
	unsigned int nonce;
	unsigned int transxCount;
	struct CmdTranx *txns;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct Headers {
	long int count;
	struct BlockHeader *headers;
};
#pragma pack(pop)




#endif /* PROTOCOL_H_ */
