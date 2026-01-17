# BMP180 I2C Project Plan

## Documentation Requirements

**For every code change, provide:**

1. **Summary of changes** - What was added/modified and why
2. **Datasheet references** - Specific sections, tables, figures, and page numbers from:
   - PIC16F877A Datasheet (Microchip DS39582B)
   - BMP180 Datasheet (Bosch BST-BMP180-DS000-09)
3. **How to review** - Step-by-step instructions to verify the code against the datasheets

---

## Approach: Hardware I2C Master

Use the PIC16F877A's **MSSP module in I2C master mode**.

- **SDA:** RC4
- **SCL:** RC3
- **BMP180 address:** 0xEE (write) / 0xEF (read)

---

## Implementation Steps

### 1. I2C Master Driver
- Configure MSSP for I2C master mode
- Functions: `i2c_init()`, `i2c_start()`, `i2c_stop()`, `i2c_restart()`, `i2c_write()`, `i2c_read_ack()`, `i2c_read_nack()`
- Set baud rate via SSPADD register (100kHz is safe for BMP180)

### 2. BMP180 Driver
- `bmp180_init()` - verify chip ID (0x55 at register 0xD0), read 11 calibration coefficients
- `bmp180_read_raw_temp()` - write 0x2E to 0xF4, wait 4.5ms, read 16-bit result
- `bmp180_read_raw_pressure()` - write 0x34 to 0xF4, wait 4.5ms, read 16-19 bit result
- `bmp180_compensate()` - apply calibration algorithm to get true °C and hPa

### 3. Main Loop
- Initialize I2C and BMP180
- Every ~1 second:
  1. Read temperature (required before pressure for compensation)
  2. Read pressure
  3. Calculate compensated values
  4. LED blink for feedback (or output to another I2C device if desired)

---

## File Structure

| File | Purpose |
|------|---------|
| `main.c` | Main loop, initialization, timing |
| `i2c.c` / `i2c.h` | Hardware I2C master driver |
| `bmp180.c` / `bmp180.h` | BMP180 sensor driver + compensation math |
| `CONFIG.h` | MCU configuration (existing) |

---

## Hardware Wiring

| BMP180 Pin | PIC Pin | Notes |
|------------|---------|-------|
| VDD | 3.3V | **Not 5V tolerant** |
| GND | GND | |
| SDA | RC4 | 4.7kΩ pull-up to 3.3V |
| SCL | RC3 | 4.7kΩ pull-up to 3.3V |

**Voltage note:** If the PIC runs at 5V, you'll need a level shifter or run the PIC at 3.3V.

---

## Key Register Values (from BMP180 datasheet)

| Register | Address | Purpose |
|----------|---------|---------|
| Chip ID | 0xD0 | Should read 0x55 |
| Control | 0xF4 | Write 0x2E (temp) or 0x34 (pressure oss=0) |
| Data MSB | 0xF6 | Result high byte |
| Data LSB | 0xF7 | Result low byte |
| Data XLSB | 0xF8 | Extended bits (pressure only) |
| Calibration | 0xAA-0xBF | 11 x 16-bit coefficients |

---

## Calibration Coefficients

Read from EEPROM at addresses 0xAA-0xBF (11 words, 16-bit each):

| Parameter | MSB | LSB | Type |
|-----------|-----|-----|------|
| AC1 | 0xAA | 0xAB | short |
| AC2 | 0xAC | 0xAD | short |
| AC3 | 0xAE | 0xAF | short |
| AC4 | 0xB0 | 0xB1 | unsigned short |
| AC5 | 0xB2 | 0xB3 | unsigned short |
| AC6 | 0xB4 | 0xB5 | unsigned short |
| B1 | 0xB6 | 0xB7 | short |
| B2 | 0xB8 | 0xB9 | short |
| MB | 0xBA | 0xBB | short |
| MC | 0xBC | 0xBD | short |
| MD | 0xBE | 0xBF | short |

---

## Status

