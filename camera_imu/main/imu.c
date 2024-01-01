#include "imu.h"

#include "imu_meas.h"

#include "freertos/FreeRTOS.h"
#include "qma7981.h"
#include "esp_log.h"
#include "freertos/task.h"

static QueueHandle_t xQueueImuOut = NULL;

static void imu_task_process_handler(void *arg)
{
  ImuMeas imu_meas;
  while (1)
  {
    esp_err_t ret_val = qma7981_get_acce(&imu_meas.acc_x, &imu_meas.acc_y, &imu_meas.acc_z);
    if ((ret_val == ESP_OK))
    {
      if (xQueueImuOut)
      {
        xQueueSend(xQueueImuOut, &imu_meas, portMAX_DELAY);
      }
    }
    vTaskDelay(10);
  }
}

void registerImu(const QueueHandle_t imu_meas_out)
{
  if (qma7981_init() != ESP_OK)
  {
    return;
  }

  xQueueImuOut = imu_meas_out;

  xTaskCreatePinnedToCore(imu_task_process_handler, "gox_imu", 4 * 1024, NULL,
                          5, NULL, 0);
}