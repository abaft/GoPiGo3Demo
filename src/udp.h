#ifndef _UDP_H
#define _UDP_H
#include <arpa/inet.h>

typedef struct {
	int sock;
	struct sockaddr_in addr;
}UDP_conn;

UDP_conn UDP_connection(const char* ipaddress);
int poll_server(UDP_conn con, int* left_pow, int* right_pow, int dist, double bt_ping, double udp_ping, bool* overr);

#endif
