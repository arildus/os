#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#include "bbuffer.h"

#define MAXREQ (4096*1024)

int webSocket;
BNDBUF* bnbuffer;
char* path;
int port, numOfThreads, bufferSlots;


int parse_request(int fd, char client_request[MAXREQ]) {
		char *requested_file= strchr(client_request, ' ') + 1;
		char *end_of_path = strchr(requested_file, ' ');

		if(end_of_path != NULL) {
			*end_of_path = '\0';
		}

		//Create a copy of path variable
		char* requested_path = malloc(strlen(path)+strlen(requested_file));
		strcpy(requested_path, path);
		strcat(requested_path, requested_file);

		char* buffer = malloc(MAXREQ);
		char* body = malloc(MAXREQ);
		char* msg;

		char* buff = malloc(MAXREQ);
		char ch;
		int length_of_file = 0;

		FILE *file_ptr;
		file_ptr = fopen(requested_path, "r+");
		if (file_ptr == NULL) {
			//Since we always know the size of 404 message we set msg as a constant
			msg = malloc(MAXREQ);

			printf("Error reading from file. Returning file not found\n");

			snprintf (body, MAXREQ, "<html>\n<body>\n<h1>404 ERROR</h1>\nCould not find the requested page!</body>\n</html>\n");
			snprintf (msg, MAXREQ,"HTTP/1.0 404 Not Found\nContent-Type: text/html\nContent-Length: %ld\n\n%s", strlen (body), body);


			if(write(fd, msg, strlen(msg)) < 0) {
				printf("Something went wrong while sending response");
				return -1;
			}


		}
		else {
			while (!feof(file_ptr)) {
				ch = fgetc(file_ptr);
				buff[length_of_file] = ch;
				length_of_file++;
			}

			fclose(file_ptr);

			//Add constant 1000 for header data
			msg = malloc(length_of_file*sizeof(char) + 1000);

			snprintf (body, length_of_file, buff, buffer);
			snprintf (msg, length_of_file*sizeof(char) + 1000,"HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: %ld\n\n%s", strlen (body), body);
			

			if(write(fd, msg, strlen(msg)) < 0) {
				printf("Something went wrong while sending response");
				return -1;
			}
		}
		
		free(buff);
		free(buffer);
		free(body);
		free(msg);
		free(requested_path);
		close(fd);
		return 0;
}

void *worker(void *parm) {
	while(1) {

		// Try to get element from buffer. Semaphore makes the worker thread stop when buffer is empty
		int fd = bb_get(bnbuffer);
		char client_request[MAXREQ];
		int http_request = read(fd, client_request, MAXREQ-1);
		if(http_request < 0) {
			printf("Error reading from socket\n");
		}
		parse_request(fd, client_request);
		printf("Response was sent from thread: 0x%.8lx\n", pthread_self());
	}
}

int initiate_server() {
	int clientSock, clientLength, http_request;
	struct sockaddr_in6 server_addr, client_addr;

	bzero((char *) &server_addr, sizeof(server_addr));

	// Generate a socket
	webSocket = socket(AF_INET6, SOCK_STREAM, 0);
	if (webSocket < 0) {
		printf("ERROR opening socket\n");
		return -1;
	} 
	printf("Socket created\n");

	// Set port and IP
	server_addr.sin6_family = AF_INET6;
	server_addr.sin6_addr = in6addr_any;
	server_addr.sin6_port = htons(port);

	// Assign socket to address
	int binded_socket = bind(webSocket, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if (binded_socket < 0) {
		printf("ERROR on binding\n");
		return -1;
	}

	//Prepare the socket for connection requests
	if(listen(webSocket, 1) < 0) {
		printf("Error while listeing\n");
		return -1;
	}
	printf("Listening for connection\n");
	return 0;
}

int main_worker_loop(socklen_t clientLength, struct sockaddr_in client_addr) {
		clientLength = sizeof(client_addr);

		int fd = accept(webSocket, (struct sockaddr *) &client_addr, &clientLength);
		if (fd < 0) {
			printf("Could not accept\n");
			return -1;
		}
		bb_add(bnbuffer, fd);
		return 0;
}

int main( int argc, char *argv[]) {

	//Parse user input
  path = argv[1];
	port = atoi(argv[2]);
	numOfThreads = atoi(argv[3]);
	bufferSlots = atoi(argv[4]);

	printf("Path: %s\nPort: %d\nNumber of threads: %d\nBufferslots: %d\n", path, port, numOfThreads, bufferSlots);

	// Create bounded buffer
	bnbuffer = bb_init(bufferSlots);

	//Create threads
	pthread_t* thread_id = malloc(sizeof(pthread_t)*numOfThreads);

	for(int i=0; i < numOfThreads; i++)
   {
      pthread_create( &thread_id[i], NULL, &worker, NULL );
   }

	socklen_t clientLength;
	struct sockaddr_in client_addr;

	//Start server
	if(initiate_server() < 0) {
		printf("Failed to initiate server!");
		return -1;
	}

	// The main loop of the program
	while(1) {
		main_worker_loop(clientLength, client_addr);
	}
	
	/*
		Wait with continuing main() until the threads are done
	*/
	for(int j=0; j < numOfThreads; j++)
	{
		pthread_join( thread_id[j], NULL); 
	} 

	// Close the websocket when finished
	close(webSocket);

	// Delete the buffer
	bb_del(bnbuffer);

	return 0;
}