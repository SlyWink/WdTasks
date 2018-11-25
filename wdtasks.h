#include "wdtasks_def.h"
#include <avr/wdt.h>

#define WDTASK_ENABLED 1

typedef struct {
  uint8_t flags ;
  uint8_t taskid ;
  uint8_t ticks ;
  uint8_t trigger ;
  void    (*callback)(void) ;
} WDTASK ;

void WdSched_Init(uint8_t p_wdtime) ;
uint8_t WdTask_New(uint8_t p_trigger, void *p_callback) ;
void WdTask_Enable(uint8_t p_taskid) ;
void WdTask_Disable(uint8_t p_taskid) ;
uint8_t WdTask_IsEnabled(uint8_t p_taskid) ;
void WdTask_Pause(uint8_t p_taskid) ;
void WdTask_Unpause(uint8_t p_taskid) ;
void WdSched_Run(void) ;
