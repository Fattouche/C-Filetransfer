#include <stdio.h>

#include <sys/types.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <stdbool.h>

#define IN_PACKET  1024
#define STRING_SIZE 256
#define NUMTHREADS 4

/**
* Pthread information was found from the various youtube videos by DrDBFraser:
*
*      1. https://www.youtube.com/watch?v=ynCc-v0K-do
*      2. https://www.youtube.com/watch?v=1ks-oMotUjc
*
*
*
*
* inet_ntoa found at http://pubs.opengroup.org/onlinepubs/7908799/xns/arpainet.h.html
*
**/

//This struct holds all the header information. It can be used with or without threading
 struct threading {
  char directory[STRING_SIZE];
  socklen_t addressSize;
  size_t socket;
  size_t port;
  struct sockaddr_in clientAddress;
  char buffer[STRING_SIZE];
};

//This is a simple function that was created because sendto was being called frequently.
void sendBack(struct threading *thread, char *response){
	int divisors = strlen(response)/500;
	int i;
	for(i=0;i<divisors;i++){
		if (sendto(thread->socket, &response[i*500], 500, 0, (struct sockaddr *)&thread->clientAddress, thread->addressSize) == -1) {
				printf("Couldnt send message to: %s\n", inet_ntoa(thread->clientAddress.sin_addr));
		}
	}
	int leftover = strlen(response)-(500*divisors);
	if (sendto(thread->socket, &response[i*500], leftover, 0, (struct sockaddr *)&thread->clientAddress, thread->addressSize) == -1) {
				printf("Couldnt send message to: %s\n", inet_ntoa(thread->clientAddress.sin_addr));
	}
}

