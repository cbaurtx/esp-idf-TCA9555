# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/opt/esp32/esp-adf/esp-idf/components/ulp/cmake"
  "/home/iris/Development/Source/esp-idf-TCA9555/build/esp-idf/rot_btn_i2c_riscv/ulp_main"
  "/home/iris/Development/Source/esp-idf-TCA9555/build/esp-idf/rot_btn_i2c_riscv/ulp_main-prefix"
  "/home/iris/Development/Source/esp-idf-TCA9555/build/esp-idf/rot_btn_i2c_riscv/ulp_main-prefix/tmp"
  "/home/iris/Development/Source/esp-idf-TCA9555/build/esp-idf/rot_btn_i2c_riscv/ulp_main-prefix/src/ulp_main-stamp"
  "/home/iris/Development/Source/esp-idf-TCA9555/build/esp-idf/rot_btn_i2c_riscv/ulp_main-prefix/src"
  "/home/iris/Development/Source/esp-idf-TCA9555/build/esp-idf/rot_btn_i2c_riscv/ulp_main-prefix/src/ulp_main-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/iris/Development/Source/esp-idf-TCA9555/build/esp-idf/rot_btn_i2c_riscv/ulp_main-prefix/src/ulp_main-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/iris/Development/Source/esp-idf-TCA9555/build/esp-idf/rot_btn_i2c_riscv/ulp_main-prefix/src/ulp_main-stamp${cfgdir}") # cfgdir has leading slash
endif()
