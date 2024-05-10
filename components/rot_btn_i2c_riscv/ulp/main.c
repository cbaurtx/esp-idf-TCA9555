#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include "ulp_riscv.h"
#include "ulp_riscv_utils.h"
#include "ulp_riscv_gpio.h"

#include "rot_btn_i2c_riscv.h"
#include "sdkconfig.h"

#define SDA_HIGH (ulp_riscv_gpio_output_level(CONFIG_I2C_SDA_RTCIO, 1))
#define SDA_LOW (ulp_riscv_gpio_output_level(CONFIG_I2C_SDA_RTCIO, 0))
#define SDA_READ (ulp_riscv_gpio_get_level(CONFIG_I2C_SDA_RTCIO))
#define SCL_HIGH (ulp_riscv_gpio_output_level(CONFIG_I2C_SCL_RTCIO, 1))
#define SCL_LOW (ulp_riscv_gpio_output_level(CONFIG_I2C_SCL_RTCIO, 0))
#define SCL_READ (ulp_riscv_gpio_get_level(CONFIG_I2C_SCL_RTCIO))
#define I2C_NACK    1
#define I2C_ACK     0

/* Refer to NXP I2C user guide UM10204 */
#if CONFIG_I2C_FAST
const float THD_STA = 0.6;  // START hold time
const float TSU_STA = 0.6;  // (RE)START setup time
const float THD_STO = 0.6;  // STOP hold time
const float THD_DAT = 0.0;  // data hold time
const float TSU_DAT = 0.25; // data setup time
const float TVD_ACK = 0.9;  // acknowledge  valid acknowledge time
const float TVD_DAT = 0.9;  // data valid acknowledge time
const float T_HIGH  = 0.6;  // clk high time
const float T_LOW   = 1.3;  // clk low time
const float T_BUF   = 1.3;  // bus free time
#endif

#if CONFIG_I2C_STANDARD
const float THD_STA = 4.0;   // START hold time
const float TSU_STA = 4.7;   // (RE)START setup time
const float THD_STO = 4.0;   // STOP hold time
const float THD_DAT = 5.0;   // data hold time
const float TSU_DAT = 0.25;  // data setup time
const float TVD_ACK = 3.45;  // acknowledge valid acknowledge time
const float TVD_DAT = 3.45;  // data valid acknowledge time
const float T_HIGH  = 4.0;   // clk high time
const float T_LOW   = 4.7;   // clk low time
const float T_BUF   = 4.7;   // bus free time
#endif

#define I2C_OK 0
#define I2C_ERR_ACK 1
#define I2C_ERR_ARB_CLAIM 2
#define I2C_ERR_ARB_TX 4

/* TCA9555 registers */
#define INPUT_PORT0 0x00
#define INPUT_PORT1 0x01
#define OUTPUT_PORT0 0x02
#define OUTPUT_PORT1 0x03
#define INV_PORT0 0x04
#define INV_PORT1 0x05
#define CONF_PORT0 0x06
#define CONF_PORT1 0x07

#define DELAY(wait_us) ulp_riscv_delay_cycles((wait_us)*ULP_RISCV_CYCLES_PER_US)

uint32_t i2c_err = 0;          // Set to OK
uint32_t count_m = 0x20000;
uint32_t data = 0;
uint32_t evt = 0;


/* Claim I2C bus; need to add wait and timeout */
/* SDA and SCL high on return */
void i2c_claim(void)
{
  uint16_t i2c_arb_timeout;

  SDA_HIGH;
  SCL_HIGH;
  DELAY(T_BUF);
  for (i2c_arb_timeout = CONFIG_I2C_ARB_TIMEOUT; i2c_arb_timeout > 0;
       i2c_arb_timeout--) {
    if ((SDA_READ == 1) && (SCL_READ == 1))
      break;
    DELAY(1.0);
    }
    if ((SDA_READ != 1) || (SCL_READ != 1))
      i2c_err |= I2C_ERR_ARB_CLAIM;
}


/* I2C START condition, SCL and SDA must be already high */
/* SDA and SCL low on return */
void i2c_start(void)
{
  DELAY(TSU_STA);
  SDA_LOW;
  DELAY(THD_STA);
  SCL_LOW;
}


/* I2C RESTART condition */
/* SDA and SCL high on return */
void i2c_restart(void)
{
  SCL_HIGH;
  SDA_HIGH;
  DELAY(TSU_STA);
  SDA_LOW;
  DELAY(THD_STA);
  SCL_LOW;
}


/* I2C STOP condition, SDA and SCL must be low already */
/* SDA and SCL high on return */
void i2c_stop(void)
{
  SCL_HIGH;
  while (SCL_READ == 0);  // Clock stretching
  DELAY(THD_STO);
  SDA_HIGH;
  DELAY(T_BUF);
}


