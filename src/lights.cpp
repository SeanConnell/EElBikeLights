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

#include "Arduino.h"
#include "avr/wdt.h"
#include <SPI.h>
#include "baremetal.h"
#include "colors.h"
#include "lights.h"
#include "lookup_tables.h"
#include "command_dispatcher.h"

//colors offset by 120 degrees
ITERATOR red_itr = { 1, 0 };
ITERATOR grn_itr = { 1, 333 };
ITERATOR blu_itr = { 1, 666 };
ITERATOR dim_itr = { 2, 231 };
ITERATOR dly_itr = { 10, 0 };

void increment_frame(void) {
    //First, move all the current colors down one spot on the strip
    for(uint8_t x = (STRIP_LENGTH -1); x > 0; x--){
        copy_color(&strip_colors[x-1], &strip_colors[x]);
    }
    COLOR * next_color = &strip_colors[0];
    next_color->red = get_next_value(&red_itr, &sine_table);
    next_color->grn = get_next_value(&grn_itr, &sine_table);
    next_color->blu = get_next_value(&blu_itr, &sine_table);
    dim_color(next_color, &dim_itr, &triangle_table);//apply dimming effect
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
	setup_baremetal();
	red_itr.loc = random(1000);
	grn_itr.loc = random(1000);
	blu_itr.loc = random(1000);
}

void loop(){
    while(WORLD_EXISTS){ 
        increment_frame(); // get new data and setup buffer with next state
        push_frame(); //update display to reflect new buffer state
        delay((get_next_value(&dly_itr, &triangle_table)>>3) + 5);//adjusts update rate
    }
}
