/**
 * @file rot_btn_i2c_riscv.h
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

#ifndef ROT_BIN_I2C_RISCV_H
#define ROT_BIN_I2C_RISCV_H

#include "esp_err.h"

#define BTN_PUSHED 0x01
#define BTN_RELEASED 0x02
#define ENC 0x04

void rot_btn_i2c_init(void);

esp_err_t rot_btn_i2c_read(uint32_t *key_code_p, int timeout);

#endif
