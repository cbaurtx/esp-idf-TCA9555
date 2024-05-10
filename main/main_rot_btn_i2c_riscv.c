#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <inttypes.h>
#include <stdio.h>
#include "esp_log.h"

#include "rot_btn_i2c_riscv.h"
#define TAG "main_rot_btn_i2c_riscv.c"

void app_main(void) {
  esp_err_t err;
  int timeout = 1000;
  uint32_t key_code;

  rot_btn_i2c_init();
  for (;;) {
    err = rot_btn_i2c_read(&key_code, timeout);

    ESP_LOGI(TAG, "Read error %x", (int)err);
    ESP_LOGI(TAG, "Read kex_code %x", key_code);
    //  vTaskDelay(1);
  };

  ESP_LOGI(TAG, "End app_main");
}
