/*
 * File:   i2c.c
 *
 * I2C Master Driver Implementation for PIC16F877A
 *
 * Reference: PIC16F877A Datasheet (DS39582B), Section 9.4 "I2C Mode"
 */

#include "i2c.h"

/*
 * Wait for the MSSP module to complete the current operation.
 *
 * The SSPIF flag (PIR1<3>) is set by hardware when:
 *   - Start condition complete
 *   - Stop condition complete
 *   - Data byte transmitted/received
 *   - Acknowledge sequence complete
 *   - Repeated Start complete
 *
 * Reference: Datasheet Section 9.4.6, page 96
 */
static void i2c_wait(void) {
    while (!PIR1bits.SSPIF);    // Wait for SSPIF to be set
    PIR1bits.SSPIF = 0;         // Clear the flag for next operation
}

void i2c_init(void) {
    /*
     * Step 1: Configure I/O pins
     *
     * RC3 (SCL) and RC4 (SDA) must be configured as inputs.
     * The MSSP module overrides these to open-drain outputs when needed.
     * External pull-up resistors (4.7k to VDD) are required.
     *
     * Reference: Datasheet Section 9.4.2, page 85
     *   "Selection of any I2C mode, with the SSPEN bit set, forces the
     *    SCL and SDA pins to be open-drain, provided these pins are
     *    programmed to inputs by setting the appropriate TRISC bits."
     */
    TRISC3 = 1;     // SCL pin as input
    TRISC4 = 1;     // SDA pin as input

    /*
     * Step 2: Configure SSPSTAT register (address 0x94)
     *
     * Bit 7 (SMP): Slew Rate Control
     *   1 = Slew rate control disabled for standard speed (100 kHz)
     *   0 = Slew rate control enabled for high speed (400 kHz)
     *
     * Bit 6 (CKE): SMBus Select
     *   0 = Disable SMBus specific inputs (use I2C levels)
     *
     * Bits 5-0: Status bits, read-only in most cases
     *
     * Reference: Datasheet Register 9-3, page 82
     */
    SSPSTAT = 0b10000000;   // SMP=1 (slew rate disabled for 100kHz)

    /*
     * Step 3: Configure baud rate via SSPADD register (address 0x93)
     *
     * Formula: FSCL = FOSC / (4 * (SSPADD + 1))
     *
     * For FOSC = 4 MHz and desired FSCL = 100 kHz:
     *   SSPADD = (FOSC / (4 * FSCL)) - 1
     *   SSPADD = (4,000,000 / (4 * 100,000)) - 1
     *   SSPADD = (4,000,000 / 400,000) - 1
     *   SSPADD = 10 - 1 = 9 = 0x09
     *
     * Verification: FSCL = 4,000,000 / (4 * (9 + 1)) = 4,000,000 / 40 = 100,000 Hz
     *
     * Reference: Datasheet Section 9.4.7, Table 9-3, page 98
     */
    SSPADD = 0x09;          // 100 kHz @ 4 MHz FOSC

    /*
     * Step 4: Configure SSPCON register (address 0x14)
     *
     * Bit 7 (WCOL): Write Collision Detect - cleared
     * Bit 6 (SSPOV): Receive Overflow Indicator - cleared
     * Bit 5 (SSPEN): SSP Enable - SET to enable module
     * Bit 4 (CKP): Clock Polarity - unused in master mode
     * Bits 3-0 (SSPM3:SSPM0): Mode Select
     *   1000 = I2C Master mode, clock = FOSC/(4*(SSPADD+1))
     *
     * Reference: Datasheet Register 9-4, page 83
     */
    SSPCON = 0b00101000;    // SSPEN=1, I2C Master mode (0b1000)

    /*
     * Step 5: Clear SSPCON2 register (address 0x91)
     *
     * All control bits should be 0 at init:
     *   GCEN=0 (no general call), ACKSTAT=0, ACKDT=0,
     *   ACKEN=0, RCEN=0, PEN=0, RSEN=0, SEN=0
     *
     * Reference: Datasheet Register 9-5, page 84
     */
    SSPCON2 = 0x00;

    /*
     * Step 6: Clear interrupt flag
     *
     * SSPIF (PIR1<3>) may be set from previous operations.
     * Clear it to prepare for first I2C operation.
     */
    PIR1bits.SSPIF = 0;
}

void i2c_start(void) {
    /*
     * Initiate Start condition by setting SEN bit (SSPCON2<0>).
     *
     * Sequence (performed by hardware):
     *   1. SDA and SCL are sampled high
     *   2. BRG counts down one period (TBRG)
     *   3. SDA is driven low (Start condition)
     *   4. S bit (SSPSTAT<3>) is set
     *   5. BRG counts down again
     *   6. SEN is auto-cleared, SSPIF is set
     *
     * Reference: Datasheet Section 9.4.8, page 100, Figure 9-19
     */
    SSPCON2bits.SEN = 1;    // Initiate Start condition
    i2c_wait();             // Wait for Start to complete
}

