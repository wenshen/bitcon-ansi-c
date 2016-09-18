/*
 *  LibevSockets.c
 *
 *  Created on: 12/11/2012
 *  Created by: Attila Peter Toth
 *  Copyright (c) 2012 Bitcoin Project Team
 */

#include "LibevSockets.h"
#include <pthread.h>


/**
 * @params socketID; actually is a return parameter, it will be a non-blocking sigpipe-free socket
 * @params IPv6 - boolean value specifying the IP version
 * @returns @see Constants.h
 */
SocketReturn createNewSocket(uint64_t * socketID, short int IPv6) {

	*socketID = socket(IPv6 ? PF_INET6 : PF_INET, SOCK_STREAM, 0);

	if ((int)*socketID == -1) {

		if (errno == EAFNOSUPPORT || errno == EPROTONOSUPPORT) {

			return SOCKET_NO_SUPPORT;
		}
		return SOCKET_BAD;
	}

	/* Stop SIGPIPE annoying us.*/
	if (NOSIGPIPE) {

		int i = 1;
		/*&i is the option value, which is non-zero, aka. TRUE*/
		setsockopt((int)*socketID, SOL_SOCKET, SO_NOSIGPIPE, &i, sizeof(i));
	}

	/* Make socket non-blocking*/
	fcntl((int)*socketID, F_SETFL, fcntl((int)*socketID, F_GETFL, 0) | O_NONBLOCK);
	return SOCKET_OK;
}

/**
 * @brief Binds a newly created socket to the local address to the specified port. Used when creating
 *  a server side socket. It return TRUE on success, FALSE otherwise.
 */
short int socketBind(uint64_t * socketID, short int IPv6, uint16_t port) {

	struct sockaddr_in loc_addr;
	struct sockaddr_in6 loc_addr6;
	/*Define and set up IPv4 code*/

	if (!IPv6) {

		memset(&loc_addr,0,sizeof(loc_addr));
		loc_addr.sin_family = AF_INET;
		loc_addr.sin_port = htons(port);
		loc_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	} else/*IPv6 code comes here*/ {

		memset(&loc_addr6,0,sizeof(loc_addr6));
		loc_addr6.sin6_family = AF_INET6;
		loc_addr6.sin6_port = htons(port);
		loc_addr6.sin6_addr = in6addr_any;
	}

	if (createNewSocket(socketID,IPv6)!=SOCKET_OK) {

		return FALSE;
	}

	if(!IPv6) {

		/*Use the IPv4 code*/
		if (bind((int)*socketID, (struct sockaddr*)&loc_addr, sizeof(loc_addr))<0) {

			close(*socketID);
			return FALSE;
		}

	} else/*Use IPv6 code*/ {

		if (bind((int)*socketID, (struct sockaddr*)&loc_addr6, sizeof(loc_addr6))<0) {

			close(*socketID);
			return FALSE;
		}
	}
	return TRUE;
}

boolean socketConnect(uint64_t socketID, uint8_t * IP, short int  IPv6, uint16_t port) {

	/* Create sockaddr_in6 information for a IPv6 address*/
	int res;

	if (IPv6) {

		struct sockaddr_in6 address;
		memset(&address, 0, sizeof(address)); /* Clear structure.*/
		address.sin6_family = AF_INET6;
		memcpy(&address.sin6_addr, IP, 16); /* Move IP address into place.*/
		address.sin6_port = htons(port); /* Port number to network order*/
		res = connect((int)socketID, (struct sockaddr *)&address, sizeof(address));

	} else {

		struct sockaddr_in address;
		memset(&address, 0, sizeof(address)); /* Clear structure.*/
		address.sin_family = AF_INET;
		memcpy(&address.sin_addr, IP + 12, 4); /* Move IP address into place. Last 4 bytes for IPv4.*/
		address.sin_port = htons(port); /* Port number to network order*/
		res = connect((int)socketID, (struct sockaddr *)&address, sizeof(address));
	}

	if (res < 0 && errno == EINPROGRESS) {

		return TRUE;
	}

	return FALSE;
}

short int socketListen(uint64_t socketID, uint16_t maxConnections) {

	if(listen((int)socketID, maxConnections) == -1) {

		return FALSE;
	}

	return TRUE;
}

/*Accept connection on socketID. The accepted connection socketID will be socketID*/
boolean socketAccept(uint64_t socketID,uint64_t * connectionSocketID) {

	int *ret = (int*)malloc(sizeof(int));
	*ret = accept((int)socketID, NULL, NULL);

	if (*ret == -1) {

		return FALSE;
	}

	*connectionSocketID = *ret;

	/* Stop SIGPIPE*/
	if (NOSIGPIPE) {

		int i = 1;
		setsockopt((int)*connectionSocketID, SOL_SOCKET, SO_NOSIGPIPE, &i, sizeof(i));
	}

	/* Make socket non-blocking*/
	fcntl((int)*connectionSocketID, F_SETFL, fcntl((int)*connectionSocketID, F_GETFL, 0) | O_NONBLOCK);
	return TRUE;
}

