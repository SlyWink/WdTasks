#include "wdtasks.h"
#include <avr/interrupt.h>
#include <avr/wdt.h>

#if WDTASKS_SLEEP == 1
#include <avr/sleep.h>
#endif

#define WDTASK_ENABLED 1
#define WDTASK_PAUSED  2

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
#if WDTASKS_SLEEP == 1
  set_sleep_mode(SLEEP_MODE_IDLE) ;
#endif
}


void WdTask_Init(uint8_t p_tasknum, uint8_t p_trigger, void *p_callback) {
  WDTASK *l_wdt ;

  l_wdt = &(_wdsched.tasks[p_tasknum]) ;
  l_wdt->trigger = p_trigger ;
  l_wdt->callback = p_callback ;
  l_wdt->flags = 0 ;
}


void WdTask_Enable(uint8_t p_tasknum) {
  WDTASK *l_wdt ;

  l_wdt = &(_wdsched.tasks[p_tasknum]) ;
  l_wdt->ticks = 0 ;
  l_wdt->flags |= WDTASK_ENABLED ;
}


void WdTask_Disable(uint8_t p_tasknum) {
  _wdsched.tasks[p_tasknum].flags &= ~WDTASK_ENABLED ;
}


void WdTask_Pause(uint8_t p_tasknum) {
  _wdsched.tasks[p_tasknum].flags |= WDTASK_PAUSED ;
}


void WdTask_Unpause(uint8_t p_tasknum) {
  _wdsched.tasks[p_tasknum].flags &= ~WDTASK_PAUSED ;
}


uint8_t WdTask_IsEnabled(uint8_t p_tasknum) {
  return _wdsched.tasks[p_tasknum].flags & WDTASK_ENABLED ;
}


int8_t WdSched_CurrentTask(void) {
  return _wdsched.current ;
}


void WdSched_Run(void) {
  static uint8_t l_wdticks = 0 ;
  WDTASK *l_wdt ;

  if (l_wdticks != _wd_ticks) {
    for (_wdsched.current=0 ; _wdsched.current<_wdsched.taskcount ; _wdsched.current++) {
      l_wdt = &(_wdsched.tasks[_wdsched.current]) ;
      if ((l_wdt->flags & (WDTASK_ENABLED|WDTASK_PAUSED)) != WDTASK_ENABLED) continue ;
      if (++(l_wdt->ticks) == l_wdt->trigger) {
        (*(l_wdt->callback))() ;
        l_wdt->ticks = 0 ;
      }
    }
    l_wdticks = _wd_ticks ;
#if WDTASKS_SLEEP == 1
    sleep_mode() ;
#endif
  }
}
