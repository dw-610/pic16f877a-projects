/*
 * File:   main.c
 * Author: dylwh
 *
 * Created on May 19, 2022, 4:44 PM
 * 
 * This program shows how to use PWM (pin CCP1) to control the brightness of
 *  an LED, based on an analog input.
 * 
 */

// configure the settings of the MCU
#include "CONFIG.h"

// define oscillator frequency
#define _XTAL_FREQ 4000000

// define ADC bit configurations
//      use logical statements to isolate single bits

// leave all but bit 2 the same, sets bit 2 high to start sampling process
#define ADC_START_CONVERSION (ADCON0 |= 0b00000100)
// will be all zeros except for bit 2, which will be 0 when sampling finished
#define ADC_DONE (~ADCON0 & 0x04)


// define function to sample the ADC and write PWM duty cycle to CCP1
void sampleADCwritePWM(void) {
    
    // start sampling process
    ADC_START_CONVERSION;
    
    // wait for sampling to finish
    while (!ADC_DONE);
    
    // read ADC result registers and write to duty cycle register
    CCPR1L      =   ADRESH;    // 8 Msb's go in CCPR1L
   
    // to include the 2 Lsb's, need to write 2 Msb's of ADRESL to CCP1CON<5:4>
    // not sure how to do this without changing other bits of CCP1CON
    // will ignore for now

    return;      
}

void main(void) {
    
    // configure inputs/outputs
    TRISA   = 0b11111111;       // set all of PORTA to inputs
    TRISC2  = 0;                // TRISC2 must be clear for CCP1 to be o/p
    
    // configure ADC
    ADCON0  = 0b00000001;       // turn ADC on (pin AN0)
    ADCON1  = 0b00000000;       // left justified, PORTA all analog
    
    __delay_ms(10);             // delay to let ADC cap charge up
    
    // configure timer2 and PWM period (timer2 speed: Fosc/4 = 1MHz)
    // the following settings will yield a PWM period of ~3.9KHz
    T2CON   = 0b00000100;       // timer2 on, 1:1 pre- and post-scaling
    PR2     = 0b11111111;       // set PR2 register to 255
    
    // configure CCP1 module
    CCP1CON = 0b00001100;       // bits 2,3 need to be 1 for PWM enabled
    
    while(1){
        
        // sample the ADC and write value to PWM duty cycle
        sampleADCwritePWM();
        
    }     
}