/* Transmit 8 bit data to slave and read ack/nack; SCL must be already low */
/* SDA and SCL low on return */
int i2c_tx_8(uint8_t dat)
{
  uint8_t count;
  uint8_t nack;

  SCL_LOW;
  DELAY(T_LOW);
  SDA_LOW;
  for (count = 8; count > 0; count--) {
    // DELAY(CLOCK_LOW_HALF);
    (dat & 0x80) ? SDA_HIGH : SDA_LOW;
    DELAY(TSU_DAT);
    SCL_HIGH;
    if ((dat & 0x80) && (!SDA_READ)) // arbitration error
       i2c_err |= I2C_ERR_ARB_TX;
    DELAY(T_HIGH);
    while (SCL_READ == 0);  // allow for clock stretching
    SCL_LOW;
    dat <<= 1;
    DELAY(THD_DAT);
    /* SDA low or high; SCL low at end of loop */
  }

  SDA_HIGH;                 // so slave can change SDA
  SCL_HIGH;
  DELAY(TVD_ACK);
  while (SCL_READ == 0);  // allow for clock stretching

  nack = SDA_READ;          // read acknowledge bit
  if (nack)
    i2c_err |= I2C_ERR_ACK;
  DELAY(T_HIGH);
  SCL_LOW;
  SDA_LOW;

  return nack;
}


/* Read 8 bits from slave and transmit ack / nack */
/* SDA and SCL low on return */
int i2c_rx_8(uint8_t nack)
{
  uint8_t count;
  uint8_t rx_data;

  SCL_LOW;
  DELAY(T_LOW);
  rx_data = 0x00;
  SDA_HIGH;                 // so slave can change SDA

  for (count = 8; count > 0; count--) {
    rx_data <<= 1;
    SCL_HIGH;
    DELAY(TVD_DAT);
    rx_data |= SDA_READ;
    while (SCL_READ == 0);  // allow for clock stretching
    SCL_LOW;
    DELAY(T_LOW);
    /* SDA low or high; SCL low at end of loop */
  }
  /* send ACK / NACK */
  nack? SDA_HIGH : SDA_LOW;
  DELAY(TSU_DAT);
  SCL_HIGH;
  DELAY(T_HIGH);
  while (SCL_READ == 0);  // allow for clock stretching
  SCL_LOW;
  DELAY(T_LOW);
  SDA_LOW;
  return(rx_data);
}


int i2c_write_slave_register(uint8_t slave_address, uint8_t reg_address,
                             uint8_t data)
{
  i2c_claim();
  i2c_start();
  i2c_tx_8(slave_address << 1);
  i2c_tx_8(reg_address);
  i2c_tx_8(data);
  i2c_stop();
}


int i2c_read_slave_register(uint8_t slave_address, uint8_t reg_address)
{
  uint8_t data;

  i2c_claim();
  i2c_start();
  i2c_tx_8(slave_address << 1);
  i2c_tx_8(reg_address);
  i2c_restart();
  i2c_tx_8((slave_address << 1) | 0x01);
  data = i2c_rx_8(I2C_NACK);
  i2c_stop();
  return(data);
}


int i2c_read_2_slave_registers(uint8_t slave_address, uint8_t reg_address)
{
  uint16_t data;

  i2c_claim();
  i2c_start();
  i2c_tx_8(slave_address << 1);
  i2c_tx_8(reg_address);
  i2c_restart();
  i2c_tx_8((slave_address << 1) | 0x01);
  data = i2c_rx_8(I2C_ACK);
  data |= i2c_rx_8(I2C_NACK) << 8;
  i2c_stop();
  return(data);
}


int main(void)
{
  uint32_t prev_data = 0;
  bool was_pushed = false;

  /* invert all inputs as contacts are connected to ground */
  i2c_write_slave_register(CONFIG_I2C_SLAVE_ADR, 0x04, 0xff);
  i2c_write_slave_register(CONFIG_I2C_SLAVE_ADR, 0x05, 0xff);

  for(;;) {
    data = i2c_read_2_slave_registers(CONFIG_I2C_SLAVE_ADR, INPUT_PORT0);
    if (data != prev_data) {
      #ifdef CONFIG_ENABLE_TEST_POINT_OUT
      ulp_riscv_gpio_output_level(CONFIG_TEST_POINT_RTCIO,
                !ulp_riscv_gpio_get_level(CONFIG_TEST_POINT_RTCIO));
      #endif
      if (data == 0x03) {        // AB encoder; transition to detend
        if ((prev_data & 0x03) == 0x01) {
          count_m++;
          evt = ENC;
          ulp_riscv_wakeup_main_processor();
        }
        if ((prev_data & 0x03) == 0x02) {
          count_m--;
          evt = ENC;
          ulp_riscv_wakeup_main_processor();
        }
      }
      // a button changed?
      if ((data & 0xfffc) || (was_pushed && ((data & 0xfffc) == 0)))  {
        DELAY(CONFIG_WAIT_DEBOUNCE_SET);
        if (was_pushed) {
          if (0 == (i2c_read_2_slave_registers(CONFIG_I2C_SLAVE_ADR,
              INPUT_PORT0) & 0xfc)) {
            evt = BTN_RELEASED;
            was_pushed = false;
            ulp_riscv_wakeup_main_processor();
          };
        } else {
          if ((data & 0xfc) == (i2c_read_2_slave_registers(CONFIG_I2C_SLAVE_ADR,
                                INPUT_PORT0) & 0xfc)) {
            evt = BTN_PUSHED;
            was_pushed = true;
            ulp_riscv_wakeup_main_processor();
          };
        }
     }
     prev_data = data;
  }
  }
}
