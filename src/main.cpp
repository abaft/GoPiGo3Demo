#include "libs/GoPiGo3.h"
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
extern "C" {
#include "libs/tof.h" // time of flight sensor library
}
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#define PORT 3322

#define CAR0ADDR "B8:27:EB:6A:C6:A8"



GoPiGo3 gpg;
using namespace std;
long long unsigned int packetsSent;
char BTLock;
int Dist;

typedef struct {
	int sock;
	struct sockaddr_in addr;
}UDPConn;

int getDist()
{
	int iDistance;

	iDistance = tofReadDistance();
	if (iDistance < 4096) // valid range?
		return iDistance;
	else
		return -1;

} 

UDPConn UDPConnection(const char* ipaddress)
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

int pollServer(UDPConn con, int* left_pow, int* right_pow, int dist)
{
	char buffer[32];
	memset(buffer, '\0', 32);
	snprintf(buffer, 32, "%i,%llu", dist, packetsSent);
	sendto(con.sock, buffer, 32, 0, (struct sockaddr *) &con.addr, sizeof(con.addr));

	struct timeval timeout; //set timeout for 2 seconds
	timeout.tv_sec = 0;
	timeout.tv_usec = 20000000;

	setsockopt(con.sock,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));
	char message[32];
	memset(message, '\0', 32);
	size_t len = sizeof(con.addr);
	if( recvfrom(con.sock, message, 32, 0, (struct sockaddr *) &con.addr, &len) != -1)
	{
		printf("%s\n",message );
		*left_pow = atoi(strtok(message, ","));
		*right_pow = atoi(strtok(NULL, ","));
	}
	else
	{
		printf("\nrecv failed\n");
		return -1;
	}
	return 0;
}

int connectBT(int* s)
{
	struct sockaddr_rc addr = { 0 };

	*s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
	addr.rc_family = AF_BLUETOOTH;
	addr.rc_channel = (uint8_t) 1;
	str2ba( CAR0ADDR, &addr.rc_bdaddr);

	return connect(*s, (struct sockaddr *) &addr, sizeof(addr));
}

void* bluetoothConn(void* arg)
{
	int sock;
	while (connectBT(&sock))
	{
		printf("Failed to connect\n");
	}
	printf("Connected Bluetooth as Client\n");
	struct timespec req = {0, 900000000};
	nanosleep(&req, NULL);

        struct timeval timeout; //set timeout for 2 seconds
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

	setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));
	while (1){
	write(sock, &Dist, sizeof(Dist));
	int extDist;
	recv(sock, &extDist, sizeof(extDist), 0);
	if ((Dist < 100 && Dist != -1)  || (extDist < 100 && extDist != -1))
		BTLock = 1;
	else
		BTLock = 0;
	}
}

void* distLoop(void* arg)
{
while(1)
  Dist = getDist();
}

int main(int argc, char const *argv[])
{
	BTLock = 0;

	int err;
	err = tofInit(1, 0x29, 1); // set long range mode (up to 2m)
	if (err != 1)
	{
		return -1; // problem - quit
	}
	printf("VL53L0X device successfully opened.\n");

	int  model, revision;
	tofGetModel(&model, &revision);
	printf("Model ID - %d\n", model);
	printf("Revision ID - %d\n", revision);

	UDPConn sock = UDPConnection(argv[1]);

	packetsSent = 0;
	pthread_t bltooth;
	pthread_create(&bltooth, NULL, bluetoothConn, NULL);

	pthread_t dist_loop;
	pthread_create(&dist_loop, NULL, distLoop, NULL);
	while (1)
	{
		int r, l;
		if (pollServer(sock, &r, &l, Dist) == -1) continue;

		if (!BTLock)
		{
		  gpg.set_motor_power(MOTOR_LEFT, l);
		  gpg.set_motor_power(MOTOR_RIGHT, r);
		}
		else
		{
		  gpg.set_motor_power(MOTOR_LEFT, 0);
		  gpg.set_motor_power(MOTOR_RIGHT, 0);
		  printf("LOCK");
		}
		struct timespec req = {0, 90000000};
		nanosleep(&req, NULL);
		++packetsSent;
	}
	return 0;
}
