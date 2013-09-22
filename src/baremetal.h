#ifndef baremetal_h
#define baremetal_h

//Readability defines
//Interrupts
#define CLEAR_PENDING_INTERRUPTS EIFR = 0
#define ENABLE_INTERRUPTS sei()
#define DISABLE_INTERRUPTS cli()
//Timers
#define STOP_TIMER TCCR1B &= 0B11111000 //Clear CS12,11,10 I think this is clearer than doing the shift over invert thing
#define CLEAR_TIMER TCNT1 = 0B00000000
#define START_TIMER TCCR1B |= (1 << CS10) | (1 << CS12) //Set prescaler to 1024, and start timer
//SPI
#define SDI 2
#define CKI 3 
#define SS 10
#define LED_PIN 13 //On board LED

//Kill MCU via watchdog and subequent infinite loop
void reset();
//Initialize SPI hardware
void hard_spi_init();
//Initialize timer 
void initialize_timer();
//Clear relevant watchdog registers
void clear_watchdog();
//Initialize everything in a sane manner
void setup_baremetal();

#endif
