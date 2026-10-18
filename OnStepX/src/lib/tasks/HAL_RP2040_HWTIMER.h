//--------------------------------------------------------------------------------------------------
// Raspberry Pi RP2040/RP2350 hardware timers (Earle Philhower Arduino-Pico core)

// uses the 64 bit 1MHz system timer and its hardware alarms, one alarm is claimed per timer
// the default alarm pool of the Pico SDK holds one alarm, which leaves three for our use
// each period is accumulated in 1/16us so the average rate is exact even though alarms are 1us

#include "hardware/timer.h"
#include "hardware/sync.h"

#define TIMER_RATE_MHZ 1L
#define TIMER_RATE_16MHZ_TICKS 16L

#if defined(TASKS_HWTIMER1_ENABLE) || defined(TASKS_HWTIMER2_ENABLE) || defined(TASKS_HWTIMER3_ENABLE) || defined(TASKS_HWTIMER4_ENABLE)
  typedef struct Rp2040HwTimer {
    int alarm;                         // hardware alarm number or -1 if not claimed
    volatile uint32_t nextPeriod;      // period in 1/16us waiting to be adopted
    volatile uint16_t nextRep;         // 0 = stopped (callback not run), 1 = running
    volatile uint32_t period;          // period in 1/16us in use
    volatile uint64_t target;          // time of the next alarm in 1/16us
  } Rp2040HwTimer;

  Rp2040HwTimer _rpTimer[4] = { {-1, 16000, 0, 16000, 0}, {-1, 16000, 0, 16000, 0}, {-1, 16000, 0, 16000, 0}, {-1, 16000, 0, 16000, 0} };

  // prepare hw timer for interval in sub-microseconds (1/16us)
  void HAL_HWTIMER_PREPARE_PERIOD(uint8_t num, unsigned long period) {
    uint32_t counts;
    uint16_t reps = 0;
    if (period != 0 && period <= 2144000000) {
      if (period < 16) period = 16;    // minimum time is 1us
      counts = period;
      reps = 1;
    } else counts = 16000;             // set for a 1ms period, stopped

    uint32_t s = save_and_disable_interrupts();
    _rpTimer[num - 1].nextPeriod = counts;
    _rpTimer[num - 1].nextRep = reps;
    restore_interrupts(s);
  }

  // set the alarm for the current target, if that time has already passed fire as soon as possible
  static inline void _rpTimerArm(Rp2040HwTimer *t) {
    while (hardware_alarm_set_target(t->alarm, from_us_since_boot(t->target >> 4))) {
      t->target = (time_us_64() + 2) << 4;
    }
  }

  // start a timer, returns false if no hardware alarm is available
  static bool _rpTimerInit(Rp2040HwTimer *t, hardware_alarm_callback_t callback) {
    if (t->alarm < 0) t->alarm = hardware_alarm_claim_unused(false);
    if (t->alarm < 0) return false;
    hardware_alarm_set_callback(t->alarm, callback);
    uint32_t s = save_and_disable_interrupts();
    t->period = 16000;                 // startup one millisecond
    t->target = (time_us_64() << 4) + t->period;
    _rpTimerArm(t);
    restore_interrupts(s);
    return true;
  }

  static void _rpTimerDone(Rp2040HwTimer *t) {
    if (t->alarm < 0) return;
    hardware_alarm_cancel(t->alarm);
    hardware_alarm_set_callback(t->alarm, NULL);
    hardware_alarm_unclaim(t->alarm);
    t->alarm = -1;
  }

  // adopt the next period, the timer was just serviced so the next alarm follows the last
  static inline void _rpTimerNext(Rp2040HwTimer *t) {
    t->period = t->nextPeriod;
    t->target += t->period;
    _rpTimerArm(t);
  }

  // adopt the next period right away (called outside of the ISR)
  static void _rpTimerSetPeriod(Rp2040HwTimer *t) {
    if (t->alarm < 0) return;
    uint32_t s = save_and_disable_interrupts();
    t->target = t->target - t->period + t->nextPeriod;
    t->period = t->nextPeriod;
    _rpTimerArm(t);
    restore_interrupts(s);
  }
#else
  void HAL_HWTIMER_PREPARE_PERIOD(uint8_t num, unsigned long period) { (void)(num); (void)(period); }
#endif

