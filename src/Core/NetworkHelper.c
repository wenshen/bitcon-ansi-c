/*
 * NetworkHelper.c
 *
 * Created on: Dec 9, 2012
 * Created by: Wen Shen
 * Copyright (c) 2012 MIBitcoin Project Team
 */
#include "NetworkHelper.h"

int appendRecordToWallet(char *jsonString) {
	int ch;
		FILE *f = fopen("wallet", "r+");
		int i = 0;
		while ((ch = fgetc(f)) != ']') {
			i++;
		}
		fseek(f, i, SEEK_SET);
		char footer[] = "]}";
		fprintf(f, ",");
		fprintf(f, jsonString);
		fprintf(f, footer);
		fclose(f);
}

int daemon(int argc, char **argvs) {
	if (getppid() == 1) { /*already a daemon*/
		return 0;
	}
	pid_t pid, sid;
	char *argv = getGlobalIPAddress();

	/* Clone ourselves to make a child */
	pid = fork();

	/* If the pid is less than zero,
	 *    something went wrong when forking */
	if (pid < 0) {
		exit(EXIT_FAILURE);
	}

	/* If the pid we got back was greater
	 *    than zero, then the clone was
	 *       successful and we are the parent. */
	if (pid > 0) {
		exit(EXIT_SUCCESS);
	}

	/* If execution reaches this point we are the child */
	/* Set the umask to zero */
	umask(0);

	/* Open a connection to the syslog server */
	openlog(argvs[0], LOG_NOWAIT | LOG_PID, LOG_USER);

	/* Sends a message to the syslog daemon */

	/* Try to create our own process group */
	sid = setsid();
	if (sid < 0) {
		syslog(LOG_ERR, "Could not create process group\n");
		exit(EXIT_FAILURE);
	}

	/* Change the current working directory */
	if ((chdir(".")) < 0) {
		syslog(LOG_ERR, "Could not change working directory to /\n");
		exit(EXIT_FAILURE);
	}

	/* Close the standard file descriptors */
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	/* A useless payload to demonstrate */
	int sockfd, newfd;
	struct sockaddr_in s_addr, c_addr;
	char buf[BUFLEN];
	socklen_t len;
	unsigned int port, listnum;
	fd_set rfds;
	struct timeval tv;
	int retval, maxfd;

	/*Initiate the socket*/
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(errno);
	} else
		printf("socket create success!\n");
	/*Configure the sever port*/
	port = 4567;
	/*Configure the length of listening queue */

	listnum = 3;
	/*Configure the server IP*/
	bzero(&s_addr, sizeof(s_addr));
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(port);
	if (argv)
		s_addr.sin_addr.s_addr = inet_addr(argv);
	else
		s_addr.sin_addr.s_addr = INADDR_ANY;
	/*Bind the address and port to the socket*/
	if ((bind(sockfd, (struct sockaddr*) &s_addr, sizeof(struct sockaddr)))
			== -1) {
		perror("bind");
		exit(errno);
	} else
		printf("bind success!\n");
	/*Listen to the local port*/
	if (listen(sockfd, listnum) == -1) {
		perror("listen");
		exit(errno);
	} else
		printf("the server is listening!\n");

	while (1) {

		printf("*****************Start Communicating***************\n");
		len = sizeof(struct sockaddr);
		if ((newfd = accept(sockfd, (struct sockaddr*) &c_addr, &len)) == -1) {
			perror("accept");
			exit(errno);
		} else
			printf("The client you are communicating is：%s: %d\n",
					inet_ntoa(c_addr.sin_addr), ntohs(c_addr.sin_port));
		while (1) {
			/*Clear the readable file descriptor set*/FD_ZERO(&rfds);
			/*Add the file descriptor of the standard input to the set*/FD_SET(0, &rfds);
			maxfd = 0;
			/*Add current connecting file descriptor to the set*/FD_SET(newfd, &rfds);
			/*Find the max file descriptor in the set*/
			if (maxfd < newfd)
				maxfd = newfd;
			/*Configure timeout*/
			tv.tv_sec = 5;
			tv.tv_usec = 0;
			/*Waiting for connecting*/
			retval = select(maxfd + 1, &rfds, NULL, NULL, &tv);
			if (retval == -1) {
				printf("select error，server exit\n");
				break;
			} else if (retval == 0) {
				/*printf("waiting for user input form client，waiting...\n");*/
				continue;
			} else {
				/*Start to send message*/
				if (FD_ISSET(0, &rfds)) {
					_retry:
					/******send message*******/
					bzero(buf, BUFLEN);
					/*fgets function：read BUFLEN-1 buffers from the stream*/
					fgets(buf, BUFLEN, stdin);
					/*Print the message*/
					if (!strncasecmp(buf, "quit", 4)) {
						printf("server requests quit the communicating!\n");
						break;
					}
					/*invalid input*/
					if (!strncmp(buf, "\n", 1)) {
						printf(" It only contains enter, invalid input！！！\n");
						goto _retry;
					}
					/*buf contains '\n'*/
					if (strchr(buf, '\n'))
						len = send(newfd, buf, strlen(buf) - 1, 0);
					/*buf does not contain '\n'*/
					else
						len = send(newfd, buf, strlen(buf), 0);
					if (len > 0)
						printf(
								"\tMessage sent successfully ，the number of bytes for this message is： %d\n",
								len);
					else {
						printf("Message sent with failure!\n");
						break;
					}
				}
				/*Message from the client*/
				if (FD_ISSET(newfd, &rfds)) {
					/******Receive the message*******/
					bzero(buf, BUFLEN);
					len = recv(newfd, buf, BUFLEN, 0);
					syslog(LOG_INFO, "message received before%i\n", len);

					if (len > 0) {
						syslog(LOG_INFO, "message received, %s",buf);
						appendRecordToWallet(buf);
					} else {
						if (len < 0)
							printf("Message receiving failed！\n");
						else
							printf(
									"The client quit the communicating, connection exit！\n");
						break;
					}
				}
			}
		}
		/*close the communicating socket*/
		close(newfd);
	}
	/*Close the server socket*/
	close(sockfd);
	/* this is optional and only needs to be done when your daemon exits */
	closelog();

}

