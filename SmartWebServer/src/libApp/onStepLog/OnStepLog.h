// -----------------------------------------------------------------------------------
// Collects OnStepX's debug log (:GXLG) in the background so the Log tile can show it
#pragma once

#include <Arduino.h>

#ifndef ONSTEP_LOG_SIZE
  #define ONSTEP_LOG_SIZE 32768   // bytes of RAM, the most recent part of the log is kept
#endif

#ifndef ONSTEP_LOG_POLLING_RATE_MS
  #define ONSTEP_LOG_POLLING_RATE_MS 1000
#endif

class OnStepLog {
  public:
    void init();

    // fetch any new log text from OnStep
    void poll();

    // copies up to maxLen characters starting at position "from" (count of characters collected since boot)
    // into data, start returns where the copy actually starts (later than from if that part was overwritten)
    // and the return value is the position after the copy
    uint32_t read(uint32_t from, String &data, size_t maxLen, uint32_t *start);

    // start the log over, returns the new position
    uint32_t clear();

    // false if OnStep doesn't answer :GXLG (firmware without the debug log)
    bool supported = true;

  private:
    void append(const char *s);

    char buffer[ONSTEP_LOG_SIZE];
    uint32_t head = 0;            // count of characters collected
    uint32_t tail = 0;            // characters before this were cleared
    uint32_t onStepPos = 0;       // next position to ask OnStep for
    unsigned long retryTime = 0;
};

extern OnStepLog onStepLog;
