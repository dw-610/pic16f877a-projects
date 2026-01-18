/*
 * File:   main.c
 *
 * BMP180 I2C Sensor Demo for PIC16F877A
 *
 * This program demonstrates reading temperature and pressure from a BMP180
 * sensor using the I2C interface. An LED on RD0 blinks to indicate status:
 *   - Slow blink (1s): Normal operation, reading sensor
 *   - Fast blink (100ms): Error during initialization
 *
 * Hardware:
 *   - BMP180 sensor connected to RC3 (SCL) and RC4 (SDA) with 4.7k pull-ups
 *   - LED connected to RD0 (with current limiting resistor)
 *
 * Reference: BMP180 Datasheet (Bosch BST-BMP180-DS000-09)
 *            PIC16F877A Datasheet (Microchip DS39582B)
 */

#include "CONFIG.h"
#include "i2c.h"
#include "bmp180.h"

/* Define oscillator frequency for __delay_ms() */
#define _XTAL_FREQ 4000000

/* Oversampling setting for pressure measurement (0-3) */
#define PRESSURE_OSS    0       /* Ultra low power mode, fastest */

/* Global variables to store sensor data */
static bmp180_cal_t cal;        /* Calibration coefficients (22 bytes) */
static bmp180_result_t result;  /* Measurement results (6 bytes) */

/*
 * Blink LED rapidly to indicate an error condition.
 * Loops forever - requires reset to recover.
 */
static void error_blink(void) {
    while (1) {
        RD0 = 1;
        __delay_ms(100);
        RD0 = 0;
        __delay_ms(100);
    }
}

void main(void) {
    uint8_t status;
    int32_t raw_temp;
    int32_t raw_pressure;

    /*
     * Step 1: Configure I/O pins
     *
     * RD0: Output for LED feedback
     * RC3, RC4: Configured by i2c_init() as I2C pins
     */
    TRISD0 = 0;     /* RD0 as output for LED */
    RD0 = 0;        /* LED off initially */

    /*
     * Step 2: Initialize I2C master
     *
     * Configures MSSP module for 100 kHz I2C master mode.
     * RC3 = SCL, RC4 = SDA with external pull-ups required.
     *
     * Reference: PIC16F877A Datasheet, Section 9.4
     */
    i2c_init();

    /*
     * Step 3: Initialize BMP180 sensor
     *
     * Verifies chip ID (0x55) and reads 11 calibration coefficients.
     * If initialization fails, blink LED rapidly to indicate error.
     *
     * Reference: BMP180 Datasheet, Section 3.4, page 13
     */
    status = bmp180_init(&cal);
    if (status != BMP180_OK) {
        /* Initialization failed - blink error pattern */
        error_blink();
    }

    /* Brief LED flash to indicate successful initialization */
    RD0 = 1;
    __delay_ms(200);
    RD0 = 0;
    __delay_ms(200);

    /*
     * Step 4: Main measurement loop
     *
     * Continuously reads temperature and pressure at ~1 second intervals.
     * Temperature must be read before pressure because the pressure
     * compensation algorithm requires the B5 value from temperature.
     *
     * Reference: BMP180 Datasheet, Section 3.5, Figure 4, page 14
     */
    while (1) {
        /*
         * Read raw temperature
         *
         * Writes 0x2E to control register, waits 5ms, reads 16-bit result.
         *
         * Reference: BMP180 Datasheet, Section 5.4, Table 8, page 21
         */
        status = bmp180_read_raw_temp(&raw_temp);
        if (status != BMP180_OK) {
            error_blink();
        }

        /*
         * Read raw pressure
         *
         * Writes command based on OSS setting, waits, reads result.
         * OSS=0: Ultra low power, 4.5ms conversion, 16-bit result
         *
         * Reference: BMP180 Datasheet, Section 3.3.1, Table 3, page 12
         */
        status = bmp180_read_raw_pressure(&raw_pressure, PRESSURE_OSS);
        if (status != BMP180_OK) {
            error_blink();
        }

        /*
         * Apply compensation algorithm
         *
         * Converts raw ADC values to calibrated temperature and pressure:
         *   result.temp     = temperature in 0.1 C (e.g., 250 = 25.0 C)
         *   result.pressure = pressure in Pa (e.g., 101325 = 1013.25 hPa)
         *
         * Reference: BMP180 Datasheet, Section 3.5, Figure 4, page 14
         */
        bmp180_compensate(&cal, raw_temp, raw_pressure, PRESSURE_OSS, &result);

        /*
         * At this point, result.temp and result.pressure contain the
         * calibrated sensor readings. These could be:
         *   - Sent over UART for debugging
         *   - Displayed on an LCD
         *   - Used to control other hardware
         *
         * For this demo, we just blink the LED to show the loop is running.
         * Expected values at room temperature near sea level:
         *   Temperature: ~200-300 (20-30 C)
         *   Pressure: ~100000-102000 Pa
         */

        /* Blink LED to indicate successful measurement cycle */
        RD0 = 1;
        __delay_ms(50);
        RD0 = 0;

        /* Wait before next measurement (~1 second total loop time) */
        __delay_ms(950);
    }
}
