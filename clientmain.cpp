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
  calcMessage msg;
  
  if((rv=getaddrinfo(Desthost,Destport, &sa,&si)) != 0)
  {
    fprintf(stderr, "getadrrinfo: %s\n", gai_strerror(rv));
  }
  struct sockaddr_in servaddr;
  for(p = si; p != NULL; p = p->ai_next)
  {
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
  msg.type = 22;
  msg.message = 0;
  msg.protocol = 17;
  msg.major_version = 1;
  msg.minor_version = 0;
  if((sentbytes = sendto(sockfd,&msg,sizeof(msg),0,p->ai_addr,p->ai_addrlen)) == -1)
  {
    printf("Failed to send via sento function. \n");
    close (sockfd);
    exit(0);
  }

  printf("Message sent.\n");
  
  socklen_t addr_len = sizeof(servaddr);
  calcProtocol protmsg;
  if(recvfrom(sockfd, &protmsg,sizeof(protmsg),0,(struct sockaddr*)&servaddr, &addr_len) == -1)
  {
    printf("Failed to recive message.\n");
    close(sockfd);
    exit(0);
  }

  if(protmsg.arith == 1)
  {
    protmsg.inResult = protmsg.inValue1+protmsg.inValue2;
    printf ("Add %d %d\n Result was: %d",protmsg.inValue1, protmsg.inValue2, protmsg.inResult);
  }
  else if(protmsg.arith == 2)
  {
    protmsg.inResult = protmsg.inValue1-protmsg.inValue2;
    printf ("Sub %d %d\n Result was: %d",protmsg.inValue1, protmsg.inValue2, protmsg.inResult);
  }
  else if(protmsg.arith == 3)
  {
      protmsg.inResult = protmsg.inValue1*protmsg.inValue2;
      printf ("Mult %d %d\n Result was: %d",protmsg.inValue1, protmsg.inValue2, protmsg.inResult);
  }
  else if(protmsg.arith == 4)
  {
      protmsg.inResult = protmsg.inValue1/protmsg.inValue2;
      printf ("Div %d %d\n Result was: %d",protmsg.inValue1, protmsg.inValue2, protmsg.inResult);
  }
  else if(protmsg.arith == 5)
  {
    protmsg.flResult = protmsg.flValue1 + protmsg.flValue2;
    printf ("fAdd %lf %lf\n Result was: %lf",protmsg.flValue1, protmsg.flValue2, protmsg.flResult);
  }  
  else if(protmsg.arith == 6)
  {
    protmsg.flResult = protmsg.flValue1 - protmsg.flValue2;
    printf ("fSub %lf %lf\n Result was: %lf",protmsg.flValue1, protmsg.flValue2, protmsg.flResult);
  }
    else if(protmsg.arith == 7)
  {
    protmsg.flResult = protmsg.flValue1 * protmsg.flValue2;
    printf ("fMult %lf %lf\n Result was: %lf",protmsg.flValue1, protmsg.flValue2, protmsg.flResult);
  }
    else if(protmsg.arith == 8)
  {
    protmsg.flResult = protmsg.flValue1 / protmsg.flValue2;
    printf ("fDiv %lf %lf\n Result was: %lf",protmsg.flValue1, protmsg.flValue2, protmsg.flResult);
  }

  printf("Arith: %d\n", protmsg.arith);

  close(sockfd);
  return 0;

}
