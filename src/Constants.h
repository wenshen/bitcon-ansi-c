/*
* Constants.h
*
*  Created on: 03/10/2012
*  Created by: Abdulfatai Popoola
*  Modified by: Wen Shen & Attila-Peter Toth
*  Copyright (c) 2012 Bitcoin Project Team
*/
#ifndef CONSTANTS_H_
#define CONSTANTS_H_

/*
 @brief pseudo-boolean type
 */
typedef enum {
	TRUE = 1,
	FALSE = 0
}boolean;

/*
@brief The possible types of messages
*/
typedef enum{
	MESSAGE_TYPE_VERSION = 1, /**< @see Version.h */
	MESSAGE_TYPE_VERACK = 2, /**< Acknowledgment and acceptance of a peer's version and connection. */
	MESSAGE_TYPE_ADDR = 4, /**< @see AddressBroadcast.h */
	MESSAGE_TYPE_INV = 8, /**< @see InventoryBroadcast.h */
	MESSAGE_TYPE_GETDATA = 16, /**< @see InventoryBroadcast.h */
	MESSAGE_TYPE_GETBLOCKS = 32, /**< @see GetBlocks.h */
	MESSAGE_TYPE_GETHEADERS = 64, /**< @see GetBlocks.h */
	MESSAGE_TYPE_TX = 128, /**< @see Transaction.h */
	MESSAGE_TYPE_BLOCK = 256, /**< @see Block.h */
	MESSAGE_TYPE_HEADERS = 512, /**< @see BlockHeaders.h */
	MESSAGE_TYPE_GETADDR = 1024, /**< Request for "active peers". bitcoin-qt consiers active peers to be those that have sent messages in the last 30 minutes. */
	MESSAGE_TYPE_PING = 2048, /**< @see PingPong.h */
	MESSAGE_TYPE_PONG = 4096, /**< @see PingPong.h */
	MESSAGE_TYPE_ALERT = 8192, /**< @see Alert.h */
	MESSAGE_TYPE_ALT = 16384, /**< The message was defined by "alternativeMessages" in a NetworkCommunicator */
	MESSAGE_TYPE_ADDRMAN = 32768, /**< @see AddressManager.h */
	MESSAGE_TYPE_CHAINDESC = 65536 /**< @see ChainDescriptor.h */
}MessageType;

typedef enum{
	ERROR_MESSAGE_CHECKSUM_BAD_SIZE,
	ERROR_MESSAGE_NO_SERIALISATION_IMPLEMENTATION,
	ERROR_MESSAGE_NO_DESERIALISATION_IMPLEMENTATION,
	ERROR_MESSAGE_DESERIALISATION_BAD_BYTES,
	ERROR_MESSAGE_DESERIALISATION_NULL_BYTES,
	ERROR_MESSAGE_SERIALISATION_BAD_BYTES,
	ERROR_MESSAGE_SERIALISATION_NULL_BYTES,
	ERROR_MESSAGE_SERIALISATION_BAD_DATA,
	ERROR_SHA_256_HASH_BAD_BYTE_ARRAY_LENGTH,
	ERROR_BASE58_DECODE_CHECK_TOO_SHORT,
	ERROR_BASE58_DECODE_CHECK_INVALID,
	ERROR_TRANSACTION_FEW_INPUTS,
	ERROR_TRANSACTION_FEW_OUTPUTS,
	ERROR_NETWORK_COMMUNICATOR_LOOP_FAIL,
	ERROR_NETWORK_COMMUNICATOR_LOOP_CREATE_FAIL,
	ERROR_NETWORK_COMMUNICATOR_CONNECT_FAILURE,
	ERROR_OUT_OF_MEMORY,
	ERROR_JSON_READ,
	ERROR_BALANCE,
	ERROR_INIT_FAIL
}Error;

typedef enum{
	COMPARE_MORE_THAN = 1,
	COMPARE_EQUAL = 0,
	COMPARE_LESS_THAN = -1
}Compare;

