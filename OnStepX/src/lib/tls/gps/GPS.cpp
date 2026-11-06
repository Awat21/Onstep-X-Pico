// -----------------------------------------------------------------------------------
// Time/Location source GPS support
// uses the specified serial port

#include "GPS.h"

#if defined(TIME_LOCATION_SOURCE) && TIME_LOCATION_SOURCE == GPS

#ifdef TLS_TIMELIB
  #include <TimeLib.h> // https://github.com/PaulStoffregen/Time/archive/master.zip
#endif

#ifndef SERIAL_GPS
  #error "SERIAL_GPS must be set to the serial port object if TIME_LOCATION_SOURCE GPS is used"
#endif

#ifndef SERIAL_GPS_BAUD
  #error "SERIAL_GPS_BAUD must be set to the baud rate if TIME_LOCATION_SOURCE GPS is used"
#endif

#if SERIAL_GPS == SoftSerial || SERIAL_GPS == HardSerial
  #ifndef SERIAL_GPS_RX
    #error "SERIAL_GPS_RX must be set to the serial port RX pin if SoftSerial or HardSerial is used"
  #endif
  #ifndef SERIAL_GPS_TX
    #error "SERIAL_GPS_TX must be set to the serial port TX pin if SoftSerial or HardSerial is used"
  #endif
#endif

#include "../PPS.h"
#include "../../tasks/OnTask.h"

#include <TinyGPSPlus.h> // https://github.com/mikalhart/TinyGPSPlus/releases
TinyGPSPlus gps;

// provide for using software serial
#if SERIAL_GPS == SoftSerial
  #include <SoftwareSerial.h>
  #undef SERIAL_GPS
  SoftwareSerial SWSerialGPS(SERIAL_GPS_RX, SERIAL_GPS_TX);
  #define SERIAL_GPS SWSerialGPS
  #define SERIAL_GPS_RXTX_SET
#endif

// provide for using hardware serial
#if SERIAL_GPS == HardSerial
  #include <HardwareSerial.h>
  #undef SERIAL_GPS
  HardwareSerial HWSerialGPS(SERIAL_GPS_RX, SERIAL_GPS_TX);
  #define SERIAL_GPS HWSerialGPS
  #define SERIAL_GPS_RXTX_SET
#endif

TlsGPS *tlsGps;

void gpsPoll() {
  // status every 30 seconds until the GPS is ready
  #if DEBUG == VERBOSE
    static unsigned long nextStatus = 30000UL;
    if ((long)(millis() - nextStatus) > 0) {
      nextStatus = millis() + 30000UL;
      VF("MSG: TLS, GPS status chars="); V(gps.charsProcessed());
      VF(" fixes="); V(gps.sentencesWithFix());
      VF(" bad="); V(gps.failedChecksum());
      VF(" sats="); if (gps.satellites.isValid()) { V(gps.satellites.value()); } else { VF("?"); }
      VF(" location="); V(gps.location.isValid() ? "valid" : "none");
      VF(" date/time="); V(gps.date.isValid() && gps.time.isValid() ? "valid" : "none");
      #if (TIME_LOCATION_PPS_SENSE) != OFF
        VF(" pps pulses="); V(pps.pulses); VF(pps.synced ? " (synced)" : " (not synced)");
      #endif
      VL("");
    }
  #endif

  #if (TIME_LOCATION_PPS_SENSE) != OFF && TIME_LOCATION_PPS_REQUIRED == ON
    if (pps.synced) {
  #endif

  if (!tlsGps->isReady()) tlsGps->poll();

  #if (TIME_LOCATION_PPS_SENSE) != OFF && TIME_LOCATION_PPS_REQUIRED == ON
    }
  #endif
}

// initialize
bool TlsGPS::init() {
  tlsGps = this;

  #if defined(SERIAL_GPS_RX) && defined(SERIAL_GPS_TX) && !defined(SERIAL_GPS_RXTX_SET)
    SERIAL_GPS.begin(SERIAL_GPS_BAUD, SERIAL_8N1, SERIAL_GPS_RX, SERIAL_GPS_TX);
  #else
    SERIAL_GPS.begin(SERIAL_GPS_BAUD);
  #endif
  activeLink = true;

  // check to see if the GPS is present
  tasks.yield(1000);
  if (!SERIAL_GPS.available()) {
    DLF("WRN: TLS, GPS serial RX interface is quiet!");
    return false;
  } else {
    unsigned long timeout = millis() + 1000U;
    while (SERIAL_GPS.available() > 0) {
      if (gps.encode(SERIAL_GPS.read())) break;
      if ((long)(millis() - timeout) > 0) {
        DLF("WRN: TLS, GPS serial RX interface no NMEA sentences detected!");
        return false;
      }
      Y;
    }
  }

  // show the raw data as received, garbled characters point to a signal (or baud rate) problem
  #if DEBUG == VERBOSE
    char raw[121];
    int n = 0;
    unsigned long rawTimeout = millis() + 1500U;
    while ((long)(millis() - rawTimeout) < 0 && n < 120) {
      while (SERIAL_GPS.available() > 0 && n < 120) {
        char c = SERIAL_GPS.read();
        gps.encode(c);
        if (c == '\r') continue;
        raw[n++] = (c == '\n') ? '|' : ((c >= 32 && c < 127) ? c : '~');
      }
      Y;
    }
    raw[n] = 0;
    VF("MSG: TLS, GPS raw data: "); VL(raw);
    VF("MSG: TLS, GPS sentences so far good="); V(gps.passedChecksum()); VF(" bad="); VL(gps.failedChecksum());
  #endif

  VF("MSG: TLS, GPS start monitor task (rate 10ms priority 7)... ");
  if (tasks.add(10, 0, true, 7, gpsPoll, "gpsPoll")) { VLF("success"); active = true; } else { VLF("FAILED!"); }

  return active;
}

