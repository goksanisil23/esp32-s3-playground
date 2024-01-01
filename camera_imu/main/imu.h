#pragma once

#include "esp_event.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef __cplusplus
extern "C" {
#endif

void registerImu(const QueueHandle_t imu_meas_out);

#ifdef __cplusplus
}
#endif