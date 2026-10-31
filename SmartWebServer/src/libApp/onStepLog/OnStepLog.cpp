// -----------------------------------------------------------------------------------
// Collects OnStepX's debug log (:GXLG) in the background so the Log tile can show it

#include "OnStepLog.h"

#include "../../Common.h"
#include "../../lib/tasks/OnTask.h"
#include "../cmd/Cmd.h"
#include "../status/Status.h"

void pollOnStepLog() { onStepLog.poll(); }

void OnStepLog::init() {
  VF("MSG: OnStepLog, start polling task (rate "); V(ONSTEP_LOG_POLLING_RATE_MS); VF("ms, priority 7)... ");
  if (tasks.add(ONSTEP_LOG_POLLING_RATE_MS, 0, true, 7, pollOnStepLog, "LogPoll")) { VLF("success"); } else { VLF("FAILED!"); }
}

void OnStepLog::poll() {
  if (!status.onStepFound || status.getVersionMajor() < 10) return;

  // if OnStep didn't support the command try again once a minute (it may have been updated)
  if (!supported) {
    if ((long)(millis() - retryTime) < 0) return;
    supported = true;
  }

  // up to 40 chunks (about 2K) per poll, enough to catch up with startup messages quickly
  for (int i = 0; i < 40; i++) {
    char cmd[24], reply[80];
    snprintf(cmd, sizeof(cmd), ":GXLG,%lu#", (unsigned long)onStepPos);
    if (!onStep.command(cmd, reply)) return;

    // reply is s,e|text
    char *bar = strchr(reply, '|');
    char *comma = strchr(reply, ',');
    if (bar == NULL || comma == NULL || comma > bar) {
      supported = false;
      retryTime = millis() + 60000UL;
      return;
    }
    *bar = 0;
    uint32_t start = strtoul(reply, NULL, 10);
    uint32_t next = strtoul(comma + 1, NULL, 10);

    if (start > onStepPos) append("\n--- part of the OnStep log was lost (OnStep's buffer overflowed) ---\n"); else
    if (start < onStepPos) append("\n--- OnStep restarted ---\n");

    append(bar + 1);
    if (next == start) { onStepPos = next; return; }
    onStepPos = next;
    Y;
  }
}

void OnStepLog::append(const char *s) {
  while (*s) {
    buffer[head % ONSTEP_LOG_SIZE] = *s++;
    head++;
  }
}

uint32_t OnStepLog::read(uint32_t from, String &data, size_t maxLen, uint32_t *start) {
  uint32_t oldest = head > ONSTEP_LOG_SIZE ? head - ONSTEP_LOG_SIZE : 0;
  if (oldest < tail) oldest = tail;
  if (from > head || from < oldest) from = oldest;
  *start = from;

  size_t n = 0;
  while (from < head && n < maxLen) {
    data.concat(buffer[from % ONSTEP_LOG_SIZE]);
    from++;
    n++;
  }
  return from;
}

uint32_t OnStepLog::clear() {
  tail = head;
  return head;
}

OnStepLog onStepLog;
