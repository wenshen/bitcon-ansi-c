/*
 *  LibevSockets.h
 *
 *  Created on: 12/11/2012
 *  Created by: Attila Peter Toth
 *  Copyright (c) 2012 Bitcoin Project Team
 */

/**
 @file
 @brief This is an implementation of the networking dependencies for cbitcoin.
  It can be included as as a source file in any projects using cbitcoin which
 require networking capabilities. This file uses libev for efficient event-based
  sockets and POSIX threads. The sockets are POSIX sockets with compatibility functions
   taken from libev to try and be compatible with windows.
 */

#include <pthread.h> /* POSIX threads*/
 #include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include "../Object.h"
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

#ifndef LIBEVSOCKETS_H_
#define LIBEVSOCKETS_H_

/* Define if we have SO_NOSIGPIPE
 * Pipes are used for IPC (Inter-Process Communication)
 * */

#ifdef SO_NOSIGPIPE
#define NOSIGPIPE TRUE
#else
#define NOSIGPIPE FALSE
#define SO_NOSIGPIPE 0
#endif

/* Define send flags*/

#ifdef MSG_NOSIGNAL
#define SEND_FLAGS MSG_NOSIGNAL
#else
#define SEND_FLAGS 0
#endif

struct ev_async{

};

typedef struct {

	struct ev_async base;
	void * loop;

} AsyncEvent;

typedef struct {

	struct ev_loop * base;
	void (*onError)(void *);
	void (*onTimeOut)(void *,void *,TimeOutType);               /**< Callback for timeouts */
	void * communicator;
	pthread_t loopThread;                                       /**< The thread ID for the event loop. */
	void  (*userCallback)(void *);
	void * userArg;
	AsyncEvent * userEvent;
	                                                           /*---MY CODE BEGINS---*/

	volatile short int  term_cond;                             /*Set to TRUE before destroying the EventLoop. Caution, it will result
	                                                           in the termination of the inner thread, which does the work*/

} EventLoop;

union OnEvent {

	void (*i)(void *,uint64_t);
	void (*ptr)(void *,void *);
};                                                               /*used at the callbacks methods*/

/*-------------------MY CODE -------------------------*/

typedef struct _list {

	struct _list *next;
	uint64_t ioEventID;                                        /*It holds basically an _IOEvent object by casting the structure to an unsigned int*/

} EventList;

typedef struct {

	uint16_t maxTimeOut;                                      /*defines the maximum timeout allowed before an event has to happen*/
	int socketID;                                             /*socket used for the IO event*/
	void * peer;                                              /*helps to identify the peer*/
	uint64_t loopID;                                          /*Cast to _EventLoop when used*/
	UMode mode;                                               /*Signals if an event is input or output event*/
	time_t added;                                             /*the time when the event was added to the _EventLoop*/
	union OnEvent onEvent;                                    /*callback method in case of the event, either (i)nput or (o)utput*/
	void (*timeoutCallback)(void*);                           /*callback if the event timed out*/

} _IOEvent;

typedef struct {

	void * communicator;
	pthread_mutex_t iEv_mutex, oEv_mutex, cond_mutex;        /*peer_ protects the peer list in the AddressManager. cond_ protects the thread - @see network_thread below */
	pthread_t network_thread;                                /*the actual worker thread*/
	volatile short int  term_cond;                           /*Set to TRUE before destroying the EventLoop. Caution, it will result in the termination of the inner thread, which does the work. Do not modify without the cond_mutex. You should use this only a the exitEventLoop function*/
	void (*onError)(void *);                                 /*Callback method for error*/
	void (*onTimeOut)(void *,void *,TimeOutType);            /**< Callback for timeouts */
	EventList *iEvents;                                      /*list of _IOEvents. Store only INPUT events here! Loop through this list every time the select returns and find out which event(s) can be executed */
	EventList *oEvents;                                      /*list of OUTPUT events. YOur job is the same as above*/
} _EventLoop;

/*---------------------------MY CODE ENDS -------------------------*/


struct ev_timer {

};

struct ev_io{

};

typedef struct {

	struct ev_timer base;
	EventLoop * loop;
	void (*callback)(void *);
	void * arg;
	void * peer;
} Timer;

typedef struct {

	struct ev_io base;                                           /**< libev event.*/
	EventLoop * loop;                                            /**< For getting timeout events */
	union OnEvent onEvent;
	int socket;
	void * peer;
	void (*timerCallback)(struct ev_loop *, struct ev_timer *, int);
	Timer * timeout;

} IOEvent;


/**
 * @params socketID; actually is a return parameter, it will be a non-blocking sigpipe-free socket
 * @params IPv6 - boolean value specifying the IP version
 * @returns @see Constants.h
 */
SocketReturn createNewSocket(uint64_t * socketID,short int IPv6);


/**
 * @params sockectID: closes the specified file descriptor/socket
 */
void closeSocket(uint64_t socketID);

boolean socketConnect(uint64_t socketID, uint8_t * IP, short int  IPv6, uint16_t port);
/**
 * @brief creates a new EventList setting  it's fields.
 */
void initEventList(EventList *e,uint64_t eventID);
/**
 * @brief Add an event to the eventloop loop.
 */
boolean socketAddEvent(uint64_t newEventID,uint16_t timeout);
/**
 * @brief removes an EventList form the eventloop.
 */
boolean socketRemoveEvent(uint64_t beRemoved);

void * startEventLoop(void* vloop);

void exitEventLoop(uint64_t loopID);

boolean newEventLoop(uint64_t * loopID,void (*onError)(void *),void (*onDidTimeout)(void *,void *,TimeOutType),void * communicator);



void canAccept(struct ev_loop * loop,struct ev_io * watcher,int eventID);

void didConnect(struct ev_loop * loop,struct ev_io * watcher,int eventID);

void didConnectTimeout(struct ev_loop * loop,struct ev_timer * watcher,int eventID);

void canSend(struct ev_loop * loop,struct ev_io * watcher,int eventID);

void canSendTimeout(struct ev_loop * loop,struct ev_timer * watcher,int eventID);

void canReceive(struct ev_loop * loop,struct ev_io * watcher,int eventID);

void canReceiveTimeout(struct ev_loop * loop,struct ev_timer * watcher,int eventID);

void fireTimer(struct ev_loop * loop,struct ev_timer * watcher,int eventID);

void doRun(struct ev_loop * loop,struct ev_async * watcher,int event);

/**
 @brief Runs a callback on the network thread.
 @param loopID The loop ID
 @returns true if successful, false otherwise.
 */
 boolean runOnNetworkThread(uint64_t loopID,void (*callback)(void *),void * arg);
#endif
