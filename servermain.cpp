#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

/* You will to add includes here */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <vector>
// Included to get the support library
#include "calcLib.h"

#include "protocol.h"

//using namespace std;
/* Needs to be global, to be rechable by callback and main */
int loopCount = 0;
int terminate = 0;
struct cli
{
  struct sockaddr_in adress;
  calcProtocol work;
  struct timeval tid;
};
std::vector<cli> clients;
/* Call back function, will be called when the SIGALRM is raised when the timer expires. */
void checkJobbList(int signum)
{
  // As anybody can call the handler, its good coding to check the signal number that called it.
  struct timeval t;
  printf("Let me be, I want to sleep.\n");
  gettimeofday(&t, NULL);
  for (size_t i = 0; i < clients.size(); i++)
  {
    if (t.tv_sec - clients.at(i).tid.tv_sec > 10)
    {
      clients.erase(clients.begin() + i);
      printf("Client slept...\n");
    }
  }
  if (loopCount > 20)
  {
    printf("I had enough.\n");
    terminate = 1;
  }

  return;
}

int main(int argc, char *argv[])
{

  /* Do more magic */
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
  sa.ai_family = AF_UNSPEC;
  sa.ai_socktype = SOCK_DGRAM;
  sa.ai_protocol = 17;
  int sockfd;
  int rv;
  struct timeval tv;
  tv.tv_sec = 2;
  tv.tv_usec = 0;
  if ((rv = getaddrinfo(Desthost, Destport, &sa, &si)) != 0)
  {
    fprintf(stderr, "getadrrinfo: %s\n", gai_strerror(rv));
  }
  struct sockaddr_in client;
  calcProtocol protmsg;
  calcMessage *message = new calcMessage{};
  socklen_t client_len;
  for (p = si; p != NULL; p = p->ai_next)
  {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
    {
      printf("Failed to creat socket.\n");
      continue;
    }
    if ((bind(sockfd, p->ai_addr, p->ai_addrlen)) != 0)
    {
      printf("Failed to bind.\n");
      close(sockfd);
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

  /* 
     Prepare to setup a reoccurring event every 10s. If it_interval, or it_value is omitted, it will be a single alarm 10s after it has been set. 
  */
  struct itimerval alarmTime;

  alarmTime.it_interval.tv_sec = 2;
  alarmTime.it_interval.tv_usec = 2;
  alarmTime.it_value.tv_sec = 2;
  alarmTime.it_value.tv_usec = 2;

  /* Regiter a callback function, associated with the SIGALRM signal, which will be raised when the alarm goes of */
  signal(SIGALRM, checkJobbList);
  setitimer(ITIMER_REAL, &alarmTime, NULL); // Start/register the alarm.
  int bytes = -1;
  bool messageChange = false;
  calcProtocol sendProt;
  client_len = sizeof(client);
  char kek[123];
  initCalcLib();
  bool clientFound = false;
  int clientNr = 0;
  int currentClient = -1;
  while (terminate == 0)
  {
    clientFound = false;
    printf("This is the main loop, %d time.\n", loopCount);
    bytes = recvfrom(sockfd, &protmsg, sizeof(protmsg), 0, (struct sockaddr *)&client, &client_len);
    if (bytes == -1)
    {
      printf("Client did not respond... Trying again.\n");
    }
    else
    {
      inet_ntop(client.sin_family, &client.sin_addr, kek, sizeof(kek));
      printf("%s\n", kek);
      printf("%d\n", client.sin_port);
    }
    if (sizeof(calcMessage) == bytes)
    {
      if (!messageChange)
      {
        delete message;
        messageChange = true;
      }
      message = (calcMessage *)&protmsg;

      for (size_t i = 0; i < clients.size() && !clientFound; i++)
      {
        if (clients.at(i).adress.sin_addr.s_addr == client.sin_addr.s_addr && clients.at(i).adress.sin_port == client.sin_port)
        {
          clientFound = true;
        }
      }
      if (clientFound == false)
      {
        message->message = ntohl(message->message);
        message->type = ntohs(message->type);
        message->major_version = ntohs(message->major_version);
        message->minor_version = ntohs(message->minor_version);
        message->protocol = ntohs(message->protocol);
        if (message->type == 22 && message->message == 0 && message->major_version == 1 && message->minor_version == 0 && message->protocol == 17)
        {
          sendProt.arith = randomInt() % 8 + 1;
          if (sendProt.arith > 4)
          {
            sendProt.flValue1 = randomFloat();
            sendProt.flValue2 = randomFloat();
            if (sendProt.arith == 5)
            {
              sendProt.flResult = sendProt.flValue1 + sendProt.flValue2;
            }
            else if (sendProt.arith == 6)
            {
              sendProt.flResult = sendProt.flValue1 - sendProt.flValue2;
            }
            else if (sendProt.arith == 7)
            {
              sendProt.flResult = sendProt.flValue1 * sendProt.flValue2;
            }
            else if (sendProt.arith == 8)
            {
              sendProt.flResult = sendProt.flValue1 / sendProt.flValue2;
            }
          }
          else
          {
            sendProt.inValue1 = randomInt();
            sendProt.inValue2 = randomInt();
            if (sendProt.arith == 1)
            {
              sendProt.inResult = sendProt.inValue1 + sendProt.inValue2;
            }
            else if (sendProt.arith == 2)
            {
              sendProt.inResult = sendProt.inValue1 - sendProt.inValue2;
            }
            else if (sendProt.arith == 3)
            {
              sendProt.inResult = sendProt.inValue1 * sendProt.inValue2;
            }
            else if (sendProt.arith == 4)
            {
              sendProt.inResult = sendProt.inValue1 / sendProt.inValue2;
            }
          }
          sendProt.id = clientNr;
          clientNr++;
          sendProt.type = 1;
          sendProt.major_version = 1;
          sendProt.minor_version = 0;
          sendProt.id = htonl(sendProt.id);
          sendProt.type = htons(sendProt.type);
          sendProt.major_version = htons(sendProt.major_version);
          sendProt.minor_version = htons(sendProt.minor_version);
          sendProt.inValue1 = htonl(sendProt.inValue1);
          sendProt.inValue2 = htonl(sendProt.inValue2);
          sendProt.arith = htonl(sendProt.arith);
          struct cli theClient;
          theClient.adress = client;
          gettimeofday(&theClient.tid, NULL);
          theClient.work = sendProt;
          clients.push_back(theClient);
        }
      }
      if (sendto(sockfd, &sendProt, sizeof(sendProt), 0, (struct sockaddr *)&client, client_len) == -1)
      {
        printf("Failed to send\n");
      }
    }
    else if (bytes == sizeof(calcProtocol))
    {
      for (size_t i = 0; i < clients.size() && !clientFound; i++)
      {
        if (clients.at(i).adress.sin_addr.s_addr == client.sin_addr.s_addr && clients.at(i).adress.sin_port == client.sin_port && ntohl(clients.at(i).work.id) == ntohl(protmsg.id))
        {
          clientFound = true;
          currentClient = i;
        }
      }
      if (clientFound == true)
      {
        clients.at(currentClient).work.arith = ntohl(clients.at(currentClient).work.arith);
        protmsg.inResult = ntohl(protmsg.inResult);
        message->major_version = 1;
        message->minor_version = 0;
        message->protocol = 17;
        message->type = 2;
        message->type = htons(message->type);
        message->major_version = htons(message->major_version);
        message->minor_version = htons(message->minor_version);
        message->protocol = htons(message->protocol);
        if (clients.at(currentClient).work.arith > 4)
        {
          if (clients.at(currentClient).work.flResult == protmsg.flResult)
          {
            message->message = 1;
          }
          else
          {
            message->message = 2;
          }
        }
        else
        {
          if (clients.at(currentClient).work.inResult == protmsg.inResult)
          {
            message->message = 1;
          }
          else
          {
            message->message = 2;
          }
        }
      }
      else if(clientFound == false)
      {
        message->message = 0;
      }
      message->message = htonl(message->message);
      if (sendto(sockfd, message, sizeof(*message), 0, (struct sockaddr *)&client, client_len) == -1)
      {
        printf("Failed to send\n");
      }
      else if(clientFound == true)
      {
        clients.erase(clients.begin() + currentClient);
      }
    }
    loopCount++;
  }
  close(sockfd);
  printf("done.\n");
  return (0);
}
