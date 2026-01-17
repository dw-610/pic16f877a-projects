/*
 * File:   main.c
 * Author: dylwh
 *
 * Created on May 19, 2022, 4:44 PM
 * 
 * This program shows how to read in a digital input and trigger
 * a digital output based on this input.
 */

// configure the settings of the MCU
#include "CONFIG.h"

// define oscillator frequency
#define _XTAL_FREQ 4000000

// define pins and digital switch states
#define LED             RD0
#define SWITCH          RD1
#define SWITCH_PRESSED  0sy

// define function to read button press and turn on light
void buttonRead(void) {
    if (SWITCH == SWITCH_PRESSED) {
        LED = 1;
        __delay_ms(10);         // add debounce time
    }
    else {
        LED = 0;
        __delay_ms(10);         // add debounce time
    }
}

void main(void) {
    
    // configure inputs/outputs
    TRISD1  = 1;                // D1 is in input mode
    TRISD0  = 0;                // D0 is in output mode
    
    while(1){
        
        // call function to read state of button and turn LED on/off
        buttonRead();
        
    }     
}
