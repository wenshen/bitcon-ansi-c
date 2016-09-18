/*
*  NodeDiscovery.h
*
*  Created on: 20/10/2012.
*  Created by: Attila Peter Toth.
*  Modified by: 
*  Copyright (c) 2012 Bitcoin Project Team
*/

/*
*  SEE HEADER FILE FOR DOCUMENTATION
*/

#include "NodeDiscovery.h"
#include "assert.h"





void onNodeDiscoveryError(Error e,char* t){
	perror("ByteArrayFailed to create from string\n");
	printf("Error code %i\n",e);
}
/*
* Constructor
*/
NodeDiscovery * newNodeDiscovery(){
	NodeDiscovery * self = malloc(sizeof(*self));
	if (NULL == self)
		return NULL;
	getObject(self)->destroy = destroyNodeDiscovery;
	if(initNodeDiscovery(self))
		return self;
	free(self);
	return NULL;
}

/*
* Object Getter
*/

NodeDiscovery * getNodeDiscovery(void * self){
	return self;
}

/*
*  Initialiser
*/

int initNodeDiscovery(NodeDiscovery * self){
	if(getObject(self) == NULL){
		return FALSE;
	}
	getObject(self)->referenceCount = 1;
	return TRUE;
}

/*
*  Destructor
*/

void destroyNodeDiscovery(void * self){
	destroyObject(self);
}

/*
* Functions
*/

char* extractIp(char* str1)
{
	char* returnIP;
	struct sockaddr_in sa;
	char delims[] = "\"< >\"";
	char *result = NULL;
	char* str= malloc(strlen(str1) + 1);
	strcpy(str, str1);


	result = strtok( str, delims );

	while( result != NULL ) {

		    if(inet_pton(AF_INET6, result, &(sa.sin_addr))!=0 || inet_pton(AF_INET, result, &(sa.sin_addr))!=0)
		    	{
		    		returnIP = malloc(strlen(result)+1);
		    		strcpy(returnIP,result);
		    		free(str);
		    		return returnIP;
		    	}
	    result = strtok( NULL, delims );
	}
	free(str);
	return NULL;
}

/**
 *
 *
 *find flags here http://cboard.cprogramming.com/cplusplus-programming/128326-undefined-symbol-when-compiling-socket-program.html
*/
char* getExtIP(char *dest)
{
	int sockfd,conn,recieved;
	int len,bytes_sent,max_rec_len;
	struct sockaddr_in serv_addr;
	struct hostent *serv_detail;
	struct in_addr **addr_list;
	char *buff,*ret;
	char *msg="GET / HTTP/1.1\r\n"
               "Host: checkip.dyndns.org\r\n"
               "User-Agent: Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1)\r\n"
               "Connection: close\r\n"
               "\r\n";


	assert(dest !=NULL);

	max_rec_len = 1500;
	ret = NULL;

	memset((char*)&serv_addr,0,sizeof (serv_addr));

	serv_detail = gethostbyname(dest);

	if (serv_detail == NULL)
	{
		strerror(errno);
		assert(serv_detail != NULL);
		return NULL;
	}

	serv_addr.sin_family = serv_detail->h_addrtype;
	serv_addr.sin_port = htons(80);

	/*extracts the IP address from the */
	addr_list = (struct in_addr **)serv_detail->h_addr_list;

	memcpy(&serv_addr.sin_addr,addr_list[0] , serv_detail->h_length);

	/*create a socket file descriptor. It is not created from serv_addr.sin_family
	 * because it caused an error --- strange*/
	sockfd = socket(AF_INET,SOCK_STREAM,0);

	if (sockfd==-1)
	{
		strerror(errno);
		assert(sockfd != -1);
		return NULL;
	}

	if ((conn = connect (sockfd,(struct sock_addr*)&serv_addr, sizeof(serv_addr))) < 0)
	{
		strerror(errno);
		assert(conn>=0);
		return NULL;
	}

	len = strlen(msg);
	bytes_sent = send(sockfd,msg,len,0);
	buff = (char*)malloc(sizeof(char)*max_rec_len);

	if (bytes_sent==-1)
	{
		strerror(errno);
		assert(bytes_sent!=-1);
		return NULL;
	}

	if((recieved = recv(sockfd,buff,max_rec_len,0))==0)
	{
		perror("Connection lost for IP gathering.\n");
		return NULL;
	}else
	{
		if(recieved==-1)
		{
			strerror(errno);
			return NULL;
		}else
		{

			ret = extractIp(buff);
			/*printf("Received: %s\n",extractIp(buff));*/
		}
	}

	free(buff);

	close(sockfd);
	return ret;
}

ByteArray* getIPByteArray(){
	char *pistike = getExtIP("checkip.dyndns.org");
	printf("%s\n",pistike);
	return getIPInByteArray(pistike);
}


ByteArray *getIPInByteArray(char *ip){

	struct sockaddr_in address;
	uint8_t ar [16];
	memset(ar,0,16);

	if (inet_pton(AF_INET,ip,&(address.sin_addr))==1){
		uint32_t acton = address.sin_addr.s_addr;

		ar[10] = 0xFF;
		ar[11] = 0xFF;
		ar[12] = acton&0xFF;
		ar[13] = (acton&0xFF00)/0xFF;
		ar[14] = (acton&0xFF0000)/0xFFFF;
		ar[15] = (acton&0xFF000000)/0xFFFFFF;


	}
	ByteArray *ret = createNewByteArrayUsingDataCopy(ar, 16, onNodeDiscoveryError);


	return ret;
}
