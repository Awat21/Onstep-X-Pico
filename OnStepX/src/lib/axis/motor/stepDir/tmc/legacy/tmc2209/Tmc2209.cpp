// -----------------------------------------------------------------------------------
// axis step/dir motor driver, TMC2209Stepper

// note: LegacyUART requires UART RX but TX is optional

#include "Tmc2209.h"

#if !defined(DRIVER_TMC_STEPPER) && defined(TMC2209_PRESENT)

#include "../../../../../../gpioEx/GpioEx.h"

// help with pin names
#define rx m3
#define tx m2

#if defined(ARDUINO_ARCH_RP2040) && defined(SERIAL_TMC_HARDWARE_UART)
  // The TMC2209 library calls begin() on the shared UART for every driver. On the RP2040 a second begin() first
  // ends the port, which glitches the TX line, so the next driver misses its first command, the library stops
  // talking to it and that driver runs on its power up defaults. This wrapper starts the port only once.
  class TmcSerialOnce : public HardwareSerial {
    public:
      void begin(unsigned long baud) { if (!started) { SERIAL_TMC.begin(baud); started = true; } }
      void begin(unsigned long baud, uint16_t config) { if (!started) { SERIAL_TMC.begin(baud, config); started = true; } }
      void end() {}
      int available(void) { return SERIAL_TMC.available(); }
      int peek(void) { return SERIAL_TMC.peek(); }
      int read(void) { return SERIAL_TMC.read(); }
      void flush(void) { SERIAL_TMC.flush(); }
      size_t write(uint8_t b) { return SERIAL_TMC.write(b); }
      using Print::write;
      operator bool() { return (bool)SERIAL_TMC; }
    private:
      bool started = false;
  };
  static TmcSerialOnce tmcSerialOnce;
  #define SERIAL_TMC_PORT tmcSerialOnce
#else
  #define SERIAL_TMC_PORT SERIAL_TMC
#endif

