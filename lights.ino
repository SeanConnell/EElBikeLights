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
#include <SPI.h>

//Represents the state of each color on each RGB LED
uint8_t strip_colors[STRIP_LENGTH][N_COLORS];
//colors offset by 120 degrees
uint16_t table_position[N_COLORS] = { 0, 333, 666};
uint16_t slowdown_factor = 0;

void setup(){
    soft_spi_init();
    hard_spi_init();
}


void soft_spi_init(){
    pinMode(SDI, OUTPUT);
    pinMode(CKI, OUTPUT);
}

void hard_spi_init(){
    SPI.begin();
    SPI.setBitOrder(MSBFIRST);
    SPI.setClockDivider(SPI_CLOCK_DIV2);
    SPI.setDataMode(1);//Rising edge clock, normal data polarities
    pinMode(SS, OUTPUT);//keep from getting accidentally reset
}

void loop(){
    while(1){ 
        increment_frame(); // get new data and setup buffer with next state
        push_frame(); //update display to reflect new buffer state
        delay(slowdown_factor); //TODO adjust this for effect
    }
}

void move_color(uint8_t *from, uint8_t *to){
    for(uint8_t i=0; i<N_COLORS; i++){
        to[i] = from[i];
    }
}

void increment_frame(void) {
    //First, move all the current colors down one spot on the strip
    for(uint8_t x = 0; x < (STRIP_LENGTH -1); x++){
        move_color(strip_colors[x], strip_colors[x+1]);
    }
    
    //Add new color to the head of the queue
    strip_colors[0][0] = calculate_next_color(
        &table_position[0], sinewave_table, TABLE_LENGTH);
    strip_colors[0][1] = calculate_next_color(
        &table_position[1], sinewave_table, TABLE_LENGTH);
    strip_colors[0][2] = calculate_next_color(
        &table_position[2], sinewave_table, TABLE_LENGTH);
}

//Currently just loops around a data.
//TODO: Use subsample to allow stretching of data
uint8_t calculate_next_color(uint16_t *index, uint8_t * data, uint16_t data_size){
    const uint16_t i = *index + 1;
    const uint8_t value = data[i];
    *index = i;
    if(*index >= data_size){
        *index = 0;
    }
    return value;
}

//TODO: implement a linearly interpolated subsampler
uint8_t subsample(uint8_t stepsize, uint16_t * position, uint8_t * data){
}

//Bitbanged communication with the LED strip
void send_byte_soft(uint8_t byte){
    for(uint8_t color_bit = 8 ; color_bit != 0 ; color_bit--) {
      digitalWrite(CKI, LOW); //Only change data when clock is low
      
      uint8_t mask = 1 << (color_bit - 1);

      if(byte & mask) 
        digitalWrite(SDI, HIGH);
      else
        digitalWrite(SDI, LOW);
  
      digitalWrite(CKI, HIGH); //Data is latched when clock goes high
    }
}

//Hardware communication. A lot faster than software implementation.
void send_byte_hard(uint8_t byte){
    SPI.transfer(byte);
}

void push_frame(void) {
  //Each LED requires 24 bits of data
  //MSB: R7, R6, R5..., G7, G6..., B7, B6... B0 
  //Once the 24 bits have been delivered, the IC immediately relays these bits to its neighbor
  //Pulling the clock low for > 500us + some wiggle causes the IC to post the data.

    for(int LED_number = 0; LED_number < STRIP_LENGTH; LED_number++) {
        for(uint8_t color = 0; color < N_COLORS; color++){
            uint8_t current_led = strip_colors[LED_number][color]; //24 bits of color data
            send_byte_hard(current_led);
        }
    }
    //Pull clock low to put strip into reset/post mode
    digitalWrite(CKI, LOW);
    delayMicroseconds(1000); //Wait for data to latch
}
