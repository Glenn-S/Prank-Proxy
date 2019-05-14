/**
 * Assignment 1
 * CPSC 441
 * Glenn Skelton
 * 10041868
 *
 * Purpose: A joke proxy that transfers requests from he client to the server
 * and if the response from the server is a text or an html file, it will
 * modify it with a specified number of random spelling mistake
 *
 * inspiration for portions of code borrowed from:
 * https://www.binarytides.com/socket-programming-c-linux-tutorial/
 * Tutorial: Reza's example files and slides
 *
 * Filename: proxy.c
 */

// https://support.mozilla.org/en-US/questions/905902 for changing the cache to turn off

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <strings.h>
#include <netdb.h>

#define SIZE 500			// temporary buffer size
#define INIT "127.0.0.1"	// macro for proxy address
#define ERRORS 3			// macro for total number of errors to inject a file with

#define DEBUG 1 			// 1 to turn debugging mode on
#define SINGLE_LOOP 0 		// 1 to turn on single request mode for debuggin


/* PROTOTYPS */
int getHeaderLength(const char*, int);
int getResponseCode(const char*, int);
void getContentType(const char*, int, char*);
int getContentLength(const char*, int);
void editMessage(char*, int);
char randLetter(char);

/**
 * Purpose: To run the main proxy operations of creating and maintaining the sockets
 */
