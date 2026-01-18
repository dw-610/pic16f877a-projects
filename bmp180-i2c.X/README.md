# BMP180 I2C

Read temperature and pressure from a BMP180 sensor via hardware I2C.

## Hardware

- **RC3/SCL**: I2C clock
- **RC4/SDA**: I2C data
- **RD0**: LED status indicator

Uses a BMP180 breakout board with onboard 3.3V regulator and pull-ups.
No external pull-ups needed.

## Peripherals Used

- MSSP (I2C Master mode, 100 kHz)

## Files

| File | Purpose |
|------|---------|
| `main.c` | Initialization and main loop |
| `i2c.c/h` | Hardware I2C master driver |
| `bmp180.c/h` | BMP180 sensor driver with compensation |

## Notes

See [PLAN.md](PLAN.md) for detailed implementation notes and datasheet references.
