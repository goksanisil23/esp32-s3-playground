#pragma once

#include "sys/time.h"

typedef struct {
  float acc_x;
  float acc_y;
  float acc_z;
  struct timeval timestamp;
} ImuMeas;