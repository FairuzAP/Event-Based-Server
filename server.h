#ifndef SERVER
#define SERVER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>

// The max number of parallel connection and event handled
#define MAX_CONN 32
#define MAX_EVENT 32

// The default port the server will listen to
#define DEF_PORT 5001

typedef struct Server {
	int sockfd;
	
	int epollfd;
	struct epoll_event *events;
} Server;

/*
 * This procedure will initiate the server, doing:
 * 1. Start  the socket listener on localhost:<portno>
 * 2. Make socket unblockable
 * 3. Initialize the server's epoll 
 * 4. Add an epoll event for the server socket
 * 5. Initialize the events array
 * */
void initServer(Server *s, int portno);

void startListening(Server *s);

#endif