typedef enum{
	STORAGE_BRANCHES,
	STORAGE_ORPHANS,
	STORAGE_NUM_BRANCHES,
	STORAGE_NUM_ORPHANS,
	STORAGE_MAIN_BRANCH,
	STORAGE_FIRST_ORPHAN
} StorageParts;

/**
 @brief The branch validator parts for a storage key.
 */
typedef enum{
	BRANCH_BLOCK_HEADER,
	BRANCH_BLOCK_TRANSACTIONS,
	BRANCH_NUM_REFS,
	BRANCH_TABLE_HASH,
	BRANCH_TABLE_INDEX,
	BRANCH_LAST_RETARGET,
	BRANCH_PARENT_BRANCH,
	BRANCH_PARENT_INDEX,
	BRANCH_START_HEIGHT,
	BRANCH_LAST_VALIDATION,
	BRANCH_NUM_UNSPENT_OUTPUTS,
	BRANCH_WORK,
	BRANCH_UNSPENT_OUTPUT_BRANCH,
	BRANCH_UNSPENT_OUTPUT_COINBASE,
	BRANCH_UNSPENT_OUTPUT_HASH,
	BRANCH_UNSPENT_OUTPUT_INDEX,
	BRANCH_UNSPENT_OUTPUT_POS,
	BRANCH_UNSPENT_OUTPUT_BLOCK_INDEX
} BranchStorageParts;

typedef enum{
	RECORD_SENT = 0,
	RECORD_RECEIVED = 1
} RecordType;

typedef enum{
	Address_TO = 0,
	Address_FROM = 1
}AddressType;

typedef enum {
	ERROR,  /**< Any data of with this number may be ignored. */
	MSG_TX, /**<Hash is related to a transaction. */
	MSG_BLOCK /*<Hash is related to a data block. */
} InventoryVectorType;

typedef enum{
	SERVICE_FULL_BLOCKS = 1 /**< Service for full blocks. Node maintains the entire blockchain. */
}VersionServices;
/*
 @brief The return values for executeScript. @see Script.h
 */
typedef enum{
	GOOD_SCRIPT, /**< Script is valid. */
	BAD_SCRIPT, /**< Script is invalid */
	SCRIPT_ERR /**< An error occurred, handle the error. */
} ScriptExecutionStatus;

/*
@brief The return values for getTransactionInputHashForVerification.
@see Transaction.h
*/
typedef enum{
   TX_HASH_GOOD, /**< Transaction hash was made OK */
   TX_HASH_BAD, /**< The transaction is invalid and a hash cannot be made. */
   TX_HASH_FAIL /**< An error occured while making the hash. */
} TransactionHashStatus;

#define TRANSACTION_INPUT_FINAL 0xFFFFFFFF /* Transaction input is final*/
#define SEND_QUEUE_MAX_SIZE 10 /* Sent no more than 10 messages at once to a peer.*/

typedef enum{
	SIGHASH_ALL = 0x00000001,
	SIGHASH_NONE = 0x00000002,
	SIGHASH_SINGLE = 0x00000003,
	SIGHASH_ANYONECANPAY = 0x00000080
} SignatureType;

/*
 *@brief The field sizes for various fields in a transaction
 */
typedef enum{
	TX_VERSION_FIELD_SIZE = 4,
	TX_LOCKTIME_FIELD_SIZE = 4,
	TX_INPUT_FIELD_SIZE_EXCLUDING_SCRIPT = 40,
	TX_OUTPUT_FIELD_SIZE_EXCLUDING_SCRIPT = 8
} TransactionFields;


