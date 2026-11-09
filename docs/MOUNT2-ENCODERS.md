# Mount 2: closed-loop steppers, 100:1 harmonic, encoders on the SmartWebServer

This branch (`mount2-encoders`) is for a second, separate mount. `main` stays the setup for the first mount (17HM19-2004S motors, 4:1 belt into 100:1 harmonic). Nothing in this file has been built yet; it records the decisions made so far so a new conversation can carry on.

## Hardware

- Controller: the same BTT SKR Pico (RP2040) running OnStepX, and the same Seeed XIAO ESP32-C5 running the SmartWebServer, wired as on `main` (see the README and `docs/SKR-Pico-wiring.png`).
- Motors: StepperOnline **17HS24-2004D-E1K**, 1.8°, 2.0 A/phase, 1.30 Ω, 2.7 mH, 0.80 N·m holding. Driven directly into a **100:1 harmonic reducer** (no belts), so 100:1 total.
- Encoders: built onto the motor's rear shaft, **1000 PPR = 4000 counts per motor turn**, differential **EA+/EA−, EB+/EB−**, no index channel. The datasheet doesn't state the encoder supply voltage; plan on **5V**.
- Differential receivers: **MusRock MAX3485 TTL to RS485 modules (3.3V)**, 5 pack. Four are used, one per encoder signal pair.

### Motor cable (4 wires) to an SKR Pico motor socket

| Socket pin | Wire | Signal |
|---|---|---|
| 1 | BLK | A |
| 2 | GRN | A\ |
| 3 | RED | B |
| 4 | BLU | B\ |

(Pins 1+2 are one coil, 3+4 the other. Check about 1.3 Ω across each pair and open between pairs.)

### Encoder cable (6 wires, shielded)

| Wire | Signal |
|---|---|
| BLK | EA+ |
| BLU | EA− |
| YEL | EB+ |
| GRN | EB− |
| RED | VCC (5V) |
| WHT | GND |

The motor and encoder cables both use black, green, red and blue for different things. Label the encoder cable first.

### MAX3485 modules (receive only), one per signal pair

| Module pin | Connect to |
|---|---|
| VCC | XIAO 3V3 |
| GND | GND |
| A | encoder EA+ (or EB+) |
| B | encoder EA− (or EB−) |
| RXD / RO | XIAO input pin |
| TXD / DI | not connected (idle high keeps the board in receive mode) |
| DE and RE, if the board has them | GND |

| Module | Encoder pair | XIAO pin (tentative) |
|---|---|---|
| 1 | RA EA+/EA− | D0 |
| 2 | RA EB+/EB− | D1 |
| 3 | Dec EA+/EA− | D2 |
| 4 | Dec EB+/EB− | D3 |

- Confirm the module's printed pin labels before soldering (these compact boards vary).
- The modules carry a 120 Ω termination resistor (marked 121); leave it unless the signal looks weak.
- Encoder power: RED to 5V, WHT to GND. Shield to GND at the controller end only.
- Test: with a module and encoder powered, RXD should flip between about 0V and 3.3V while turning the motor shaft slowly by hand.
- If a count runs backwards, swap that pair's A/B or use the reverse option.

## Numbers

| | Value |
|---|---|
| Motor steps per turn | 200 (1.8°) |
| OnStepX steps/degree at 16 microsteps | 888.89 (~4.1″/step) |
| OnStepX steps/degree at 64 microsteps | **3555.556** (~1.0″/step) |
| Encoder counts per axis turn | 4000 × 100 = 400,000 |
| Encoder ticks/degree | **1111.111** (~3.2″/tick) |
| Encoder count rate at a 3°/s slew | ~3,300/s |
| OnStepX step rate limit on the RP2040 | ~13,000 steps/s (76.8 µs) |

The encoders sit before the reducer, so they catch missed steps and stalls of the motor, not anything after the gearbox (a direct harmonic drive has essentially no backlash or slip).

## Planned firmware changes (not done yet)

### OnStepX (`OnStepX/Config.h`)

- `AXIS1/2_STEPS_PER_DEGREE 3555.556`
- `AXIS1/2_DRIVER_MICROSTEPS 64`, `AXIS1/2_DRIVER_MICROSTEPS_GOTO 16`
- `AXIS1/2_DRIVER_IRUN` about 1100 mA (motors rated 2 A); keep the always-on driver cooling fan
- Slew rate to suit (64 microsteps caps the step rate at about 3.7°/s; the goto microstep switch raises that)
- Everything else (pinmap, GPS, DS3231, BME280 at 0x76, MB85RC256V FRAM, OnStep Log) as on `main`

### SmartWebServer

- Add the encoder pins to `SmartWebServer/src/pinmaps/Pins.XiaoEsp32c5.h` (`AXIS1_ENCODER_A_PIN`, `AXIS1_ENCODER_B_PIN`, `AXIS2_ENCODER_A_PIN`, `AXIS2_ENCODER_B_PIN`), after checking which ESP32-C5 pins are boot strapping pins.
- `SmartWebServer/Config.h`: `AXIS1/2_ENCODER AB`, `AXIS1/2_ENCODER_TICKS_DEG 1111.111`, and suitable `ENCODER_DIFF_LIMIT_TO/FROM` values.
- Check whether the `AB_ESP32` hardware-counter mode works on the ESP32-C5; plain `AB` (interrupts) works on any chip and is plenty for these rates.
- The XIAO's D6/D7 stay the serial link to the SKR Pico.

## Open questions

- Confirm the encoder supply voltage (5V assumed).
- Confirm the MAX3485 module pin labels from a photo.
- Choose the slew rate and goto microstepping once the motors are on the mount.
