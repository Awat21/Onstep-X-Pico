# Prebuilt firmware

Built with the settings in `OnStepX/Config.h` and `SmartWebServer/Config.h` at the commits below. If you change any settings, rebuild from source (see the main README) and replace these files.

| File | Board | Built from | Build |
|---|---|---|---|
| `OnStepX-SKR-Pico.uf2` | BTT SKR Pico v1.0 | `510f957` | Arduino-Pico core 6.3.0, board "Raspberry Pi Pico" |
| `SmartWebServer-XIAO-ESP32C5-merged.bin` | Seeed Studio XIAO ESP32-C5 | `29061de` | Espressif ESP32 core 3.3.12, board "XIAO_ESP32C5" |

## Flashing the SKR Pico

1. Hold the BOOT button while plugging in USB-C.
2. Copy `OnStepX-SKR-Pico.uf2` onto the `RPI-RP2` drive that appears. The board restarts by itself.

## Flashing the XIAO ESP32-C5

1. Hold the BOOT button while plugging in USB, then release it.
2. Flash `SmartWebServer-XIAO-ESP32C5-merged.bin` at address `0x0`, either with [Espressif's web flasher](https://espressif.github.io/esptool-js/) (Chrome or Edge) or with:
   `esptool.py --chip esp32c5 write_flash 0x0 SmartWebServer-XIAO-ESP32C5-merged.bin`
3. Unplug and replug. It starts the "OnStep-SWS" Wi-Fi network (password `password`), with the web interface at http://192.168.0.1.
