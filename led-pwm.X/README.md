# LED PWM

Control LED brightness using PWM based on potentiometer position.

## Hardware

- **RA0/AN0**: Potentiometer input (analog)
- **RC2/CCP1**: LED output (PWM)

## Peripherals Used

- ADC (10-bit, left-justified)
- Timer2 (PWM timebase)
- CCP1 (PWM output)

## Notes

PWM frequency is ~3.9kHz. ADC result directly sets duty cycle.
