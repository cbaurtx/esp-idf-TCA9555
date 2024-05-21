/**
 * @file rot_btn_i2c_riscv.c
 * @version 0.9
 * @author  Cbaurtx
 * @date    19.05.2024
 * @copyright MIT License
 * @brief ESP32-S3 component of rot_btn_i2c
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
 * ESP32-S3 component to read a rotary encoder and 14 buttons.
 */


#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdbool.h>

#include "driver/rtc_io.h"
#include "esp_err.h"
#include "esp_log.h"
#include "ulp_riscv.h"

#include "ulp_main.h"

#include "rot_btn_i2c_riscv.h"
#include "sdkconfig.h"

static const char *TAG = "rot_btn_i2c_riscv.c";

extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t ulp_main_bin_end[] asm("_binary_ulp_main_bin_end");

/* shared with ISR */
static TaskHandle_t volatile recv_task;
static uint32_t volatile rot_btn_code;

static void init_ulp_program(void);
static void rot_btn_isr(void *);

void rot_btn_i2c_init(void) {
/* If test point is enabled configure the RTCIO as output */
#ifdef CONFIG_ENABLE_TEST_POINT_OUT
  ESP_ERROR_CHECK(rtc_gpio_init(CONFIG_TEST_POINT_RTCIO));
  ESP_ERROR_CHECK(rtc_gpio_set_direction(CONFIG_TEST_POINT_RTCIO,
                                         RTC_GPIO_MODE_INPUT_OUTPUT));
  ESP_ERROR_CHECK(rtc_gpio_pulldown_dis(CONFIG_TEST_POINT_RTCIO));
  ESP_ERROR_CHECK(rtc_gpio_pullup_dis(CONFIG_TEST_POINT_RTCIO));
#endif

  /* Configure SCL as open drain ourput with RTCIO read */
  ESP_ERROR_CHECK(rtc_gpio_init(CONFIG_I2C_SCL_RTCIO));
  ESP_ERROR_CHECK(rtc_gpio_set_direction(CONFIG_I2C_SCL_RTCIO,
                                         RTC_GPIO_MODE_INPUT_OUTPUT_OD));
  ESP_ERROR_CHECK(rtc_gpio_pullup_dis(CONFIG_I2C_SCL_RTCIO));

  /* Configure SDA as open drain ourput with RTCIO read */
  ESP_ERROR_CHECK(rtc_gpio_init(CONFIG_I2C_SDA_RTCIO));
  ESP_ERROR_CHECK(rtc_gpio_set_direction(CONFIG_I2C_SDA_RTCIO,
                                         RTC_GPIO_MODE_INPUT_OUTPUT_OD));
  ESP_ERROR_CHECK(rtc_gpio_pullup_dis(CONFIG_I2C_SDA_RTCIO));

  /* Configure expander INT signal as input */
  ESP_ERROR_CHECK(rtc_gpio_init(CONFIG_CHG_EXP_IO));
  ESP_ERROR_CHECK(
      rtc_gpio_set_direction(CONFIG_CHG_EXP_IO, RTC_GPIO_MODE_INPUT_ONLY));
  ESP_ERROR_CHECK(rtc_gpio_pullup_dis(CONFIG_CHG_EXP_IO));

  ulp_riscv_isr_register(rot_btn_isr, NULL, ULP_RISCV_SW_INT);

  /* init riscv ULP */
  init_ulp_program();
  recv_task = xTaskGetCurrentTaskHandle();
};

esp_err_t rot_btn_i2c_read(uint32_t *p_key_code, int timeout) {
  recv_task = xTaskGetCurrentTaskHandle();
  ulp_i2c_err = 0x00;

  if (xTaskNotifyWait(0x00, ULONG_MAX, p_key_code, timeout) == pdTRUE) {
    ESP_LOGD(TAG, "---------------");
    ESP_LOGD(TAG, "ULP i2c error = 0x%x", ulp_i2c_err);
    ESP_LOGD(TAG, "ULP btn_data = 0x%x", ulp_btn_data);
    ESP_LOGD(TAG, "ULP evt = 0x%x", ulp_evt);
    ESP_LOGD(TAG, "ULP count_m = %d", ulp_count_m);
  } else {
    ESP_LOGD(TAG, "Timeout\n");
  };
  return (ulp_i2c_err);
}

/* Init and run riscv ulp program*/
static void init_ulp_program(void) {
  ESP_ERROR_CHECK(ulp_riscv_load_binary(
      ulp_main_bin_start, (ulp_main_bin_end - ulp_main_bin_start)));
  ESP_LOGI(TAG, "Loaded riscv ULP");

  ESP_ERROR_CHECK(ulp_riscv_run());
  ESP_LOGI(TAG, "Started riscv ULP");
}


/**
 * ULP interrupt service routine. ULP interrupt triggered when ULP executes
 * 'wake' instruction and the Xtensa is not sleeping.
 */
static void IRAM_ATTR rot_btn_isr(void *arg)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  /* notify blocked task*/
  xHigherPriorityTaskWoken = pdFALSE;
  xTaskNotifyFromISR(recv_task,
                     (ulp_count_m & 0x3ffff) | ((ulp_btn_data) << 18),
                     eSetValueWithoutOverwrite, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR();
}