typedef enum{
	SCRIPT_OPCODE_0 = 0x00,
    SCRIPT_OPCODE_FALSE = SCRIPT_OPCODE_0,
    SCRIPT_OPCODE_PUSHDATA1 = 0x4c,
    SCRIPT_OPCODE_PUSHDATA2 = 0x4d,
    SCRIPT_OPCODE_PUSHDATA4 = 0x4e,
    SCRIPT_OPCODE_1NEGATE = 0x4f,
    SCRIPT_OPCODE_RESERVED = 0x50,
    SCRIPT_OPCODE_1 = 0x51,
    SCRIPT_OPCODE_TRUE = 0x51,
    SCRIPT_OPCODE_2 = 0x52,
    SCRIPT_OPCODE_3 = 0x53,
    SCRIPT_OPCODE_4 = 0x54,
    SCRIPT_OPCODE_5 = 0x55,
    SCRIPT_OPCODE_6 = 0x56,
    SCRIPT_OPCODE_7 = 0x57,
    SCRIPT_OPCODE_8 = 0x58,
    SCRIPT_OPCODE_9 = 0x59,
    SCRIPT_OPCODE_10 = 0x5a,
    SCRIPT_OPCODE_11 = 0x5b,
    SCRIPT_OPCODE_12 = 0x5c,
    SCRIPT_OPCODE_13 = 0x5d,
    SCRIPT_OPCODE_14 = 0x5e,
    SCRIPT_OPCODE_15 = 0x5f,
    SCRIPT_OPCODE_16 = 0x60,
    SCRIPT_OPCODE_NOP = 0x61,
    SCRIPT_OPCODE_VER = 0x62,
    SCRIPT_OPCODE_IF = 0x63,
    SCRIPT_OPCODE_NOTIF = 0x64,
    SCRIPT_OPCODE_VERIF = 0x65,
    SCRIPT_OPCODE_VERNOTIF = 0x66,
    SCRIPT_OPCODE_ELSE = 0x67,
    SCRIPT_OPCODE_ENDIF = 0x68,
    SCRIPT_OPCODE_VERIFY = 0x69,
    SCRIPT_OPCODE_RETURN = 0x6a,
    SCRIPT_OPCODE_TOALTSTACK = 0x6b,
    SCRIPT_OPCODE_FROMALTSTACK = 0x6c,
    SCRIPT_OPCODE_2DROP = 0x6d,
    SCRIPT_OPCODE_2DUP = 0x6e,
    SCRIPT_OPCODE_3DUP = 0x6f,
    SCRIPT_OPCODE_2OVER = 0x70,
    SCRIPT_OPCODE_2ROT = 0x71,
    SCRIPT_OPCODE_2SWAP = 0x72,
    SCRIPT_OPCODE_IFDUP = 0x73,
    SCRIPT_OPCODE_DEPTH = 0x74,
    SCRIPT_OPCODE_DROP = 0x75,
    SCRIPT_OPCODE_DUP = 0x76,
    SCRIPT_OPCODE_NIP = 0x77,
    SCRIPT_OPCODE_OVER = 0x78,
    SCRIPT_OPCODE_PICK = 0x79,
    SCRIPT_OPCODE_ROLL = 0x7a,
    SCRIPT_OPCODE_ROT = 0x7b,
    SCRIPT_OPCODE_SWAP = 0x7c,
    SCRIPT_OPCODE_TUCK = 0x7d,
    SCRIPT_OPCODE_CAT = 0x7e, /* Disabled */
    SCRIPT_OPCODE_SUBSTR = 0x7f, /* Disabled */
    SCRIPT_OPCODE_LEFT = 0x80, /* Disabled */
    SCRIPT_OPCODE_RIGHT = 0x81, /* Disabled */
    SCRIPT_OPCODE_SIZE = 0x82,
    SCRIPT_OPCODE_INVERT = 0x83, /* Disabled */
    SCRIPT_OPCODE_AND = 0x84, /* Disabled */
    SCRIPT_OPCODE_OR = 0x85, /* Disabled */
    SCRIPT_OPCODE_XOR = 0x86, /* Disabled */
    SCRIPT_OPCODE_EQUAL = 0x87,
    SCRIPT_OPCODE_EQUALVERIFY = 0x88,
    SCRIPT_OPCODE_RESERVED1 = 0x89,
    SCRIPT_OPCODE_RESERVED2 = 0x8a,
    SCRIPT_OPCODE_1ADD = 0x8b,
    SCRIPT_OPCODE_1SUB = 0x8c,
    SCRIPT_OPCODE_2MUL = 0x8d, /* Disabled */
    SCRIPT_OPCODE_2DIV = 0x8e, /* Disabled */
    SCRIPT_OPCODE_NEGATE = 0x8f,
    SCRIPT_OPCODE_ABS = 0x90,
    SCRIPT_OPCODE_NOT = 0x91,
    SCRIPT_OPCODE_0NOTEQUAL = 0x92,
    SCRIPT_OPCODE_ADD = 0x93,
    SCRIPT_OPCODE_SUB = 0x94,
    SCRIPT_OPCODE_MUL = 0x95, /* Disabled */
    SCRIPT_OPCODE_DIV = 0x96, /* Disabled */
    SCRIPT_OPCODE_MOD = 0x97, /* Disabled */
    SCRIPT_OPCODE_LSHIFT = 0x98, /* Disabled */
    SCRIPT_OPCODE_RSHIFT = 0x99, /* Disabled */
    SCRIPT_OPCODE_BOOLAND = 0x9a,
    SCRIPT_OPCODE_BOOLOR = 0x9b,
    SCRIPT_OPCODE_NUMEQUAL = 0x9c,
    SCRIPT_OPCODE_NUMEQUALVERIFY = 0x9d,
    SCRIPT_OPCODE_NUMNOTEQUAL = 0x9e,
    SCRIPT_OPCODE_LESSTHAN = 0x9f,
    SCRIPT_OPCODE_GREATERTHAN = 0xa0,
    SCRIPT_OPCODE_LESSTHANOREQUAL = 0xa1,
    SCRIPT_OPCODE_GREATERTHANOREQUAL = 0xa2,
    SCRIPT_OPCODE_MIN = 0xa3,
    SCRIPT_OPCODE_MAX = 0xa4,
    SCRIPT_OPCODE_WITHIN = 0xa5,
    SCRIPT_OPCODE_RIPEMD160 = 0xa6,
    SCRIPT_OPCODE_SHA1 = 0xa7,
    SCRIPT_OPCODE_SHA256 = 0xa8,
    SCRIPT_OPCODE_HASH160 = 0xa9,
    SCRIPT_OPCODE_HASH256 = 0xaa,
    SCRIPT_OPCODE_CODESEPARATOR = 0xab,
    SCRIPT_OPCODE_CHECKSIG = 0xac,
    SCRIPT_OPCODE_CHECKSIGVERIFY = 0xad,
    SCRIPT_OPCODE_CHECKMULTISIG = 0xae,
    SCRIPT_OPCODE_CHECKMULTISIGVERIFY = 0xaf,
    SCRIPT_OPCODE_NOP1 = 0xb0,
    SCRIPT_OPCODE_NOP2 = 0xb1,
    SCRIPT_OPCODE_NOP3 = 0xb2,
    SCRIPT_OPCODE_NOP4 = 0xb3,
    SCRIPT_OPCODE_NOP5 = 0xb4,
    SCRIPT_OPCODE_NOP6 = 0xb5,
    SCRIPT_OPCODE_NOP7 = 0xb6,
    SCRIPT_OPCODE_NOP8 = 0xb7,
    SCRIPT_OPCODE_NOP9 = 0xb8,
    SCRIPT_OPCODE_NOP10 = 0xb9,
    SCRIPT_OPCODE_SMALLINTEGER = 0xfa,
    SCRIPT_OPCODE_PUBKEYS = 0xfb,
    SCRIPT_OPCODE_PUBKEYHASH = 0xfd,
    SCRIPT_OPCODE_PUBKEY = 0xfe,
    SCRIPT_OPCODE_INVALID = 0xff
}ScriptOpcode;

