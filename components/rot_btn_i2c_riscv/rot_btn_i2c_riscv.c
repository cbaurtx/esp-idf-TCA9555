/**
 * @file rot_btn_i2c_riscv.c
 * @version 0.1
 * @author Cbaurtx
 * @date 29.03.2024
 * @copyright GNU Public License GPL3
 * @brief ESP32 custom component to read a rotary encoder and buttons with an
 * I2C expander. Uses riscv ULP
 *
 * LICENSE
 * This file is part of rot_btn_i2c_riscv.
 *
 * rot_btn_i2c_riscv is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * rot_btn_i2c_riscv is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with CyclingGNS. If not, see <https://www.gnu.org/licenses/>.
 *
 * DESCRIPTION
 * ESP32 custom component to read a rotary encoder and buttons with an I2C
 * expander. Uses riscv ULP. For now only works with TCA9555 expander. Uses bit
 * banging i2c.
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
    ESP_LOGD(TAG, "ULP data = 0x%x", ulp_data);
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

  // ESP_ERROR_CHECK(ulp_riscv_config_and_run());

  ESP_ERROR_CHECK(ulp_riscv_run());
  // SET_PERI_REG_MASK(RTC_CNTL_COCPU_CTRL_REG, RTC_CNTL_COCPU_CLKGATE_EN);
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
                     (ulp_count_m & 0x3ffff) | ((ulp_data & 0xfffc) << 16),
                     eSetValueWithoutOverwrite, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR();
}