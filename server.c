#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>

#include <string.h>

#include "server.h"

void startServer(Server *s, int portno) {   
	/* First call to socket() function */
	(*s).sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
	if ((*s).sockfd < 0) {
		perror("ERROR opening socket");
		exit(1);
	}
   
	/* Initialize socket structure */
	bzero((char *) &((*s).serv_addr), sizeof((*s).serv_addr));
	(*s).portno = portno;
   
	(*s).serv_addr.sin_family = AF_INET;
	(*s).serv_addr.sin_addr.s_addr = INADDR_ANY;
	(*s).serv_addr.sin_port = htons((*s).portno);
   
	/* Now bind the host address using bind() call.*/
	if (bind((*s).sockfd, (struct sockaddr *) &((*s).serv_addr), sizeof((*s).serv_addr)) < 0) {
		perror("ERROR on binding");
		exit(1);
	}	
}