typedef enum{
	SCRIPT_OP_0 = 0x00,
    SCRIPT_OP_FALSE = SCRIPT_OP_0,
    SCRIPT_OP_PUSHDATA1 = 0x4c,
    SCRIPT_OP_PUSHDATA2 = 0x4d,
    SCRIPT_OP_PUSHDATA4 = 0x4e,
    SCRIPT_OP_1NEGATE = 0x4f,
    SCRIPT_OP_RESERVED = 0x50,
    SCRIPT_OP_1 = 0x51,
    SCRIPT_OP_TRUE = SCRIPT_OP_1,
    SCRIPT_OP_2 = 0x52,
    SCRIPT_OP_3 = 0x53,
    SCRIPT_OP_4 = 0x54,
    SCRIPT_OP_5 = 0x55,
    SCRIPT_OP_6 = 0x56,
    SCRIPT_OP_7 = 0x57,
    SCRIPT_OP_8 = 0x58,
    SCRIPT_OP_9 = 0x59,
    SCRIPT_OP_10 = 0x5a,
    SCRIPT_OP_11 = 0x5b,
    SCRIPT_OP_12 = 0x5c,
    SCRIPT_OP_13 = 0x5d,
    SCRIPT_OP_14 = 0x5e,
    SCRIPT_OP_15 = 0x5f,
    SCRIPT_OP_16 = 0x60,
    SCRIPT_OP_NOP = 0x61,
    SCRIPT_OP_VER = 0x62,
    SCRIPT_OP_IF = 0x63,
    SCRIPT_OP_NOTIF = 0x64,
    SCRIPT_OP_VERIF = 0x65,
    SCRIPT_OP_VERNOTIF = 0x66,
    SCRIPT_OP_ELSE = 0x67,
    SCRIPT_OP_ENDIF = 0x68,
    SCRIPT_OP_VERIFY = 0x69,
    SCRIPT_OP_RETURN = 0x6a,
    SCRIPT_OP_TOALTSTACK = 0x6b,
    SCRIPT_OP_FROMALTSTACK = 0x6c,
    SCRIPT_OP_2DROP = 0x6d,
    SCRIPT_OP_2DUP = 0x6e,
    SCRIPT_OP_3DUP = 0x6f,
    SCRIPT_OP_2OVER = 0x70,
    SCRIPT_OP_2ROT = 0x71,
    SCRIPT_OP_2SWAP = 0x72,
    SCRIPT_OP_IFDUP = 0x73,
    SCRIPT_OP_DEPTH = 0x74,
    SCRIPT_OP_DROP = 0x75,
    SCRIPT_OP_DUP = 0x76,
    SCRIPT_OP_NIP = 0x77,
    SCRIPT_OP_OVER = 0x78,
    SCRIPT_OP_PICK = 0x79,
    SCRIPT_OP_ROLL = 0x7a,
    SCRIPT_OP_ROT = 0x7b,
    SCRIPT_OP_SWAP = 0x7c,
    SCRIPT_OP_TUCK = 0x7d,
    SCRIPT_OP_CAT = 0x7e, /*Disabled*/
    SCRIPT_OP_SUBSTR = 0x7f, /*Disabled*/
    SCRIPT_OP_LEFT = 0x80, /*Disabled*/
    SCRIPT_OP_RIGHT = 0x81, /*Disabled*/
    SCRIPT_OP_SIZE = 0x82,
    SCRIPT_OP_INVERT = 0x83, /*Disabled*/
    SCRIPT_OP_AND = 0x84, /*Disabled*/
    SCRIPT_OP_OR = 0x85, /*Disabled*/
    SCRIPT_OP_XOR = 0x86, /*Disabled*/
    SCRIPT_OP_EQUAL = 0x87,
    SCRIPT_OP_EQUALVERIFY = 0x88,
    SCRIPT_OP_RESERVED1 = 0x89,
    SCRIPT_OP_RESERVED2 = 0x8a,
    SCRIPT_OP_1ADD = 0x8b,
    SCRIPT_OP_1SUB = 0x8c,
    SCRIPT_OP_2MUL = 0x8d, /*Disabled*/
    SCRIPT_OP_2DIV = 0x8e, /*Disabled*/
    SCRIPT_OP_NEGATE = 0x8f,
    SCRIPT_OP_ABS = 0x90,
    SCRIPT_OP_NOT = 0x91,
    SCRIPT_OP_0NOTEQUAL = 0x92,
    SCRIPT_OP_ADD = 0x93,
    SCRIPT_OP_SUB = 0x94,
    SCRIPT_OP_MUL = 0x95, /*Disabled*/
    SCRIPT_OP_DIV = 0x96, /*Disabled*/
    SCRIPT_OP_MOD = 0x97, /*Disabled*/
    SCRIPT_OP_LSHIFT = 0x98, /*Disabled*/
    SCRIPT_OP_RSHIFT = 0x99, /*Disabled*/
    SCRIPT_OP_BOOLAND = 0x9a,
    SCRIPT_OP_BOOLOR = 0x9b,
    SCRIPT_OP_NUMEQUAL = 0x9c,
    SCRIPT_OP_NUMEQUALVERIFY = 0x9d,
    SCRIPT_OP_NUMNOTEQUAL = 0x9e,
    SCRIPT_OP_LESSTHAN = 0x9f,
    SCRIPT_OP_GREATERTHAN = 0xa0,
    SCRIPT_OP_LESSTHANOREQUAL = 0xa1,
    SCRIPT_OP_GREATERTHANOREQUAL = 0xa2,
    SCRIPT_OP_MIN = 0xa3,
    SCRIPT_OP_MAX = 0xa4,
    SCRIPT_OP_WITHIN = 0xa5,
    SCRIPT_OP_RIPEMD160 = 0xa6,
    SCRIPT_OP_SHA1 = 0xa7,
    SCRIPT_OP_SHA256 = 0xa8,
    SCRIPT_OP_HASH160 = 0xa9,
    SCRIPT_OP_HASH256 = 0xaa,
    SCRIPT_OP_CODESEPARATOR = 0xab,
    SCRIPT_OP_CHECKSIG = 0xac,
    SCRIPT_OP_CHECKSIGVERIFY = 0xad,
    SCRIPT_OP_CHECKMULTISIG = 0xae,
    SCRIPT_OP_CHECKMULTISIGVERIFY = 0xaf,
    SCRIPT_OP_NOP1 = 0xb0,
    SCRIPT_OP_NOP2 = 0xb1,
    SCRIPT_OP_NOP3 = 0xb2,
    SCRIPT_OP_NOP4 = 0xb3,
    SCRIPT_OP_NOP5 = 0xb4,
    SCRIPT_OP_NOP6 = 0xb5,
    SCRIPT_OP_NOP7 = 0xb6,
    SCRIPT_OP_NOP8 = 0xb7,
    SCRIPT_OP_NOP9 = 0xb8,
    SCRIPT_OP_NOP10 = 0xb9,
    SCRIPT_OP_SMALLINTEGER = 0xfa,
    SCRIPT_OP_PUBKEYS = 0xfb,
    SCRIPT_OP_PUBKEYHASH = 0xfd,
    SCRIPT_OP_PUBKEY = 0xfe,
    SCRIPT_OP_INVALIDOPCODE = 0xff,
}ScriptOp;

