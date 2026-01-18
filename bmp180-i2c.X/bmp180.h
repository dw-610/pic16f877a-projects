/*
 * File:   bmp180.h
 *
 * BMP180 Digital Pressure Sensor Driver for PIC16F877A
 *
 * Hardware: Bosch BMP180 connected via I2C
 * Interface: Uses i2c.c/i2c.h master driver
 *
 * Reference: BMP180 Datasheet (Bosch BST-BMP180-DS000-09)
 */

#ifndef BMP180_H
#define BMP180_H

#include <xc.h>
#include <stdint.h>

/*
 * I2C Addresses
 *
 * The BMP180 has a fixed I2C address. The 7-bit address is 0x77.
 * With the R/W bit appended:
 *   Write: 0xEE (0x77 << 1 | 0)
 *   Read:  0xEF (0x77 << 1 | 1)
 *
 * Reference: BMP180 Datasheet, Section 5.2, Table 7, page 20
 */
#define BMP180_ADDR_WRITE       0xEE
#define BMP180_ADDR_READ        0xEF

/*
 * Register Addresses
 *
 * Reference: BMP180 Datasheet, Section 4, page 18
 */
#define BMP180_REG_CHIP_ID      0xD0    /* Chip ID register, should read 0x55 */
#define BMP180_REG_SOFT_RESET   0xE0    /* Soft reset register */
#define BMP180_REG_CTRL_MEAS    0xF4    /* Measurement control register */
#define BMP180_REG_OUT_MSB      0xF6    /* ADC output MSB */
#define BMP180_REG_OUT_LSB      0xF7    /* ADC output LSB */
#define BMP180_REG_OUT_XLSB     0xF8    /* ADC output XLSB (pressure only) */

/*
 * Calibration Coefficient Register Addresses
 *
 * 11 calibration coefficients stored in EEPROM at addresses 0xAA-0xBF.
 * Each coefficient is 16-bit, MSB first.
 *
 * Reference: BMP180 Datasheet, Section 3.4, Table 5, page 13
 */
#define BMP180_REG_CAL_AC1      0xAA    /* AC1 MSB */
#define BMP180_REG_CAL_AC2      0xAC    /* AC2 MSB */
#define BMP180_REG_CAL_AC3      0xAE    /* AC3 MSB */
#define BMP180_REG_CAL_AC4      0xB0    /* AC4 MSB */
#define BMP180_REG_CAL_AC5      0xB2    /* AC5 MSB */
#define BMP180_REG_CAL_AC6      0xB4    /* AC6 MSB */
#define BMP180_REG_CAL_B1       0xB6    /* B1 MSB */
#define BMP180_REG_CAL_B2       0xB8    /* B2 MSB */
#define BMP180_REG_CAL_MB       0xBA    /* MB MSB */
#define BMP180_REG_CAL_MC       0xBC    /* MC MSB */
#define BMP180_REG_CAL_MD       0xBE    /* MD MSB */

/*
 * Expected Chip ID
 *
 * Reading register 0xD0 should return 0x55 for BMP180.
 *
 * Reference: BMP180 Datasheet, Section 4, page 18
 */
#define BMP180_CHIP_ID          0x55

/*
 * Measurement Control Commands
 *
 * Write to register 0xF4 to start a measurement.
 *
 * Temperature: Write 0x2E, wait 4.5ms, read 16-bit from 0xF6-0xF7
 *
 * Pressure: Write 0x34 + (oss << 6), wait according to oss, read from 0xF6-0xF8
 *   oss=0: 0x34, wait 4.5ms,  read 16-bit
 *   oss=1: 0x74, wait 7.5ms,  read 17-bit
 *   oss=2: 0xB4, wait 13.5ms, read 18-bit
 *   oss=3: 0xF4, wait 25.5ms, read 19-bit
 *
 * Reference: BMP180 Datasheet, Section 3.3.1, Table 3, page 12
 *            Section 5.4, Table 8, page 21
 */
#define BMP180_CMD_TEMP         0x2E    /* Start temperature measurement */
#define BMP180_CMD_PRESS_OSS0   0x34    /* Start pressure, oss=0 (ultra low power) */
#define BMP180_CMD_PRESS_OSS1   0x74    /* Start pressure, oss=1 (standard) */
#define BMP180_CMD_PRESS_OSS2   0xB4    /* Start pressure, oss=2 (high resolution) */
#define BMP180_CMD_PRESS_OSS3   0xF4    /* Start pressure, oss=3 (ultra high res) */

/*
 * Error Codes
 */
#define BMP180_OK               0       /* Operation successful */
#define BMP180_ERR_CHIP_ID      1       /* Chip ID mismatch (not 0x55) */
#define BMP180_ERR_I2C_NACK     2       /* I2C NACK received */
#define BMP180_ERR_CAL_INVALID  3       /* Calibration data invalid */

/*
 * Calibration Coefficients Structure
 *
 * Stores the 11 calibration coefficients read from the sensor's EEPROM.
 * These are factory-programmed and unique to each sensor.
 *
 * Note on signedness:
 *   AC1, AC2, AC3, B1, B2, MB, MC, MD are signed (short)
 *   AC4, AC5, AC6 are unsigned (unsigned short)
 *
 * Reference: BMP180 Datasheet, Section 3.4, Table 5, page 13
 */
