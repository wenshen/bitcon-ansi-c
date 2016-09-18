/*
 * NetworkCommunicator.h
 *
 * Created on: Nov 12, 2012
 * Created by: Wen Shen
 * Copyright (c) 2012 MIBitcoin Project Team
 */

/**
 @file
 @brief Used for communicating to other peers. The network communicator can send and receive bitcoin messages and uses function pointers for message handlers. The timeouts are in milliseconds. It is important to understant that a NetworkCommunicator does not guarentee thread safety for everything. Thread safety is only given to the "peers" list. This means it is completely okay to add and remove peers from multiple threads. Two threads may try to access the list at once such as if the NetworkCommunicator receives a socket timeout event and tries to remove an peer at the same time as a thread made by a program using cbitcoin tries to add a new peer. When using a NetworkCommunicator, threading and networking dependencies need to be satisfied, @see Dependencies.h Inherits Object
*/
#ifndef NETWORKCOMMUNICATOR_H_
#define NETWORKCOMMUNICATOR_H_

#include "Peer.h"
#include "../LibevSockets/LibevSockets.h"
#include "AddressManager.h"
#include "../Message/Inventory.h"
#include "../Message/GetBlocks.h"
#include "../Message/Transaction.h"
#include "../Message/BlockHeaders.h"
#include "PingPongMessage.h"
#include "../Message/Alert.h"
#include "NetworkAddress.h"

/**
 @brief Structure for NetworkCommunicator objects. @see NetworkCommunicator.h
*/
typedef struct {
	Object base; /**< Object base structure */
	uint32_t networkID; /**< The 4 byte id for sending and receiving messages for a given network. */
	NetworkCommunicatorFlags flags; /**< Flags for the operation of the NetworkCommunicator. */
	int32_t version; /**< Used for automatic handshaking. This version will be advertised to peers. */
	VersionServices services; /**< Used for automatic handshaking. These services will be advertised */
	ByteArray * userAgent; /**< Used for automatic handshaking. This user agent will be advertised. */
	int32_t blockHeight; /** Set to the current block height for advertising to peers during the automated handshake. */
	NetworkAddress * ourIPv4; /**< IPv4 network address for us. */
	NetworkAddress * ourIPv6; /**< IPv6 network address for us. */
	uint32_t attemptingOrWorkingConnections; /**< All connections being attempted or sucessful */
	uint32_t maxConnections; /**< Maximum number of peers allowed to connect to. */
	uint32_t numIncomingConnections; /**< Number of incomming connections made */
	uint32_t maxIncommingConnections; /**< Maximum number of incomming connections. */
	AddressManager * addresses; /**< All addresses both connected and unconnected */
	uint16_t heartBeat; /**< If the _NETWORK_COMMUNICATOR_AUTO_PING flag is set, the NetworkCommunicator will send a "ping" message to all peers after this interval. bitcoin-qt uses 1800 (30 minutes) */
	uint16_t timeOut; /**< Time of zero contact from a peer before timeout. bitcoin-qt uses 5400 (90 minutes) */
	uint16_t sendTimeOut; /**< Time to wait for a socket to be ready to write before a timeout. */
	uint16_t recvTimeOut; /**< When receiving data after the initial response, the time to wait for the following data before timeout. */
	uint16_t responseTimeOut; /**< Time to wait for a peer to respond to a request before timeout.  */
	uint16_t connectionTimeOut; /**< Time to wait for a socket to connect before timeout. */
	ByteArray * alternativeMessages; /**< Alternative messages to accept. This should be the 12 byte command names each after another with nothing between. Pass NULL for no alternative message types. */
	uint32_t * altMaxSizes; /**< Sizes for the alternative messages. Will be freed by this object, so malloc this and give it to this object. Send in NULL for a default _BLOCK_MAX_SIZE. */
	uint64_t listeningSocketIPv4; /**< The id of a listening socket on the IPv4 network. */
	uint64_t listeningSocketIPv6; /**< The id of a listening socket on the IPv6 network. */
	boolean isListeningIPv4; /**< True when listening for incomming connections on the IPv4 network. False when not. */
	boolean isListeningIPv6; /**< True when listening for incomming connections on the IPv6 network. False when not. */
	uint64_t eventLoop; /**< Socket event loop */
	uint64_t acceptEventIPv4; /**< Event for accepting connections on IPv4 */
	uint64_t acceptEventIPv6; /**< Event for accepting connections on IPv6 */
	uint64_t nonce; /**< Value sent in version messages to check for connections to self */
	uint64_t pingTimer; /**< Timer for ping event */
	boolean isStarted; /**< True if the NetworkCommunicator is running. */
	void * callbackHandler; /**< Sent to event callbacks */
	boolean stoppedListening; /**< True if listening was stopped because there are too many connections */
	void (*onTimeOut)(void *,void *,void *,TimeOutType); /**< Timeout event callback with a void pointer argument for the callback handler followed by a NetworkCommunicator and NetworkAddress. The callback should return as quickly as possible. Use threads for operations that would otherwise delay the event loop for too long. The second argument is the NetworkCommunicator responsible for the timeout. The third argument is the peer with the timeout. Lastly there is the TimeOutType */
	OnMessageReceivedAction (*onMessageReceived)(void *,void *,void *); /**< The callback for when a message has been received from a peer. The first argument in the void pointer for the callback handler. The second argument is the NetworkCommunicator responsible for receiving the message. The third argument is the NetworkAddress peer the message was received from. Return the action that should be done after returning. Access the message by the "receive" field in the NetworkAddress peer. Lookup the type of the message and then cast and/or handle the message approriately. The alternative message bytes can be found in the peer's "alternativeTypeBytes" field. Do not delay the thread for very long. */
	void (*onNetworkError)(void *,void *); /**< Called when both IPv4 and IPv6 fails. Has an argument for the callback handler and then the NetworkCommunicator. */
	void (*onErrorReceived)(Error error,char *,...); /**< Pointer to error callback */
} NetworkCommunicator;