#if defined(ARDUINO_ARCH_RP2040) && defined(SERIAL_TMC_HARDWARE_UART) && DEBUG == VERBOSE
  // raw TMC2209 UART diagnostic, independent of the TMC2209 library: CRC checked replies and the IFCNT write counter
  static uint8_t tmcCrc(const uint8_t *d, int len) {
    uint8_t crc = 0;
    for (int i = 0; i < len; i++) {
      uint8_t b = d[i];
      for (int j = 0; j < 8; j++) {
        if ((crc >> 7) ^ (b & 0x01)) crc = (crc << 1) ^ 0x07; else crc = crc << 1;
        b >>= 1;
      }
    }
    return crc;
  }

  // collect up to n bytes within timeoutUs
  static int tmcCollect(uint8_t *buf, int n, uint32_t timeoutUs) {
    int c = 0; uint32_t t0 = micros();
    while (c < n && (micros() - t0) < timeoutUs) { if (SERIAL_TMC.available()) buf[c++] = SERIAL_TMC.read(); }
    return c;
  }

  static void tmcHex(char *out, const uint8_t *b, int n) {
    out[0] = 0; for (int i = 0; i < n; i++) sprintf(out + strlen(out), "%02X ", b[i]);
  }

  // returns 1 good, 0 no/short reply, -1 bad CRC or framing; logs the raw bytes
  static int tmcRawRead(uint8_t addr, uint8_t reg, uint32_t *value, bool log = true) {
    while (SERIAL_TMC.available()) SERIAL_TMC.read();
    uint8_t req[4] = { 0x05, addr, reg, 0 }; req[3] = tmcCrc(req, 3);
    SERIAL_TMC.write(req, 4); SERIAL_TMC.flush();
    uint8_t buf[16]; int n = tmcCollect(buf, 12, 20000);
    char hex[60]; tmcHex(hex, buf, n);
    int result = 0;
    if (n >= 12) {
      uint8_t *r = buf + 4;
      if (r[0] == 0x05 && r[1] == 0xFF && r[2] == reg && tmcCrc(r, 7) == r[7]) {
        *value = ((uint32_t)r[3] << 24) | ((uint32_t)r[4] << 16) | ((uint32_t)r[5] << 8) | r[6];
        result = 1;
      } else result = -1;
    }
    if (log) {
      char msg[140];
      if (result == 1) sprintf(msg, "MSG: TMC raw, addr %u reg 0x%02X = 0x%08lX  [%s]", addr, reg, (unsigned long)*value, hex); else
      if (result == -1) sprintf(msg, "MSG: TMC raw, addr %u reg 0x%02X BAD REPLY (crc/framing) [%s]", addr, reg, hex); else
        sprintf(msg, "MSG: TMC raw, addr %u reg 0x%02X no reply, got %d of 12 bytes [%s]", addr, reg, n, hex);
      VL(msg);
    }
    delay(2);
    return result;
  }

  static void tmcRawWrite(uint8_t addr, uint8_t reg, uint32_t value) {
    while (SERIAL_TMC.available()) SERIAL_TMC.read();
    uint8_t d[8] = { 0x05, addr, (uint8_t)(reg | 0x80), (uint8_t)(value >> 24), (uint8_t)(value >> 16), (uint8_t)(value >> 8), (uint8_t)value, 0 };
    d[7] = tmcCrc(d, 7);
    SERIAL_TMC.write(d, 8); SERIAL_TMC.flush();
    uint8_t echo[8]; tmcCollect(echo, 8, 5000);
    delay(2);
  }

  static void tmcRawDiagnostic() {
    VLF("MSG: TMC raw, ---- UART diagnostic start (IOIN 0x06 version should be 0x21 in the top byte) ----");
    uint32_t v;
    for (uint8_t a = 0; a < 4; a++) {
      tmcRawRead(a, 0x06, &v);   // IOIN
      tmcRawRead(a, 0x00, &v);   // GCONF
      tmcRawRead(a, 0x6C, &v);   // CHOPCONF
      tmcRawRead(a, 0x02, &v);   // IFCNT
    }

    // write test on the Axis2 (Dec) driver, IFCNT counts the writes the driver accepted
    uint8_t a = SERIAL_TMC_ADDRESS_MAP(1);
    uint32_t cnt0 = 0, cnt1 = 0, gconf = 0, chop = 0;
    char msg[140];
    if (tmcRawRead(a, 0x02, &cnt0, false) == 1 && tmcRawRead(a, 0x6C, &chop, false) == 1) {
      tmcRawWrite(a, 0x00, 0x000001C5UL);                                     // GCONF: analog scale, spreadCycle, pdn_disable, mstep_reg_select, multistep_filt
      tmcRawWrite(a, 0x6C, (chop & ~(0x0FUL << 24)) | (4UL << 24));          // CHOPCONF mres=4 (16 microsteps)
      tmcRawRead(a, 0x02, &cnt1, false);
      tmcRawRead(a, 0x00, &gconf, false);
      tmcRawRead(a, 0x6C, &chop, false);
      sprintf(msg, "MSG: TMC raw, addr %u write test: IFCNT %lu -> %lu (should go up by 2), GCONF=0x%08lX (want 0x000001C5), CHOPCONF=0x%08lX (mres=%lu, want 4)",
        a, (unsigned long)cnt0, (unsigned long)cnt1, (unsigned long)gconf, (unsigned long)chop, (unsigned long)((chop >> 24) & 0x0F));
      VL(msg);
    } else {
      sprintf(msg, "MSG: TMC raw, addr %u write test skipped, no clean reply", a); VL(msg);
    }
    VLF("MSG: TMC raw, ---- UART diagnostic end ----");
  }
#endif

#define CurrentToPercent(current) ((current/1000.0)/(0.325/(rSense + 0.02)))*100.0

// constructor
StepDirTmc2209::StepDirTmc2209(uint8_t axisNumber, const StepDirDriverPins *Pins, const StepDirDriverSettings *Settings,
                               int16_t currentHold, int16_t currentRun, int16_t currentSlewing, int8_t intpol)
                               :TmcStepDirDriverNSG(axisNumber, Pins, Settings, currentHold, currentRun, currentSlewing, intpol) {
  strcpy(axisPrefix, " Axis_Tmc2209StepDir legacy, ");
  axisPrefix[5] = '0' + axisNumber;
}

