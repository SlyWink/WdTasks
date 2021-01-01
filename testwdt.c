#include "wdtasks.h"

#define WDTASKS_LIGHT_SLEEP

#define PIN_LED1 PB1
#define PIN_LED2 PB2
#define PIN_LED3 PB3
#define PIN_LED4 PB4

#define TASK_COUNT 4
#define TASK1 0
#define TASK2 1
#define TASK3 2
#define TASK4 3

WDTASK g_tasks[TASK_COUNT] ;

void cbTk1(void) {
  static uint8_t l_led = 0 ;

  l_led = 1 - l_led ;
  if (l_led) PORTB |= _BV(PIN_LED1) ;
    else PORTB &= ~_BV(PIN_LED1) ;
}

void cbTk2(void) {
  static uint8_t l_led = 0 ;

  l_led = 1 - l_led ;
  if (l_led) {
    PORTB |= _BV(PIN_LED2) ;
    WdTask_Pause(TASK3) ;
  } else {
    PORTB &= ~_BV(PIN_LED2) ;
    WdTask_Unpause(TASK3) ;
  }
}

void cbTk3(void) {
  static uint8_t l_led = 0 ;

  l_led = 1 - l_led ;
  if (l_led) PORTB |= _BV(PIN_LED3) ;
    else PORTB &= ~_BV(PIN_LED3) ;
}

void cbTk4(void) {
  static uint8_t l_led = 0 ;

  l_led = 1 - l_led ;
  if (l_led) PORTB |= _BV(PIN_LED4) ;
    else PORTB &= ~_BV(PIN_LED4) ;
  if (WdSched_FirstCall()) WdTask_SetTrigger(WdSched_CurrentTask(),10) ;
}


void Init_Pins(void) {
  DDRB = _BV(PIN_LED1) | _BV(PIN_LED2) | _BV(PIN_LED3) | _BV(PIN_LED4) ;
  PORTB = ~(_BV(PIN_LED1) | _BV(PIN_LED2) | _BV(PIN_LED3) | _BV(PIN_LED4)) ;
}

void Init_Tasks(void) {
  WdSched_Init(g_tasks,TASK_COUNT,WDTO_60MS) ;

  WdTask_Init(TASK1,25,&cbTk1) ;
  WdTask_Init(TASK2,150,&cbTk2) ;
  WdTask_Init(TASK3,10,&cbTk3) ;
  WdTask_Init(TASK4,100,&cbTk4) ;
  WdTask_Enable(TASK1) ;
  WdTask_Enable(TASK2) ;
  WdTask_Enable(TASK3) ;
  WdTask_EnableDelayed(TASK4) ;
}


int main(void) {
  Init_Pins() ;
  Init_Tasks() ;
  for (;;) {
    WdSched_Run() ;
  }
  return 1 ;
}
