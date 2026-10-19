// -------------------------------------------------------------------------------------------------
// Pin map for Seeed Studio XIAO ESP32-C5
#pragma once

// locate ESP32-C5 serial port pins, D7 (RX) and D6 (TX)
#ifndef SERIAL_ONSTEP
  #define SERIAL_ONSTEP Serial1
#endif
#ifndef SERIAL_RX
  #define SERIAL_RX          12
#endif
#ifndef SERIAL_TX
  #define SERIAL_TX          11
#endif
#ifndef SERIAL_SWAPPED_RX
  #define SERIAL_SWAPPED_RX  OFF
#endif
#ifndef SERIAL_SWAPPED_TX
  #define SERIAL_SWAPPED_TX  OFF
#endif

#ifndef LED_STATUS_PIN
  #define LED_STATUS_PIN     27  // onboard user LED (on when LOW)
#endif

#define BOOT0_PIN            OFF // to Boot0 of STM32 etc. (no swapped serial if active)

#define ETHERNET_RESET_PIN   OFF // pin# for controlling the reset of W5500 so it comes up properly
#define SD_CARD_CS_PIN       OFF // pin 4 to CS for SD adapter on Arduino Ethernet Shield

#define AXIS1_ENCODER_A_PIN  OFF // pin# for Axis1 encoder, for A or CW  or MA/CLK
#define AXIS1_ENCODER_B_PIN  OFF // pin# for Axis1 encoder, for B or CCW or SLO
#define AXIS2_ENCODER_A_PIN  OFF // pin# for Axis1 encoder, for A or CW  or MA/CLK
#define AXIS2_ENCODER_B_PIN  OFF // pin# for Axis1 encoder, for B or CCW or SLO

#if AXIS1_ENCODER == OFF || AXIS2_ENCODER == OFF
  #define VGPIO_PIN_0        AXIS1_ENCODER_A_PIN
  #define VGPIO_PIN_1        AXIS1_ENCODER_B_PIN
  #define VGPIO_PIN_2        AXIS2_ENCODER_A_PIN
  #define VGPIO_PIN_3        AXIS2_ENCODER_B_PIN
#endif
