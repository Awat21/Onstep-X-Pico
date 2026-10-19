# Onstep X Pico

Combined repository containing:

- `OnStepX/` — telescope mount controller firmware, copied from [hjd1964/OnStepX](https://github.com/hjd1964/OnStepX) (commit b0f18a2)
- `SmartWebServer/` — WiFi/web interface for OnStep, copied from [hjd1964/SmartWebServer](https://github.com/hjd1964/SmartWebServer) (commit 9a65d35)

Both projects are licensed under the GPL-3.0; see the LICENSE file in each folder.

## BTT SKR Pico support

OnStepX has been adapted to run on the [BigTreeTech SKR Pico v1.0](https://github.com/bigtreetech/skr-pico) (RP2040):

- New pinmap `BTT_SKR_PICO` (`OnStepX/src/pinmaps/Pins.SKR_PICO.h`), selected in `OnStepX/Config.h`.
- Hardware step timers for the RP2040 (`OnStepX/src/lib/tasks/HAL_RP2040_HWTIMER.h`). Without these the motors don't step.
- Settings are saved to the Pico's flash, because the board has no EEPROM. You can still pick an external I2C EEPROM with `NV_DRIVER`.
- `Config.h` is preset for the onboard TMC2209 drivers on Axis1/Axis2 at 16 microsteps. Set `AXIS1/2_STEPS_PER_DEGREE`, `AXIS1/2_DRIVER_IRUN` and the rest to suit your mount and motors.

### Board connections

| Board connector | OnStepX use |
|---|---|
| X driver | Axis1 (RA/Azm), TMC2209 UART address 0 |
| Y driver | Axis2 (Dec/Alt), TMC2209 UART address 2 |
| Z driver (Z1/Z2) | Axis3 (rotator), TMC2209 UART address 1 |
| E driver | Axis4 (focuser 1), TMC2209 UART address 3 |
| X-STOP (GPIO4) / Y-STOP (GPIO3) | Axis1 / Axis2 home switch |
| Z-STOP (GPIO25) | Limit switch |
| E0-STOP (GPIO16) | GPS PPS input |
| PROBE (GPIO22, P_2/PROBE jumper fitted) | GPS serial (receive only, PIO serial port) |
| RGB (GPIO24 = SDA) + SERVOS (GPIO29 = SCL) | I2C (BME280, DS3231 + AT24C32 EEPROM) |
| HE / HB / FAN1 / FAN2 / FAN3 | AUX1..AUX5 MOSFET outputs (dew heaters, switches) |
| TH0 (GPIO27) / THB (GPIO26) | Thermistor inputs for features 1 and 2 |
| 5V_Power header: TXD0 (GPIO0), RXD0 (GPIO1), 5V, GND | Serial B (9600 baud) for the SmartWebServer |
| USB-C | Serial A |

### Building and flashing OnStepX

1. Install the [Arduino-Pico core](https://github.com/earlephilhower/arduino-pico) (Earle Philhower). Board manager URL: `https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json`
2. Install the libraries: [hjd1964/TMC2209](https://github.com/hjd1964/TMC2209) (from GitHub, as a zip), "Adafruit BME280 Library", "RTC by Makuna" and "TinyGPSPlus".
3. Open `OnStepX/OnStepX.ino` and select the board **Raspberry Pi Pico**.
4. Hold the board's BOOT button while plugging in USB, then upload (or copy the `.uf2` onto the drive that appears).

### GPS, clock, weather sensor and EEPROM

`Config.h` is set up for an Adafruit Ultimate GPS (PA1616D), a ZS-042 DS3231 module (which includes an AT24C32 EEPROM) and a BME280:

- `TIME_LOCATION_SOURCE GPS`, with `TIME_LOCATION_SOURCE_FALLBACK DS3231` so the time is right at power-on before the GPS has a fix
- `TIME_LOCATION_PPS_SENSE HIGH` and `TIME_LOCATION_PPS_SYNC ON`, so the GPS pulse keeps the sidereal clock accurate
- `WEATHER BME280`, at I2C address 0x77 (Adafruit). Many generic purple modules use 0x76; for those, use `BME280_0x76`
- `NV_DRIVER NV_AT24C32`, so settings go to the ZS-042's EEPROM (I2C address 0x57) instead of the Pico's flash

| Device pin | SKR Pico |
|---|---|
| GPS VIN / GND | 5V / GND (any 5V header, e.g. PROBE or E0-STOP) |
| GPS TX | PROBE signal pin (GPIO22), with the P_2/PROBE jumper fitted |
| GPS PPS | E0-STOP signal pin (GPIO16) |
| GPS RX | not connected |
| ZS-042 and BME280 VCC / GND | **3.3V** (SWD header pin 5) / GND |
| ZS-042 and BME280 SDA | RGB signal pin (GPIO24) |
| ZS-042 and BME280 SCL | SERVOS signal pin (GPIO29) |

The Pico's pins don't tolerate 5V, and the ZS-042 pulls SDA/SCL up to its own supply, so power the I2C modules from 3.3V, not the 5V on those headers. The ZS-042 also has a battery charging circuit meant for a rechargeable LIR2032. With a regular CR2032, remove the charging resistor (or its diode) so the battery isn't charged.

The PEC index input has moved off E0-STOP. To use one, add `#define PEC_SENSE_PIN 3` (Y-STOP, if not used for homing) or similar to `Config.h`.

### SmartWebServer

The SmartWebServer runs on a Seeed Studio XIAO ESP32-C5, with a pinmap in `SmartWebServer/src/pinmaps/Pins.XiaoEsp32c5.h`. In the Arduino IDE, use the Espressif ESP32 package (3.3 or newer) and select the board **XIAO_ESP32C5**. Out of the box it starts its own Wi-Fi network, "OnStep-SWS" (password `password`), with the web interface at http://192.168.0.1. From there you can join it to your home network, or change the defaults in `SmartWebServer/Extended.config.h` before flashing.

| XIAO ESP32-C5 | SKR Pico 5V_Power header |
|---|---|
| D6 (TX, GPIO11) | RXD0 (GPIO1) |
| D7 (RX, GPIO12) | TXD0 (GPIO0) |
| 5V | 5V |
| GND | GND |