void TlsGPS::set(JulianDate ut1) {
  ut1 = ut1;
}

void TlsGPS::set(int year, int month, int day, int hour, int minute, int second) {
  #ifdef TLS_TIMELIB
    setTime(hour, minute, second, day, month, year);
  #else
    (void)year; (void)month; (void)day; (void)hour; (void)minute; (void)second;
  #endif
}

bool TlsGPS::get(JulianDate &ut1) {
  if (!ready) return false;
  if (!timeIsValid()) return false;

  // get the date/time right now if the serial link is active
  bool sentence = true;
  if (activeLink) {
    while (SERIAL_GPS.available() > 0) {
      sentence = gps.encode(SERIAL_GPS.read());
    }
  }

  GregorianDate greg;
  greg.year = gps.date.year();
  greg.month = gps.date.month();
  greg.day = gps.date.day();
  ut1 = calendars.gregorianToJulianDay(greg);
  // DUT1 = UT1 − UTC
  // UT1 = DUT1 + UTC

  double second = gps.time.second() + gps.time.age()/1000.0;
  #if defined(TIME_LOCATION_PPS_SENSE) && (TIME_LOCATION_PPS_SENSE) != OFF
    // only use PPS if a pulse arrived within the last second, otherwise the wait below would never end
    if (pps.synced && micros() - pps.lastMicros < 1000000UL) {
      unsigned long t;
      unsigned long start = millis();
      // wait until we're roughly in the middle of a second (give up after 1.5s if the pulses stop)
      do { t = micros() - pps.lastMicros; } while ((t < 400000 || t > 600000) && millis() - start < 1500UL);
      if (t >= 400000 && t <= 600000) second = floor(gps.time.second() + gps.time.age()/1000.0) + t/1000000.0;
    }
  #endif

  ut1.hour = gps.time.hour() + gps.time.minute()/60.0 + (second + DUT1)/3600.0;

  // adjust date/time for DUT1 as needed
  if (ut1.hour >= 24.0L) { ut1.hour -= 24.0L; ut1.day += 1.0L; } else
  if (ut1.hour < 0.0L) { ut1.hour += 24.0L; ut1.day -= 1.0L; }

  // shutdown the link if the date/time is current
  if (sentence && activeLink) {
    VLF("MSG: TLS, closing GPS serial port");
    SERIAL_GPS.end();
    activeLink = false;
  }

  return sentence;
}

void TlsGPS::getSite(double &latitude, double &longitude, float &elevation) {
  if (!ready) return;
  if (!siteIsValid()) return;

  latitude = gps.location.lat();
  longitude = -gps.location.lng();
  elevation = gps.altitude.meters();
}

void TlsGPS::poll() {
  while (SERIAL_GPS.available() > 0) {
    gps.encode(SERIAL_GPS.read());
  }

  if (gps.location.isValid() && siteIsValid()) {
    if (gps.date.isValid() && gps.time.isValid() && timeIsValid()) {
      if (waitIsValid()) {
        VLF("MSG: TLS, GPS date/time/location is ready");

        VLF("MSG: TLS, stopping GPS monitor task");
        tasks.setDurationComplete(tasks.getHandleByName("gpsPoll"));

        #ifdef TLS_TIMELIB
          setTime(gps.time.hour(), gps.time.minute(), gps.time.second(), gps.date.day(), gps.date.month(), gps.date.year());
        #endif

        ready = true;
      }
    }
  }
}

// starts keeping track of the wait once (PPS is synced, if applicable) and GPS has a lock 
bool TlsGPS::waitIsValid() {
  if (startTime == 0) startTime = millis();
  unsigned long t = millis() - startTime;
  return (t/1000UL)/60UL >= GPS_MIN_WAIT_MINUTES;
}

bool TlsGPS::timeIsValid() {
  if (gps.date.year() <= 3000 && gps.date.month() >= 1 && gps.date.month() <= 12 && gps.date.day() >= 1 && gps.date.day() <= 31 &&
      gps.time.hour() <= 23 && gps.time.minute() <= 59 && gps.time.second() <= 59) return true; else return false;
}

bool TlsGPS::siteIsValid() {
  if (gps.location.lat() >= -90 && gps.location.lat() <= 90 && gps.location.lng() >= -360 && gps.location.lng() <= 360) return true; else return false;
}

#endif
