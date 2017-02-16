#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

// UDP Client to show the multi threading feature is using multiple cpu's and is faster

int main(void)
{
  int sock;
  struct sockaddr_in sa;
  int bytes_sent;
  char buffer[200];

  strcpy(buffer, "GET / HTTP/1.0");
  struct timespec tim;
  tim.tv_sec = 0;
  tim.tv_nsec = 5000000;   //timeout so that we make sure the server recieves all the requests
  /* create an Internet, datagram, socket using UDP */
  sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (-1 == sock) {
      printf("Error Creating Socket");
      exit(EXIT_FAILURE);
    }
  memset(&sa, 0, sizeof sa);

  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = inet_addr("10.10.1.100");
  sa.sin_port = htons(8080);
int i;
for(i=0;i<1000;i++){
  nanosleep(&tim, NULL); //sleep
  bytes_sent = sendto(sock, buffer, strlen(buffer), 0,(struct sockaddr*)&sa, sizeof sa); //send bytes
  if (bytes_sent < 0) {
    printf("Error sending packet: %s\n", strerror(errno));
   exit(EXIT_FAILURE);
  }
}

  close(sock); 
  return 0;
}