/**
 @brief Creates a new NetworkCommunicator object.
 @returns A new NetworkCommunicator object.
 */
NetworkCommunicator * createNewNetworkCommunicator(void (*onErrorReceived)(Error error,char *,...));

/**
 @brief Gets a NetworkCommunicator from another object. Use this to avoid casts.
 @param self The object to obtain the NetworkCommunicator from.
 @returns The NetworkCommunicator object.
 */
NetworkCommunicator * getNetworkCommunicator(void * self);

/**
 @brief Initializes a NetworkCommunicator object
 @param self The NetworkCommunicator object to initialize
 @returns true on success, false on failure.
 */
boolean initializeNetworkCommunicator(NetworkCommunicator * self,void (*onErrorReceived)(Error error,char *,...));

/**
 @brief Frees a NetworkCommunicator object.
 @param self The NetworkCommunicator object to free.
 */
void destroyNetworkCommunicator(void * self);

/**
 @brief Accepts an incomming connection.
 @param vself The NetworkCommunicator object.
 @param socket The listening socket for accepting a connection.
 */
void acceptIncomingConnection(void * vself, uint64_t socket);
/**
 @brief Returns true if it is beleived the network address can be connected to, otherwise false.
 @param self The NetworkCommunicator object.
 @param addr The NetworkAddress.
 @returns true if it is beleived the peer can be connected to, otherwise false.
 */
boolean canNetworkCommunicatorConnect(NetworkCommunicator * self,NetworkAddress * addr);
/**
 @brief Connects to a peer. This peer will be added to the peer list if it connects correctly.
 @param self The NetworkCommunicator object.
 @param peer The peer to connect to.
 @returns CONNECT_OK if successful. CONNECT_NO_SUPPORT if the IP version is not supported.
 CONNECT_BAD if the connection failed and the address will be penalised.
 CONNECT_FAIL if the connection failed but the address will not be penalised.
 */
ConnectionReturn connectToPeerUsingNetworkCommunicator(NetworkCommunicator * self, Peer * peer);
/**
 @brief Callback for the connection to a peer.
 @param vself The NetworkCommunicator object.
 @param vpeer The Peer that connected.
 */
void onPeerConnectCallback(void * vself, void * vpeer);
/**
 @brief Called when a peer socket is ready for reading.
 @param vself The NetworkCommunicator object.
 @param vpeer The Peer index with data to read.
 */
void onPeerReadyForReadingCallback(void * vself, void * vpeer);
/**
 @brief Called when a peer socket is ready for writing.
 @param vself The NetworkCommunicator object.
 @param vpeer The Peer
 */
void onPeerReadyForWritingCallback(void * vself,void * vpeer);
/**
 @brief Called when a header is received.
 @param self The NetworkCommunicator object.
 @param peer The Peer.
 */
void onHeaderReceivedCallback(NetworkCommunicator * self,Peer * peer);
/**
 @brief Called on an error with the socket event loop. The error event is given with ERROR_NETWORK_COMMUNICATOR_LOOP_FAIL.
 @param vself The NetworkCommunicator object.
 */
void onSocketEventLoopErrorCallback(void * vself);
/**
 @brief Disconnects a peer.
 @param self The NetworkCommunicator object.
 @param peer The peer.
 @param penalty Penalty to the score of the address.
 @param stopping If true, do not call "onNetworkError" because the NetworkCommunicator is stopping.
 */
void disconnectPeer(NetworkCommunicator * self, Peer * peer, uint16_t penalty, boolean stopping);
/**
 @brief Gets a new version message for this.
 @param self The NetworkCommunicator object.
 @param addRecv The NetworkAddress of the receipient.
 */
Version * getNetworkCommunicatorVersion(NetworkCommunicator * self,NetworkAddress * addRecv);
/**
 @brief Processes a new received message for auto discovery.
 @param self The NetworkCommunicator object.
 @param peer The peer
 @returns true if peer should be disconnected, false otherwise.
 */
