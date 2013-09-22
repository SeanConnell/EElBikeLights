#ifndef colors_h
#define colors_h

#define STRIP_LENGTH 32 //32 LEDs on this strip
#define N_COLORS 3
#define SINE_LENGTH 1000
#define TRIANGLE_LENGTH 502

#include "Arduino.h"

typedef struct {
        uint8_t red;
        uint8_t grn;
        uint8_t blu;
} COLOR;

typedef struct {
        uint16_t length;
        uint8_t* data;
} TABLE;

typedef struct {
        uint8_t step_size;
        uint16_t loc;
} ITERATOR;

//Iterate a data array in a circle
uint8_t get_next_value(ITERATOR * itr, TABLE * values);
//mutate color to be dimmed by a mask value 
void dim_color(COLOR * color, ITERATOR * dim_itr, TABLE * dim_table);
//Copy color values from one struct to another
void copy_color(COLOR * from, COLOR * to);

#endif