//This is the core function for parsing data and using threads
void *HandleRequest(void *req){
	//apparently we cant directly access this void pointer so we need to assign it to a type.
	struct threading *thread = req;

	//We create the strings for the three possible situations
	char *notFound = "HTTP/1.0 404 Not Found";
	char *badRequest = "HTTP/1.0 400 Bad Request";
	char *okayRequest = "HTTP/1.0 200 OK";
	int responseCode = 0;

	//These are the strings that we will store the parsed data in
	char requestType[STRING_SIZE];
	char filePath[STRING_SIZE];
	char httpVersion[STRING_SIZE];

	//just a quick input sanitation so the server doesnt crash
	if(thread->buffer[0]=='\0'){
		sendBack(thread,badRequest);
		return (void *) 0;
	}

	//Handle all the string manipulation
	int i=0;
	for(i=0;thread->buffer[i]!=' ';i++){
		if(thread->buffer[i]=='\0'){
			sendBack(thread, badRequest);
			return (void *) 0;
		}
		requestType[i] = thread->buffer[i];
	}
	requestType[i] = '\0';


	//Handle the case where they pass in not GET
	i++;
	int counter=0;
	for(;thread->buffer[i]!=' ';i++){
		if(thread->buffer[i]=='\0'){
			sendBack(thread, badRequest);
			return (void *) 0;
		}
		filePath[counter] = thread->buffer[i];
		counter++;
	}
	filePath[counter] = '\0';

	//Handle the last input ie. HTTP/1.0
	i++;
	counter=0;
	for(;thread->buffer[i]!='\0' && thread->buffer[i]!='\n' && thread->buffer[i]!='\r' && thread->buffer[i]!=' ';i++){
		httpVersion[counter] = thread->buffer[i];
		counter++;
	}
	httpVersion[counter] = '\0';

	//C doesnt have a simple strcmpr with ignoring case
	int k;
	for(k = 0; k < strlen(requestType); k++)
		requestType[k] = toupper(requestType[k]);
	for(k = 0; k < strlen(httpVersion); k++)
		httpVersion[k] = toupper(httpVersion[k]);

	//We want to check for more bad requests, such as incorrectly spelling get, putting in the wrong httpversion.
	if ((strncmp(filePath, "/",1) != 0) || (strcmp(requestType, "GET") != 0) || (strcmp(httpVersion, "HTTP/1.0") !=0 )) {
		responseCode = 400;
		sendBack(thread, badRequest);
    }

	//If the entire file is a backslash we default to index.html
	if(strcmp(filePath, "/")==0){
		strncat(filePath, "index.html", 12);
	}


	//We are now starting to work with parsing the file.
	char *dirPath = strncat(thread->directory,filePath, 256);
	struct stat fileInfo;
	char *fileReader;
	long fileSize;
	FILE *file;

	//If we still havent categorized as 400,404 or 200
	if(responseCode==0){
		//Cant find the file or trying to move backwards
		if (stat(dirPath,&fileInfo) < 0 || (strncmp(filePath, "/../", 4) == 0)) {
			responseCode = 404;
			sendBack(thread, notFound);
		}else{
			//We can open the file now, read all the contents and then store it into the filreader pointer.
			file = fopen(dirPath, "r");
			if(!file){
				printf("error reading file %s", dirPath);
				exit(1);
			}
			fseek (file , 0L , SEEK_END);
			fileSize = ftell (file);
			rewind (file);
			fileReader = calloc(1, fileSize+1);
			if( !fileReader ){
				fclose(file);
				printf("Couldnt allocate memory for the fileReader");
				exit(-1);
			}
			//this is where we store in char* filereader
			if(fread( fileReader , fileSize, 1 , file)!=1){
				responseCode = 400;
				sendBack(thread, badRequest);
			}else{
				responseCode = 200;
			}

		}
	}

	//If we succesfully read the file we can send back the contents
	if(responseCode==200){
		char output[fileSize+50];
		strcat(output, "HTTP/1.0 200 OK\r\n\r\n");
		strcat(output, fileReader);
		strcat(output, "\0");
		sendBack(thread, output);
		free(fileReader);
	}

	//Hand the log message after the response code has been finalized
	char *logMessage;
	if(responseCode==200){
		logMessage = okayRequest;
	}else if(responseCode == 400){
		logMessage = badRequest;
	}else if(responseCode == 404){
		logMessage = notFound;
	}else{
		logMessage = "No Assigned Response code";
	}
  int t;
  for(t=0;thread->buffer[t]!='\0';t++){
    if(thread->buffer[t]!='\r' || thread->buffer[t]!='\n'){
      thread->buffer[t]='\0';
    }
  }

	//Formatting the log time on the server side.
	time_t logTime;
	char timeFormat[25];
	struct tm* timeInfo;
	time (&logTime);
	timeInfo = localtime(&logTime);
	// strftime example was found at http://stackoverflow.com/questions/3673226/how-to-print-time-in-format-2009-08-10-181754-811
	strftime(timeFormat,24,"%b %d %H:%M:%S", timeInfo);
	printf("%s %s:%hu %s; %s; %s\n", timeFormat, inet_ntoa(thread->clientAddress.sin_addr), thread->clientAddress.sin_port, thread->buffer, logMessage, dirPath);

	//print to the file to show that main continues while thread works
	FILE *f = fopen("thread_out.txt", "ab");
	fprintf(f,"finished thread: %s\n\n", timeFormat);
	fclose(f);

	//Since we dont need to return any value from this function we can return a void pointer
	return (void *) 0;
}

/**
* Our main function simply takes in the UDP requests and creates new threads using pthread
*
* We will have our main thread in the function and then we will make n more threads
* depending on the number of concurrent requests we recieve
**/

