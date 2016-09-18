/*
*  Protocol.h
*
*  Created on: 23/10/2012.
*  Created by: Prajowal Manandhar
*  Modified by:
*  Copyright (c) 2012 Bitcoin Project Team
*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netdb.h>
/*#include <x86_64-linux-gnu/sys/types.h> //Please Change this to "sys/types.h" if it  doesn't compile*/
/*#include <x86_64-linux-gnu/sys/socket.h> //Please Change this to "sys/socket.h" if it  doesn't compile*/
#include <arpa/inet.h>
#include "IRC.h"

static int ircConnect(char *host, int port)
{
return 0;
}

/*
static int ircConnect1(char *s[], int port)
{

	struct sockaddr_in server;
	struct hostent *hp;
	int	sd;

	if ((hp = gethostbyname(s[0])) == 0) {
		perror("");
		exit(1);
	}

	memset(&server, 0, sizeof(server));
	server.sin_family = 'AF_INET';
	inet_pton('AF_INET',s[1],&server.sin_addr.s_addr);
	server.sin_port = htons(atoi(s[2]));
	    if((sd = socket('AF_INET','SOCK_STREAM',0))!= 0)
	        printf("%s : socket successfully initialized.\n",s[0]);
	    if(connect(sd,(struct sockaddr *)&server,sizeof(server))==0)
	        printf("%s : successfully connected.\n",s[0]);
	    if ((sd = socket('AF_INET', 'SOCK_STREAM', 0)) == -1) {
	    		perror("socket");
	    		return -1;
	    	}
	    	if (connect(sd,(struct sockaddr *)  &server, sizeof(server)) == -1) {
	    		perror("connect");
	    		return -1;
	    	}
	    	return sd;

}
*/

static int ircSend(int sock, char* data)
{
	return send(sock, data, strlen(data), 0);
}

static int ircRecvline(int sock, char*buf)
{
	char chRecv = 0;
	int strpos;
	char vchs[MAX_LINE_LENGTH] = {0};
	int retCode = 0;
	strpos = 0;

	while(chRecv != '\n' && retCode >=0) {
		retCode = recv(sock, &chRecv, 1, 0);
		vchs[strpos++] = chRecv;
	}
	if (retCode < 0) {
		return -1;
	}
	strcpy(buf, vchs);
	return strlen(vchs);
}

int ircDisconnect(int sock)
{
	return close(sock);
}


static char* getNick(const char *str) {
	char *tmp = malloc(strlen(str)+1);
	char *tok ;
	strcpy(tmp, str);
	tok = strtok(tmp, " ");
	int i = 0;
	while (tok != NULL && (i++)<7) {
		tok = strtok(NULL, " ");
	}
	free(tmp);
	return tok;
}

static char* getCmd(const char *str) {
	char *tmp = malloc(strlen(str)+1);
	strcpy(tmp, str);
	char *tok = strtok(tmp, " ");
	tok = strtok(NULL, " ");
	free(tmp);
	return tok;
}


char ** bootstrap(char *chan) {
	int sockd;
	char *nodelist[100];
	int nodec = 1;
	sockd = ircConnect("irc.lfnet.org", 6667);
	if (sockd <= 0)
		return NULL;
	ircSend(sockd, "NICK netcrash_test\r\n");
	ircSend(sockd, "USER netcrash 8 * : netcrash\r\n");
	int retVal =0;
	while(retVal >=0) {
		char *buf = malloc(MAX_LINE_LENGTH);
		retVal = ircRecvline(sockd, buf);
		char *cmd = getCmd(buf);

		if (strcmp(cmd, "MODE") == 0) {
			char joincmd[128] = {0};
			char whocmd[128] = {0};
			sprintf(joincmd, "JOIN %s\r\n", chan);
			sprintf(whocmd, "WHO %s\r\n", chan);
			ircSend(sockd, joincmd);
			ircSend(sockd, whocmd);
		}
		else if (strcmp(cmd, "352") == 0) {
			char *nick = getNick(buf);
			printf("*** %s\n", nick);
			nodelist[nodec] = nick;
		}
	}
	return nodelist;

}
