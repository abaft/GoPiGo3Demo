#ifndef _BLUETOOTH_H
#define _BLUETOOTH_H

typedef struct
{
	int socket;
	char order;
}bluetooth_conn;

bluetooth_conn connect_BT_client();
int bluetooth_poll(bluetooth_conn conn, int distance, int* ext_distance);

#endif
