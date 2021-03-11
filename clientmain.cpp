#include <stdio.h>
#include <stdlib.h>
/* You will to add includes here */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
// Included to get the support library
#include "calcLib.h"
#define DEBUG
#include "protocol.h"

void intSignal(int sig)
{
  exit(0);
}
int main(int argc, char *argv[])
{

  /* Do magic */
  if (argc != 2)
  {
    printf("Wrong format.\n");
    exit(0);
  }
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
  memset(&sa, 0, sizeof(sa));
  sa.ai_family = AF_INET;
  sa.ai_socktype = SOCK_DGRAM;
  sa.ai_protocol = 17;
  int sockfd;
  int rv;
  calcMessage msg;
  struct timeval tv;
  tv.tv_sec = 2;
  tv.tv_usec = 0;
  if ((rv = getaddrinfo(Desthost, Destport, &sa, &si)) != 0)
  {
    fprintf(stderr, "getadrrinfo: %s\n", gai_strerror(rv));
  }
  struct sockaddr_in servaddr;
  for (p = si; p != NULL; p = p->ai_next)
  {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
    {
      printf("Failed to creat socket.\n");
      continue;
    }
    printf("Socket created\n");
    break;
  }
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));

  if (p == NULL)
  {
    printf("Failed to create socket, p=NULL.\n");
    freeaddrinfo(si);
    exit(0);
  }

  ssize_t sentbytes;
  msg.type = htons(22);
  msg.message = htonl(0);
  msg.protocol = htons(17);
  msg.major_version = htons(1);
  msg.minor_version = htons(0);

  printf("Message sent.\n");

  socklen_t addr_len = sizeof(servaddr);
  calcProtocol protmsg;
  calcMessage *message = new calcMessage{};
  int bytes = -1;
  int tries = 0;
  signal(SIGINT, intSignal);
  while (tries < 3 && bytes < 0)
  {
    if ((sentbytes = sendto(sockfd, &msg, sizeof(msg), 0, p->ai_addr, p->ai_addrlen)) == -1)
    {
      printf("Failed to send via sento function. \n");
      exit(0);
    }
    bytes = recvfrom(sockfd, &protmsg, sizeof(protmsg), 0, (struct sockaddr *)&servaddr, &addr_len);
    if (bytes == -1 && tries < 2)
    {
      printf("Server did not respond... Trying again.\n");
    }
    tries++;
  }
  if (bytes == -1)
  {
    printf("Failed to recive message or server did not respond. Exiting...\n");
    exit(0);
  }
  else
  {
    sleep(3);
  }
  if (sizeof(calcMessage) == bytes)
  {
    delete message;
    message = (calcMessage *)&protmsg;
    message->message = ntohl(message->message);
    message->type = ntohs(message->type);
    message->major_version = ntohs(message->major_version);
    message->minor_version = ntohs(message->minor_version);
    if(message->message == 0)
    {
      printf("Server send N/A, Exiting... \n");
      exit(0);
    }
    else if(message->message == 2)
    {
      printf("Recived an Error/NOT OK from server, something went wrong. Exiting... \n");
      exit(0);
    }
  }
  else if(bytes == sizeof(calcProtocol))
  {
    protmsg.arith = ntohl(protmsg.arith);
    protmsg.inResult = ntohl(protmsg.inResult);
    protmsg.inValue1 = ntohl(protmsg.inValue1);
    protmsg.inValue2 = ntohl(protmsg.inValue2);
    protmsg.id = ntohl(protmsg.id);
    protmsg.major_version = ntohs(protmsg.major_version);
    protmsg.minor_version = ntohs(protmsg.minor_version);
    protmsg.type = ntohs(protmsg.type);

    if (protmsg.arith == 1)
    {
      protmsg.inResult = protmsg.inValue1 + protmsg.inValue2;
      printf("Add %d %d\n Result was: %d\n", protmsg.inValue1, protmsg.inValue2, protmsg.inResult);
    }
    else if (protmsg.arith == 2)
    {
      protmsg.inResult = protmsg.inValue1 - protmsg.inValue2;
      printf("Sub %d %d\n Result was: %d\n", protmsg.inValue1, protmsg.inValue2, protmsg.inResult);
    }
    else if (protmsg.arith == 3)
    {
      protmsg.inResult = protmsg.inValue1 * protmsg.inValue2;
      printf("Mult %d %d\n Result was: %d\n", protmsg.inValue1, protmsg.inValue2, protmsg.inResult);
    }
    else if (protmsg.arith == 4)
    {
      protmsg.inResult = protmsg.inValue1 / protmsg.inValue2;
      printf("Div %d %d\n Result was: %d\n", protmsg.inValue1, protmsg.inValue2, protmsg.inResult);
    }
    else if (protmsg.arith == 5)
    {
      protmsg.flResult = protmsg.flValue1 + protmsg.flValue2;
      printf("fAdd %lf %lf\n Result was: %lf\n", protmsg.flValue1, protmsg.flValue2, protmsg.flResult);
    }
    else if (protmsg.arith == 6)
    {
      protmsg.flResult = protmsg.flValue1 - protmsg.flValue2;
      printf("fSub %lf %lf\n Result was: %lf\n", protmsg.flValue1, protmsg.flValue2, protmsg.flResult);
    }
    else if (protmsg.arith == 7)
    {
      protmsg.flResult = protmsg.flValue1 * protmsg.flValue2;
      printf("fMult %lf %lf\n Result was: %lf\n", protmsg.flValue1, protmsg.flValue2, protmsg.flResult);
    }
    else if (protmsg.arith == 8)
    {
      protmsg.flResult = protmsg.flValue1 / protmsg.flValue2;
      printf("fDiv %lf %lf\n Result was: %lf\n", protmsg.flValue1, protmsg.flValue2, protmsg.flResult);
    }
    protmsg.arith = htonl(protmsg.arith);
    protmsg.inResult = htonl(protmsg.inResult);
    protmsg.inValue1 = htonl(protmsg.inValue1);
    protmsg.inValue2 = htonl(protmsg.inValue2);
    protmsg.id = htonl(protmsg.id);
    protmsg.major_version = htons(protmsg.major_version);
    protmsg.minor_version = htons(protmsg.minor_version);
    protmsg.type = htons(protmsg.type);

    bytes = -1;
    tries = 0;
    while (bytes < 0 && tries < 3)
    {
      if ((sentbytes = sendto(sockfd, &protmsg, sizeof(protmsg), 0, p->ai_addr, p->ai_addrlen)) == -1)
      {
        printf("Failed to send via sento function. \n");
        exit(0);
      }
      bytes = recvfrom(sockfd, message, sizeof(*message), 0, (struct sockaddr *)&servaddr, &addr_len);
      if (bytes == -1 && tries < 2)
      {
        printf("Server did not respond... Trying again.\n");
      }
      tries++;
    }
    if (bytes == -1)
    {
      printf("Failed to recive message or server did not respond. Exiting...\n");
      delete message;
      exit(0);
    }
    message->message = ntohl(message->message);
    if (message->message == 1)
    {
      printf("OK\n");
    }
    else if (message->message == 2)
    {
      printf("NOT OK\n");
    }
    else
    {
      printf("N/A \n");
    }
  }
  else
  {
    printf("I don't know what I recived.\n");
  }
  struct sockaddr_in local;
  socklen_t addrlenght = sizeof(addrlenght);
  getsockname(sockfd, (struct sockaddr *)&local, &addrlenght);
#ifdef DEBUG
  printf("Local address:%s Port:%d\nHost:%s Port:%d\n", (inet_ntoa)(local.sin_addr), (int)ntohs(local.sin_port), Desthost, port);
#endif
  close(sockfd);
  delete message;
  return 0;
}
