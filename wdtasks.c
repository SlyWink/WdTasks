#include "wdtasks.h"
#include <avr/interrupt.h>
#include <avr/wdt.h>

#ifndef WDTASKS_COUNT
#warning "WDTASKS_COUNT undefined -> set to 10"
#define WDTASKS_COUNT 10
#endif

#define WDTASK_ENABLED 1
#define WDTASK_PAUSED  2

struct {
  WDTASK  tasks[WDTASKS_COUNT] ;
  uint8_t taskcount ;
} _wdsched ;

static volatile uint8_t _wd_ticks = 0 ;


#define WdTask_Idx2Ptr(idx) &(_wdsched.tasks[idx])


ISR(WDT_vect) {
  WDTCR |= _BV(WDIE) | _BV(WDE) ;
  _wd_ticks++ ;
}


void _Set_WdTimebase(uint8_t p_wdp) {
  if (p_wdp & 8) p_wdp = _BV(WDP3) | (p_wdp & 7) ;
  WDTCR &= ~(_BV(WDP3) | _BV(WDP2) | _BV(WDP1) | _BV(WDP0)) ;
  WDTCR |= p_wdp ;
}


void WdSched_Init(uint8_t p_wdtime) {
  _wdsched.taskcount = 0 ;
  _Set_WdTimebase(p_wdtime) ;
  sei() ;
  WDTCR |= _BV(WDIE) | _BV(WDE) ;
}


uint8_t WdTask_New(uint8_t p_trigger, void *p_callback) {
  WDTASK *l_wdt ;

  if (_wdsched.taskcount >= WDTASKS_COUNT) return 0 ;
  l_wdt = WdTask_Idx2Ptr(_wdsched.taskcount) ;
  l_wdt->trigger = p_trigger ;
  l_wdt->callback = p_callback ;
  l_wdt->flags = 0 ;
  l_wdt->taskid = ++_wdsched.taskcount ;
  return _wdsched.taskcount ;
}


void WdTask_Enable(uint8_t p_taskid) {
  WDTASK *l_wdt ;

  l_wdt = WdTask_Idx2Ptr(p_taskid-1) ;
  l_wdt->ticks = 0 ;
  l_wdt->flags |= WDTASK_ENABLED ;
}


void WdTask_Disable(uint8_t p_taskid) {
  _wdsched.tasks[p_taskid-1].flags &= ~WDTASK_ENABLED ;
}


void WdTask_Pause(uint8_t p_taskid) {
  _wdsched.tasks[p_taskid-1].flags |= WDTASK_PAUSED ;
}


void WdTask_Unpause(uint8_t p_taskid) {
  _wdsched.tasks[p_taskid-1].flags &= ~WDTASK_PAUSED ;
}


uint8_t WdTask_IsEnabled(uint8_t p_taskid) {
  return _wdsched.tasks[p_taskid-1].flags & WDTASK_ENABLED ;
}


void WdSched_Run(void) {
  static uint8_t l_wdticks = 0 ;
  uint8_t l_index ;
  WDTASK *l_wdt ;

  if (l_wdticks != _wd_ticks) {
    for (l_index=0 ; l_index<_wdsched.taskcount ; l_index++) {
      l_wdt = WdTask_Idx2Ptr(l_index) ;
      if ((l_wdt->flags & (WDTASK_ENABLED|WDTASK_PAUSED)) != WDTASK_ENABLED) continue ;
      if (++(l_wdt->ticks) == l_wdt->trigger) {
        (*(l_wdt->callback))() ;
        l_wdt->ticks = 0 ;
      }
    }
    l_wdticks = _wd_ticks ;
  }
}
