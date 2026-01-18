/*
 * File:   bmp180.c
 *
 * BMP180 Digital Pressure Sensor Driver Implementation
 *
 * Reference: BMP180 Datasheet (Bosch BST-BMP180-DS000-09)
 */

#include "bmp180.h"
#include "i2c.h"

/* Define oscillator frequency for __delay_ms() */
#define _XTAL_FREQ 4000000

/*===========================================================================
 * Internal Helper Functions
 *===========================================================================*/

/*
 * Write a single byte to a BMP180 register.
 *
 * I2C Sequence:
 *   [S] [0xEE] [ACK] [reg] [ACK] [data] [ACK] [P]
 *
 * @param reg   Register address to write to
 * @param data  Byte value to write
 * @return      0 on success, 1 if NACK received
 */
static uint8_t bmp180_write_reg(uint8_t reg, uint8_t data) {
    i2c_start();

    if (i2c_write(BMP180_ADDR_WRITE)) {
        i2c_stop();
        return 1;   /* NACK on address */
    }

    if (i2c_write(reg)) {
        i2c_stop();
        return 1;   /* NACK on register */
    }

    if (i2c_write(data)) {
        i2c_stop();
        return 1;   /* NACK on data */
    }

    i2c_stop();
    return 0;
}

/*
 * Read a single byte from a BMP180 register.
 *
 * I2C Sequence:
 *   [S] [0xEE] [ACK] [reg] [ACK] [Sr] [0xEF] [ACK] [data] [NACK] [P]
 *
 * @param reg   Register address to read from
 * @param data  Pointer to store the read byte
 * @return      0 on success, 1 if NACK received
 */
static uint8_t bmp180_read_reg(uint8_t reg, uint8_t *data) {
    i2c_start();

    if (i2c_write(BMP180_ADDR_WRITE)) {
        i2c_stop();
        return 1;   /* NACK on address */
    }

    if (i2c_write(reg)) {
        i2c_stop();
        return 1;   /* NACK on register */
    }

    /* Repeated start to switch to read mode */
    i2c_restart();

    if (i2c_write(BMP180_ADDR_READ)) {
        i2c_stop();
        return 1;   /* NACK on read address */
    }

    *data = i2c_read_nack();    /* Read single byte with NACK */
    i2c_stop();

    return 0;
}

/*
 * Read a 16-bit value from two consecutive BMP180 registers (MSB first).
 *
 * I2C Sequence (burst read):
 *   [S] [0xEE] [ACK] [reg] [ACK] [Sr] [0xEF] [ACK] [MSB] [ACK] [LSB] [NACK] [P]
 *
 * @param reg   Starting register address (MSB)
 * @param data  Pointer to store 16-bit result
 * @return      0 on success, 1 if NACK received
 */
static uint8_t bmp180_read_reg16(uint8_t reg, uint16_t *data) {
    uint8_t msb, lsb;

    i2c_start();

    if (i2c_write(BMP180_ADDR_WRITE)) {
        i2c_stop();
        return 1;
    }

    if (i2c_write(reg)) {
        i2c_stop();
        return 1;
    }

    i2c_restart();

    if (i2c_write(BMP180_ADDR_READ)) {
        i2c_stop();
        return 1;
    }

    msb = i2c_read_ack();       /* Read MSB, send ACK for more data */
    lsb = i2c_read_nack();      /* Read LSB, send NACK to end */
    i2c_stop();

    *data = ((uint16_t)msb << 8) | lsb;
    return 0;
}

/*
 * Read a 24-bit value from three consecutive BMP180 registers (MSB first).
 *
 * I2C Sequence (burst read):
 *   [S] [0xEE] [ACK] [reg] [ACK] [Sr] [0xEF] [ACK] [MSB] [ACK] [LSB] [ACK] [XLSB] [NACK] [P]
 *
 * @param reg   Starting register address (MSB)
 * @param data  Pointer to store 24-bit result (in lower 24 bits of uint32_t)
 * @return      0 on success, 1 if NACK received
 */
static uint8_t bmp180_read_reg24(uint8_t reg, uint32_t *data) {
    uint8_t msb, lsb, xlsb;

    i2c_start();

    if (i2c_write(BMP180_ADDR_WRITE)) {
        i2c_stop();
        return 1;
    }

    if (i2c_write(reg)) {
        i2c_stop();
        return 1;
    }

    i2c_restart();

    if (i2c_write(BMP180_ADDR_READ)) {
        i2c_stop();
        return 1;
    }

    msb = i2c_read_ack();       /* Read MSB, send ACK */
    lsb = i2c_read_ack();       /* Read LSB, send ACK */
    xlsb = i2c_read_nack();     /* Read XLSB, send NACK to end */
    i2c_stop();

    *data = ((uint32_t)msb << 16) | ((uint32_t)lsb << 8) | xlsb;
    return 0;
}

/*===========================================================================
 * Public API Functions
 *===========================================================================*/

