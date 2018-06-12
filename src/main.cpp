#include "libs/GoPiGo3.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
extern "C" {
#include "libs/tof.h" // time of flight sensor library
}

#include "distance.h"
#include "bluetooth.h"
#include "udp.h"


GoPiGo3 gpg;
using namespace std;

void* dist_loop(int* arg)
{
  while(1)
	  *arg = dist_poll();
}
void* bluetooth_loop(int* arg)
{
	bluetooth_poll(, int* args)
}

int main(int argc, char const *argv[])
{
	int distance, right_M, left_M, ext_distance;

	if (dist_init())
	{
		printf("Failed to connect to i2c tof")
		return -1;
	}

	UDPConn sock = UDP_connection(argv[1]);

	long long unsigned int packetsSent = 0;

	pthread_t bltooth;
	pthread_create(&bltooth, NULL, bluetoothConn, NULL);

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
