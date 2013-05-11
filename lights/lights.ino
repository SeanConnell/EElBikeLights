/*
 * Code to control an RGB LED strip. Meant to be used for my bicycle 
 * lighting project.
 * 
 * Author: Sean Connell, based on sample code from Nate @ Sparkfun
 * https://www.sparkfun.com/datasheets/Components/LED/LED_Strip_Example.pde
 * (I owe you a beer, Nate.)
 * Relevant Hardware: Any arduino MCU, RGB led strip: 
 * https://www.sparkfun.com/products/11272
 */

#include "lights.h"
#include "avr/wdt.h"
#include <SPI.h>

//Represents the state of each color on each RGB LED
uint8_t curr_output[STRIP_LENGTH][N_COLORS];
//The buffer we mix our effects into before shifting them out
uint8_t draw_buffer[STRIP_LENGTH][N_COLORS];

//The effect the alcohol gas sensor causes
uint8_t alcohol_effect[ALCOHOL_EFFECT_LENGTH][N_COLORS] = {
    {0,1,1},
    {0,1,2},
    {0,5,2},
    {0,7,2},
    {0,9,2},
    {0,12,2},
    {0,14,2},
    {0,15,4},
    {0,14,5},
    {0,12,6},
    {0,9,7},
    {0,7,9},
    {0,5,12},
    {0,3,13},
    {0,2,15},
    {0,1,18}
};

//The effect the speed sensor causes
uint8_t speed_effect[SPEED_EFFECT_LENGTH][N_COLORS] = {
    {40,30,25},
    {55,36,15},
    {130,100,70},
    {215,165,130},
    {195,170,120},
    {12,8,10}
};

uint8_t black[3] = {0, 0, 0};
uint8_t some_color[3] = {43, 206, 97};

//Interrupt set scaling variables for the effect patterns
volatile uint8_t n_mix_alcohol = 1;
volatile uint8_t n_mix_speed = 1;
volatile uint8_t STATE = 0;
uint8_t gas_inject = 0;
uint8_t alcohol_inject = 0;

void setup(){
    clear_watchdog();
    pinMode(LED_PIN, OUTPUT);
    hard_spi_init();
    //attachInterrupt(0, pulses_counter, CHANGE);
    initialize_timer();
    STATE = DATA_GATHERING;
    ENABLE_INTERRUPTS;
    START_TIMER;
}

//The data gathering window has ended and we should move on to mixing colors
ISR(TIMER1_COMPA_vect){  
    if(STATE != DATA_GATHERING){
        /* We somehow got to the end of our data gathering window without resetting state
        Might be a bad calc somewhere in the DATA_TRANSFORM step
        Try to restart and hope that it doesn't happen again */ 
        STOP_TIMER;
        STATE = RESTART; 
        DISABLE_INTERRUPTS;//We're going to bail out and restart. Don't need to worry about the world
    }

    else{
        STOP_TIMER;
        STATE = DATA_TRANSFORM;
        DISABLE_INTERRUPTS;//Don't want to collect any data while we copy some values
    }
}

void loop(){ 
    /* State machine transitions and actions that 
    aren't time critical or are too heavy to do
    in interrupts are done here. */
    switch(STATE){
        //Not concerned with DATA_GATHERING 
        case DATA_GATHERING:
            break;
          
        //If the data is ready to process 
        //mix effects into the draw buffer
        case DATA_TRANSFORM:
            if(gas_inject == 71){
                mix_into_buffer(alcohol_effect, ALCOHOL_EFFECT_LENGTH, n_mix_alcohol, draw_buffer);
                gas_inject = 0;
            }else{
                gas_inject++;
            }
            if(alcohol_inject == 251){
                mix_into_buffer(speed_effect, SPEED_EFFECT_LENGTH, n_mix_speed, draw_buffer);
                alcohol_inject = 0;
            }else{
                alcohol_inject++;
            }
            STATE = PERFORM_ACTIONS;
            digitalWrite(LED_PIN, !digitalRead(LED_PIN));
            break;
        
        //push the draw buffer head out onto the output buffer
        //and then push a new black color onto the draw frame
        case PERFORM_ACTIONS:
            push_color_into_buffer(draw_buffer[STRIP_LENGTH-1], curr_output, STRIP_LENGTH); 
            push_color_into_buffer(black, draw_buffer, STRIP_LENGTH);
            //push_color_into_buffer(some_color, curr_output, STRIP_LENGTH);
            post_frame(); //update display to reflect new buffer state
            STATE = CLEAR_STATE;
            break;
        
        //reset everything to be ready for the next cycle
        case CLEAR_STATE:
            CLEAR_PENDING_INTERRUPTS;
            STATE = DATA_GATHERING;
            CLEAR_TIMER;
            START_TIMER;
            ENABLE_INTERRUPTS;
            break;
          
        case RESTART:
            reset();
            
        default:
            reset();
    }
}

//Mixes a source into a buffer and performs limiting to avoid overflow
//from buffer len must be <= to buffer len
void mix_into_buffer(uint8_t from[][N_COLORS], 
                     uint8_t from_len, 
                     uint8_t enhancer, 
                     uint8_t to_sum[][N_COLORS]){
    for(uint8_t i=0; i<from_len; i++){
        add_color(from[i], enhancer, to_sum[i]);
    }
}

void add_color(uint8_t *add, uint8_t enhancer, uint8_t *accumulator){
    uint16_t sum;
    for(uint8_t j=0; j<N_COLORS; j++){
        sum = (add[j] * enhancer) + accumulator[j];
        if(sum >= 256){
            sum = 255;//"limit" to max value. May compress later instead of hard limit
        }
        accumulator[j] = sum;
    }
}

void copy_color(uint8_t *from, uint8_t *to){
    for(uint8_t i=0; i<N_COLORS; i++){
        to[i] = from[i];
    }
}

void push_color_into_buffer(uint8_t *next_color, 
                            uint8_t buffer[][N_COLORS], 
                            uint16_t buf_len){
    //move all the current colors down one spot
    for(uint8_t x = (buf_len -1); x > 0; x--){
        copy_color(buffer[x-1], buffer[x]);
    }
    //Add new color to the back of the queue
    copy_color(next_color, buffer[0]);
}

//Hardware communication via SPI
void send_byte_hard(uint8_t byte){
    SPI.transfer(byte);
}

//utility function to push output
void post_frame(void) {
    for(int LED_number = 0; LED_number < STRIP_LENGTH; LED_number++) {
        for(uint8_t color = 0; color < N_COLORS; color++){
            uint8_t current_led = curr_output[LED_number][color];
            send_byte_hard(current_led);
        }
    }
    //Pull clock low to put strip into reset/post mode
    digitalWrite(CKI, LOW);
    delayMicroseconds(1000); //Wait for data to latch
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
