#include <stdio.h>
#include <stdlib.h>
/* You will to add includes here */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
// Included to get the support library
#include "calcLib.h"


#include "protocol.h"

int main(int argc, char *argv[]){
  
  /* Do magic */
  char delim[] = ":";
  char *Desthost = strtok(argv[1], delim);
  char *Destport = strtok(NULL, delim);
  if (Desthost == NULL || Destport == NULL)
  {
    printf("Wrong format\n");
    exit(0);
  }
  int port = atoi(Destport);
  addrinfo sa, *si, *p;
  memset(&sa,0,sizeof(sa));
  sa.ai_family = AF_INET;
  sa.ai_socktype = SOCK_DGRAM;
  sa.ai_protocol = 17;
  int sockfd;
  int rv;
  struct protoent *servptr;
  
  if((rv=getaddrinfo(Desthost,Destport, &sa,&si)) != 0)
  {
    fprintf(stderr, "getadrrinfo: %s\n", gai_strerror(rv));
  }

  int k=0;

  for(p = si; p != NULL; p = p->ai_next)
  {
  #ifdef DEBUG
      servptr = getprotobynumber(p->ai_protocol);
  #endif  
      k++;
    if((sockfd = socket(p->ai_family,p->ai_socktype,p->ai_protocol)) == -1)
    {
      printf("Failed to creat socket.\n");
      continue;
    }
    printf("Socket created\n");
    break;
  }
  
  if(p==NULL)
  {
    printf("Failed to create socket, p=NULL.\n");
    freeaddrinfo(si);
    close(sockfd);
    exit(0);
  }
  
  ssize_t sentbytes;

  rv=connect(sockfd,p->ai_addr, p->ai_addrlen);
  printf("Connected\n");

  if(sentbytes = send(sockfd,0,0,NULL) == -1)
  {
    printf("Failed to send via send function. \n");
    close(sockfd);
    exit(0);
  }
  /*if(sentbytes ==-1 && (sentbytes = sendto(sockfd,0,0,NULL,p->ai_addr,p->ai_addrlen)) == -1)
  {
    printf("Failed to send via sento function. \n");
  }*/

  printf("Message sent.\n");




}
