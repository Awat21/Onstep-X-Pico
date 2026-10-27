// -------------------------------------------------------------------------------------------------
// Pin map for the BTT SKR Pico Version 1.0 (RP2040)
#pragma once

#if defined(ARDUINO_ARCH_RP2040)

// Serial: Built in USB (not TTL)
// Serial2: RX2 Pin GPIO9, TX2 Pin GPIO8 (TMC2209 UART, all four drivers)
// GPIO0/GPIO1 (UART0) are used for I2C instead, see below, so Serial B and the GPS use PIO based serial ports

#if SERIAL_A_BAUD_DEFAULT != OFF
  #define SERIAL_A              Serial
#endif
#if SERIAL_B_BAUD_DEFAULT != OFF
  #define SERIAL_B              SoftSerial       // For an ESP SmartWebServer
  #define SERIAL_B_RX           22               // Connector Labeled PROBE (with the P_2/PROBE jumper fitted)
  #define SERIAL_B_TX           29               // Connector Labeled SERVOS
#endif

// Receive only since OnStep never transmits to the GPS. The endstop input has an RC filter (100R, 0.1uF) so keep the
// GPS at 9600 baud
#if SERIAL_GPS_BAUD != OFF
  #ifndef SERIAL_GPS
    #define SERIAL_GPS          SoftSerial
    #define SERIAL_GPS_RX       16               // Connector Labeled E0-STOP
    #define SERIAL_GPS_TX       NOPIN
  #endif
#endif

// Use the following settings for the onboard TMC2209 drivers
#if defined(STEP_DIR_TMC_UART_PRESENT) || defined(SERVO_TMC2209_PRESENT)
  #define SERIAL_TMC_HARDWARE_UART
  #define SERIAL_TMC            Serial2          // Use a single hardware serial port to up to four drivers
  #define SERIAL_TMC_BAUD       460800           // Baud rate
  #define SERIAL_TMC_RX         9                // Recieving data
  #define SERIAL_TMC_TX         8                // Transmit data (through 1K to PDN_UART on each driver)
  #define SERIAL_TMC_ADDRESS_MAP(x) ((x==1)?2 : ((x==2)?1 : x)) // X(Axis1) is 0, Y(Axis2) is 2, Z(Axis3) is 1, E(Axis4) is 3
#endif

// allow RX from TMC UART drivers
#ifndef SERIAL_TMC_RX_DISABLE
  #define SERIAL_TMC_RX_DISABLE false
#endif

// I2C on GPIO0 (SDA) and GPIO1 (SCL), Connector Labeled 5V_Power (pins TXD0 and RXD0), the only free pins with a
// direct connection that support I2C (GPIO24 on the RGB connector goes through a one way buffer). This connector has
// 5V and GND but power I2C devices from 3.3V (an ESP SmartWebServer's 3V3 pin, or SWD connector pin 5), the RP2040 isn't 5V tolerant.
#ifndef I2C_SDA_PIN
  #define I2C_SDA_PIN           0
#endif
#ifndef I2C_SCL_PIN
  #define I2C_SCL_PIN           1
#endif

// MOSFET outputs, these switch the main (12/24V) power low side and are well suited for dew heaters
#define HEATER0_PIN             23               // Connector Labeled HE
#define HEATER1_PIN             21               // Connector Labeled HB
#define HEATER2_PIN             17               // Connector Labeled FAN1
#define HEATER3_PIN             18               // Connector Labeled FAN2
#define HEATER4_PIN             20               // Connector Labeled FAN3

// The multi-purpose pins (Aux3..Aux8 can be analog pwm/dac if supported)
#define AUX1_PIN                HEATER0_PIN
#define AUX2_PIN                HEATER1_PIN
#define AUX3_PIN                HEATER2_PIN
#define AUX4_PIN                HEATER3_PIN
#define AUX5_PIN                HEATER4_PIN

// Thermistor (temperature) sensor inputs have built-in 4.7K Ohm pullups and a 0.1uF cap for noise supression
#define TEMP0_PIN               27               // Connector Labeled TH0
#define TEMP1_PIN               26               // Connector Labeled THB

#ifndef FEATURE1_TEMPERATURE_PIN
  #define FEATURE1_TEMPERATURE_PIN TEMP0_PIN
#endif
#ifndef FEATURE2_TEMPERATURE_PIN
  #define FEATURE2_TEMPERATURE_PIN TEMP1_PIN
#endif

// The endstop inputs are buffered (74LVC2G34) and have 10K pullups to 3.3V, so they are inputs only

// The PEC index sense is a logic level input, resets the PEC index on rising edge then waits for 60 seconds before allowing another reset
// (there are no free inputs left for it, add a #define PEC_SENSE_PIN to Config.h to use one, the Y-STOP for instance)

// The PPS pin is a 3.3V logic input, OnStep measures time between rising edges and adjusts the internal sidereal clock frequency
#ifndef PPS_SENSE_PIN
  #define PPS_SENSE_PIN         25               // Connector Labeled Z-STOP
#endif

// The limit switch sense is a logic level input normally pull high (2k resistor,) shorted to ground it stops gotos/tracking
// (there are no free inputs left for it, add a #define LIMIT_SENSE_PIN to Config.h to use one, the Y-STOP for instance)

// Axis1 RA/Azm step/dir driver (Labeled X)
#define AXIS1_ENABLE_PIN        12
#define AXIS1_M0_PIN            OFF              // hardwired for TMC UART address 0
#define AXIS1_M1_PIN            OFF              // hardwired for TMC UART address 0
#define AXIS1_M2_PIN            OFF              // UART TX
#define AXIS1_M3_PIN            OFF              // UART RX
#define AXIS1_STEP_PIN          11
#define AXIS1_DIR_PIN           10
#ifndef AXIS1_SENSE_HOME_PIN
  #define AXIS1_SENSE_HOME_PIN  4                // Connector Labeled X-STOP
#endif

// Axis2 Dec/Alt step/dir driver (Labeled Y)
#define AXIS2_ENABLE_PIN        7
#define AXIS2_M0_PIN            OFF              // hardwired for TMC UART address 2
#define AXIS2_M1_PIN            OFF              // hardwired for TMC UART address 2
#define AXIS2_M2_PIN            OFF              // UART TX
#define AXIS2_M3_PIN            OFF              // UART RX
#define AXIS2_STEP_PIN          6
#define AXIS2_DIR_PIN           5
#ifndef AXIS2_SENSE_HOME_PIN
  #define AXIS2_SENSE_HOME_PIN  3                // Connector Labeled Y-STOP
#endif

// For rotator stepper driver (Labeled Z, the Z1 and Z2 motor connectors are wired in parallel)
#define AXIS3_ENABLE_PIN        2
#define AXIS3_M0_PIN            OFF              // hardwired for TMC UART address 1
#define AXIS3_M1_PIN            OFF              // hardwired for TMC UART address 1
#define AXIS3_M2_PIN            OFF              // UART TX
#define AXIS3_M3_PIN            OFF              // UART RX
#define AXIS3_STEP_PIN          19
#define AXIS3_DIR_PIN           28

// For focuser1 stepper driver (Labeled E)
#define AXIS4_ENABLE_PIN        15
#define AXIS4_M0_PIN            OFF              // hardwired for TMC UART address 3
#define AXIS4_M1_PIN            OFF              // hardwired for TMC UART address 3
#define AXIS4_M2_PIN            OFF              // UART TX
#define AXIS4_M3_PIN            OFF              // UART RX
#define AXIS4_STEP_PIN          14
#define AXIS4_DIR_PIN           13

#else
#error "Wrong processor for this configuration!"

#endif
