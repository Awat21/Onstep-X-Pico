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
| E0-STOP (GPIO16) | PEC index sense |
| PROBE (GPIO22, P_2/PROBE jumper fitted) | PPS input |
| RGB (GPIO24 = SDA) + SERVOS (GPIO29 = SCL) | I2C (BME280, DS3231, EEPROM, ...) |
| HE / HB / FAN1 / FAN2 / FAN3 | AUX1..AUX5 MOSFET outputs (dew heaters, switches) |
| TH0 (GPIO27) / THB (GPIO26) | Thermistor inputs for features 1 and 2 |
| 5V_Power header: TXD0 (GPIO0), RXD0 (GPIO1), 5V, GND | Serial B (9600 baud) for the SmartWebServer |
| USB-C | Serial A |

### Building and flashing OnStepX

1. Install the [Arduino-Pico core](https://github.com/earlephilhower/arduino-pico) (Earle Philhower). Board manager URL: `https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json`
2. Install the libraries: [hjd1964/TMC2209](https://github.com/hjd1964/TMC2209) (from GitHub, as a zip). Add "Adafruit BME280 Library" if you use a weather sensor and "RTC by Makuna" if you use a DS3231.
3. Open `OnStepX/OnStepX.ino` and select the board **Raspberry Pi Pico**.
4. Hold the board's BOOT button while plugging in USB, then upload (or copy the `.uf2` onto the drive that appears).

### SmartWebServer

The SmartWebServer runs on a separate ESP8266/ESP32 board, so its code didn't need changes. Wire the ESP to the 5V_Power header: ESP TX to RXD0 (GPIO1), ESP RX to TXD0 (GPIO0), plus 5V and GND. Both sides default to 9600 baud.
