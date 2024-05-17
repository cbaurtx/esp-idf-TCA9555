### rot_btn_i2c

rot_btn_i2c is a ESP32 IDF component to read and decode a mechanical rotary encoder and up to 14 buttons connected to the ESP32 by a TCA9555 port expander. It uses the RiskV ULP of the ESP32-S3 and minimizes the load for the main processor cores. Note that the ULP code uses bit banging as the RTC I2C controller cannot read 16bits at once slowing down data transmission significantly. Configuration is by menuconfig (execute idf.py menuconfig). Navigate to the Component config entry "Rotary decoder & buttons i2c".
Check the documentation directory for a schematic on connecting the TCA9555 port expander. Compilation checked with IDF version 5.1
