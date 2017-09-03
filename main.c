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

	initServer(&server, DEF_PORT);
	startListening(&server);
		
	return 0;
}
