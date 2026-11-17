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
