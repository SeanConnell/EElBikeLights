/*
 * Code to control an RGB LED strip.
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


//colors offset by 120 degrees
ITERATOR red_itr = { 1, 0 };
ITERATOR grn_itr = { 1, 333 };
ITERATOR blu_itr = { 1, 666 };
ITERATOR dim_itr = { 2, 231 };
ITERATOR dly_itr = { 10, 0 };

void hard_spi_init(){
    SPI.begin();
    SPI.setBitOrder(MSBFIRST);
    SPI.setClockDivider(SPI_CLOCK_DIV2);
    SPI.setDataMode(1);//Rising edge clock, normal data polarities
    pinMode(SS, OUTPUT);//keep from getting accidentally reset
}

//Iterate a data array in a circle
uint8_t get_next_value(ITERATOR * itr, TABLE * values){
	itr->loc = itr->loc + itr->step_size;
	if(itr->loc >= values->length){
		itr->loc = itr->loc - values->length;
	}
	return values->data[itr->loc]; 
}

//mutate color to be dimmed by a mask value 
void dim_color(COLOR * color){
	uint8_t mask = get_next_value(&dim_itr, &triangle_table);
	color->red &= mask;
	color->grn &= mask;
	color->blu &= mask;
}

void copy_color(COLOR * from, COLOR * to){
       to->red = from->red;
       to->grn = from->grn;
       to->blu = from->blu; 
}

void increment_frame(void) {
    //First, move all the current colors down one spot on the strip
    for(uint8_t x = (STRIP_LENGTH -1); x > 0; x--){
        copy_color(&strip_colors[x-1], &strip_colors[x]);
    }
    COLOR * next_color = &strip_colors[0];
    next_color->red = get_next_value(&red_itr, &sine_table);
    next_color->grn = get_next_value(&grn_itr, &sine_table);
    next_color->blu = get_next_value(&blu_itr, &sine_table);
    dim_color(next_color);//apply dimming effect
}

void push_frame(void) {
    //Send all values of LEDs in order
    for(int LED_number = 0; LED_number < STRIP_LENGTH; LED_number++) {
	SPI.transfer(strip_colors[LED_number].red);
	SPI.transfer(strip_colors[LED_number].grn);
	SPI.transfer(strip_colors[LED_number].blu);
    }
    //Pull clock low to put strip into reset/post mode
    digitalWrite(CKI, LOW);
    delayMicroseconds(1000); //Wait for data to latch
}

void setup(){
	hard_spi_init();
}

void loop(){
    while(WORLD_EXISTS){ 
        increment_frame(); // get new data and setup buffer with next state
        push_frame(); //update display to reflect new buffer state
        delay((get_next_value(&dly_itr, &triangle_table)>>3) + 5);//adjusts update rate
    }
}
