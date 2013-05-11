
#ifndef lights_h
#define lights_h

#include "Arduino.h"

//States for the system
#define INITIAL         0
#define DATA_TRANSFORM  1
#define PERFORM_ACTIONS 2
#define DATA_GATHERING  3
#define CLEAR_STATE     4
#define RESTART         5

//Readability defines
#define CLEAR_PENDING_INTERRUPTS EIFR = 0
#define ENABLE_INTERRUPTS sei()
#define DISABLE_INTERRUPTS cli()
#define STOP_TIMER TCCR1B &= 0B11111000 //Clear CS12,11,10 I think this is clearer than doing the shift over invert thing
#define CLEAR_TIMER TCNT1 = 0B00000000
#define START_TIMER TCCR1B |= (1 << CS10) | (1 << CS12) //Set prescaler to 1024, and start timer

#define ALCOHOL_EFFECT_LENGTH 16
#define SPEED_EFFECT_LENGTH 6

#define SDI 2
#define CKI 3 
#define SS 10
#define LED_PIN 13 //On board LED

#define STRIP_LENGTH 32 //32 LEDs on this strip
#define N_COLORS 3
#define TABLE_LENGTH 1000

#endif