// setup driver
bool StepDirTmc2209::init() {
  if (!TmcStepDirDriver::init()) return false;

  // get TMC UART driver ready
  pinModeEx(Pins->m0, OUTPUT);
  pinModeEx(Pins->m1, OUTPUT);

  driver = new TMC2209Stepper();
  if (driver == NULL) return false; 

  int16_t rxPin = Pins->rx;

  delay(1);
  #if defined(SERIAL_TMC_HARDWARE_UART)
    #if defined(DEDICATED_MODE_PINS)
      // program the device address 0,1,2,3 since M0 and M1 are all unique
      int deviceAddress = SERIAL_TMC_ADDRESS_MAP(axisNumber - 1);
      digitalWriteEx(Pins->m0, bitRead(deviceAddress, 0));
      digitalWriteEx(Pins->m1, bitRead(deviceAddress, 1));
    #else
      // help user hard code the device address 0,1,2,3 by cutting pins
      digitalWriteEx(Pins->m0, HIGH);
      digitalWriteEx(Pins->m1, HIGH);
    #endif

    VF("MSG:"); V(axisPrefix);
    VF("HW UART driver pins rx="); V(SERIAL_TMC_RX); VF(", tx="); V(SERIAL_TMC_TX); VF(", baud="); V(SERIAL_TMC_BAUD); VLF(" bps");
    pinModeEx(SERIAL_TMC_RX, INPUT_PULLUP);
    rxPin = SERIAL_TMC_RX;
  #else
    // pull MS1 and MS2 low for device address 0
    digitalWriteEx(Pins->m0, LOW);
    digitalWriteEx(Pins->m1, LOW);

    #if SERIAL_TMC_RX_DISABLE == true
      rxPin = OFF;
    #endif
    VF("MSG:"); V(axisPrefix);
    VF("SW UART driver pins rx="); V(rxPin); VF(", tx="); V(Pins->tx); VF(", baud="); V(SERIAL_TMC_BAUD); VLF(" bps");
    pinModeEx(rxPin, INPUT_PULLUP);
    driver->setup(SERIAL_TMC_BAUD, SERIAL_TMC_ADDRESS_MAP(axisNumber - 1), rxPin, Pins->tx);
  #endif

  // configure the driver, then read the settings back and start over if any didn't take (on a shared UART line one
  // failed reply makes the library ignore all further writes to that driver)
  for (int attempt = 1; attempt <= 5; attempt++) {
    #if defined(SERIAL_TMC_HARDWARE_UART)
      #if SERIAL_TMC_INVERT == ON
        driver->setup(SERIAL_TMC_PORT, SERIAL_TMC_BAUD, SERIAL_TMC_ADDRESS_MAP(axisNumber - 1), SERIAL_TMC_RX, SERIAL_TMC_TX, true);
      #else
        driver->setup(SERIAL_TMC_PORT, SERIAL_TMC_BAUD, SERIAL_TMC_ADDRESS_MAP(axisNumber - 1), SERIAL_TMC_RX, SERIAL_TMC_TX);
      #endif
    #endif

    driver->useExternalSenseResistors();
    driver->enableAnalogCurrentScaling();

    driver->enable();
    driver->moveUsingStepDirInterface();
    driver->setPwmOffset(pc_pwm_ofs);
    driver->setPwmGradient(pc_pwm_grad);
    if (pc_pwm_auto) driver->enableAutomaticCurrentScaling();
    if (!intpol.value) {
      VF("MSG:"); V(axisPrefix); VLF("driver interpolation control not supported");
    }

    modeMicrostepTracking();
    driver->setRunCurrent(CurrentToPercent(iRun));
    driver->setHoldCurrent(CurrentToPercent(iHold));
    driver->disableStealthChop();

    #if defined(SERIAL_TMC_HARDWARE_UART)
      TMC2209Stepper::Settings check = driver->getSettings();
      if (check.is_communicating && check.is_setup && check.analog_current_scaling_enabled &&
          check.microsteps_per_step == normalizedMicrosteps && !check.stealth_chop_enabled) break;
      DF("WRN:"); D(axisPrefix); DF("settings didn't verify on attempt "); D(attempt); DLF(", setting up again");
      delay(20);
    #else
      break;
    #endif
  }

  // this driver automatically switches to one-way communications, even if a RX pin is set
  // so the following only returns false if communications are "half working"
  #ifdef MOTOR_DRIVER_DETECT
    if (rxPin != OFF) {
      if (!driver->isSetupAndCommunicating()) {
        DF("ERR:"); D(axisPrefix); DLF("no motor driver device detected!");
        return false;
      } else { VF("MSG:"); V(axisPrefix); VLF("motor driver device detected"); }
    }
  #endif

  // read the settings back over UART, a driver that doesn't answer runs on its power up defaults (often weak)
  #if DEBUG == VERBOSE && defined(SERIAL_TMC_HARDWARE_UART)
    TMC2209Stepper::Settings settings = driver->getSettings();
    VF("MSG:"); V(axisPrefix); VF("UART address "); V(SERIAL_TMC_ADDRESS_MAP(axisNumber - 1));
    if (!settings.is_communicating) {
      VLF(" NO REPLY, the driver didn't get its settings (current, microsteps) and runs on defaults");
    } else {
      VF(" replied, UART control="); V(settings.is_setup ? "yes" : "NO");
      VF(", microsteps="); V(settings.microsteps_per_step);
      VF(", "); V(settings.stealth_chop_enabled ? "StealthChop" : "SpreadCycle");
      VF(", analog current scaling="); V(settings.analog_current_scaling_enabled ? "on" : "off");
      VF(", enabled="); VL(settings.enabled ? "yes" : "no");
    }
  #endif

  #if defined(ARDUINO_ARCH_RP2040) && defined(SERIAL_TMC_HARDWARE_UART) && DEBUG == VERBOSE
    if (axisNumber == 2) tmcRawDiagnostic();
  #endif

  return true;
}

