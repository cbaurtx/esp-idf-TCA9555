/**
 * @file main_rot_btn_i2c_riscv.c
 * @version 0.1
 * @author  Cbaurtx
 * @date    17.05.2024
 * @copyright MIT License
 * @brief C test application (main) of rot_btn_i2c
 *
 * LICENSE
 * This file is part of rot_btn_i2c.
 *
 * rot_btn_i2c is free software: you can redistribute it and/or modify it
 * under the terms of the MIT License. Check file LICENSE for details
 *
 * rot_btn_i2c is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * DESCRIPTION
 * Test application for ESP32-S3 component to read a rotary encoder and 14 buttons.
 */


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