int main(int argc, char *argv[]) {
	short PORT;					// storage for port number entered by user
	char hostRequest[SIZE]; 	// used to store the host name from parsing the GET message
	char webIP[100];
	struct hostent *he;
	struct in_addr **addr_list;

	// check arguments
	if (argc != 2) {
		printf("./proxy <port_num>\n");
		exit(EXIT_FAILURE);
	}
	// get port number to use for communications
	if ((PORT = (short)atoi(argv[1])) == 0) {
		printf("Failed to convert argument to integer\n");
		exit(EXIT_FAILURE);
	}
#if DEBUG
	printf("Using port %s\n", argv[1]);
#endif

	// setup server side
	struct sockaddr_in server, client;
	int server_desc;
	memset(&server, 0, sizeof(struct sockaddr_in));
	server.sin_addr.s_addr = inet_addr(INIT);
	server.sin_family = AF_INET; // set to IPv4
	server.sin_port = htons(PORT);

	// create listening socket
	server_desc = socket(AF_INET, SOCK_STREAM, 0); // set up socket for TCP
	if (server_desc < 0) {
		printf("Error in creating listening socket");
		exit(EXIT_FAILURE);
	}

	// bind the socket
	if (bind(server_desc, (struct sockaddr*)&server, sizeof(server)) < 0) {
		printf("Error binding socket\n");
		close(server_desc);
		exit(EXIT_FAILURE);
	}
#if DEBUG
	printf("Bind success\n");
#endif

	// set socket to listen
	if (listen(server_desc, 10) < 0) { // ten participants can wait for connection
		printf("Error in connection\n");
		close(server_desc);
		exit(EXIT_FAILURE);
	}
	printf("listening for requests...\n");
	/************************* end of initializations ************************/


	// main client requests loop
	while (1) {
		// accept client request
		int client_desc;
		client_desc = accept(server_desc, NULL, NULL);
		if (client_desc < 0) {
			printf("Error accepting connection\n");
			close(client_desc);
			continue;
		}
		printf("Connection established\n");

		// get client message to parse the address
		int clientMsgSize; // store the size of the message sent by the client
		if ((clientMsgSize = recv(client_desc, hostRequest, sizeof(hostRequest), 0)) < 0) {
			printf("Error receiving message\n");
			close(client_desc);
			continue;
		}
#if DEBUG
		printf("client message length: %d\n", clientMsgSize);
		printf("client: \n%s\n", hostRequest); // received client request
#endif

		// get the host and address from the GET request
		char HOSTNAME[clientMsgSize], GETREQ[clientMsgSize]; // host and address storage
		char *hostPtr, *ptr;
		if (strstr(hostRequest, "GET http://") != NULL) { // check to make sure it is a GET
			if ((hostPtr = strstr(hostRequest, "\r\n")) != NULL) {
				int index = (int)(hostPtr-hostRequest); // get the GET line length
				memset(GETREQ, 0, sizeof(GETREQ)); // clear the memory of GETREQ
				if ((strncpy(GETREQ, hostRequest, index)) == NULL) { // copy request line
					printf("Error copying\n");
					continue;
				}
				if ((ptr = strstr(GETREQ+11, "/")) != NULL) { // get the host address
					index = (int)(ptr-(GETREQ+11));
					memset(HOSTNAME, 0, sizeof(HOSTNAME));
					if ((ptr = strncpy(HOSTNAME, GETREQ+11, index)) == NULL) {
						printf("Error copying\n");
						continue;
					}
				}
			}
		}
		else {
			close(client_desc);
			continue;
		}
#if DEBUG
		printf("\n%s\n", HOSTNAME);
#endif

		// receive and parse a message here and then create client with it
		// borrowed from https://www.binarytides.com/socket-programming-c-linux-tutorial/
		if ((he = gethostbyname(HOSTNAME)) == NULL) {
			printf("Ip resolve failure\n");
			close(client_desc);
			continue;
		}
		addr_list = (struct in_addr**) he->h_addr_list;
		for (int i = 0; addr_list[i] != NULL; i++) strcpy(webIP, inet_ntoa(*addr_list[i]));
#if DEBUG
		printf("%s resolved to: %s\n", HOSTNAME, webIP);
#endif

		// initialize address for browser client
		memset(&client, 0, sizeof(struct sockaddr_in));
		client.sin_addr.s_addr = inet_addr(webIP);
		client.sin_family = AF_INET; // assign IPv4
		client.sin_port = htons(80); // port 80 for http access

		// create the socket to web server
		int web_server_sock = socket(AF_INET, SOCK_STREAM, 0); // assign the description
		if (web_server_sock < 0) {
			printf("Can\'t create client socket\n");
			close(client_desc);
			close(web_server_sock);
			continue;
		}
#if DEBUG
		printf("Client socket created\n");
#endif

		// connect to the server
		if (connect(web_server_sock, (struct sockaddr*)&client, sizeof(struct sockaddr_in)) < 0) {
			printf("Connection error\n");
			close(client_desc);
			close(web_server_sock);
			continue;
		}
#if DEBUG
		printf("Connection established\n");
#endif

		// send the reqeust
		int sendCode;
		if ((sendCode = send(web_server_sock, hostRequest, sizeof(hostRequest), 0)) < 0) {
			printf("Failed to send data\n");
			close(client_desc);
			close(web_server_sock);
			continue;
		}
#if DEBUG
		printf("sent to server\n");
#endif

		// get first packet, if it's not done, enter the while loop
		char server_seg[1000]; // server messages buffer
		int recvCode;

		memset(server_seg, 0, sizeof(server_seg)); // zero the server_reply
		if ((recvCode = recv(web_server_sock, server_seg, sizeof(server_seg), 0)) < 0) {
			printf("Reply failure\n");
			close(client_desc);
			close(web_server_sock);
			continue;
		}

		// get the header and its length, and content length
		int headerLen = getHeaderLength(server_seg, sizeof(server_seg));
		int contentLen = getContentLength(server_seg, sizeof(server_seg));
		int messageLength = headerLen + contentLen;
		char server_reply[messageLength+1];
		memset(server_reply, 0, sizeof(server_reply)); // reset the server_reply
		bcopy(server_seg, server_reply, sizeof(server_seg)); // move data from temp buffer into full buffer
#if DEBUG
		printf("CONTENT LEN: %d\nHEADER LEN: %d\nSIZE OF SERVER BUFFER %d\n",
				contentLen, headerLen, (int)sizeof(server_reply));
#endif

		// use message length as the incrementer
		if (recvCode < messageLength) { // if not all of the data was received
			int msgErrVal = 0; // for tracking errors
			int totalRead = recvCode; // used for indexing
#if DEBUG
			printf("\n\nMSG LEN: %d\nRECV LEN: %d\n", messageLength, recvCode);
#endif
			// loop to get the the rest of the content
			while (1) {
				memset(server_seg, 0, sizeof(server_seg)); // clear the temp buffer
				if ((recvCode = recv(web_server_sock, server_seg, sizeof(server_seg), 0)) < 0) {
					printf("Reply failure\n");
					msgErrVal = -1;
				}
				if (recvCode == 0) { // all content has been read in so exit
					break;
				}
				bcopy(server_seg, server_reply+totalRead, recvCode); //read in the total read
				totalRead += recvCode;
			}
			if (msgErrVal < 0) { // if there was an error, close sockets and loop through again
				printf("Error in getting server segments\n");
				close(client_desc);
				close(web_server_sock);
				continue;
			}
#if DEBUG
			printf("ORIGINAL MESSAGE:\n\n%s", server_reply);
#endif
		}

		// now that buffer has all the data, process and send to client
		int code = getResponseCode(server_reply, sizeof(server_reply));
#if DEBUG
		printf("CODE RETURNED: %d\n", code);
#endif

		switch (code) { // check the codes
		case 200:
			editMessage(server_reply, sizeof(server_reply)); // possibly modify content
			break;
		case 206: // partial content, fall through
		case 301: // moved permanently, fall through
		case 302: // found, fall through
		case 304: // not modified, fall through
		case 403: // forbidden, fall through
		case 404: // not found, fall through
		default: break;
		}
#if DEBUG
		printf("SENT MESSAGE (Possibly Modified):\n\n%s", server_reply);
#endif

		// send browser information back
		if (send(client_desc, server_reply, sizeof(server_reply), 0) < 0) {
			printf("Error sending response\n");
			close(client_desc);
			close(web_server_sock);
			continue;
		}
#if DEBUG
		printf("Sent response to client\n");
#endif

		// when done with clients request, close the sockets
		close(web_server_sock);
		close(client_desc);
#if SINGLE_LOOP
		break;
#endif
	}
	close(server_desc); // close the initial server socket and exit
	return 0;
}


