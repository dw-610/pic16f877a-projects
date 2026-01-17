/*
 * File:   main.c
 * Author: dylwh
 *
 * Created on June 1, 2022
 * 
 * This program will use the sleep function of the PIC16f877a to blink an LED
 * 
 */

// configure the settings of the MCU (note that watchdog is enabled in CONFIG)
#include "CONFIG.h"

// define oscillator frequency
#define _XTAL_FREQ 4000000

// define pins and digital switch states
#define LED         RD0

// define counter to set time delay
int counter     = 0;

void main(void) {
    
    // configure inputs/outputs - set all I/O ports as outputs
    TRISA   = 0x00;
    TRISB   = 0x00;
    TRISC   = 0x00;
    TRISD   = 0x00;
    TRISE   = 0x00;
    
    // Set all I/O pins low
    PORTA   = 0x00;
    PORTB   = 0x00;
    PORTC   = 0x00;
    PORTD   = 0x00;
    PORTE   = 0x00;
    
    
    
    // set watchdog postscaler value
    OPTION_REG = 0b11111110;        // 1:64 postscaler (~1.1s per interrupt)
    
    while(1){
        
        // increment counter
        counter++;
        
        // Toggle LED every ~3 seconds
        if (counter == 3) {
            LED     = !LED;
            counter = 0;
        }
        
        // Go into LOW-POWER mode until watchdog timer interrupt
        SLEEP();
        
    }     
}
