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
	printf("\nServer initialized successfully at port %d\n", DEF_PORT);
}

void startListening(Server *s) {
	if(listen ((*s).sockfd, MAX_CONN) == -1) {
		perror("ERROR on listen");
		exit(1);
	}

	printf("\nListening for connection...\n\n");
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
				acceptAllNewConnection(s);
			}
			
			/* Notification from a connection socket with client */
			else {
				serveRequest((*s).events[i].data.fd);
            }
				
		}
			
	}
}

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

void acceptAllNewConnection(Server *s) {
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
		
		//printf("New connection added at socket %d\n", newsockfd);
	}
}

void serveRequest(int sockfd) {
	
	/* PLACEHOLDER REQUEST HANDLER FOR TESTING */
	
	char buf[1024];
	char *req = NULL;
	int done = 0;
	//printf("Request received from connection socket %d\n", sockfd);
	
	while (1) {
		ssize_t count = read(sockfd, buf, sizeof buf);

		/* Socket Error */
		if(count == -1) {
			
			/* Read would block, (partial data?) */
			if (errno == EAGAIN) {
				done = 1;
				//printf("Socket EAGAIN\n");
				break;
				
			} else {
				done = 1;
				perror("ERROR socket read");
				break;
			}
		}

		/* EOF - Client closed the connection */
		else if(count == 0) {
			done = 1;
			//printf("Socket EOF\n");
			break;
		}
		
		/* There is data to be processed */
		else {
			if(req == NULL) {
				req = parseRequest(buf);
			}
		}
	}
	
	/* Check if request fully processed (for now, this will always be the case) */
	if(done) {
		
		/* Check if the request is a valid GET Request */
		if(req != NULL) {
			void *resp;
			int fd;
			char *file_name;
			
			printf("Get Request received for %s\n", req);
			
			file_name = concatString("www",req);
			fd = open(file_name, O_RDONLY);
			if(fd < 0) {
				if(write(sockfd, "File Not Found", 14) == -1) {
					perror("ERROR writing to socket");
					exit(1);
				}

			} else {
				while (1) {

					// Read data into buffer.  We may not have enough to fill up buffer, so we
					// store how many bytes were actually read in bytes_read.
					int bytes_read = read(fd, buf, sizeof(buf));
					if (bytes_read == 0) // We're done reading from the file
						break;

					if (bytes_read < 0) {
						perror("ERROR reading socket");
						exit(1);
					}

					// You need a loop for the write, because not all of the data may be written
					// in one call; write will return how many bytes were written. p keeps
					// track of where in the buffer we are, while we decrement bytes_read
					// to keep track of how many bytes are left to write.
					void *p = buf;
					while (bytes_read > 0) {
						int bytes_written = write(sockfd, p, bytes_read);
						if (bytes_written <= 0) {
							perror("ERROR writing to socket");
							exit(1);
						}
						bytes_read -= bytes_written;
						p += bytes_written;
					}
				}
				
				close(fd);
			}
			
		} else if(write(sockfd, "Invalid GET Request", 19) == -1) {
			perror("ERROR writing to socket");
			exit(1);
		}
		
		
		close(sockfd);
		//printf("Closing connection socket %d\n", sockfd);
	}
}

const char newline[2] = "\n";
const char space[2] = " ";
const char get[4] = "GET";

char *parseRequest(char *buf) {
	char *line;		   
	char *tok = NULL;
	
	line = strtok(buf, newline);
	while(line != NULL) {
		
		if(strstr(line, get) != NULL) {
			strtok(line, space);
			tok = strtok(NULL, space);
			break;
		}
		
		line = strtok(NULL, newline);
	}
	
	return tok;
}


char* concatString(const char *s1, const char *s2) {
    char *result = malloc(strlen(s1)+strlen(s2)+1);//+1 for the zero-terminator
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}












