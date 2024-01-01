#include "draw.h"
#include "imu.h"
#include "imu_meas.h"
#include "udp.h"
#include "who_camera.h"
#include "who_lcd.h"

#include "esp_event.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <string.h>
#include <sys/param.h>

static QueueHandle_t xQueueAIFrame = NULL;
static QueueHandle_t xQueueLCDFrame = NULL;
static QueueHandle_t xQueueImuMeas = NULL;

extern "C" void app_main() {
  xQueueAIFrame = xQueueCreate(2, sizeof(camera_fb_t *));
  xQueueLCDFrame = xQueueCreate(2, sizeof(camera_fb_t *));
  xQueueImuMeas = xQueueCreate(2, sizeof(ImuMeas));

  register_camera(PIXFORMAT_RGB565, FRAMESIZE_240X240, 2, xQueueAIFrame);
  register_lcd(xQueueLCDFrame, NULL, true);
  registerImu(xQueueImuMeas);
  registerMirroringCamera(xQueueAIFrame, xQueueLCDFrame, xQueueImuMeas);

  // vTaskDelay(10000 / portTICK_PERIOD_MS);

  registerUdpClient();
}
