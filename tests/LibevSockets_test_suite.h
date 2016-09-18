#include "CUnit/CUnit.h"
#include "CUnit/Basic.h"
#include <fcntl.h>
#include <arpa/inet.h>
#include "../src/Object/LibevSockets/LibevSockets.h"
#include "../src/Object/Network/AddressBroadcast.h"
#include "../src/Object/Network/NetworkCommunicator.h"
#include "../src/Object/Network/AddressManager.h"

#define BITCOIN_PORT 18333

void* err(Error a,char * format,...){
	printf("Error number %i\n",a);
	return 1;
}

void erNetwork(Error a,char * f,...){
	printf("Error number %i\n",a);
}



void *badTime(void*p){
	printf("Timing went wrong");
	return 1;
}

/* Test Suite setup and cleanup functions: */
#define PR_NR 1200

void* start_server(void ){
	int sockfd, newsockfd, clilen;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	        perror("ERROR opening socket");
	memset(&serv_addr,0,sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(PR_NR);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	              perror("ERROR on binding");
	listen(sockfd,5);
	/*clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);*/
	newsockfd = accept(sockfd,NULL,NULL);
	if (newsockfd < 0)
	          perror("ERROR on accept");

	memset(&buffer,0,sizeof(buffer));

	n = read(newsockfd,buffer,255);
	if (n < 0) perror("ERROR reading from socket");
	/*printf("Here is the message: %s\n",buffer);*/
	n = write(newsockfd,"I got your message",18);
	if (n < 0) perror("ERROR writing to socket");
	return 0;
}

int init_socket_suite(void) {

	return 0;
}

int clean_socket_suite(void) {
	return 0;
}



/************* Test case functions ****************/

void test_case_openNewSocket(void) {

	int ret;
	uint64_t id;
	ret = createNewSocket(&id,FALSE);
	CU_ASSERT_EQUAL(ret,SOCKET_OK);
	ret = fcntl(id, F_GETFL);
	CU_ASSERT(ret!=-1);
	CU_ASSERT_EQUAL(close(id),0);
}

void test_case_closeSocket(void){
	uint64_t sfd;
	int ret;
	ret = createNewSocket(&sfd,FALSE);
	CU_ASSERT_EQUAL(ret,SOCKET_OK);
	ret = fcntl(sfd, F_GETFL);
	CU_ASSERT(ret!=-1);
	closeSocket(sfd);
	CU_ASSERT_EQUAL(close(sfd),-1);
}

void test_case_connectArtificialServer(void){


	ByteArray *ip = createNewByteArrayUsingDataCopy((uint8_t []){0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x7F,0x00,0x00,0x01}, 16, onErrorReceived);
	pthread_t test_thread;
	char comp[19];
	uint64_t sockfd;
	struct sockaddr_in address;
	int ret;
	char buff[256];
	fd_set rd,wr;
	struct timeval timeout;




	if(0 != pthread_create(&test_thread, NULL, (void*)start_server, NULL)) {
		CU_ASSERT(false);
	}
	ret = createNewSocket(&sockfd,FALSE);
	CU_ASSERT_EQUAL(ret,SOCKET_OK);

	memset(&address, 0, sizeof(address)); /* Clear structure.*/
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr("127.0.0.1");
	address.sin_port = htons(PR_NR); /* Port number to network order*/

	ret =  socketConnect(sockfd,ip, FALSE, PR_NR);

	FD_ZERO(&rd);

	timeout.tv_sec = 2;
	timeout.tv_usec = 0;
	FD_SET(sockfd,&rd);
	FD_SET(sockfd,&wr);
	select(FD_SETSIZE,&rd,&wr,NULL,&timeout);
	ret = write(sockfd,"Connected",9);
	if (ret < 0){
		printf("Error at send to artificial server");
		CU_ASSERT(false);
	}


	select(FD_SETSIZE,&rd,NULL,NULL,&timeout);
	ret = read(sockfd,buff,256);
	if (ret < 0){
		printf("Error at read from artificial server");
		CU_ASSERT(false);
	}

	snprintf(comp,19,"%s",buff);
	CU_ASSERT_EQUAL(0,strcmp(comp,"I got your message"));
/*	printf("%s %i",comp,strlen(buff));*/
	closeSocket(sockfd);
}

void test_case_NewNetworkCommunicator(void){
	NetworkCommunicator *nc = NULL,*nc2;
	AddressManager *adm = NULL,*adm2;
	nc = createNewNetworkCommunicator(err);
	nc2 = createNewNetworkCommunicator(err);
	if (NULL ==nc){
		perror("Could not create NetworkCOmmunicator\n");
		CU_ASSERT(false);
	}
	if (FALSE == initializeNetworkCommunicator(nc,err)){
		perror("Failed to initialize NetworkCommunicator\n");
		CU_ASSERT(false);
	}
	adm2 = newAddressManager(err,badTime);
	adm = newAddressManager(err,badTime);
	if (NULL == adm){
		perror("Failed to create address manager");
		CU_ASSERT(false);
	}
	initAddressManager(adm2,badTime,badTime);
	if (FALSE == initAddressManager(adm,badTime,badTime)){
		perror("Failed to initialize AddressManager\n");
		CU_ASSERT(false);
	}

	setAddressManager(nc,adm);
	setAddressManager(nc2,adm2);
	ByteArray *ip = getIPByteArray();
	/*time set to 10 since I have no idea what it is used for and since
	 * the current time returned by clock() is 8 bytes long, and this field
	 * accepts only 4 bytes I pass 10*/


	NetworkAddress *myIPv4 = createNewNetworkAddress(10,ip,BITCOIN_PORT,1,erNetwork);


	setOwnIPv4Address(nc,myIPv4);
	setOwnIPv4Address(nc2,myIPv4);
	/*it's a dummy thing, dont know what should it do*/
	ByteArray *userAgent = createNewByteArrayUsingDataCopy((uint8_t []){0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x7F,0x00,0x00,0x01}, 16, onErrorReceived);
	setUserAgent(nc,userAgent);
	setUserAgent(nc2,userAgent);
	nc->version = PONG_VERSION;
	nc2->version = PONG_VERSION;
	nc->maxConnections = 3;
	nc2->maxConnections = 3;
	nc->maxIncommingConnections = 3 ;
	nc2->maxIncommingConnections = 3;
	nc->timeOut = 2000;
	nc2->timeOut = 2000;
	nc->heartBeat = 1000;
	nc2->heartBeat = 1000;
	nc->connectionTimeOut = 1000;
	nc2->connectionTimeOut = 1000;
	startNetworkCommunicator(nc);
	startNetworkCommunicator(nc2);
	startListeningToIncomingConnections(nc);

	NetworkAddress *peerAddress = myIPv4;

	Peer *peer = createNewPeerFromNetworkAddress(peerAddress);
	/*sleep(10);*/
	int t = connectToPeerUsingNetworkCommunicator(nc2,peer);

	if (t == CONNECT_OK){
		printf("so far so good\n");
	}
	printf("t==>>%i\n",t);
	if (CONNECT_NO_SUPPORT == t)
	{
		printf("connection not supported\n");
	}
	/*sleep(10);*/
	stopNetworkCommunicator(nc);
	stopNetworkCommunicator(nc2);
	/*sleep(5);
	destroyNetworkAddress(myIPv4);
	destroyByteArray(ip);
	destroyByteArray(userAgent);
	destroyAddressManager(adm);
	destroyAddressManager(adm2);
	destroyNetworkCommunicator(nc);
	destroyNetworkCommunicator(nc2);*/

}


/* Grouping all test cases into an array that will be registered later with a single test suite. */
CU_TestInfo libevsocket_test_array[] = {
  { "test_case_openNewSocket", test_case_openNewSocket },
  { "test_case_closeSocket",test_case_closeSocket},
  { "test_case_connectArtificialServer",test_case_connectArtificialServer},
  /*{ "test_case_NewNetworkCommunicator" , test_case_NewNetworkCommunicator},*/
  CU_TEST_INFO_NULL,
};
