/*
 * File:   main.c
 * Author: dylwh
 *
 * Created on Jan 17, 2026, 12:45 PM
 * 
 * This program will use the I2C module to interface with a BMP180 sensor and
 * print out some readings periodically.  
 * 
 */

// configure the settings of the MCU
#include "CONFIG.h"

// define oscillator frequency
#define _XTAL_FREQ 4000000

// variable to be transmitted
int number = 0;

void main(void) {
    
    // configure inputs/outputs
    TRISD0  = 0;                // pin D0 is an output (LED feedback)
    TRISC5  = 0;                // pin C5 is an output (SDO pin)
    TRISC3  = 0;                // pin C3 is an output (SCK pin)
    TRISD1  = 0;                // pin to send slave select signal to AD2
    RD1     = 0;                // begin in idle state
    
    INTCON  = 0b11000000;       // enable interrupts
    
    // configure SPI registers - configure for Master mode only first
    SSPCON  = 0b00100000;   // no col., clock low = idle, clock = Fosc/4
    SSPSTAT = 0b01000000;   // sample in middle, TX on active to idle
    
    while(1){
        
        // increment number to be sent
        number++;
        
        // reset if more than 8 bits
        if (number == 255)
            number = 0;
        
        // indicate data is to be sent (send slave select high)
        RD1     = 1;
        
        // add number to be sent to buffer for transmission
        SSPBUF  = number;
        
        // wait until data finishes transmitting
        while(!SSPIF);
        SSPIF   = 0;    // reset interrupt bit
        
        // indicate no more data to be sent
        RD1     = 0;
        
        // transmit roughly 5 times a second
        __delay_ms(200);
        
        // blink LED for feedback
        RD0     = 1;
        __delay_ms(5);
        RD0     = 0;
        
    }    
    
}