uint64_t getEventWithSocket(EventList *events, int sock) {

	EventList *p = events;

	while(p) {

		if( ((_IOEvent*)p->ioEventID)->socketID == sock) {

			return p->ioEventID;
		}

		p = p->next;
	}
	return 0;
}

void initEventList(EventList *e, uint64_t eventID) {

	e->ioEventID = eventID;
	e->next = NULL;
}
/**
 * @brief gets
 */
boolean socketAddEvent(uint64_t newEventID, uint16_t timeout) {

	_IOEvent *ev = (_IOEvent*)newEventID;
	_EventLoop *loop = (_EventLoop*)ev->loopID;
	EventList *head;
	EventList *q = (EventList*)malloc(sizeof(EventList));

	ev->maxTimeOut = timeout;

	ev->added = time(0);
	initEventList(q,newEventID);

	if (ev->mode == INPUT) {

		pthread_mutex_lock(&loop->iEv_mutex);
		head = loop->iEvents;
	}
	else {

		pthread_mutex_lock(&loop->oEv_mutex);
		head = loop->oEvents;
	}

	if (head == NULL) {

		if (ev->mode == INPUT) {

			loop->iEvents = q;

		} else {

			loop->oEvents = q;
		}
	} else {

		EventList *p = head;

		while(p->next != NULL) {

			p = p->next;
		}

		p->next = q;
	}

	if (ev->mode == INPUT) {

		pthread_mutex_unlock(&loop->iEv_mutex);

	} else {

		pthread_mutex_unlock(&loop->oEv_mutex);
	}

	return TRUE;
}
/**
 * @brief loops through an EventList searching for the first event having the specified socket.
 * @params EventList* events : list of events that will be searched
 * @params int sock : socket file descriptor.
 * @returns the eventID of the _IOEvent of having the specified socket
 * */
boolean socketRemoveEvent(uint64_t beRemoved) {

	_IOEvent *ev = (_IOEvent*)beRemoved;
	_EventLoop *loop =(_EventLoop*) ev->loopID;
	EventList *head,*q;

	if(ev->mode==INPUT) {

		pthread_mutex_lock(&loop->iEv_mutex);
		head = loop->iEvents;

	} else {

		pthread_mutex_lock(&loop->oEv_mutex);
		head = loop->oEvents;
	}

	/*We have an empty list*/
	if (head == NULL) {

		return FALSE;
	}
	/*Remove the first element*/
	if (head->ioEventID == beRemoved) {

		if(ev->mode == INPUT) {

			loop->iEvents = loop->iEvents->next;

		} else {

			loop->oEvents = loop->oEvents->next;
		}
	}

	while (head->next!=NULL) {

		if (head->next->ioEventID == beRemoved) {

			if (head->next->next) {

				q = head->next;
				head->next = head->next->next;

			} else {

				q = head ->next;
				head->next = NULL;
			}

			free(q);
			return TRUE;
		}

		head = head->next;
	}

	if (ev->mode == INPUT) {

		pthread_mutex_unlock(&loop->iEv_mutex);

	} else {

		pthread_mutex_unlock(&loop->oEv_mutex);
	}

	return FALSE;
}
void handle_connection(_EventLoop *loop, fd_set *u_fd_set, fd_set *active_fd_set) {

	_IOEvent *ev;
	EventList *p = loop->iEvents;
	uint64_t tmp;
	int i;

	for(i = 0; i < FD_SETSIZE; i++) {

		if (FD_ISSET(i, u_fd_set)) {

			/*Remove fd, since we're handling it now*/
			FD_CLR(i, active_fd_set);

			tmp = getEventWithSocket(p, i);

			/*If can't find the event which requested the reading, skip reading from the socket and remove the socket form the reading socket set*/
			if(tmp == 0) {

				continue;
			}

			ev = (_IOEvent*)tmp;

			/*The callback function will read from the peer's socket*/

			if (ev->peer == NULL ) {
				/*acceptConnectionCallback occurred. You don't know the peer you're accepting the connection from, so you're reating hhe peer*/
				ev->onEvent.i(loop->communicator,ev->socketID);
			} else {

				/*onPeerXXXCallback occurred. This means you have to interact with the peer*/
				ev->onEvent.ptr(loop->communicator,ev->peer);
			}

		}
	}
}

