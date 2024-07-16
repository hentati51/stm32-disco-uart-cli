# UART CLI Application for stm32 dicovery board

## Overview

This project demonstrates a Command Line Interface (CLI) over UART for performing specific commands on an STM32 discovery board. The application receives commands via UART and performs actions related to those commands, such as controlling the state (ON, OFF, TOGGLE, BLINK) of the discovery board's user LEDs (red, green, blue, orange).

This project was undertaken with the goal of learning **FreeRTOS** and mastering its features. It can be extended by adding additional commands to control other peripherals and perform other actions (reading ADC values, generating PWM signals,...)

### Commands

- **<LedColor> <operation> <param (if needed)>**

**Note:** Each command should be terminated with a newline character `\n`.

#### Examples:
- `red on`
- `green off`
- `blue toggle`
- `orange blink 500`
