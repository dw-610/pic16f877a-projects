# SPI Master

SPI master mode demo - transmits incrementing byte values.

## Hardware

- **RC5/SDO**: SPI data out
- **RC3/SCK**: SPI clock
- **RD1**: Slave select (directly controlled)
- **RD0**: LED feedback

## Peripherals Used

- MSSP (SPI Master mode)
- Clock = Fosc/4 = 1 MHz

## Notes

Transmits 0-255 repeatedly at ~5 Hz. LED blinks to confirm transmission.
