# PIC16F877A Embedded Projects

A collection of learning projects for the PIC16F877A microcontroller, demonstrating core embedded concepts from GPIO to serial communication.

## Target Hardware

- **MCU:** PIC16F877A (8-bit, 4 MHz external crystal)
- **Programmer:** PICkit 4 or compatible ICSP programmer

## Projects

| Project | Description |
|---------|-------------|
| `PIC16F_LED_Digital_Input.X` | GPIO basics: button input with debouncing, LED output |
| `PIC16F_Timer2_Test.X` | Timer2 interrupts for LED control at different rates |
| `PIC16F_LED_PWM.X` | ADC input from potentiometer, PWM output for LED brightness |
| `PIC16F_Sleep.X` | Low-power sleep mode with watchdog timer wake |
| `PIC16F_SPI.X` | SPI master configuration and data transmission |

Each project includes a KiCAD schematic in its subdirectory.

## Toolchain

- **IDE:** [MPLAB X](https://www.microchip.com/mplab/mplab-x-ide)
- **Compiler:** [XC8](https://www.microchip.com/mplab/compilers) (v3.10+)
- **Hardware Design:** [KiCAD](https://www.kicad.org/) 9.0

## Building

1. Open the `.X` project folder in MPLAB X
2. Select your programmer (PICkit 4)
3. Build and program

## Datasheets

Official documentation for components used in these projects:

- [PIC16F877A Datasheet](https://ww1.microchip.com/downloads/en/devicedoc/39582b.pdf) (Microchip)
- [DHT11 Temperature/Humidity Sensor](https://www.digikey.com/htmldatasheets/production/2071184/0/0/1/DHT11-Humidity-Temp-Sensor.pdf) (Digikey)
- [BMP180 Pressure Sensor](https://www.digikey.com/htmldatasheets/production/856385/0/0/1/BMP180-Datasheet.pdf) (Digikey)
- [HD44780 LCD Controller](https://www.sparkfun.com/datasheets/LCD/HD44780.pdf) (common 2004 LCD driver)

## Repository Structure

```
PIC16F877A/
├── PIC16F_*.X/           # MPLAB X projects
│   ├── main.c            # Source code
│   ├── CONFIG.h          # MCU configuration
│   └── */                # KiCAD hardware design
├── pic16f877a_datasheet_markdown/  # Datasheet reference (markdown)
└── Documentation/Local/  # Local datasheets (not tracked)
```
