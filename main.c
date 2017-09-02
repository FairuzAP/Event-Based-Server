#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include "server.h"

int main( int argc, char *argv[] ) {

	struct Server server;
	int newsockfd, clilen;
	struct sockaddr_in cli_addr;
	int n;	
	char buffer[256];

	startServer(&server, 5001);

	/* Now start listening for the clients, here process will
		* go in sleep mode and will wait for the incoming connection
	*/
	
	listen(server.sockfd,5);
	clilen = sizeof(cli_addr);
	
	/* Accept actual connection from the client */
	newsockfd = accept(server.sockfd, (struct sockaddr *)&cli_addr, &clilen);
	
	if (newsockfd < 0) {
		perror("ERROR on accept");
		exit(1);
	}
	
	/* If connection is established then start communicating */
	bzero(buffer,256);
	n = read( newsockfd,buffer,255 );
	
	if (n < 0) {
		perror("ERROR reading from socket");
		exit(1);
	}
	
	printf("Here is the message: %s\n",buffer);
	
	/* Write a response to the client */
	n = write(newsockfd,"I got your message",18);
   
	if (n < 0) {
		perror("ERROR writing to socket");
		exit(1);
	}
	
	return 0;
}
