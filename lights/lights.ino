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
uint16_t slowdown_position = 0;
uint16_t dimming_position = 231;//offset since it's using the delay table, to keep things interesting

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
        delay(calculate_next_value(&slowdown_position, slowdown_table, SLOWDOWN_LENGTH, 5));//adjusts update rate
    }
}

void copy_color(uint8_t *from, uint8_t *to){
    for(uint8_t i=0; i<N_COLORS; i++){
        to[i] = from[i];
    }
}

void increment_frame(void) {
    //First, move all the current colors down one spot on the strip
    
    for(uint8_t x = (STRIP_LENGTH -1); x > 0; x--){
        copy_color(strip_colors[x-1], strip_colors[x]);
    }
    //Add new color to the head of the queue
    strip_colors[0][0] = calculate_next_value(
        &table_position[0], sinewave_table, TABLE_LENGTH, 10);
    strip_colors[0][1] = calculate_next_value(
        &table_position[1], sinewave_table, TABLE_LENGTH, 10);
    strip_colors[0][2] = calculate_next_value(
        &table_position[2], sinewave_table, TABLE_LENGTH, 10);
    adjust_frame_value(strip_colors[0]);//apply dimming effect
}

//mutate current output value to be adjusted down by some amount
void adjust_frame_value(uint8_t* value){
	uint16_t res[3];
	uint8_t div = calculate_next_value(&dimming_position, slowdown_table, SLOWDOWN_LENGTH, 1);
	for(uint8_t v=0; v<3; v++){
	res[v] = value[v] << 8;
	res[v] /= (div>>5);
	res[v] = res[v] >> 8;     
	value[v] = (uint8_t) res[v];
    }
}

//Currently just loops around a data.
uint8_t calculate_next_value(uint16_t *index, uint8_t * data, uint16_t data_size, uint8_t step_size){
    *index = *index + step_size;
    if(*index >= data_size){
        *index = *index - data_size;
    }
    return data[*index]>>2;
}

//Hardware communication. A lot faster than software implementation.
void send_byte_hard(uint8_t byte){
    SPI.transfer(byte);
}

void push_frame(void) {
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
