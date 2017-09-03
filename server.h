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


///////////////////
// PUBLIC METHOD //
///////////////////

/*
 * This procedure will initiate the server, doing:
 * 1. Start  the socket listener on localhost:<portno>
 * 2. Make socket unblockable
 * 3. Initialize the server's epoll 
 * 4. Add an epoll event for the server socket
 * 5. Initialize the events array
 * */
void initServer(Server *s, int portno);

/*
 * This procedure will start the server, doing:
 * 1. Start listening for incoming connection at the sockfd
 * 2. Add each incoming connection socketfd to the epoll event queue
 * 3. Serve each incoming request from a connection socketfd
 * */
void startListening(Server *s);


///////////////////////////
// PRIVATE HELPER METHOD //
///////////////////////////

/*
 * This procedure will create a listener socket at the given port
 * return -1 if an error happened
 * */
int createAndBindSocket(int portno);

/*
 * This procedure will prepare the given socket fd for unblockable-IO
 * return -1 if an error happened
 * */
int makeSocketUnblockable(int fd);

/*
 * This procedure will accept all incoming connection to the server, 
 * and register them all to the server's epoll event queue
 * */
void acceptAllNewConnection(Server *s);

/*
 * This procedure will handle the incoming request at the given sockfd,
 * DETAILS TBD
 * */
void serveRequest(int sockfd);

#endif

