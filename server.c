#include "server.h"

///////////////////
// PUBLIC METHOD //
///////////////////

void initServer(Server *s, int portno) {   
	(*s).sockfd = createAndBindSocket(portno);
	if ((*s).sockfd < 0) {
		exit(1);
	}
	
	if(makeSocketUnblockable((*s).sockfd) == -1) {
		exit(1);
	}
	
	
	struct epoll_event socket_event;
	
	(*s).epollfd = epoll_create1(0);
	if ((*s).epollfd < 0) {
		perror("ERROR creating epoll");
		exit(1);
	}
	
	socket_event.data.fd = (*s).sockfd;
	socket_event.events = EPOLLIN | EPOLLET;
	if(epoll_ctl((*s).epollfd, EPOLL_CTL_ADD, (*s).sockfd, &socket_event) == -1) {
		perror("ERROR adding socket event");
		exit(1);
	}
	
	(*s).events = calloc(MAX_EVENT, sizeof socket_event);
}

void startListening(Server *s) {
	if(listen ((*s).sockfd, MAX_CONN) == -1) {
		perror("ERROR on listen");
		exit(1);
	}
	
	while(1) {
		int i, n;
		n = epoll_wait ((*s).epollfd, (*s).events, MAX_EVENT, -1);
		
		for(i=0; i<n; i++) {
			
			/* An error has occured on this fd */
			if(((*s).events[i].events & EPOLLERR) || ((*s).events[i].events & EPOLLHUP) || (!((*s).events[i].events & EPOLLIN))) {
				fprintf(stderr, "Epoll Error\n");
				close ((*s).events[i].data.fd);
				continue;
			}
			
			/* Notification from the server socket -> New Connection(s) */
			else if((*s).events[i].data.fd == (*s).sockfd) {
				
				while(1) {
					int newsockfd, clilen;
					struct sockaddr_in cli_addr;
					clilen = sizeof(cli_addr);
					
					/* Accept actual connection from the client */
					newsockfd = accept((*s).sockfd, (struct sockaddr *)&cli_addr, &clilen);				
					if (newsockfd < 0) {
						
						/* Check if error is because no more connection exist */
						if ((errno != EAGAIN) && (errno != EWOULDBLOCK)) {
							perror("ERROR on accept");
                        }
						break;
					}
					
					/* Make socket non-blocking and add it to the event queue */
					if(makeSocketUnblockable(newsockfd) == -1) {
						exit(1);
					}
					
					struct epoll_event socket_event;
										
					socket_event.data.fd = newsockfd;
					socket_event.events = EPOLLIN | EPOLLET;
					if(epoll_ctl((*s).epollfd, EPOLL_CTL_ADD, newsockfd, &socket_event) == -1) {
						perror("ERROR adding socket event");
						exit(1);
					}
					
				}
			
			}
			
			/* Notification from a connection socket with client */
			else {
				
				char buf[1024];
				int done = 0;
				
				while (1) {
					ssize_t count = read((*s).events[i].data.fd, buf, sizeof buf);

					/* Socket Error */
					if (count == -1) {
						
						/* Read would block, (partial data?) */
						if (errno == EAGAIN) {
							done = 1;
							break;
							
						} else {
							done = 1;
							perror ("ERROR socket read");
							break;
						}
					}

					/* EOF - Client closed the connection */
					else if (count == 0) {
						done = 1;
						break;
					}
					
					/* There is data to be processed */
					else {
						if (write (1, buf, count) == -1) {
							perror ("write");
							exit(1);
						}
					}
				}
				
				if(done) { 
					if (write((*s).events[i].data.fd, "I got your message", 18) == -1) {
						perror("ERROR writing to socket");
						exit(1);
					}
					close((*s).events[i].data.fd);
				}
            }
				
		}
			
	}
}

/*
 * 	n = write(newsockfd,"I got your message",18);
   
	if (n < 0) {
		perror("ERROR writing to socket");
		exit(1);
	}
 * */

///////////////////////////
// PRIVATE HELPER METHOD //
///////////////////////////

int createAndBindSocket(int portno) {
	int sockfd;
	struct sockaddr_in serv_addr;
	
	/* First call to socket() function */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if (sockfd < 0) {
		perror("ERROR opening socket");
		return -1;
	}
	
	/* Initialize socket structure */
	bzero((char *) &(serv_addr), sizeof(serv_addr));
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	
	/* Now bind the host address using bind() call.*/
	if (bind(sockfd, (struct sockaddr *) &(serv_addr), sizeof(serv_addr)) < 0) {
		perror("ERROR on binding");
		return -1;
	}
	
	return sockfd;
}

int makeSocketUnblockable(int fd) {
	
	/*
	 * 	int old_flag, new_flag;
		
		old_flag = fcntl(fd, F_GETFL, 0);
		if (old_flag == -1) {
			perror ("ERROR fcntl get");
			return -1;
		}
		
		new_flag |= O_NONBLOCK;
	 * */
	
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
		perror ("ERROR fcntl set");
		return -1;
	}
	
	return 0;
}