void * startEventLoop(void* vloop){

	uint64_t loopId = (uint64_t)vloop;
	fd_set read_set,write_set,active_read_set,active_write_set;
	_EventLoop *loop =(_EventLoop*) loopId;
	/*set up the file descriptor sets*/
	_IOEvent *ev;
	EventList *p;
	struct timeval timeout;
	int ret;
	/*set up the timeout*/
	timeout.tv_sec = 2;
	timeout.tv_usec = 0;


	/*initialize sets with empty set*/
	FD_ZERO(&active_read_set);
	FD_ZERO(&active_write_set);

	/*Set input events*/
	p = loop->iEvents;
	while(p) {
		ev = (_IOEvent*)p->ioEventID;
		FD_SET(ev->socketID,&active_read_set);
		p = p->next;
	}

	/*Set output events*/
	p = loop->oEvents;
	while(p) {
		ev = (_IOEvent*)p->ioEventID;
		FD_SET(ev->socketID,&active_write_set);
		p = p->next;
	}

	/*Wait for connections until the term_cond is set*/
	while(1) {

		read_set = active_read_set;
		write_set = active_write_set;
		/*printf("EventLoop Looping\n");*/
		ret = select (FD_SETSIZE, &read_set, &write_set, NULL, &timeout);

		if (ret < 0) {
			perror("Server Error in select");
			return NULL;
			/*It does not end the execution*/
		}

		/*In case the select returned 0 this means the timeout has expired. Reset the timeout*/
		if (ret == 0) {

			timeout.tv_sec = 2;
			timeout.tv_usec = 0;

		} else {

			handle_connection(loop,&read_set,&active_read_set);
			handle_connection(loop,&write_set,&active_write_set);
		}

		pthread_mutex_lock(&loop->cond_mutex);
		printf("EventLoop Checking condition\n");
		if (loop->term_cond == TRUE) {

			/*printf("EventLoop Checking condition\n");*/
			break;
		}
		pthread_mutex_unlock(&loop->cond_mutex);
	}
	printf("QUitting\n");
	return NULL;
}
/**
 * @brief creates a new _EventLoop.
 * @params *loopID an output parameter. It is a unique ID for the EventLoop - it's physical address.
 * @note don't forget to set names properly -->> remove the _ from all the new functions and structures
 */
boolean newEventLoop(uint64_t * loopID, void (*onError)(void *), void (*onDidTimeout)(void *, void *, TimeOutType), void * communicator) {
	/*add this mutex to the communicator in case simultaneous changes are going to happen
	 * the content of the AddressManager is safe and we don't sync wrongly*/
	_EventLoop *loop = malloc(sizeof(*loop));
	/*dynamic initializer*/
	pthread_mutex_init(&(loop->cond_mutex),NULL);
	/*set the termination condition to FALSE*/
	loop->term_cond = FALSE;
	pthread_mutex_init(&(loop->iEv_mutex),NULL);
	pthread_mutex_init(&(loop->oEv_mutex),NULL);
	loop->communicator = communicator;
	loop->onError = onError;
	loop->onTimeOut = onDidTimeout;
	loop->iEvents = NULL;
	loop->oEvents = NULL;

	uint64_t aux = (uint64_t)loop;

	/*create a thread with default parameters*/
	if(0 != pthread_create(&(loop->network_thread), NULL, (void*)startEventLoop, (void*)(uint64_t)loop)) {


		free(loop);
		onError(communicator);
		return FALSE;
	}

	*loopID = (uint64_t)loop;
	return TRUE;
}

/*
 * @brief creates a new event from the data provided
 * @returns TRUE if succeeded, FALSE if no memory was available. eventID is also an output
 * */
boolean socketCanAcceptEvent(uint64_t * eventID, uint64_t loopID, uint64_t socketID, void (*onCanAccept)(void *, uint64_t)) {

	_IOEvent *ev = malloc(sizeof(_IOEvent));

	if (ev == NULL) {

		return FALSE;
	}

	ev->loopID = loopID;
	ev->socketID = socketID;
	ev->onEvent.i = onCanAccept;
	ev->mode = INPUT;
	ev->peer = NULL;
	*eventID = (uint64_t)ev;
	return TRUE;
}


boolean socketDidConnectEvent(uint64_t * eventID, uint64_t loopID, uint64_t socketID, void (*onDidConnect)(void *, void *), void * peer) {

	_IOEvent *ev = (_IOEvent*)malloc(sizeof(_IOEvent));

	if (ev == NULL) {

		return FALSE;
	}

	ev->mode = INPUT;
	ev->loopID = loopID;
	ev->onEvent.ptr = onDidConnect;
	ev->socketID = socketID;
	ev->peer = peer;
	/*ev->timeoutCallback = didConnectTimeout;yet I don't have a clear picture when it should be called*/
	ev->maxTimeOut = TIMEOUT_CONNECT;
	*eventID = (uint64_t)ev;
	return TRUE;
}


void didConnectTimeout(struct ev_loop * loop, struct ev_timer * watcher, int eventID) {

	/*Timer * event = (Timer *) watcher;
	event->loop->onTimeOut(event->loop->communicator,event->peer,CB_TIMEOUT_CONNECT);*/
}