char * getGlobalIPAddress() {
	struct ifaddrs * ifAddrStruct = NULL;
	struct ifaddrs * ifa = NULL;
	void * tmpAddrPtr = NULL;
	char * globalIPAddress = "";

	getifaddrs(&ifAddrStruct);

	for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa ->ifa_addr->sa_family == AF_INET) { /* check it is IP4*/
			/* is a valid IP4 Address*/
			tmpAddrPtr = &((struct sockaddr_in *) ifa->ifa_addr)->sin_addr;
			char addressBuffer[INET_ADDRSTRLEN];
			char * addrBuf = malloc(INET_ADDRSTRLEN);
			inet_ntop(AF_INET, tmpAddrPtr, addrBuf, INET_ADDRSTRLEN);
			char * tempStr = "wlan0";
			if (strcmp(ifa->ifa_name, tempStr) == 0) {
				globalIPAddress = addrBuf;
				/* printf("Your IP Address is: %s\n", addrBuf);*/
			}
		}
	}
	if (ifAddrStruct != NULL)
		freeifaddrs(ifAddrStruct);
	return globalIPAddress;
}

int sendMessageToIPAddress(char *destination, char *msg) {
	int sockfd;
	struct sockaddr_in s_addr;
	socklen_t len;
	unsigned int port = 4567;

	/*Initiate the socket*/
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(errno);
	} else {
		printf("socket create success!\n");
	}

	/*Configure the length of listening queue */
	bzero(&s_addr, sizeof(s_addr));
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(port);
	if (inet_aton(destination, (struct in_addr *) &s_addr.sin_addr.s_addr) == 0) {
		perror(destination);
		exit(errno);
	}
	/*Start to connect to the server*/
	if (connect(sockfd, (struct sockaddr*) &s_addr, sizeof(struct sockaddr))
			== -1) {
		perror("connect");
		exit(errno);
	} else {
		printf("connection successful!\n");
	}

	/******send message*******/

	len = send(sockfd, msg, strlen(msg), 0);
	if (len > 0) {
		printf("Closing");
		close(sockfd);
		return 1;
	} else {
		close(sockfd);
		return -1;
	}
}

