//#include "wdtasks_def.h"
#include <avr/wdt.h>

typedef struct {
  uint8_t flags ;
  uint8_t ticks ;
  uint8_t trigger ;
  void    (*callback)(void) ;
} WDTASK ;

void WdSched_Init(WDTASK *p_tasks, uint8_t p_wdtime) ;
int8_t WdTask_Init(uint8_t p_trigger, uint8_t p_priority, void *p_callback) ;
void WdTask_Enable(int8_t p_tasknum) ;
void WdTask_EnableDelayed(int8_t p_tasknum) ;
void  WdTask_Disable(int8_t p_tasknum) ;
void WdTask_Pause(int8_t p_tasknum) ;
void  WdTask_Unpause(int8_t p_tasknum) ;
uint8_t WdTask_IsEnabled(int8_t p_tasknum) ;
uint8_t WdSched_FirstCall(void) ;
void WdTask_SetTrigger(int8_t p_tasknum, uint8_t p_trigger) ;
int8_t WdSched_CurrentTask(void) ;
#ifdef WDTASKS_CLOCK_16
uint16_t WdSched_Clock(void) ;
#elif defined WDTASKS_CLOCK_32
uint32_t WdSched_Clock(void) ;
#else
uint8_t WdSched_Clock(void) ;
#endif
void WdSched_Run(void) ;
