#include "wdtasks_def.h"
#include <avr/wdt.h>

#define WDTASK_ENABLED 1

typedef struct {
  uint8_t flags ;
  uint8_t ticks ;
  uint8_t trigger ;
  void    (*callback)(void) ;
} WDTASK ;

void WdSched_Init(WDTASK *p_tasks, uint8_t p_count, uint8_t p_wdtime) ;
void WdTask_Init(uint8_t p_tasknum, uint8_t p_trigger, void *p_callback) ;
void WdTask_Enable(uint8_t p_tasknum) ;
void WdTask_Disable(uint8_t p_tasknum) ;
void WdTask_Pause(uint8_t p_tasknum) ;
void WdTask_Unpause(uint8_t p_tasknum) ;
uint8_t WdTask_IsEnabled(uint8_t p_tasknum) ;
int8_t WdSched_CurrentTask(void) ;
void WdSched_Run(void) ;
