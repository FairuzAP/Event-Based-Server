#ifndef SERVER
#define SERVER

#include <netdb.h>
#include <netinet/in.h>

typedef struct Server {
	int sockfd;
	int portno;
	struct sockaddr_in serv_addr;
} Server;

/*
 * This procedure will initiate the server, doing:
 * 1. Start  the socket listener on localhost:<portno>
 * 2. TBA
 * */
void startServer(Server *s, int portno);

#endif

