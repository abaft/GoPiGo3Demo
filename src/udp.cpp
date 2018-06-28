#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/socket.h>

#include "udp.h"

#define PORT 3322

using namespace std;

UDP_conn UDP_connection(const char* ipaddress)
{
  struct sockaddr_in clientAddr;
  int sockfd, clientLen, i=0;

  /*Create Client socket*/
  if((sockfd=socket(AF_INET, SOCK_DGRAM, 0)) == -1)
  {
    perror("socket failed:");
    exit(0);
  }

  /* Clear Client structure */
  memset((char *)&clientAddr, 0, sizeof(clientAddr));

  clientAddr.sin_family = AF_INET; //For IPV4 internet protocol
  clientAddr.sin_port = htons(PORT); //Assign port number on which server listening	
  clientAddr.sin_addr.s_addr = inet_addr(ipaddress); //Assign server machine address
  return {sockfd, clientAddr};
}

int poll_server(UDP_conn con, int* left_pow, int* right_pow, int dist, double bt_ping, double udp_ping, bool* overr)
{
  char buffer[32];
  memset(buffer, '\0', 32);
  snprintf(buffer, 32, CARNAME ",%i,%.5lf", dist, udp_ping);
  sendto(con.sock, buffer, 32, 0, (struct sockaddr *) &con.addr, sizeof(con.addr));

  struct timeval timeout; //set timeout for 1 seconds
  timeout.tv_sec = 1;
  timeout.tv_usec = 0;

  setsockopt(con.sock,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));
  char message[32];
  memset(message, '\0', 32);
  socklen_t len = sizeof(con.addr);
  if( recvfrom(con.sock, message, 32, 0, (struct sockaddr *) &con.addr, &len) != -1)
  {
    //printf("%s\n",message );
    *left_pow = atoi(strtok(message, ","));
    *right_pow = atoi(strtok(NULL, ","));
    *overr = atoi(strtok(NULL, ",")) == 1;
  }
  else
  {
    printf("\nrecv failed\n");
    return -1;
  }
  return 0;
}