- [x] I2C driver (`i2c.c`, `i2c.h`)
- [ ] BMP180 driver (`bmp180.c`, `bmp180.h`)
- [ ] Main loop integration
- [ ] Testing with hardware

---

## Change Log

### Change 1: I2C Master Driver (i2c.h, i2c.c)

#### Summary of Changes

Created two files:

**`i2c.h`** - Header file with function declarations and documentation.

**`i2c.c`** - Implementation with 6 functions:

| Function | Purpose |
|----------|---------|
| `i2c_init()` | Configure MSSP for I2C master at 100 kHz |
| `i2c_start()` | Generate Start condition (SEN bit) |
| `i2c_restart()` | Generate Repeated Start condition (RSEN bit) |
| `i2c_stop()` | Generate Stop condition (PEN bit) |
| `i2c_write(data)` | Transmit byte, return ACK status |
| `i2c_read_ack()` / `i2c_read_nack()` | Receive byte with ACK or NACK |

Key configuration values:
- `SSPADD = 0x09` → 100 kHz clock @ 4 MHz FOSC
- `SSPCON = 0b00101000` → SSPEN=1, I2C Master mode
- `SSPSTAT = 0b10000000` → Slew rate disabled for standard speed

#### Datasheet References (PIC16F877A DS39582B)

| Topic | Section | Page |
|-------|---------|------|
| I2C Mode Overview | 9.4 | 81-82 |
| SSPSTAT Register (I2C) | Register 9-3 | 82 |
| SSPCON Register (I2C) | Register 9-4 | 83 |
| SSPCON2 Register | Register 9-5 | 84 |
| I2C Operation/Pin Config | 9.4.2 | 85 |
| Master Mode Overview | 9.4.6 | 96 |
| Baud Rate Generator | 9.4.7, Table 9-3 | 98 |
| Start Condition Timing | 9.4.8, Figure 9-19 | 100 |
| Repeated Start Timing | 9.4.9, Figure 9-20 | 101 |
| Master Transmission | 9.4.10, Figure 9-21 | 102-103 |
| Master Reception | 9.4.11 | 102 |
| Acknowledge Timing | 9.4.12, Figure 9-23 | 105 |
| Stop Condition Timing | 9.4.13, Figure 9-24 | 105 |

#### How to Review

**A. Verify baud rate calculation (`i2c.c:60-72`):**
1. Open datasheet Section 9.4.7, Table 9-3 (page 98)
2. Formula: `FSCL = FOSC / (4 * (SSPADD + 1))`
3. With SSPADD=9: `4,000,000 / (4 * 10) = 100,000 Hz` ✓

**B. Verify SSPCON mode bits (`i2c.c:84`):**
1. Open Register 9-4 (page 83)
2. Check SSPM3:SSPM0 = 1000 for "I2C Master mode, clock = FOSC/(4*(SSPADD+1))"
3. Confirm SSPEN (bit 5) = 1 to enable module
4. `0b00101000` = bit5=1, bits3:0=1000 ✓

**C. Verify SSPSTAT configuration (`i2c.c:52`):**
1. Open Register 9-3 (page 82)
2. SMP bit (bit 7): "1 = Slew rate control disabled for standard speed mode (100 kHz)"
3. `0b10000000` = SMP=1 ✓

**D. Verify Start/Stop/Restart sequences:**
1. Compare `i2c_start()` against Figure 9-19 (page 100): SEN→wait SSPIF
2. Compare `i2c_restart()` against Figure 9-20 (page 101): RSEN→wait SSPIF
3. Compare `i2c_stop()` against Figure 9-24 (page 105): PEN→wait SSPIF

**E. Verify read/write sequences:**
1. Compare `i2c_write()` against Section 9.4.10: SSPBUF write→wait→check ACKSTAT
2. Compare `i2c_read_ack/nack()` against Section 9.4.11-12: RCEN→wait→read SSPBUF→ACKDT→ACKEN→wait