typedef struct {
    int16_t AC1;        /* 0xAA-0xAB, signed */
    int16_t AC2;        /* 0xAC-0xAD, signed */
    int16_t AC3;        /* 0xAE-0xAF, signed */
    uint16_t AC4;       /* 0xB0-0xB1, unsigned */
    uint16_t AC5;       /* 0xB2-0xB3, unsigned */
    uint16_t AC6;       /* 0xB4-0xB5, unsigned */
    int16_t B1;         /* 0xB6-0xB7, signed */
    int16_t B2;         /* 0xB8-0xB9, signed */
    int16_t MB;         /* 0xBA-0xBB, signed */
    int16_t MC;         /* 0xBC-0xBD, signed */
    int16_t MD;         /* 0xBE-0xBF, signed */
} bmp180_cal_t;

/*
 * Measurement Result Structure
 *
 * Stores the compensated temperature and pressure values.
 *
 * Temperature is in units of 0.1 degrees Celsius.
 *   Example: temp = 250 means 25.0 C
 *
 * Pressure is in Pascals (Pa).
 *   Example: pressure = 101325 means 101325 Pa = 1013.25 hPa
 *
 * Reference: BMP180 Datasheet, Section 3.5, page 14
 */
typedef struct {
    int16_t temp;           /* Temperature in 0.1 C */
    int32_t pressure;       /* Pressure in Pa */
} bmp180_result_t;

/*
 * Initialize the BMP180 sensor.
 *
 * Performs the following steps:
 *   1. Reads and verifies chip ID (must be 0x55)
 *   2. Reads all 11 calibration coefficients from EEPROM
 *   3. Validates calibration data (AC4, AC5, AC6 must not be 0x0000 or 0xFFFF)
 *
 * I2C Sequence (chip ID read):
 *   [S] [0xEE] [ACK] [0xD0] [ACK] [Sr] [0xEF] [ACK] [data] [NACK] [P]
 *
 * @param cal   Pointer to calibration structure to fill
 * @return      BMP180_OK on success, error code on failure
 *
 * Reference: BMP180 Datasheet, Section 3.4, Table 5, page 13
 *            Section 4 (Register Map), page 18
 */
uint8_t bmp180_init(bmp180_cal_t *cal);

/*
 * Read raw (uncompensated) temperature value.
 *
 * Sequence:
 *   1. Write 0x2E to control register 0xF4
 *   2. Wait at least 4.5ms for conversion
 *   3. Read 16-bit value from 0xF6-0xF7 (MSB first)
 *
 * I2C Sequence (write command):
 *   [S] [0xEE] [ACK] [0xF4] [ACK] [0x2E] [ACK] [P]
 *
 * I2C Sequence (read result, 16-bit burst):
 *   [S] [0xEE] [ACK] [0xF6] [ACK] [Sr] [0xEF] [ACK] [MSB] [ACK] [LSB] [NACK] [P]
 *
 * @param ut    Pointer to store 16-bit raw temperature
 * @return      BMP180_OK on success, BMP180_ERR_I2C_NACK on failure
 *
 * Reference: BMP180 Datasheet, Section 3.5, page 14
 *            Section 5.4, Table 8, page 21
 */
uint8_t bmp180_read_raw_temp(int32_t *ut);

/*
 * Read raw (uncompensated) pressure value.
 *
 * Sequence:
 *   1. Write (0x34 + (oss << 6)) to control register 0xF4
 *   2. Wait according to oversampling setting
 *   3. Read 24-bit value from 0xF6-0xF8, then right-shift by (8 - oss)
 *
 * Wait times by oss:
 *   oss=0: 4.5ms,  oss=1: 7.5ms,  oss=2: 13.5ms,  oss=3: 25.5ms
 *
 * I2C Sequence (write command):
 *   [S] [0xEE] [ACK] [0xF4] [ACK] [cmd] [ACK] [P]
 *
 * I2C Sequence (read result, 24-bit burst):
 *   [S] [0xEE] [ACK] [0xF6] [ACK] [Sr] [0xEF] [ACK] [MSB] [ACK] [LSB] [ACK] [XLSB] [NACK] [P]
 *
 * @param up    Pointer to store raw pressure value
 * @param oss   Oversampling setting (0-3)
 * @return      BMP180_OK on success, BMP180_ERR_I2C_NACK on failure
 *
 * Reference: BMP180 Datasheet, Section 3.3.1, Table 3, page 12
 *            Section 3.5, page 14
 */
uint8_t bmp180_read_raw_pressure(int32_t *up, uint8_t oss);

/*
 * Calculate compensated temperature and pressure.
 *
 * Applies the BMP180 compensation algorithm using the calibration
 * coefficients and raw sensor values.
 *
 * Temperature algorithm:
 *   X1 = (UT - AC6) * AC5 / 32768
 *   X2 = MC * 2048 / (X1 + MD)
 *   B5 = X1 + X2
 *   T = (B5 + 8) / 16  (result in 0.1 C)
 *
 * Pressure algorithm uses B5 from temperature calculation, plus
 * AC1-AC4, B1, B2 and the raw pressure value.
 *
 * @param cal       Pointer to calibration coefficients
 * @param ut        Raw temperature value from bmp180_read_raw_temp()
 * @param up        Raw pressure value from bmp180_read_raw_pressure()
 * @param oss       Oversampling setting used when reading pressure (0-3)
 * @param result    Pointer to result structure to fill
 *
 * Reference: BMP180 Datasheet, Section 3.5, Figure 4, page 14
 */
void bmp180_compensate(const bmp180_cal_t *cal, int32_t ut, int32_t up,
                       uint8_t oss, bmp180_result_t *result);

#endif /* BMP180_H */