boolean socketCanSendEvent(uint64_t * eventID, uint64_t loopID, uint64_t socketID, void (*onCanSend)(void *, void *), void * peer) {

	_IOEvent *ev = (_IOEvent*)malloc(sizeof(_IOEvent));

	if(ev == NULL) {

		return FALSE;
	}

	ev->loopID = loopID;
	ev->socketID = socketID;
	ev->onEvent.ptr = onCanSend;
	ev->peer = peer;
	ev->mode = OUTPUT;
	*eventID = (uint64_t)ev;

	return TRUE;
}



void canSendTimeout(struct ev_loop * loop, struct ev_timer * watcher, int eventID) {
	/*Timer * event = (Timer *) watcher;
	event->loop->onTimeOut(event->loop->communicator,event->peer,CB_TIMEOUT_SEND);*/
}

boolean socketCanReceiveEvent(uint64_t * eventID, uint64_t loopID, uint64_t socketID, void (*onCanReceive)(void *, void *), void * peer) {

	_IOEvent *ev = (_IOEvent*)malloc(sizeof(_IOEvent));

	if (ev == NULL) {

		return FALSE;
	}

	ev->loopID = loopID;
	ev->socketID = socketID;
	ev->mode = INPUT;
	ev->onEvent.ptr = onCanReceive;
	ev->peer = peer;
	return TRUE;
}


void canReceiveTimeout(struct ev_loop * loop, struct ev_timer * watcher, int eventID) {

	/*Timer * event = (Timer *) watcher;
	event->loop->onTimeOut(event->loop->communicator,event->peer,CB_TIMEOUT_RECEIVE);*/
}


void socketFreeEvent(uint64_t loop, uint64_t eventID){

	socketRemoveEvent(eventID);
	free((_IOEvent *)eventID);
}
/*
void socketFreeEvent(uint64_t eventID){
	socketRemoveEvent(eventID);
	free((IOEvent *)eventID);
}*/
int32_t socketSend(uint64_t socketID, uint8_t * data, uint32_t len) {

	/*res is the size of blocks sent using the send POSIX function*/
	ssize_t res = send((int)socketID, data, len, SEND_FLAGS);
	if (res >= 0) {

		return (int32_t)res;
	}

	if (errno == EAGAIN) {

		return 0; /* False event. Wait again.*/
	}
	return SOCKET_FAILURE; /* Failure*/

}
int32_t socketReceive(uint64_t socketID,uint8_t * data,uint32_t len) {

	/*res represents the size of bytes read from the network*/
	ssize_t res = read((int)socketID, data, len);

	if (res > 0) {

		return (int32_t)res; /* OK, read data.*/
	}

	if ( res == 0) {

		return SOCKET_CONNECTION_CLOSE; /* If read() gives zero it means the connection was closed.*/
	}

	if (errno == EAGAIN) {

		return 0; /* False event. Wait again. No bytes read.*/
	}

	return SOCKET_FAILURE; /* Failure*/

}
short int startTimer(uint64_t loopID, uint64_t * timer, uint16_t time, void (*callback)(void *), void * arg) {

	/*Timer * theTimer = malloc(sizeof(*theTimer));
	if (NOT theTimer)
		return false;
	theTimer->callback = callback;
	theTimer->arg = arg;
	theTimer->loop = (EventLoop *)loopID;
	ev_timer_init((struct ev_timer *)theTimer, fireTimer, (float)time / 1000, (float)time / 1000);
	ev_timer_start(((EventLoop *)loopID)->base, (struct ev_timer *)theTimer);
	*timer = (uint64_t)theTimer;
	*/return TRUE;
}

void fireTimer(struct ev_loop * loop, struct ev_timer * watcher, int eventID) {

	/*Timer * theTimer = (Timer *)watcher;
	theTimer->callback(theTimer->arg);*/
}
void endTimer(uint64_t timer) {

	/*Timer * theTimer = (Timer *)timer;
	ev_timer_stop(theTimer->loop->base, (struct ev_timer *)theTimer);
	free(theTimer);*/
}

void doRun(struct ev_loop * loop, struct ev_async * watcher, int event) {

	/*EventLoop * evloop = (EventLoop *)((AsyncEvent *)watcher)->loop;
	evloop->userCallback(evloop->userArg);*/
}

void closeSocket(uint64_t socketID) {

	close((int)socketID);
}

void exitEventLoop(uint64_t loopID) {

	if (loopID == 0) {

		return;
	}
	/*printf("Exiting");*/
	_EventLoop *loop = (_EventLoop*)loopID;
	pthread_mutex_lock(&loop->cond_mutex);
	loop->term_cond = TRUE;
	pthread_mutex_unlock(&loop->cond_mutex);
}

