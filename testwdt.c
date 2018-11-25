#include "wdtasks.h"

#define PIN_LED1 PB1
#define PIN_LED2 PB2
#define PIN_LED3 PB3
#define PIN_LED4 PB4

uint8_t tk1, tk2, tk3, tk4 ;


void cbTk1(void) {
  static uint8_t l_led = 0 ;

  l_led = 1 - l_led ;
  if (l_led) PORTB |= _BV(PIN_LED1) ;
    else PORTB &= ~_BV(PIN_LED1) ;
}

void cbTk2(void) {
  static uint8_t l_led = 0 ;

  l_led = 1 - l_led ;
  if (l_led) PORTB |= _BV(PIN_LED2) ;
    else PORTB &= ~_BV(PIN_LED2) ;
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
}


void Init_Pins(void) {
  DDRB = _BV(PIN_LED1) | _BV(PIN_LED2) | _BV(PIN_LED3) | _BV(PIN_LED4) ;
  PORTB = ~(_BV(PIN_LED1) | _BV(PIN_LED2) | _BV(PIN_LED3) | _BV(PIN_LED4)) ;
}

void Init_Tasks(void) {
  WdSched_Init(WDTO_250MS) ;
  tk1 = WdTask_New(1,&cbTk1) ;
  tk2 = WdTask_New(50,&cbTk2) ;
  tk3 = WdTask_New(10,&cbTk3) ;
  tk4 = WdTask_New(40,&cbTk4) ;
  WdTask_Enable(tk1) ;
  WdTask_Enable(tk2) ;
  WdTask_Enable(tk3) ;
  WdTask_Enable(tk4) ;
}


int main(void) {
  Init_Pins() ;
  Init_Tasks() ;
  for (;;) {
    WdSched_Run() ;
  }
  return 1 ;
}
