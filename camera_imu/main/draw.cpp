#include "draw.h"

#include "dl_image.hpp"
#include "esp_camera.h"
#include "imu_meas.h"

static QueueHandle_t xQueueFrameI = NULL;
static QueueHandle_t xQueueFrameO = NULL;
static QueueHandle_t xQueueImuMeasI = NULL;

static const char *TAG = "draw_task";

static void mirror_task_process_handler(void *arg) {
  camera_fb_t *frame = NULL;
  ImuMeas *imu_meas = NULL;
  float acc_x, acc_y, acc_z;
  while (true) {
    if (xQueueReceive(xQueueFrameI, &frame, portMAX_DELAY)) {

      if (xQueueFrameO) {
        // Draw something on the frame

        dl::image::draw_filled_rectangle(frame->buf, frame->height,
                                         frame->width, 20, 20, 40, 40,
                                         0b0000000011111000);

        xQueueSend(xQueueFrameO, &frame, portMAX_DELAY);
      } else {
        free(frame);
      }
    }
    if (xQueueImuMeasI) {
      if (xQueueReceive(xQueueImuMeasI, &imu_meas, portMAX_DELAY)) {
        if (imu_meas) {
          acc_x = imu_meas->acc_x;
          acc_y = imu_meas->acc_y;
          acc_z = imu_meas->acc_z;
          ESP_LOGI(TAG, "imu_meas in draw: %f %f %f", acc_x, acc_y, acc_z);
        }
      }
    }
  }
}

void registerMirroringCamera(const QueueHandle_t frame_i,
                             const QueueHandle_t frame_o,
                             const QueueHandle_t imu_meas_i) {
  xQueueFrameI = frame_i;
  xQueueFrameO = frame_o;
  xQueueImuMeasI = imu_meas_i;

  xTaskCreatePinnedToCore(mirror_task_process_handler, "gox_mirror_camera",
                          4 * 1024, NULL, 5, NULL, 0);
}