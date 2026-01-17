/*
 * File:   i2c.h
 *
 * I2C Master Driver for PIC16F877A
 *
 * Hardware: MSSP module in I2C Master mode
 * Pins:     RC3 = SCL (clock), RC4 = SDA (data)
 * Speed:    100 kHz (standard mode)
 *
 * Reference: PIC16F877A Datasheet, Section 9.4 "I2C Mode"
 */

#ifndef I2C_H
#define I2C_H

#include <xc.h>
#include <stdint.h>

/*
 * Initialize the MSSP module for I2C Master mode at 100 kHz.
 *
 * Configures:
 *   - TRISC<4:3> as inputs (required for open-drain I2C)
 *   - SSPCON for I2C Master mode
 *   - SSPADD for 100 kHz baud rate
 *   - SSPSTAT for standard speed slew rate
 */
void i2c_init(void);

/*
 * Generate a Start condition on the I2C bus.
 * SDA goes low while SCL is high.
 *
 * Reference: Datasheet Section 9.4.8, Figure 9-19
 */
void i2c_start(void);

/*
 * Generate a Repeated Start condition on the I2C bus.
 * Used to change direction (write->read) without releasing the bus.
 *
 * Reference: Datasheet Section 9.4.9, Figure 9-20
 */
void i2c_restart(void);

/*
 * Generate a Stop condition on the I2C bus.
 * SDA goes high while SCL is high, releasing the bus.
 *
 * Reference: Datasheet Section 9.4.13, Figure 9-24
 */
void i2c_stop(void);

/*
 * Write a byte to the I2C bus.
 *
 * @param data  The byte to transmit
 * @return      0 if ACK received from slave, 1 if NACK
 *
 * Reference: Datasheet Section 9.4.10, Figure 9-21
 */
uint8_t i2c_write(uint8_t data);

/*
 * Read a byte from the I2C bus and send ACK.
 * Use when more bytes will follow.
 *
 * @return  The byte received from the slave
 *
 * Reference: Datasheet Section 9.4.11, Section 9.4.12
 */
uint8_t i2c_read_ack(void);

/*
 * Read a byte from the I2C bus and send NACK.
 * Use for the last byte in a read sequence.
 *
 * @return  The byte received from the slave
 *
 * Reference: Datasheet Section 9.4.11, Section 9.4.12
 */
uint8_t i2c_read_nack(void);

#endif /* I2C_H */