void i2c_restart(void) {
    /*
     * Initiate Repeated Start condition by setting RSEN bit (SSPCON2<1>).
     *
     * Used to change bus direction without releasing it (e.g., write
     * register address, then read data).
     *
     * Sequence (performed by hardware):
     *   1. SCL is driven low
     *   2. SDA is released high for TBRG
     *   3. SCL is released high
     *   4. SDA is driven low (Repeated Start)
     *   5. RSEN is auto-cleared, SSPIF is set
     *
     * Reference: Datasheet Section 9.4.9, page 101, Figure 9-20
     */
    SSPCON2bits.RSEN = 1;   // Initiate Repeated Start condition
    i2c_wait();             // Wait for Repeated Start to complete
}

void i2c_stop(void) {
    /*
     * Initiate Stop condition by setting PEN bit (SSPCON2<2>).
     *
     * Sequence (performed by hardware):
     *   1. SDA is driven low
     *   2. SCL is released high
     *   3. After TBRG, SDA is released high (Stop condition)
     *   4. P bit (SSPSTAT<4>) is set
     *   5. PEN is auto-cleared, SSPIF is set
     *
     * Reference: Datasheet Section 9.4.13, page 105, Figure 9-24
     */
    SSPCON2bits.PEN = 1;    // Initiate Stop condition
    i2c_wait();             // Wait for Stop to complete
}

uint8_t i2c_write(uint8_t data) {
    /*
     * Transmit a byte on the I2C bus.
     *
     * Sequence:
     *   1. Write data to SSPBUF (sets BF flag, starts BRG)
     *   2. Data is shifted out on SDA, MSB first, on falling SCL edges
     *   3. After 8 bits, master releases SDA for ACK
     *   4. Slave pulls SDA low (ACK) or leaves high (NACK) during 9th clock
     *   5. ACKSTAT (SSPCON2<6>) is updated: 0=ACK received, 1=NACK
     *   6. SSPIF is set, BF is cleared
     *
     * Reference: Datasheet Section 9.4.10, page 102, Figure 9-21
     */
    SSPBUF = data;          // Load data into transmit buffer
    i2c_wait();             // Wait for transmission to complete

    /*
     * Return ACK status:
     *   ACKSTAT = 0: Slave acknowledged (ACK)
     *   ACKSTAT = 1: Slave did not acknowledge (NACK)
     *
     * Reference: Datasheet Section 9.4.10.3, page 102
     */
    return SSPCON2bits.ACKSTAT;
}

uint8_t i2c_read_ack(void) {
    /*
     * Receive a byte and send ACK (more bytes expected).
     *
     * Step 1: Enable receive mode by setting RCEN (SSPCON2<3>)
     *
     * Sequence:
     *   1. RCEN is set, BRG starts counting
     *   2. SCL toggles, data shifted into SSPSR on rising edges
     *   3. After 8 bits, RCEN auto-clears
     *   4. Data moves from SSPSR to SSPBUF, BF is set
     *   5. SSPIF is set
     *
     * Reference: Datasheet Section 9.4.11, page 102
     */
    SSPCON2bits.RCEN = 1;   // Enable receive mode
    i2c_wait();             // Wait for byte to be received

    uint8_t data = SSPBUF;  // Read received byte (clears BF)

    /*
     * Step 2: Send ACK by clearing ACKDT and setting ACKEN
     *
     * ACKDT (SSPCON2<5>): Acknowledge Data bit
     *   0 = ACK (acknowledge, SDA driven low)
     *   1 = NACK (not acknowledge, SDA released high)
     *
     * ACKEN (SSPCON2<4>): Initiate acknowledge sequence
     *   Hardware clocks out ACKDT value on SDA during 9th clock
     *
     * Reference: Datasheet Section 9.4.12, page 105, Figure 9-23
     */
    SSPCON2bits.ACKDT = 0;  // ACK value (low = acknowledge)
    SSPCON2bits.ACKEN = 1;  // Start acknowledge sequence
    i2c_wait();             // Wait for ACK to complete

    return data;
}

uint8_t i2c_read_nack(void) {
    /*
     * Receive a byte and send NACK (last byte in sequence).
     *
     * Same as i2c_read_ack(), but ACKDT=1 to signal end of read.
     * The slave will release SDA when it sees NACK.
     *
     * Reference: Datasheet Section 9.4.11 and 9.4.12
     */
    SSPCON2bits.RCEN = 1;   // Enable receive mode
    i2c_wait();             // Wait for byte to be received

    uint8_t data = SSPBUF;  // Read received byte (clears BF)

    SSPCON2bits.ACKDT = 1;  // NACK value (high = not acknowledge)
    SSPCON2bits.ACKEN = 1;  // Start acknowledge sequence
    i2c_wait();             // Wait for NACK to complete

    return data;
}
