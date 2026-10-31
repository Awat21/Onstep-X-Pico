// -----------------------------------------------------------------------------------
// Debug log, keeps the most recent debug output in RAM so it can be read back with :GXLG

#include "Debug.h"

#if DEBUG_LOG_SIZE > 0

DebugLog debugLog;

// each line starts with the time since boot, [   12.345]
size_t DebugLog::write(uint8_t c) {
  if (c == '\r') return 1;
  if (lineStart) {
    char stamp[24];
    unsigned long ms = millis();
    snprintf(stamp, sizeof(stamp), "[%5lu.%03lu] ", ms/1000UL, ms%1000UL);
    for (char *p = stamp; *p; p++) put(*p);
    lineStart = false;
  }
  put((char)c);
  if (c == '\n') lineStart = true;
  return 1;
}

void DebugLog::put(char c) {
  buffer[head % DEBUG_LOG_SIZE] = c;
  head++;
}

void DebugLog::read(uint32_t from, char *out, size_t len, uint32_t *start, uint32_t *next) {
  uint32_t oldest = head > DEBUG_LOG_SIZE ? head - DEBUG_LOG_SIZE : 0;
  if (from > head) from = oldest;    // asked for more than was ever written, OnStepX must have restarted
  if (from < oldest) from = oldest;  // that part was overwritten, start at the oldest still available
  *start = from;

  size_t n = 0;
  while (from < head && n < len - 1) {
    char c = buffer[from % DEBUG_LOG_SIZE];
    out[n++] = (c == '#') ? '*' : c;
    from++;
  }
  out[n] = 0;
  *next = from;
}

#endif