typedef enum {
	INVENTORYVECTOR_TYPE_FIELD_SIZE = 4,
	INVENTORYVECTOR_HASH_FIELD_SIZE = 32,
	INVENTORYVECTOR_TOTAL_SIZE = 36
}InventoryVectorFieldSizes;

typedef enum{
	NETWORK_COMMUNICATOR_AUTO_HANDSHAKE = 1, /**< Automatically share version and verack messages with new connections. */
	NETWORK_COMMUNICATOR_AUTO_DISCOVERY = 2, /**< Automatically discover peers and connect up to the maximum allowed connections using the supplied Version. This involves the exchange of version messages and addresses. */
	NETWORK_COMMUNICATOR_AUTO_PING = 4 /**< Send ping messages every "heartBeat" automatically. If the protocol version in the Version message is 60000 or over, bitcoin will use the new ping-pong specification. @see PingPongMessage.h */
}NetworkCommunicatorFlags;

/*
 @brief The action for a NetworkCommunicator to complete after the onMessageReceived handler returns.
 */
typedef enum{
	MESSAGE_ACTION_CONTINUE, /**< Continue as normal */
	MESSAGE_ACTION_DISCONNECT, /**< Disconnect the peer */
	MESSAGE_ACTION_STOP, /**< Stop the NetworkCommunicator */
	MESSAGE_ACTION_RETURN /**< Return from the message handler with no action. */
}OnMessageReceivedAction;

