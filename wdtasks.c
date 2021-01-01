#include "wdtasks.h"
#include <avr/interrupt.h>
#include <avr/wdt.h>

#if defined(WDTASKS_LIGHT_SLEEP) || defined(WDTASKS_DEEP_SLEEP)
#include <avr/sleep.h>
#endif

#define WDTASK_FLAG_ENABLED 1
#define WDTASK_FLAG_PAUSED  2
#define WDTASK_FLAG_DELAYED 4
#define WDTASK_FLAG_CALLED  8

struct {
  WDTASK  *tasks ;
  uint8_t taskcount ;
  int8_t current ;
} _wdsched ;

static volatile uint8_t _wd_ticks = 0 ;


ISR(WDT_vect) {
  WDTCR |= _BV(WDIE) | _BV(WDE) ;
  _wd_ticks++ ;
}


void _Set_WdTimebase(uint8_t p_wdp) {
  if (p_wdp & 8) p_wdp = _BV(WDP3) | (p_wdp & 7) ;
  WDTCR &= ~(_BV(WDP3) | _BV(WDP2) | _BV(WDP1) | _BV(WDP0)) ;
  WDTCR |= p_wdp ;
}


void WdSched_Init(WDTASK *p_tasks, uint8_t p_count, uint8_t p_wdtime) {
  _wdsched.tasks = p_tasks ;
  _wdsched.taskcount = p_count ;
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


void WdTask_Init(uint8_t p_tasknum, uint8_t p_trigger, void *p_callback) {
  WDTASK *l_wdt ;

  l_wdt = &(_wdsched.tasks[p_tasknum]) ;
  l_wdt->trigger = p_trigger ;
  l_wdt->callback = p_callback ;
  l_wdt->flags = 0 ;
}


void _WdTask_EnableFlagged(uint8_t p_tasknum, uint8_t p_flag) {
  WDTASK *l_wdt ;

  l_wdt = &(_wdsched.tasks[p_tasknum]) ;
  l_wdt->ticks = 0 ;
  l_wdt->flags |= (WDTASK_FLAG_ENABLED | p_flag) ;
}


void WdTask_Enable(uint8_t p_tasknum) {
  _WdTask_EnableFlagged(p_tasknum,0) ;
}


void WdTask_EnableDelayed(uint8_t p_tasknum) {
  _WdTask_EnableFlagged(p_tasknum,WDTASK_FLAG_DELAYED) ;
}


void WdTask_Disable(uint8_t p_tasknum) {
  _wdsched.tasks[p_tasknum].flags &= ~WDTASK_FLAG_ENABLED ;
}


void WdTask_Pause(uint8_t p_tasknum) {
  _wdsched.tasks[p_tasknum].flags |= WDTASK_FLAG_PAUSED ;
}


void WdTask_Unpause(uint8_t p_tasknum) {
  _wdsched.tasks[p_tasknum].flags &= ~WDTASK_FLAG_PAUSED ;
}


uint8_t WdTask_IsEnabled(uint8_t p_tasknum) {
  if (p_tasknum >= _wdsched.taskcount) return 0 ;
  return _wdsched.tasks[p_tasknum].flags & WDTASK_FLAG_ENABLED ;
}


uint8_t WdSched_FirstCall(void) {
  return ((_wdsched.tasks[_wdsched.current].flags & WDTASK_FLAG_CALLED) == 0) ;
}


void WdTask_SetTrigger(uint8_t p_tasknum, uint8_t p_trigger) {
  WDTASK *l_wdt ;

  l_wdt = &(_wdsched.tasks[p_tasknum]) ;
  l_wdt->trigger = p_trigger ;
  l_wdt->ticks = 0 ;
}


int8_t WdSched_CurrentTask(void) {
  return _wdsched.current ;
}


uint8_t WdSched_Ticks(void) {
  return _wd_ticks ;
}


void WdSched_Run(void) {
  static uint8_t l_wdticks = 0 ;
  WDTASK *l_wdt ;

  if (l_wdticks != _wd_ticks) {
    for (_wdsched.current=0 ; _wdsched.current<_wdsched.taskcount ; _wdsched.current++) {
      l_wdt = &(_wdsched.tasks[_wdsched.current]) ;
      if ((l_wdt->flags & (WDTASK_FLAG_ENABLED|WDTASK_FLAG_PAUSED)) != WDTASK_FLAG_ENABLED) continue ;
      if (!l_wdt->ticks) {
        if (l_wdt->flags & WDTASK_FLAG_DELAYED)
          l_wdt->flags &= ~WDTASK_FLAG_DELAYED ;
        else {
          (*(l_wdt->callback))() ;
          l_wdt->flags |= WDTASK_FLAG_CALLED ;
        }
      }
      if (++(l_wdt->ticks) == l_wdt->trigger) l_wdt->ticks = 0 ;
    }
    l_wdticks = _wd_ticks ;
#if defined(WDTASKS_LIGHT_SLEEP) || defined(WDTASKS_DEEP_SLEEP)
    sleep_mode() ;
#endif
  }
}
