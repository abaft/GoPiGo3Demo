#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include "bluetooth.h"

#define CAR0ADDR "B8:27:EB:6A:C6:A8"
using namespace std;

bluetooth_conn connect_BT_client()
{ int s;
	struct sockaddr_rc addr = { 0 };

	s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
	addr.rc_family = AF_BLUETOOTH;
	addr.rc_channel = (uint8_t) 1;
	str2ba( CAR0ADDR, &addr.rc_bdaddr);

	while(connect(s, (struct sockaddr *) &addr, sizeof(addr)))
		printf("Failed to connect to Bluetooth server\n");
	printf("Connected Bluetooth as Client\n");
        
	struct timeval timeout; //set timeout for 1 seconds
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

	setsockopt(s,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));

	return {s, 1};
}

int bluetooth_poll(bluetooth_conn conn, int* args)
{
	// int* localDistance = args;
	// int* externalDistance = args + 1;

	if (conn.order)
		write(conn.socket, args, sizeof(int));
	recv(conn.socket, args + 1, sizeof(int), 0);
	if (!conn.order)
		write(conn.socket, args, sizeof(int));
	return 0;
}