#define SEND_QUEUE_MAX_SIZE 10 /* Send no more than 10 messages at once to a peer.*/
#define ADDR_TIME_VERSION 31402 /* Version where times are included in addr messages.*/
#define MIN_PROTO_VERSION 209
#define PONG_VERSION 60000 /* Version where pings are responded with pongs.*/
#define SOCKET_CONNECTION_CLOSE -1
#define SOCKET_FAILURE -2
#define VERSION_MAX_SIZE 492 /* Includes 400 characters for the user-agent and the 9 byte var int.*/
#define NUM_SECONDS_IN_A_DAY 86400
#define MAX_RESPONSES_EXPECTED 3 /* A pong, an inventory broadcast and an address broadcast.*/
#define TARGET_INTERVAL 1209600 /* Two week interval*/
#define MAX_TARGET 0x1D00FFFF
#define LOCKTIME_THRESHOLD 500000000 /* Below this value it is a blok number, else it is a time.*/
#define COINBASE_MATURITY 100 /* Number of confirming blocks before a block reward can be spent.*/
#define MAX_SIG_OPS 20000 /* Maximum signature operations in a block.*/
#define GET_DATA_MAX_SIZE 50000
#define GET_BLOCKS_MAX_SIZE 16045
#define GET_HEADERS_MAX_SIZE 64045
#define BLOCK_HEADERS_MAX_SIZE 178009
#define INVENTORY_BROADCAST_MAX_SIZE 50000
#define BLOCK_MAX_SIZE 1000000 /* ~0.95 MB Not including header.*/
#define TRANSACTION_MAX_SIZE BLOCK_MAX_SIZE - 81 /* Minus the header*/
#define ADDRESS_BROADCAST_MAX_SIZE 1000
#define ALERT_MAX_SIZE 2100 /* Max size for payload is 2000. Extra 100 for signature though the signature will likely be less.*/
#define OUTPUT_VALUE_MINUS_ONE 0xFFFFFFFFFFFFFFFF /* In twos complement it represents -1. Bitcoin uses twos compliment.*/
#define ONE_BITCOIN 100000000 /* Each bitcoin has 100 million satoshis (individual units).*/
#define MAX_MONEY 21000000 * ONE_BITCOIN /* 21 million Bitcoins.*/
#define PRODUCTION_NETWORK_BYTE 0 // The normal network for trading
#define NUMBER_OF_SEED_PEERS 320
#define DATA_DIRECTORY "/.cbitcoin_FullNode_Data/"
#define ADDRESS_DATA_FILE "addresses.dat"
#define VALIDATION_DATA_FILE "validation.dat"
#define MAX_ORPHAN_CACHE 20
#define MAX_BRANCH_CACHE 5
#define Int16ToArray(arr,offset,i) arr[offset] = i; \
									 arr[offset + 1] = i >> 8;