int main(int argc, char *argv[])
{
	//Declare the socket as a UDP socket(SOCK_DGRAM)
   size_t sock = socket(AF_INET, SOCK_DGRAM, 0);
   int max_fd;
   int threadTest=0;
   int quit = 0;
   bool usingThread=false;

   //parse the command line arguments
   if(argc<3){
    printf("Invalid number of command line arguments \n");
	printf("To run the server: './sws <port> <directory>'\n");
	exit(EXIT_FAILURE);
   }
   size_t portNum = atoi(argv[1]);
   char *dir = argv[2];
   if(argv[3]!=NULL){
		char *threadFlag = argv[3];
		if(strcmp(threadFlag,"--pthread")==0){
			usingThread=true;
		}
   }
   struct sockaddr_in serverAddress;


  //Used for select
   fd_set watcher;

   //I had to set this for stdout
  setvbuf (stdout, NULL, _IONBF, 0);

  //Once we have we can write this right away
  printf("sws is running on UDP port %zd and serving %s\n", portNum, dir);
  printf("press 'q' to quit...\n");

  ssize_t string;
  int enable = 1;

  //Allow for us to reuse the socket
  int trySetSockOpt = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
  if (trySetSockOpt < 0) {
     fprintf(stderr, "Failed on SetSockOpt");
     exit(EXIT_FAILURE);
  }


  //clear buffer
  bzero((char *)&serverAddress, sizeof(serverAddress));
  serverAddress.sin_port = htons(portNum);
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

  //Bind the socket to our ipaddress and given port
  int tryBind = bind(sock, (struct sockaddr *)&serverAddress, sizeof serverAddress);
  if(tryBind!=0){
	  printf("The socket could not be binded with the given server address");
	  close(sock);
	  exit(EXIT_FAILURE);
  }

		int threadCount=0;
		int joinCount=0;
	    pthread_t threads[NUMTHREADS];


  while(1) {
	FD_ZERO( &watcher );
    FD_SET( STDIN_FILENO, &watcher);
    FD_SET( sock, &watcher);
    fd_set dup = watcher;
	max_fd = sock;

	struct threading *thread = calloc(1, sizeof *thread);
    thread->socket = sock;
    thread->addressSize = sizeof(thread->clientAddress);
    strncpy(thread->directory, dir, STRING_SIZE);

	fflush( stdout );

	//Since we dont care which request happens, our select waits infinitely long until either q is pressed or we recieve a request
	if (select(max_fd+1, &dup, NULL, NULL, NULL) < 0) {
        perror("select");
        return -1;
	}
	int fd;
	//we check all the file descriptors for any updates
	for (fd = 0; fd <= max_fd; fd++) {
        if (FD_ISSET(fd, &dup)) {
			//if the request is standard input and q is pressed
            if (fd == STDIN_FILENO) {
				if(getchar()=='q'){
					//Since the user could quit at any time we want to wait untill all the threads are done working to return.
					quit=1;
					break;
				}
            }
			//if the request is from our socket
            else if (fd == sock) {
                string = recvfrom(thread->socket, &thread->buffer, IN_PACKET, 0, (struct sockaddr*)&thread->clientAddress, &thread->addressSize);
				thread->buffer[string] = '\0';
				if (string < 0) {
				  perror("\nrecvfrom");
				  exit(EXIT_FAILURE);
				}
				if(usingThread==false){
					HandleRequest(thread);
				}else{
				//Using threads:

				 //create the thread on the function Handle Request, with the parameter of our struct
				 //In C if you dont call a function with brackets it will call the pointer function
				 if(threadCount<NUMTHREADS){
					pthread_create(&threads[threadCount], NULL, HandleRequest, thread);
					threadCount++;
					if(joinCount<NUMTHREADS)
						joinCount++;
				 }else if(threadCount>=NUMTHREADS){
					threadCount=0;
					pthread_create(&threads[threadCount], NULL, HandleRequest, thread);
				 }

				 //We now begin the thread test to show that we continue through the loop even while the thread is working
				 //This means that if we get multiple requests at the same time we will just continue dealing with them.
				 time_t logTime;
				 char timeFormat[25];
				 struct tm* timeInfo;
				 time (&logTime);
				 timeInfo = localtime(&logTime);
				 // strftime example was found at http://stackoverflow.com/questions/3673226/how-to-print-time-in-format-2009-08-10-181754-811
				 strftime(timeFormat,24,"%b %d %H:%M:%S", timeInfo);

				 FILE *f = fopen("thread_out.txt", "ab");
				 while(++threadTest < 100);
				 fprintf(f,"Finished Main loop thread number %d: %s\n",threadCount,timeFormat);
				 fclose(f);
				}

            }
        }
    }
	if(quit==1){
		//free our calloc pointer
		free(thread);
		break;
	}
  }
  //This is the exit gracefully part, we will wait for all threads to finish and then close the socket.
  int j;
  if(usingThread){
	  for(j=0; j <10; j++){
		  if(j<joinCount)
			pthread_join(threads[j], NULL);
	   }
	   pthread_exit(NULL);
  }
  close(sock);
  return 0;
}
