extern "C" {
#include "libs/tof.h" // time of flight sensor library
}
#include <stdio.h>
#include "distance.h"

using namespace std;

int dist_init()
{
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

  return 0;
}

int dist_poll()
{
  int iDistance;

  iDistance = tofReadDistance();
  if (iDistance < 4096) // valid range?
    return iDistance;
  else
    return -1;
}
