# Sleep with WDT Wake

Low-power sleep mode with watchdog timer wake-up to blink an LED.

## Hardware

- **RD0**: LED output

## Peripherals Used

- WDT (Watchdog Timer) - enabled in CONFIG.h
- SLEEP instruction

## Notes

WDT postscaler set to 1:64 (~1.1s per wake). LED toggles every ~3 wake cycles (~3.3s).
All unused pins set as outputs and driven low to minimize power consumption.
