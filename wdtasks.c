#include "wdtasks.h"
#include <avr/interrupt.h>
#include <avr/wdt.h>

#if defined(WDTASKS_LIGHT_SLEEP) || defined(WDTASKS_DEEP_SLEEP)
#include <avr/sleep.h>
#endif

struct {
  WDTASK  *tasks ;
  uint8_t taskinfos ;
  int8_t current ;
} _wdsched ;

#define WDSCHED_TASK_COUNT_BITS 5
#define WDSCHED_TASK_COUNT_LAST 31
#define WDSCHED_TASK_COUNT_MASK 0b00011111

#define _WdSched_IncrTaskCount() _wdsched.taskinfos++
#define _WdSched_GetTaskCount()  (_wdsched.taskinfos & WDSCHED_TASK_COUNT_MASK)

#define WDSCHED_PRIORITY_BITS 3
#define WDSCHED_PRIORITY_LAST 7
#define WDSCHED_PRIORITY_MASK 0b11100000

#define _WdSched_GetHighestPriority()     ((_wdsched.taskinfos & WDSCHED_PRIORITY_MASK) >> WDSCHED_TASK_COUNT_BITS)
#define _WdSched_SetHighestPriority(prio) _wdsched.taskinfos = ((_wdsched.taskinfos & WDSCHED_TASK_COUNT_MASK) | (prio << WDSCHED_TASK_COUNT_BITS))

#define WDTASK_PRIORITY_MASK 0b00000111
#define WDTASK_FLAG_ENABLED  0b00010000
#define WDTASK_FLAG_PAUSED   0b00100000
#define WDTASK_FLAG_DELAYED  0b01000000
#define WDTASK_FLAG_CALLED   0b10000000

#define _WdTask_GetPriority(taskptr) (taskptr->flags & WDTASK_PRIORITY_MASK)

#ifdef WDTASKS_CLOCK_16
static volatile uint16_t _wdsched_clock = 0 ;
#elif defined WDTASKS_CLOCK_32
static volatile uint32_t _wdsched_clock = 0 ;
#else
static volatile uint8_t _wdsched_clock = 0 ;
#endif


ISR(WDT_vect) {
  WDTCR |= _BV(WDIE) | _BV(WDE) ;
  _wdsched_clock++ ;
}


void _Set_WdTimebase(uint8_t p_wdp) {
  if (p_wdp & 8) p_wdp = _BV(WDP3) | (p_wdp & 7) ;
  WDTCR &= ~(_BV(WDP3) | _BV(WDP2) | _BV(WDP1) | _BV(WDP0)) ;
  WDTCR |= p_wdp ;
}


void WdSched_Init(WDTASK *p_tasks, uint8_t p_wdtime) {
  _wdsched.tasks = p_tasks ;
  _wdsched.current = -1 ;
  _Set_WdTimebase(p_wdtime) ;
  sei() ;
  WDTCR |= _BV(WDIE) | _BV(WDE) ;
#ifdef WDTASKS_LIGHT_SLEEP
  set_sleep_mode(SLEEP_MODE_IDLE) ;
#elif defined(WDTASKS_DEEP_SLEEP)
  set_sleep_mode(SLEEP_MODE_PWR_DOWN) ;
#endif
}


int8_t WdTask_Init(uint8_t p_trigger, uint8_t p_priority, void *p_callback) {
  int8_t l_tasknum ;
  WDTASK *l_wdt ;

  l_tasknum = _WdSched_GetTaskCount() ;
  if ((l_tasknum == WDSCHED_TASK_COUNT_LAST) || (p_priority > WDSCHED_PRIORITY_LAST)) return -1 ;
  l_wdt = &(_wdsched.tasks[l_tasknum]) ;
  l_wdt->trigger = p_trigger ;
  l_wdt->callback = p_callback ;
  l_wdt->flags = p_priority ;
  if (p_priority > _WdSched_GetHighestPriority()) _WdSched_SetHighestPriority(p_priority) ;
  _WdSched_IncrTaskCount() ;
  return l_tasknum ;
}


