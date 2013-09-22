#include "colors.h"

//Iterate a data array in a circle
uint8_t get_next_value(ITERATOR * itr, TABLE * values){
        itr->loc = itr->loc + itr->step_size;
        if(itr->loc >= values->length){
                itr->loc = itr->loc - values->length;
        }
        return values->data[itr->loc];
}

//mutate color to be dimmed by a mask value 
void dim_color(COLOR * color, ITERATOR * dim_itr, TABLE * dim_table){
        uint8_t mask = get_next_value(dim_itr, dim_table);
        color->red &= mask;
        color->grn &= mask;
        color->blu &= mask;
}

void copy_color(COLOR * from, COLOR * to){
       to->red = from->red;
       to->grn = from->grn;
       to->blu = from->blu;
}
