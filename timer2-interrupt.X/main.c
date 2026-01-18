/*
 * File:   main.c
 * Author: dylwh
 *
 * Created on May 25, 2022, 4:44 PM
 * 
 * This program will utilize the Timer2 module and an interrupt service
 * routine to blink two LED's at different rates.
 * 
 */

// configure the settings of the MCU
#include "CONFIG.h"

// define oscillator frequency
#define _XTAL_FREQ 4000000

// define ISR - will blink RD0 LED with 10kHz square wave
void __interrupt() blink_isr(void) {
    
    if (TMR2IF == 1) {
        RD0     = !RD0;      // toggle LED
        TMR2IF  = 0;         // clear interrupt flag
    }
    
}

void main(void) {
    
    // configure inputs/outputs
    TRISD0  = 0;                // pin D0 is an output
    TRISC2  = 0;                // pin C2 is an output
    
    // configure timer2 (timer2 speed: Fosc/4 = 1MHz)
    // Interrupt blink period = (4/Fosc) * PR2 * Pre * Post (around 0.065s)
    T2CON   = 0b01111110;       // timer2 on, 1:16 pre- and post-scaling
    PR2     = 255;               // set period register
    PIE1    = 0b00000010;       // enable timer2 interrupt
    INTCON  = 0b11000000;       // enable interrupts
    
    while(1){
        
        // blink RC2 led every second using simple delays
        RC2 = 1;
        __delay_ms(500);
        RC2 = 0;
        __delay_ms(500);
        
    }    
    
}