#define Int32ToArray(arr,offset,i) Int16ToArray(arr,offset,i) \
									 arr[offset + 2] = i >> 16; \
							         arr[offset + 3] = i >> 24;
#define Int64ToArray(arr,offset,i) Int32ToArray(arr,offset,i) \
									 arr[offset + 4] = i >> 32; \
									 arr[offset + 5] = i >> 40; \
									 arr[offset + 6] = i >> 48; \
									 arr[offset + 7] = i >> 56;
#define ArrayToInt16(arr,offset) arr[offset] \
                                     | (uint16_t)arr[offset + 1] << 8
#define ArrayToInt32(arr,offset) ArrayToInt16(arr,offset) \
                                     | (uint32_t)arr[offset + 2] << 16 \
									 | (uint32_t)arr[offset + 3] << 24
#define ArrayToInt48(arr,offset) ArrayToInt32(arr,offset) \
                                     | (uint64_t)arr[offset + 4] << 32 \
                                     | (uint64_t)arr[offset + 5] << 40
#define ArrayToInt64(arr,offset) ArrayToInt48(arr,offset) \
                                     | (uint64_t)arr[offset + 6] << 48 \
                                     | (uint64_t)arr[offset + 7] << 56
#define Int32ToArrayBigEndian(arr,offset,i) arr[offset] = i >> 24; \
										      arr[offset + 1] = i >> 16;\
                                              arr[offset + 2] = i >> 8; \
                                              arr[offset + 3] = i;
