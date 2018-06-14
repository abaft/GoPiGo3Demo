#include "libs/GoPiGo3.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <atomic>
extern "C" {
#include "libs/tof.h" // time of flight sensor library
}

#include "distance.h"
#include "bluetooth.h"
#include "udp.h"
#ifndef BTCONNECT
#define BTCONNECT connect_BT_client
#endif


GoPiGo3 gpg;
using namespace std;

typedef struct
{
  atomic<int> distance, right_M, left_M, ext_distance;
  UDP_conn udp;
  bluetooth_conn bluetooth;
}gpg_state;

void* dist_loop(void* vstate)
{
  gpg_state* state = (gpg_state*) vstate;
  while(1)
  {
    state->distance = dist_poll();
  }
}

void* bluetooth_loop(void* vstate)
{
  gpg_state* state = (gpg_state*) vstate;
  while(1)
  {
    int ext_distance;
    if (bluetooth_poll(state->bluetooth, state->distance, &ext_distance))
    {
      state->bluetooth = BTCONNECT();
      continue;
    }

    state->ext_distance = ext_distance;
  }
}

void* motor_loop(void* vstate)
{

  gpg_state* state = (gpg_state*) vstate;
  while(1)
  {
    if (state->distance < 100 && state->distance != -1 || state->ext_distance < 100 && state->ext_distance != -1)
    {
      gpg.set_motor_power(MOTOR_LEFT, 0);
      gpg.set_motor_power(MOTOR_RIGHT, 0);
    }
    else
    {
      gpg.set_motor_power(MOTOR_LEFT, state->left_M);
      gpg.set_motor_power(MOTOR_RIGHT, state->right_M);
    }
    timespec sleep = {0, 10000000};
    nanosleep(&sleep, NULL);
  }
}

void* network_loop(void* vstate)
{
  gpg_state* state = (gpg_state*) vstate;
  while(1)
  {
    int left_M, right_M;
    poll_server(state->udp, &left_M, &right_M, state->distance);
    state->left_M = left_M;
    state->right_M = right_M;
    timespec sleep = {0, 100000000};
    nanosleep(&sleep, NULL);
  }
}

char* read_config()
{
  char * buffer = 0;
  long length;
  FILE * f = fopen (SYSCONFDIR, "rb");

  if (f)
  {
    fseek (f, 0, SEEK_END);
    length = ftell (f);
    fseek (f, 0, SEEK_SET);
    buffer = (char*) malloc (length);
    if (buffer)
    {
      fread (buffer, 1, length, f);
    }
    fclose (f);
  }
  return buffer;
}


int main(int argc, char const *argv[])
{
  gpg_state state;

  if (dist_init())
  {
    printf("Failed to connect to i2c tof\n");
    return -1;
  }

  state.udp = UDP_connection(read_config());
  state.bluetooth = BTCONNECT();

  pthread_t threads[4];
  pthread_create(threads + 0, NULL, dist_loop, &state);
  pthread_create(threads + 1, NULL, network_loop, &state);
  pthread_create(threads + 2, NULL, motor_loop, &state);
  pthread_create(threads + 3, NULL, bluetooth_loop, &state);

  printf("Started all modules");

  while(1);

  return 0;
}
