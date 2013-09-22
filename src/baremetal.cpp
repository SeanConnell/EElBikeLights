#include "SPI.h"
#include "Arduino.h"
#include "avr/wdt.h"
#include "baremetal.h"

void setup_baremetal(){
    clear_watchdog();
    hard_spi_init();
    randomSeed(analogRead(0));
    pinMode(LED_PIN, OUTPUT);
    initialize_timer();
}

void reset(){
    wdt_enable(WDTO_1S); 
    while(1);//wait for the watchdog to kill us
}

void hard_spi_init(){
    SPI.begin();
    SPI.setBitOrder(MSBFIRST);
    SPI.setClockDivider(SPI_CLOCK_DIV2);
    SPI.setDataMode(1);//Rising edge clock, normal data polarities
    pinMode(SS, OUTPUT);//keep from getting accidentally reset
}

void initialize_timer(){
    TCCR1A = 0;
    TCCR1B = 0;
    // 1 second is 15623 + 1
    OCR1A = 250; //compare mode with 1 second intervals at this clock rate of 16mhz and prescale of 1024
    TCCR1B |= (1 << WGM12); //CTC mode
    TIMSK1 |= (1 << OCIE1A); //enable timer compare interrupt
}

void clear_watchdog(){
    // Clear the reset bit
    MCUSR &= ~_BV(WDRF);
    // Disable the WDT
    WDTCSR |= _BV(WDCE) | _BV(WDE); 
    WDTCSR = 0;
}
