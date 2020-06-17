# Arduino Make file. Refer to https://github.com/sudar/Arduino-Makefile

# --- nano ide 1.6
CPPFLAGS               += -DSRVM_ARDUINO_DISPLAY_SSD1306
BOARD_TAG               = mega 
BOARD_SUB               = atmega2560
ARDUINO_DIR             = $(HOME)/arduino-1.8.12
ARDUINO_LIBS            = AccelStepper SPI Wire Stepper Adafruit_VL53L0X
CXXFLAGS_STD            = -std=c++17
DIAGNOSTICS_COLOR_WHEN  = auto

include $(HOME)/include/Arduino.mk

