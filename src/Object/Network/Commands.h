/*
*  Commands.h
*
*  Created on: 23/10/2012.
*  Created by: Prajowal Manandhar
*  Modified by:
*  Copyright (c) 2012 Bitcoin Project Team
*  please read https://en.bitcoin.it/wiki/Protocol_Specification
*/

#ifndef COMMANDS_H_
#define COMMANDS_H_

/**
 @Service matching for "This node can be asked for full blocks instead of just headers."
 */
#define SERVICE_NODE_NETWORK 1



struct InventoryVector {
	/**< 0 for ERROR, 1 for MSG_TX and 2 for MSG_BLOCK*/
	uint32_t type;

	char hash[32];
};

struct TranxIn{
	struct Outpoint *previous_out;
	uint32_t scriptLength;
	unsigned char signature_script[];
	uint32_t sequence;

};

struct Outpoint{
	char hash[32];
	uint32_t index;
};

struct TranxOut{
	uint64_t value;
	uint32_t pkScriptLength;
	unsigned char pkScript[];

};

struct CmdVersion {


	/**<  Identifies protocol version being used by the node */
	int32_t version;

	/**<  Bitfield of features to be enabled for this connection */
	uint64_t services;

	/**<  Standard UNIX timestamp in seconds */
	int64_t timestamp;

	/**<  The network address of the node receiving this message */
	struct NetworkAddress addrRecv;

	/**< If version >= 106 then folowing fields added*/

	/**<  The network address of the node emitting this message */
	struct NetworkAddress addrFrom;

	/**
	 @Node random nonce, randomly generated every time
	 @a version packet is sent. This nonce is used to detect connections to self.
	 */
	uint64_t nonce;

	/**<  Secondary Version information (0x00 if string is 0 bytes long) */
	char *subVersionNum;

	/**<  if version >= 209 then folowing fields added*/

	/**< The last block received by the emitting node */
	int32_t startHeight;
};

struct CmdVerack {
	/**< Magic value indicating message origin network, and used to seek to next message when stream state is unknown  */
	uint32_t magic;

	/**< contains the verack to indicate the version ack*/
	char command[12];
	uint32_t length;
	uint32_t checksum;
	unsigned char *payload;
};

struct CmdAddr {

	/**<  Number of address entries */
	uint32_t count;
	/**<  Divided addr_list to two arrays */
	/**<  Corresponding Timestamp list */
	uint32_t *tslist;
	/**<  Network addr list */
	struct NetworkAddress *naddrlist;
};

struct CmdInv {
	/**< 0 for ERROR, 1 for MSG_TX and 2 for MSG_BLOCK */
	uint32_t type;

	char hash[32];
};

struct CmdGetData {
	/**< number of inventory entries */
	uint64_t count;

	/**< inventory vectors */
	struct InventoryVector *InvVect;
};

struct CmdGetBlocks {

	uint32_t version;
	uint32_t startCount;
	char blockLocatorHashes[32];
	char hashStop[32];

};

struct CmdGetHeaders {
	uint32_t startCount;
	char hashStart[32];
	char hashStop[32];
};

struct CmdTranx {
	uint32_t version;
	uint32_t TranxIn_count;
	struct TranxIn *TranxInput;
	uint32_t TranxOut_count;
	struct TranxOut *TranxOutput;
	uint32_t lock_time;
};

struct CmdBlock {

	uint32_t version;
	char prev_block[32];
	char markle_root[32];
	uint32_t timestamp;
	uint32_t bits;
	uint32_t nonce;
	uint32_t txnCount;
	struct tx *txns;

};

struct CmdHeaders {

};

struct CmdGetAddr {

};

struct CmdCheckOrder {

};

struct CmdSubmitOrder {

};

struct CmdReply {

};

struct CmdPing {

};

struct CmdAlert {

};


#endif /* COMMANDS_H_ */
