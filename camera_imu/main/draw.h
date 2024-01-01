#pragma once

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_event.h"
#include "esp_log.h"
#include <stdint.h>

void registerMirroringCamera(const QueueHandle_t frame_i,
                             const QueueHandle_t frame_o,
                             const QueueHandle_t imu_meas_i);
