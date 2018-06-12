#ifndef _UDP_H
#define _UDP_H

typedef struct {
	int sock;
	struct sockaddr_in addr;
}UDPConn;

int getDist();

UDPConn UDP_connection(const char* ipaddress);
int poll_server(UDPConn con, int* left_pow, int* right_pow, int dist);

#endif