uint8_t bmp180_init(bmp180_cal_t *cal) {
    uint8_t chip_id;
    uint16_t raw;

    /*
     * Step 1: Read and verify chip ID
     *
     * Register 0xD0 should contain 0x55 for BMP180.
     *
     * Reference: BMP180 Datasheet, Section 4, page 18
     */
    if (bmp180_read_reg(BMP180_REG_CHIP_ID, &chip_id)) {
        return BMP180_ERR_I2C_NACK;
    }

    if (chip_id != BMP180_CHIP_ID) {
        return BMP180_ERR_CHIP_ID;
    }

    /*
     * Step 2: Read calibration coefficients from EEPROM
     *
     * 11 coefficients stored at addresses 0xAA-0xBF (22 bytes total).
     * Each coefficient is 16-bit, MSB at lower address.
     *
     * Reference: BMP180 Datasheet, Section 3.4, Table 5, page 13
     */

    /* AC1: 0xAA-0xAB (signed) */
    if (bmp180_read_reg16(BMP180_REG_CAL_AC1, &raw)) {
        return BMP180_ERR_I2C_NACK;
    }
    cal->AC1 = (int16_t)raw;

    /* AC2: 0xAC-0xAD (signed) */
    if (bmp180_read_reg16(BMP180_REG_CAL_AC2, &raw)) {
        return BMP180_ERR_I2C_NACK;
    }
    cal->AC2 = (int16_t)raw;

    /* AC3: 0xAE-0xAF (signed) */
    if (bmp180_read_reg16(BMP180_REG_CAL_AC3, &raw)) {
        return BMP180_ERR_I2C_NACK;
    }
    cal->AC3 = (int16_t)raw;

    /* AC4: 0xB0-0xB1 (unsigned) */
    if (bmp180_read_reg16(BMP180_REG_CAL_AC4, &cal->AC4)) {
        return BMP180_ERR_I2C_NACK;
    }

    /* AC5: 0xB2-0xB3 (unsigned) */
    if (bmp180_read_reg16(BMP180_REG_CAL_AC5, &cal->AC5)) {
        return BMP180_ERR_I2C_NACK;
    }

    /* AC6: 0xB4-0xB5 (unsigned) */
    if (bmp180_read_reg16(BMP180_REG_CAL_AC6, &cal->AC6)) {
        return BMP180_ERR_I2C_NACK;
    }

    /* B1: 0xB6-0xB7 (signed) */
    if (bmp180_read_reg16(BMP180_REG_CAL_B1, &raw)) {
        return BMP180_ERR_I2C_NACK;
    }
    cal->B1 = (int16_t)raw;

    /* B2: 0xB8-0xB9 (signed) */
    if (bmp180_read_reg16(BMP180_REG_CAL_B2, &raw)) {
        return BMP180_ERR_I2C_NACK;
    }
    cal->B2 = (int16_t)raw;

    /* MB: 0xBA-0xBB (signed) */
    if (bmp180_read_reg16(BMP180_REG_CAL_MB, &raw)) {
        return BMP180_ERR_I2C_NACK;
    }
    cal->MB = (int16_t)raw;

    /* MC: 0xBC-0xBD (signed) */
    if (bmp180_read_reg16(BMP180_REG_CAL_MC, &raw)) {
        return BMP180_ERR_I2C_NACK;
    }
    cal->MC = (int16_t)raw;

    /* MD: 0xBE-0xBF (signed) */
    if (bmp180_read_reg16(BMP180_REG_CAL_MD, &raw)) {
        return BMP180_ERR_I2C_NACK;
    }
    cal->MD = (int16_t)raw;

    /*
     * Step 3: Validate calibration data
     *
     * AC4, AC5, AC6 are used as divisors and should not be 0.
     * Values of 0x0000 or 0xFFFF indicate EEPROM read failure.
     */
    if (cal->AC4 == 0 || cal->AC4 == 0xFFFF ||
        cal->AC5 == 0 || cal->AC5 == 0xFFFF ||
        cal->AC6 == 0 || cal->AC6 == 0xFFFF) {
        return BMP180_ERR_CAL_INVALID;
    }

    return BMP180_OK;
}

uint8_t bmp180_read_raw_temp(int32_t *ut) {
    uint16_t raw;

    /*
     * Step 1: Start temperature measurement
     *
     * Write 0x2E to control register 0xF4.
     *
     * Reference: BMP180 Datasheet, Section 5.4, Table 8, page 21
     */
    if (bmp180_write_reg(BMP180_REG_CTRL_MEAS, BMP180_CMD_TEMP)) {
        return BMP180_ERR_I2C_NACK;
    }

    /*
     * Step 2: Wait for conversion to complete
     *
     * Temperature measurement takes max 4.5ms.
     * We wait 5ms to be safe.
     *
     * Reference: BMP180 Datasheet, Section 3.3.1, Table 3, page 12
     */
    __delay_ms(5);

    /*
     * Step 3: Read 16-bit result from 0xF6-0xF7
     *
     * Reference: BMP180 Datasheet, Section 3.5, page 14
     */
    if (bmp180_read_reg16(BMP180_REG_OUT_MSB, &raw)) {
        return BMP180_ERR_I2C_NACK;
    }

    *ut = (int32_t)raw;
    return BMP180_OK;
}

