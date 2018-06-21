#ifndef _BLUETOOTH_H
#define _BLUETOOTH_H

typedef struct
{
	int socket;
	char order;
}bluetooth_conn;

bluetooth_conn connect_BT_client();
bluetooth_conn connect_BT_server();
int bluetooth_read(bluetooth_conn conn, int* ext_distance);
int bluetooth_write(bluetooth_conn conn, int distance);

#endif