int initiateServer(int argc, char *argv) {
	int sockfd, newfd;
	struct sockaddr_in s_addr, c_addr;
	char buf[BUFLEN];
	socklen_t len;
	unsigned int port, listnum;
	fd_set rfds;
	struct timeval tv;
	int retval, maxfd;

	/*Initiate the socket*/
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(errno);
	} else
		printf("socket create success!\n");
	/*Configure the sever port*/
	port = 4567;
	/*Configure the length of listening queue */

	listnum = 3;
	/*Configure the server IP*/
	bzero(&s_addr, sizeof(s_addr));
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(port);
	if (argv)
		s_addr.sin_addr.s_addr = inet_addr(argv);
	else
		s_addr.sin_addr.s_addr = INADDR_ANY;
	/*Bind the address and port to the socket*/
	if ((bind(sockfd, (struct sockaddr*) &s_addr, sizeof(struct sockaddr)))
			== -1) {
		perror("bind");
		exit(errno);
	} else
		printf("bind success!\n");
	/*Listen to the local port*/
	if (listen(sockfd, listnum) == -1) {
		perror("listen");
		exit(errno);
	} else
		printf("the server is listening!\n");

	while (1) {
		printf("*****************Start Communicating***************\n");
		len = sizeof(struct sockaddr);
		if ((newfd = accept(sockfd, (struct sockaddr*) &c_addr, &len)) == -1) {
			perror("accept");
			exit(errno);
		} else
			printf("The client you are communicating is：%s: %d\n",
					inet_ntoa(c_addr.sin_addr), ntohs(c_addr.sin_port));
		while (1) {
			/*Clear the readable file descriptor set*/FD_ZERO(&rfds);
			/*Add the file descriptor of the standard input to the set*/FD_SET(0, &rfds);
			maxfd = 0;
			/*Add current connecting file descriptor to the set*/FD_SET(newfd, &rfds);
			/*Find the max file descriptor in the set*/
			if (maxfd < newfd)
				maxfd = newfd;
			/*Configure timeout*/
			tv.tv_sec = 5;
			tv.tv_usec = 0;
			/*Waiting for connecting*/
			retval = select(maxfd + 1, &rfds, NULL, NULL, &tv);
			if (retval == -1) {
				printf("select error，server exit\n");
				break;
			} else if (retval == 0) {
				/*printf("waiting for user input form client，waiting...\n");*/
				continue;
			} else {
				/*Start to send message*/
				if (FD_ISSET(0, &rfds)) {
					_retry:
					/******send message*******/
					bzero(buf, BUFLEN);
					/*fgets function：read BUFLEN-1 buffers from the stream*/
					fgets(buf, BUFLEN, stdin);
					/*Print the message*/
					if (!strncasecmp(buf, "quit", 4)) {
						printf("server requests quit the communicating!\n");
						break;
					}
					/*invalid input*/
					if (!strncmp(buf, "\n", 1)) {
						printf(" It only contains enter, invalid input！！！\n");
						goto _retry;
					}
					/*buf contains '\n'*/
					if (strchr(buf, '\n'))
						len = send(newfd, buf, strlen(buf) - 1, 0);
					/*buf does not contain '\n'*/
					else
						len = send(newfd, buf, strlen(buf), 0);
					if (len > 0)
						printf(
								"\tMessage sent successfully ，the number of bytes for this message is： %d\n",
								len);
					else {
						printf("Message sent with failure!\n");
						break;
					}
				}
				/*Message from the client*/
				if (FD_ISSET(newfd, &rfds)) {
					/******Receive the message*******/
					bzero(buf, BUFLEN);
					len = recv(newfd, buf, BUFLEN, 0);
					if (len > 0)
						printf(
								"The message received from the client is： %s, the number of bytes is: %d\n",
								buf, len);
					else {
						if (len < 0)
							printf("Message receiving failed！\n");
						else
							printf(
									"The client quit the communicating, connection exit！\n");
						break;
					}
				}
			}
		}
		/*close the communicating socket*/
		close(newfd);
	}
	/*Close the server socket*/
	close(sockfd);
	return 0;
}
