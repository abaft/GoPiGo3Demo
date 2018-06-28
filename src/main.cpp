#include "libs/GoPiGo3.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <atomic>
#include <chrono>
extern "C" {
#include "libs/tof.h" // time of flight sensor library
}

#include "distance.h"
#include "bluetooth.h"
#include "udp.h"

#ifndef BTCONNECT
#define BTCONNECT connect_BT_server
#endif


GoPiGo3 gpg;
using namespace std;

typedef struct
{
  atomic<int> distance, right_M, left_M, ext_distance;
  atomic<bool> overr;
  atomic<double> bt_ping, udp_ping;
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

void* bluetooth_loop_r(void* vstate)
{

  gpg_state* state = (gpg_state*) vstate;
  while(1)
  {
    int ext_distance;
    bluetooth_read(state->bluetooth, &ext_distance);

    state->ext_distance = ext_distance;
  }
}

void* bluetooth_loop_w(void* vstate)
{

  gpg_state* state = (gpg_state*) vstate;

  char stop = 0;
  while(1)
  {
    if (state->distance < 100)
    {
      bluetooth_write(state->bluetooth, state->distance);
      stop = 1;
    }
    else if (stop && state->distance >= 100)
    {
      bluetooth_write(state->bluetooth, state->distance);
      stop--;
    }
    timespec sleep = {0, 70000000};
    nanosleep(&sleep, NULL);
  }
}

void* motor_loop(void* vstate)
{

  gpg_state* state = (gpg_state*) vstate;
  while(1)
  {
    if ((state->distance < 100 && state->distance != -1 || state->ext_distance < 100 && state->ext_distance != -1) &&
        !state->overr)
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
    bool overr;
    chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();

    poll_server(state->udp, &left_M, &right_M, state->distance, state->bt_ping, state->udp_ping, &overr);

    chrono::high_resolution_clock::time_point stop = chrono::high_resolution_clock::now();
    auto udp_ping = chrono::duration_cast<chrono::duration<double>>(stop - start);

    state->udp_ping = udp_ping.count();

    state->left_M = left_M;
    state->right_M = right_M;
    state->overr = overr;
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

  pthread_t threads[5];
  pthread_create(threads + 0, NULL, dist_loop, &state);
  pthread_create(threads + 1, NULL, network_loop, &state);
  pthread_create(threads + 2, NULL, motor_loop, &state);
  pthread_create(threads + 3, NULL, bluetooth_loop_r, &state);
  pthread_create(threads + 4, NULL, bluetooth_loop_w, &state);

  printf("Started all modules");

  while(1);

  return 0;
}
