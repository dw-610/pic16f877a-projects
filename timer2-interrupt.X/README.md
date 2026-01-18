# Timer2 Interrupt

Demonstrates Timer2 interrupt to blink an LED independently of main loop.

## Hardware

- **RD0**: LED controlled by ISR (~7.6 Hz)
- **RC2**: LED controlled by main loop (1 Hz)

## Peripherals Used

- Timer2 (1:16 prescaler, 1:16 postscaler, PR2=255)
- Interrupt on Timer2 match

## Notes

Shows how ISR-driven timing runs independently of blocking delays in main().