OnMessageReceivedAction processMessageForAutoDiscovery(NetworkCommunicator * self,Peer * peer);
/**
 @brief Processes a new received message for auto handshaking.
 @param self The NetworkCommunicator object.
 @param peer The peer
 @returns true if peer should be disconnected, false otherwise.
 */
OnMessageReceivedAction processMessageForAutoHandshake(NetworkCommunicator * self,Peer * peer);
/**
 @brief Processes a new received message for auto ping pongs.
 @param self The NetworkCommunicator object.
 @param peer The peer
 @returns true if peer should be disconnected, false otherwise.
 */
OnMessageReceivedAction processMessageForAutoPingPongs(NetworkCommunicator * self, Peer * peer);
/**
 @brief Called when an entire message is received.
 @param self The NetworkCommunicator object.
 @param peer The Peer.
 */
void onMessageReceivedCallback(NetworkCommunicator * self, Peer * peer);

/**
 @brief Called on a timeout error. The peer is removed.
 @param vself The NetworkCommunicator object.
 @param vpeer The Peer index which timedout.
 @param type The type of the timeout
 */
void onTimeOutErrorCallback(void * vself, void * vpeer, TimeOutType type);
/**
 @brief Sends a message by placing it on the send queue. Will serialise standard messages (unless serialised already) but not alternative messages or alert messages.
 @param self The NetworkCommunicator object.
 @param peer The Peer.
 @param message The Message to send.
 @returns true if successful, false otherwise.
 */
boolean sendMessageToPeer(NetworkCommunicator * self, Peer * peer, Message * message);
/**
 @brief Sends pings to all connected peers.
 @param self The NetworkCommunicator object.
 */
void pingConnectedPeers(void * vself);
/**
 @brief Sets the AddressManager.
 @param self The NetworkCommunicator object.
 @param addr The AddressManager
 */
void setAddressManager(NetworkCommunicator * self, AddressManager * addrMan);
/**
 @brief Sets the alternative messages
 @param self The alternative messages as a ByteArray with 12 characters per message command, one after the other.
 @param altMaxSizes An allocated memory block of 32 bit integers with the max sizes for the alternative messages.
 @param addr The AddressManager
 */
void setAlternativeMessages(NetworkCommunicator * self,ByteArray * altMessages,uint32_t * altMaxSizes);
/**
 @brief Sets the IPv4 address for the NetworkCommunicator.
 @param self The NetworkCommunicator object.
 @param addr The IPv4 address as a NetworkAddress.
 */
void setOwnIPv4Address(NetworkCommunicator * self, NetworkAddress * ourIPv4);
/**
 @brief Sets the IPv6 address for the NetworkCommunicator.
 @param self The NetworkCommunicator object.
 @param addr The IPv6 address as a NetworkAddress.
 */
void setOwnIPv6Address(NetworkCommunicator * self, NetworkAddress * ourIPv6);
/**
 @brief Sets the user agent.
 @param self The NetworkCommunicator object.
 @param addr The user agent as a ByteArray.
 */
void setUserAgent(NetworkCommunicator * self, ByteArray * userAgent);
/**
 @brief Starts a NetworkCommunicator by connecting to the peers in the peers list. This starts the socket event loop.
 @param self The NetworkCommunicator object.
 @returns true if the NetworkCommunicator started successfully, false otherwise.
 */
boolean startNetworkCommunicator(NetworkCommunicator * self);
/**
 @brief Causes the NetworkCommunicator to begin listening for incoming connections. "isListeningIPv4" and/or "isListeningIPv6" should be set to true if either IPv4 or IPv6 sockets are active.
 @param vself The NetworkCommunicator object.
 */
void startListeningToIncomingConnections(NetworkCommunicator * self);
/**
 @brief Starts the timer event for sending pings (heartbeat).
 @param vself The NetworkCommunicator object.
 */
void startNetworkCommunicatorPingTimer(NetworkCommunicator * self);
/**
 @brief Closes all connections. This may be neccessary in case of failure in which case NetworkCommunicatorStart can be tried again to reconnect to the listed peers.
 @param vself The NetworkCommunicator object.
 */
void stopNetworkCommunicator(NetworkCommunicator * self);
/**
 @brief Stops listening for both IPv6 connections and IPv4 connections.
 @param vself The NetworkCommunicator object.
 */
void stopListeningToIncomingConnections(NetworkCommunicator * vself);
/**
 @brief Stops the timer event for sending pings (heartbeat).
 @param vself The NetworkCommunicator object.
 */
void stopNetworkCommunicatorPingTimer(NetworkCommunicator * self);
/**
 @brief Looks at the stored addresses and tries to connect to addresses up to the maximum number of allowed connections or as many as
 there are in the case the maximum number of connections is greater than the number of addresses, plus connected peers.
 @param self The NetworkCommunicator object.
 */
void tryConnectingToStoredAddresses(NetworkCommunicator * self);

#endif /* NETWORKCOMMUNICATOR_H_ */
