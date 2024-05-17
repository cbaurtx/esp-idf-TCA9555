/**
 * @file rot_btn_i2c_riscv.h
 * @version 0.1
 * @author  Cbaurtx
 * @date    17.05.2024
 * @copyright MIT License
 * @brief Include file of rot_btn_i2c ESP32-S3 component
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


#ifndef ROT_BIN_I2C_RISCV_H
#define ROT_BIN_I2C_RISCV_H

#include "esp_err.h"

#define BTN_PUSHED 0x01
#define BTN_RELEASED 0x02
#define ENC 0x04

void rot_btn_i2c_init(void);

esp_err_t rot_btn_i2c_read(uint32_t *key_code_p, int timeout);

#endif