#define ArrayToInt32BigEndian(arr,offset) arr[offset] << 24 \
                                              | (uint16_t)arr[offset + 1] << 16 \
                                              | (uint32_t)arr[offset + 2] << 8 \
                                              | (uint32_t)arr[offset + 3]
#define BTREE_ORDER 32 /* Algorithm only works with even values. Best with powers of 2. This refers to the number of elements and not children.*/
#define BTREE_HALF_ORDER BTREE_ORDER/2
#define WALLET  "wallet"

typedef enum{
	IP_INVALID = 0,
	IP_IPv4 = 1,
	IP_IPv6 = 2,
	IP_LOCAL = 4,
	IP_TOR = 8,
	IP_I2P = 16,
	IP_SITT = 32,
	IP_RFC6052 = 64,
	IP_6TO4 = 128,
	IP_TEREDO = 256,
	IP_HENET = 512
}IPType;

typedef enum{
	CONNECT_OK, /** < Connection is successful*/
	CONNECT_NO_SUPPORT, /** < IP version not supported*/
	CONNECT_BAD, /** < Connection failed, address will be penalized*/
	CONNECT_FAIL /** < Connection failed, address will be not penalized */
} ConnectionReturn;

typedef enum{
	TIMEOUT_CONNECT,
	TIMEOUT_RESPONSE,
	TIMEOUT_NO_DATA,
	TIMEOUT_SEND,
	TIMEOUT_RECEIVE
} TimeOutType;

typedef enum{
	SOCKET_OK,
	SOCKET_NO_SUPPORT,
	SOCKET_BAD
} SocketReturn;
typedef enum{
	INPUT = 0,
	OUTPUT = 1
}UMode;
typedef enum{
	ADDRESS_PUBKEYHASH=0,
	ADDRESS_SCRIPTHASH=5,
	ADDRESS_TESTNETPUBKEYHASH=111
} AddressNetVersionType;

typedef enum{
	BLOCK_STATUS_MAIN, /**< The block has extended the main branch. */
	BLOCK_STATUS_SIDE, /**< The block has extended a branch which is not the main branch. */
	BLOCK_STATUS_ORPHAN, /**< The block is an orphan */
	BLOCK_STATUS_BAD, /**< The block is not ok. */
	BLOCK_STATUS_BAD_TIME, /**< The block has a bad time */
	BLOCK_STATUS_DUPLICATE, /**< The block has already been received. */
	BLOCK_STATUS_ERROR, /**< There was an error while processing a block */
	BLOCK_STATUS_ERROR_BAD_DATA, /**< There was an error while processing a block and it has left corrupted data on the filesystem. Recovery is required before continuing. */
	BLOCK_STATUS_ERROR_BAD_MEMORY, /**< There was an error while processing a block and the memory is corrupted. The branch for the added block needs to be reloaded. */
	BLOCK_STATUS_CONTINUE, /**< Continue with the validation */
} BlockStatus;
/**
 @brief The return type for FullValidatorLoadBranchValidator and FullValidatorLoadValidator.
 */
typedef enum{
	VALIDATOR_LOAD_OK, /**< Load was successful */
	VALIDATOR_LOAD_ERR, /**< Load failure */
	VALIDATOR_LOAD_ERR_DATA, /**< Load failure, ensure all related validation files are deleted before continuing. */
} ValidatorLoadResult;

/**
 @brief The return type for FullValidatorCompleteBlockValidation
 */
typedef enum{
	BLOCK_VALIDATION_OK, /**< The validation passed with no problems. */
	BLOCK_VALIDATION_BAD, /**< The block failed valdiation. */
	BLOCK_VALIDATION_ERR, /**< There was an error during the validation processing. */
} BlockValidationResult;

#endif /* CONSTANTS_H_ */