uint8_t bmp180_read_raw_pressure(int32_t *up, uint8_t oss) {
    uint8_t cmd;
    uint32_t raw;

    /*
     * Step 1: Calculate command byte and start pressure measurement
     *
     * Command = 0x34 + (oss << 6)
     *   oss=0: 0x34 (ultra low power)
     *   oss=1: 0x74 (standard)
     *   oss=2: 0xB4 (high resolution)
     *   oss=3: 0xF4 (ultra high resolution)
     *
     * Reference: BMP180 Datasheet, Section 5.4, Table 8, page 21
     */
    cmd = BMP180_CMD_PRESS_OSS0 + (oss << 6);

    if (bmp180_write_reg(BMP180_REG_CTRL_MEAS, cmd)) {
        return BMP180_ERR_I2C_NACK;
    }

    /*
     * Step 2: Wait for conversion to complete
     *
     * Wait time depends on oversampling setting:
     *   oss=0: 4.5ms,  oss=1: 7.5ms,  oss=2: 13.5ms,  oss=3: 25.5ms
     *
     * We round up to safe integer delays.
     *
     * Reference: BMP180 Datasheet, Section 3.3.1, Table 3, page 12
     */
    switch (oss) {
        case 0:
            __delay_ms(5);
            break;
        case 1:
            __delay_ms(8);
            break;
        case 2:
            __delay_ms(14);
            break;
        case 3:
        default:
            __delay_ms(26);
            break;
    }

    /*
     * Step 3: Read 24-bit result from 0xF6-0xF8
     *
     * The raw value needs to be right-shifted by (8 - oss).
     *
     * Reference: BMP180 Datasheet, Section 3.5, page 14
     */
    if (bmp180_read_reg24(BMP180_REG_OUT_MSB, &raw)) {
        return BMP180_ERR_I2C_NACK;
    }

    *up = (int32_t)(raw >> (8 - oss));
    return BMP180_OK;
}

void bmp180_compensate(const bmp180_cal_t *cal, int32_t ut, int32_t up,
                       uint8_t oss, bmp180_result_t *result) {
    /*
     * BMP180 Compensation Algorithm
     *
     * This algorithm converts raw ADC readings to calibrated temperature
     * and pressure values using the factory-programmed coefficients.
     *
     * Reference: BMP180 Datasheet, Section 3.5, Figure 4, page 14
     */

    int32_t X1, X2, X3, B3, B5, B6;
    uint32_t B4, B7;
    int32_t p;

    /*
     * Temperature Compensation
     *
     * X1 = (UT - AC6) * AC5 / 2^15
     * X2 = MC * 2^11 / (X1 + MD)
     * B5 = X1 + X2
     * T = (B5 + 8) / 2^4   (result in 0.1 C)
     */
    X1 = ((ut - (int32_t)cal->AC6) * (int32_t)cal->AC5) >> 15;
    X2 = ((int32_t)cal->MC << 11) / (X1 + (int32_t)cal->MD);
    B5 = X1 + X2;
    result->temp = (int16_t)((B5 + 8) >> 4);

    /*
     * Pressure Compensation
     *
     * Uses B5 from temperature calculation.
     * Algorithm from datasheet Figure 4, page 14.
     */
    B6 = B5 - 4000;

    /* Calculate X1, X2, X3 for B3 */
    X1 = ((int32_t)cal->B2 * ((B6 * B6) >> 12)) >> 11;
    X2 = ((int32_t)cal->AC2 * B6) >> 11;
    X3 = X1 + X2;
    B3 = ((((int32_t)cal->AC1 * 4 + X3) << oss) + 2) >> 2;

    /* Calculate X1, X2 for B4 */
    X1 = ((int32_t)cal->AC3 * B6) >> 13;
    X2 = ((int32_t)cal->B1 * ((B6 * B6) >> 12)) >> 16;
    X3 = ((X1 + X2) + 2) >> 2;
    B4 = ((uint32_t)cal->AC4 * (uint32_t)(X3 + 32768)) >> 15;

    /* Calculate B7 and pressure */
    B7 = ((uint32_t)up - (uint32_t)B3) * (uint32_t)(50000 >> oss);

    if (B7 < 0x80000000) {
        p = (int32_t)((B7 * 2) / B4);
    } else {
        p = (int32_t)((B7 / B4) * 2);
    }

    /* Final pressure adjustment */
    X1 = (p >> 8) * (p >> 8);
    X1 = (X1 * 3038) >> 16;
    X2 = (-7357 * p) >> 16;

    result->pressure = p + ((X1 + X2 + 3791) >> 4);
}
