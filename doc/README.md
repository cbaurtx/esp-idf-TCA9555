# rot_btn_i2c

rot_btn_i2c is an ESP32 IDF component to read and decode a mechanical rotary encoder and up to 14 buttons connected to the ESP32 by a TCA9555 port expander. It uses the RiskV ULP of the ESP32-S3 and minimizes the load for the main processor cores. Note that the ULP code uses bit banging as the ESP32 RTC I2C controller (accessable by the ULP) cannot read 16bits at once slowing down data transmission significantly due to overhead when reading 8 bit at a time.


If no rotary encoder is connected, connect two buttons to port 0.0 and port 0.1. Button 1 counts down, button 2 counts up.


Configuration is by means of menuconfig (execute idf.py menuconfig). Navigate to the Component config entry "Rotary decoder & buttons i2c".
Check the documentation directory for a schematic showing how to connect the TCA9555 expander.


Compilation checked with IDF version 5.1.4


# Hardware Connections
Depend on the configuration. Only 3.3V, GND, SDA and SCL need to be connected to the
ESP32-S3. Check the schematic for details.
![Schematic](./rot_btn.kicad_sch.png?raw=true "TCA955 port expander connections")


# Configure the Project
Execute: idf.py menuconfig and navigate to Components => Rotary decoder & buttons i2c


# License
See file LICENSE


# Contributing
Contributions are welcome! Please fork this repository and submit pull requests for any features, bug fixes, or improvements.


# Contact
For any questions or feedback, please open an issue on GitHub.