/**
 * Purpose: to get the server response code and convert into to an integer
 * Param: msg - the message to be parsed
 * len - the length of the message
 * Return: the integer value of the response code, 0 if not a specified
 */
int getResponseCode(const char *msg, int len) {
	char *ptr;
	if ((ptr = strstr(msg, "200 OK\r\n")) != NULL) return 200;
	else if ((ptr = strstr(msg, "206 Partial Content\r\n")) != NULL) return 206;
	else if ((ptr = strstr(msg, "301 Moved Permanently\r\n")) != NULL) return 301;
	else if ((ptr = strstr(msg, "302 Found - Redirection\r\n")) != NULL) return 302;
	else if ((ptr = strstr(msg, "304 Not Modified\r\n")) != NULL) return 304;
	else if ((ptr = strstr(msg, "403 Forbidden\r\n")) != NULL) return 403;
	else if ((ptr = strstr(msg, "404 Not Found\r\n")) != NULL) return 404;
	else return 0; // unspecified type
}

/**
 * Purpose: to get the message content type for parsing
 * Param: msg - the message to obtain the content type from
 * len - the length of the message
 * type - a string address to copy and return the type to the caller
 * Return: None
 * Side Effect: If there is an error, type will contain NULL
 */
void getContentType(const char *msg, int len, char *type) {
	// get the message content type
	char *ptr, *tmp;
	if ((ptr = strstr(msg, "Content-Type: ")) != NULL) { // check message for proper syntax
		if ((tmp = strstr(ptr, "; charset")) != NULL) {
			if (strncpy(type, ptr+14, (int)(tmp-(ptr+14))) != NULL) {
				return; // copy successful so return with the type
			}
		}
	}
#if DEBUG
	printf("Error in content type\n");
#endif
	type = NULL;
	return;
}

/**
 * Purpose: to get the content length of the mesage for parsing
 * Param: msg - the message to obtain the content length from
 * len - the length of the message
 * Return: length of the content, -1 if there is an error
 */
