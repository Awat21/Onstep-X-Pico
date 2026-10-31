// -----------------------------------------------------------------------------------
// Debug log, keeps the most recent debug output in RAM so it can be read back with :GXLG
// (use "#define SERIAL_DEBUG debugLog" and a DEBUG_LOG_SIZE in bytes to enable)
#pragma once

#include <Arduino.h>

#ifndef DEBUG_LOG_SIZE
  #define DEBUG_LOG_SIZE 0
#endif

#if DEBUG_LOG_SIZE > 0

// number of log characters returned by one :GXLG command
#define DEBUG_LOG_READ_LEN 50

class DebugLog : public Print {
  public:
    void begin(long baud) { (void)baud; }
    operator bool() { return true; }

    size_t write(uint8_t c) override;
    using Print::write;

    // copies up to len - 1 characters of the log into out, starting at position "from" (the count of
    // characters written since boot), '#' is replaced since it terminates command replies
    // start returns the position the copy actually starts at (later than from if that part was overwritten,
    // earlier if from is past the end, for instance after a restart) and next the position after the copy
    void read(uint32_t from, char *out, size_t len, uint32_t *start, uint32_t *next);

  private:
    void put(char c);

    char buffer[DEBUG_LOG_SIZE];
    uint32_t head = 0;      // count of characters written since boot
    bool lineStart = true;
};

extern DebugLog debugLog;

#endif
