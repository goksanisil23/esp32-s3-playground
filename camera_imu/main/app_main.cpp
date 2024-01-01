#include "dl_image.hpp"
#include "qma7981.h"
#include "who_camera.h"
#include "who_lcd.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include <string.h>
#include <sys/param.h>

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

static QueueHandle_t xQueueAIFrame = NULL;
static QueueHandle_t xQueueLCDFrame = NULL;

static QueueHandle_t xQueueFrameI = NULL;
static QueueHandle_t xQueueFrameO = NULL;
static QueueHandle_t xQueueEvent = NULL;
static QueueHandle_t xQueueResult = NULL;

static bool kDraw{false};

// Below are obtained from a file generated via menuconfig when we set the IP in
// the GUI
#define HOST_IP_ADDR "192.168.1.229"
#define PORT 3333

static void mirror_task_process_handler(void *arg) {
  camera_fb_t *frame = NULL;
  while (true) {
    if (xQueueReceive(xQueueFrameI, &frame, portMAX_DELAY)) {

      if (xQueueFrameO) {
        // Draw something on the frame
        if (kDraw) {
          dl::image::draw_filled_rectangle(frame->buf, frame->height,
                                           frame->width, 20, 20, 40, 40,
                                           0b0000000011111000);
        }
        xQueueSend(xQueueFrameO, &frame, portMAX_DELAY);
      } else {
        free(frame);
      }
    }
  }
}

static void imu_task_process_handler(void *arg) {
  float x{0.F};
  float y{0.F};
  float z{0.F};
  while (true) {
    auto ret_val = qma7981_get_acce(&x, &y, &z);
    if ((ret_val == ESP_OK)) {
      ESP_LOGI("imu_result", "%f %f %f", x, y, z);
      kDraw = !kDraw;
    }
    vTaskDelay(10);
  }
}

void registerImu() {
  if (qma7981_init() != ESP_OK) {
    return;
  }

  xTaskCreatePinnedToCore(imu_task_process_handler, "gox_imu", 4 * 1024, NULL,
                          5, NULL, 0);
}

void registerMirroringCamera(const QueueHandle_t frame_i,
                             const QueueHandle_t frame_o) {
  xQueueFrameI = frame_i;
  xQueueFrameO = frame_o;
  xQueueEvent = NULL;
  xQueueResult = NULL;

  xTaskCreatePinnedToCore(mirror_task_process_handler, "gox_mirror_camera",
                          4 * 1024, NULL, 5, NULL, 0);
}

static void udp_client_task(void *pvParameters) {
  char rx_buffer[128];
  char host_ip[] = HOST_IP_ADDR;
  int addr_family = 0;
  int ip_protocol = 0;

  const char *TAG = "example";
  const char *payload = "Message from ESP32 ";

  while (1) {

    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT);
    addr_family = AF_INET;
    ip_protocol = IPPROTO_IP;

    int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
    if (sock < 0) {
      ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
      break;
    }

    // Set timeout
    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);

    ESP_LOGI(TAG, "Socket created, sending to %s:%d", HOST_IP_ADDR, PORT);

    while (1) {

      int err = sendto(sock, payload, strlen(payload), 0,
                       (struct sockaddr *)&dest_addr, sizeof(dest_addr));
      if (err < 0) {
        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
        break;
      }
      ESP_LOGI(TAG, "Message sent");

      // sending message
      // {
      //   struct sockaddr_storage
      //       source_addr; // Large enough for both IPv4 or IPv6
      //   socklen_t socklen = sizeof(source_addr);
      //   int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0,
      //                      (struct sockaddr *)&source_addr, &socklen);

      //   // Error occurred during receiving
      //   if (len < 0) {
      //     ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
      //     break;
      //   }
      //   // Data received
      //   else {
      //     rx_buffer[len] =
      //         0; // Null-terminate whatever we received and treat like a
      //         string
      //     ESP_LOGI(TAG, "Received %d bytes from %s:", len, host_ip);
      //     ESP_LOGI(TAG, "%s", rx_buffer);
      //     if (strncmp(rx_buffer, "OK: ", 4) == 0) {
      //       ESP_LOGI(TAG, "Received expected message, reconnecting");
      //       break;
      //     }
      //   }
      // }

      vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    if (sock != -1) {
      ESP_LOGE(TAG, "Shutting down socket and restarting...");
      shutdown(sock, 0);
      close(sock);
    }
  }
  vTaskDelete(NULL);
}

void registerUdpClient() {
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  ESP_ERROR_CHECK(example_connect());

  // xTaskCreate(udp_client_task, "udp_client", 4096, NULL, 5, NULL);
  xTaskCreatePinnedToCore(udp_client_task, "udp_client", 4096, NULL, 5, NULL,
                          1);
}

extern "C" void app_main() {
  xQueueAIFrame = xQueueCreate(2, sizeof(camera_fb_t *));
  xQueueLCDFrame = xQueueCreate(2, sizeof(camera_fb_t *));

  register_camera(PIXFORMAT_RGB565, FRAMESIZE_240X240, 2, xQueueAIFrame);
  register_lcd(xQueueLCDFrame, NULL, true);
  registerMirroringCamera(xQueueAIFrame, xQueueLCDFrame);
  registerImu();

  // vTaskDelay(10000 / portTICK_PERIOD_MS);

  registerUdpClient();
}