int getContentLength(const char *msg, int len) {
	// get the message content-length
	int bodyLen;
	char *ptr, *tmp;
	if ((ptr = strstr(msg, "Content-Length: ")) != NULL) { // check the message syntax
		if ((tmp = strstr(msg+(ptr-msg), "\r\n")) != NULL) {
			char strBodyLen[(int)(tmp-(ptr+16))+1];
			memset(strBodyLen, 0, sizeof(strBodyLen));
			if (strncpy(strBodyLen, (ptr+16), (int)(tmp-(ptr+16))) != NULL){
				if ((bodyLen = atoi(strBodyLen)) == 0) {
					return -1;
				}
				return bodyLen;
			}
		}
	}
	return -1;
}

/**
 * Purpose: To get the header length of the message passed in
 * Param: msg - the message to obtain the header length from
 * len - the length of the message
 * Return: The length of the header, -1 if there is an error
 */
int getHeaderLength(const char *msg, int len) {
	int headerLen;
	char *ptr;
	if ((ptr = strstr(msg, "\r\n\r\n")) != NULL) { // find the end of the header
		headerLen = (ptr+4)-msg;
		return headerLen;
	}
	else return -1;
}

/**
 * Purpose: To check and see if the message should be modified
 * and if so, modify it for its specific type.
 * Param: msg - the message to edit
 * len - the length of the message
 * Side Effect: Errors can show up in any part of the file, even in the
 * title for html.
 */
void editMessage(char *msg, int len) {
	// get the content type
	char type[100];
	int errorIndices[ERRORS], flag = 0; // array for storing used indices
	memset(type, 0, sizeof(type)); // zero out the memory
	getContentType(msg, len, type);

	// get the content length
	int bodyLen = getContentLength(msg, len);

	// if all is well, change the message up
	time_t t; // used to seed the srand() random number generator
	srand((unsigned) time(&t));

	for (int i = 0; i < ERRORS; i++) {
		int index = (len-bodyLen) + (rand() % bodyLen); // get index numbers in the range of the size of the content

		// add errors to a text file
		if (strcmp(type, "text/plain") == 0) {
			while (1) {
				if (((msg[index] >= 65) && (msg[index] <= 90)) ||
						((msg[index] >= 97) && (msg[index] <= 122))) {
					for (int err = 0; err < ERRORS; err++) { // make sure index isn't repeated
						if (errorIndices[err] != index) {
							flag = 1;
							break;
						}
					}
					if (flag == 1) { // if valid index for chaning, exit while loop
						break;
					}
				}
				index = (rand() % bodyLen) + (len-bodyLen); // pick a new random number
			}
#if DEBUG
			printf("CHARACTER TO REPLACE: %c AT %d\n", msg[index], index);
#endif
			msg[index] = randLetter(msg[index]); // change character
			errorIndices[i] = index; // save index so not to repeat it
		}

		// add errors to an html file
		else if (strcmp(type, "text/html") == 0) {
			int val = 1;
			while (val) {
				if (((msg[index] >= 65) && (msg[index] <= 90)) ||
						((msg[index] >= 97) && (msg[index] <= 122))) {
					for (int j = index; j < len; j++) { // search to the right
						if (msg[j] == '>') {
							break; // in a tag so choose again
						}
						else if (msg[j] == '<') { // not in a tag so use it
							for (int err = 0; err < ERRORS; err++) { // make sure an index isn't repeated
								if (errorIndices[err] != index) {
									flag = 1;
									break;
								}
							}
							if (flag == 1) {
#if DEBUG
								printf("CHARACTER TO REPLACE: %c AT %d\n", msg[index], index);
#endif
								msg[index] = randLetter(msg[index]);
								val = 0; // terminate the while loop to begin next iteration
								errorIndices[i] = index; // save index so not to repeat it
								break;
							}
						}
					}
				}
				index = (rand() % bodyLen) + (len-bodyLen); // pick a new random number
			}
		}
	}
}

/**
 * Purpose: To change a letter passed in into a random letter of the same casing
 * Param: let - the character to be changed
 * Return: the converted character value
 */
char randLetter(char let) {
	char newLet = rand() % 26;
	if (let < 97) {
		return 65+newLet;
	}
	else return 97+newLet;
}