void StepDirTmc2209::modeMicrostepTracking() {
  driver->setMicrostepsPerStep(normalizedMicrosteps);
}

int StepDirTmc2209::modeMicrostepSlewing() {
  if (microstepRatio > 1) {
    driver->setMicrostepsPerStep(normalizedMicrostepsSlewing);
  }
  return microstepRatio;
}

void StepDirTmc2209::modeDecayTracking() {
  if (normalizedDecay == SPREADCYCLE) driver->disableStealthChop(); else driver->enableStealthChop();
  driver->setRunCurrent(CurrentToPercent(iRun));
  driver->setHoldCurrent(CurrentToPercent(iHold));
}  

void StepDirTmc2209::modeDecaySlewing() {
  if (decaySlewing.value == SPREADCYCLE) driver->disableStealthChop(); else driver->enableStealthChop();
  driver->setRunCurrent(CurrentToPercent(iGoto));
  driver->setHoldCurrent(CurrentToPercent(iHold));
}

void StepDirTmc2209::readStatus() {
  TMC2209Stepper::Status status_result = driver->getStatus();
  status.outputA.shortToGround  = (bool)status_result.short_to_ground_a || (bool)status_result.low_side_short_a;
  status.outputA.openLoad       = (bool)status_result.open_load_a;
  status.outputB.shortToGround  = (bool)status_result.short_to_ground_b || (bool)status_result.low_side_short_b;
  status.outputB.openLoad       = (bool)status_result.open_load_b;
  status.overTemperatureWarning = (bool)status_result.over_temperature_warning;
  status.overTemperature        = (bool)status_result.over_temperature_shutdown;
  status.standstill             = (bool)status_result.standstill;
}

// secondary way to power down not using the enable pin
bool StepDirTmc2209::enable(bool state) {
  if (state) {
    modeDecayTracking();
  } else {
    driver->enableStealthChop();
    driver->setHoldCurrent(0);
  }

  return true;
}

// calibrate the motor driver if required
void StepDirTmc2209::calibrateDriver() {
  if (decay.value == STEALTHCHOP || decaySlewing.value == STEALTHCHOP) {
    VF("MSG: StepDirDriver"); V(axisNumber); VL(", TMC standstill automatic current calibration");
    driver->setRunCurrent(CurrentToPercent(iRun));
    driver->setHoldCurrent(CurrentToPercent(iRun));
    driver->enableStealthChop();
    delay(1000);
    driver->setRunCurrent(CurrentToPercent(iRun));
    driver->setHoldCurrent(CurrentToPercent(iHold));
    driver->disableStealthChop();
  }
}

#endif