void _WdTask_EnableFlagged(int8_t p_tasknum, uint8_t p_flag) {
  WDTASK *l_wdt ;

  l_wdt = &(_wdsched.tasks[p_tasknum]) ;
  l_wdt->ticks = 0 ;
  l_wdt->flags |= (WDTASK_FLAG_ENABLED | p_flag) ;
}


void WdTask_Enable(int8_t p_tasknum) {
  _WdTask_EnableFlagged(p_tasknum,0) ;
}


void WdTask_EnableDelayed(int8_t p_tasknum) {
  _WdTask_EnableFlagged(p_tasknum,WDTASK_FLAG_DELAYED) ;
}


void WdTask_Disable(int8_t p_tasknum) {
  _wdsched.tasks[p_tasknum].flags &= ~WDTASK_FLAG_ENABLED ;
}


void WdTask_Pause(int8_t p_tasknum) {
  _wdsched.tasks[p_tasknum].flags |= WDTASK_FLAG_PAUSED ;
}


void WdTask_Unpause(int8_t p_tasknum) {
  _wdsched.tasks[p_tasknum].flags &= ~WDTASK_FLAG_PAUSED ;
}


uint8_t WdTask_IsEnabled(int8_t p_tasknum) {
  if (p_tasknum >= _WdSched_GetTaskCount()) return 0 ;
  return _wdsched.tasks[p_tasknum].flags & WDTASK_FLAG_ENABLED ;
}


uint8_t WdSched_FirstCall(void) {
  return ((_wdsched.tasks[_wdsched.current].flags & WDTASK_FLAG_CALLED) == 0) ;
}


void WdTask_SetTrigger(int8_t p_tasknum, uint8_t p_trigger) {
  WDTASK *l_wdt ;

  l_wdt = &(_wdsched.tasks[p_tasknum]) ;
  l_wdt->trigger = p_trigger ;
  l_wdt->ticks = 0 ;
}


int8_t WdSched_CurrentTask(void) {
  return _wdsched.current ;
}


#ifdef WDTASKS_CLOCK_16
uint16_t WdSched_Clock(void) {
#elif defined WDTASKS_CLOCK_32
uint32_t WdSched_Clock(void) {
#else
uint8_t WdSched_Clock(void) {
#endif
  return _wdsched_clock ;
}


void WdSched_Run(void) {
#ifdef WDTASKS_CLOCK_16
  static uint16_t l_clock = 0 ;
#elif defined WDTASKS_CLOCK_32
  static uint32_t l_clock = 0 ;
#else
  static uint8_t l_clock = 0 ;
#endif
  static uint8_t l_priority = 0 ;
  WDTASK *l_wdt ;

  if (l_clock != _wdsched_clock) {
    for (_wdsched.current=0 ; _wdsched.current<_WdSched_GetTaskCount() ; _wdsched.current++) {
      l_wdt = &(_wdsched.tasks[_wdsched.current]) ;
      if ((l_wdt->flags & (WDTASK_FLAG_ENABLED|WDTASK_FLAG_PAUSED)) != WDTASK_FLAG_ENABLED) continue ;
      if (!l_wdt->ticks) {
        if (l_priority >= _WdTask_GetPriority(l_wdt)) {
          if (l_wdt->flags & WDTASK_FLAG_DELAYED)
            l_wdt->flags &= ~WDTASK_FLAG_DELAYED ;
          else {
            (*(l_wdt->callback))() ;
            l_wdt->flags |= WDTASK_FLAG_CALLED ;
          }
          l_wdt->ticks = 1 ;
        }
      } else
        l_wdt->ticks++ ;
      if (l_wdt->ticks == l_wdt->trigger) l_wdt->ticks = 0 ;
    }
    l_clock = _wdsched_clock ;
    if (++l_priority > _WdSched_GetHighestPriority()) l_priority = 0 ;
#if defined(WDTASKS_LIGHT_SLEEP) || defined(WDTASKS_DEEP_SLEEP)
    sleep_mode() ;
#endif
  }
}
