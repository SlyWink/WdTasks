#include <avr/cpufunc.h>
#include "wdtasks.h"

#define WDTASKS_LIGHT_SLEEP

#define PIN_LED1 PB1
#define PIN_LED2 PB2
#define PIN_LED3 PB3
#define PIN_LED4 PB4

#define TASK_COUNT 5

WDTASK g_tasks[TASK_COUNT] ;

int8_t g_tk3 ;


void cbTk1(void) {
  PORTB ^= _BV(PIN_LED1) ;
}


void cbTk2(void) {
  PORTB ^= _BV(PIN_LED2) ;
  _NOP() ;
  if (PINB & _BV(PIN_LED2)) WdTask_Pause(g_tk3) ;
    else WdTask_Unpause(g_tk3) ;
}


void cbTk3(void) {
  PORTB ^= _BV(PIN_LED3) ;
}


void cbTk4(void) {
  PORTB ^= _BV(PIN_LED4) ;
  if (WdSched_FirstCall()) WdTask_SetTrigger(WdSched_CurrentTask(),10) ;
}


void cbTkStart(void) {
  static int8_t l_task = 0 ;

  switch(++l_task) {
    case 1 :
      PORTB |= _BV(PIN_LED1) ; break ;
    case 2 :
      PORTB |= _BV(PIN_LED2) ; break ;
    case 3 :
      PORTB |= _BV(PIN_LED3) ; break ;
    case 4 :
      PORTB |= _BV(PIN_LED4) ; break ;
    case 5 :
      PORTB &= ~(_BV(PIN_LED1) | _BV(PIN_LED2) | _BV(PIN_LED3) | _BV(PIN_LED4)) ; break ;
    default :
      WdTask_Enable(0) ;
      WdTask_Enable(1) ;
      WdTask_Enable(2) ;
      WdTask_EnableDelayed(3) ;
      WdTask_Disable(WdSched_CurrentTask()) ;
  }
}


void Init_Pins(void) {
  DDRB = _BV(PIN_LED1) | _BV(PIN_LED2) | _BV(PIN_LED3) | _BV(PIN_LED4) ;
  PORTB = ~(_BV(PIN_LED1) | _BV(PIN_LED2) | _BV(PIN_LED3) | _BV(PIN_LED4)) ;
}

void Init_Tasks(void) {
  WdSched_Init(g_tasks,WDTO_60MS) ;
  WdTask_Init(25,0,&cbTk1) ;
  WdTask_Init(150,1,&cbTk2) ;
  g_tk3 = WdTask_Init(10,0,&cbTk3) ;
  WdTask_Init(100,2,&cbTk4) ;
  WdTask_Enable(WdTask_Init(8,0,&cbTkStart)) ;
}


int main(void) {
  Init_Pins() ;
  Init_Tasks() ;
  for (;;) {
    WdSched_Run() ;
  }
  return 1 ;
}
