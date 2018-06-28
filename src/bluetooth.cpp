#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include "bluetooth.h"

using namespace std;

bluetooth_conn connect_BT_client()
{
  int s;
  struct sockaddr_rc addr = { 0 };

  s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
  addr.rc_family = AF_BLUETOOTH;
  addr.rc_channel = (uint8_t) 1;
  str2ba( CAR0ADDR, &addr.rc_bdaddr);

  while(connect(s, (struct sockaddr *) &addr, sizeof(addr)))
  {
    printf("Failed to connect to Bluetooth server\n");
    close(s);
    s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
  }

  printf("Connecting to Bluetooth as Client\n");
  struct timeval timeout; //set timeout for 1 seconds
  timeout.tv_sec = 30;
  timeout.tv_usec = 0;

  setsockopt(s,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));
  return {s, 1};
}

bluetooth_conn connect_BT_server()
{
  struct sockaddr_rc addr = { 0 }, rem_addr = { 0 };
  int s;

  s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
  addr.rc_family = AF_BLUETOOTH;
  addr.rc_channel = (uint8_t) 1;
  //addr.rc_bdaddr = *BDADDR_ANY;
  str2ba( CAR0ADDR, &addr.rc_bdaddr);

  bind(s, (struct sockaddr *) &addr, sizeof(addr));
  listen(s, 1);
  char buf[256] = { 0 };
  ba2str( &addr.rc_bdaddr, buf );
  fprintf(stdout, "local %s\n", buf);

  socklen_t opt = sizeof(rem_addr);
  s = accept(s, (struct sockaddr *)&rem_addr, &opt);

  struct timeval timeout; //set timeout for 1 seconds
  timeout.tv_sec = 0;
  timeout.tv_usec = 200000000;

  setsockopt(s,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));
 
  return {s, 0};
}

int bluetooth_read(bluetooth_conn conn, int* ext_distance)
{
  recv(conn.socket, ext_distance, sizeof(int), 0);
}

int bluetooth_write(bluetooth_conn conn, int distance)
{
    int err = write(conn.socket, &distance, sizeof(int));
    return err;
}