#ifdef TASKS_HWTIMER1_ENABLE
  void (*HAL_HWTIMER1_FUN)() = NULL; // points to task/process callback function
  void HAL_HWTIMER1_WRAPPER(uint alarm); // forward definition of the timer ISR

  bool HAL_HWTIMER1_INIT(uint8_t priority) {
    (void)(priority);
    return _rpTimerInit(&_rpTimer[0], HAL_HWTIMER1_WRAPPER);
  }

  void HAL_HWTIMER1_DONE() {
    HAL_HWTIMER1_FUN = NULL;
    _rpTimerDone(&_rpTimer[0]);
  }

  #define HAL_HWTIMER1_SET_PERIOD() _rpTimerSetPeriod(&_rpTimer[0])
  void HAL_HWTIMER1_WRAPPER(uint alarm) {
    (void)(alarm);
    TASKS_HWTIMER1_PROFILER_PREFIX;
    if (_rpTimer[0].nextRep) HAL_HWTIMER1_FUN();
    _rpTimerNext(&_rpTimer[0]);
    TASKS_HWTIMER1_PROFILER_SUFFIX;
  }
#endif

#ifdef TASKS_HWTIMER2_ENABLE
  void (*HAL_HWTIMER2_FUN)() = NULL; // points to task/process callback function
  void HAL_HWTIMER2_WRAPPER(uint alarm); // forward definition of the timer ISR

  bool HAL_HWTIMER2_INIT(uint8_t priority) {
    (void)(priority);
    return _rpTimerInit(&_rpTimer[1], HAL_HWTIMER2_WRAPPER);
  }

  void HAL_HWTIMER2_DONE() {
    HAL_HWTIMER2_FUN = NULL;
    _rpTimerDone(&_rpTimer[1]);
  }

  #define HAL_HWTIMER2_SET_PERIOD() _rpTimerSetPeriod(&_rpTimer[1])
  void HAL_HWTIMER2_WRAPPER(uint alarm) {
    (void)(alarm);
    TASKS_HWTIMER2_PROFILER_PREFIX;
    if (_rpTimer[1].nextRep) HAL_HWTIMER2_FUN();
    _rpTimerNext(&_rpTimer[1]);
    TASKS_HWTIMER2_PROFILER_SUFFIX;
  }
#endif

#ifdef TASKS_HWTIMER3_ENABLE
  void (*HAL_HWTIMER3_FUN)() = NULL; // points to task/process callback function
  void HAL_HWTIMER3_WRAPPER(uint alarm); // forward definition of the timer ISR

  bool HAL_HWTIMER3_INIT(uint8_t priority) {
    (void)(priority);
    return _rpTimerInit(&_rpTimer[2], HAL_HWTIMER3_WRAPPER);
  }

  void HAL_HWTIMER3_DONE() {
    HAL_HWTIMER3_FUN = NULL;
    _rpTimerDone(&_rpTimer[2]);
  }

  #define HAL_HWTIMER3_SET_PERIOD() _rpTimerSetPeriod(&_rpTimer[2])
  void HAL_HWTIMER3_WRAPPER(uint alarm) {
    (void)(alarm);
    TASKS_HWTIMER3_PROFILER_PREFIX;
    if (_rpTimer[2].nextRep) HAL_HWTIMER3_FUN();
    _rpTimerNext(&_rpTimer[2]);
    TASKS_HWTIMER3_PROFILER_SUFFIX;
  }
#endif

#ifdef TASKS_HWTIMER4_ENABLE
  void (*HAL_HWTIMER4_FUN)() = NULL; // points to task/process callback function
  void HAL_HWTIMER4_WRAPPER(uint alarm); // forward definition of the timer ISR

  bool HAL_HWTIMER4_INIT(uint8_t priority) {
    (void)(priority);
    return _rpTimerInit(&_rpTimer[3], HAL_HWTIMER4_WRAPPER);
  }

  void HAL_HWTIMER4_DONE() {
    HAL_HWTIMER4_FUN = NULL;
    _rpTimerDone(&_rpTimer[3]);
  }

  #define HAL_HWTIMER4_SET_PERIOD() _rpTimerSetPeriod(&_rpTimer[3])
  void HAL_HWTIMER4_WRAPPER(uint alarm) {
    (void)(alarm);
    TASKS_HWTIMER4_PROFILER_PREFIX;
    if (_rpTimer[3].nextRep) HAL_HWTIMER4_FUN();
    _rpTimerNext(&_rpTimer[3]);
    TASKS_HWTIMER4_PROFILER_SUFFIX;
  }
#endif
