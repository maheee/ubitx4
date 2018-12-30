# uBITX 4

The Raduino is a small board that includes the Arduin Nano, a 16x2 LCD display and an Si5351a frequency synthesizer.

The board is manufactured by Paradigm Ecomm Pvt Ltd.

## Hardware

### Oscillators

#### Chip
The main chip which generates upto three oscillators of various frequencies in the Raduino is the Si5351a. To learn more about Si5351a you can download the datasheet from www.silabs.com

#### Usage
The uBITX is an upconnversion transceiver. The first IF is at 45 MHz.

The first IF frequency is not exactly at 45 Mhz but about 5 khz lower, this shift is due to the loading on the 45 Mhz crystal filter by the matching L-network used on it's either sides.

The first oscillator works between 48 Mhz and 75 MHz. The signal is subtracted from the first oscillator to arrive at 45 Mhz IF. Thus, it is inverted : LSB becomes USB and USB becomes LSB.

The second IF of 12 Mhz has a ladder crystal filter. If a second oscillator is used at 57 Mhz, the signal is subtracted FROM the oscillator, inverting a second time, and arrives at the 12 Mhz ladder filter thus doouble inversion, keeps the sidebands as they originally were.

If the second oscillator is at 33 Mhz, the oscilaltor is subtracated from the signal, thus keeping the signal's sidebands inverted. The USB will become LSB.

We use this technique to switch sidebands. This is to avoid placing the lsbCarrier close to 12 MHz where its fifth harmonic beats with the arduino's 16 Mhz oscillator's fourth harmonic

### Harmonic Filters
The four harmonic filters use only three relays. The four LPFs cover 30-21 Mhz, 18 - 14 Mhz, 7-10 MHz and 3.5 to 5 Mhz.

Briefly, it works like this:
- When KT1 is OFF, the 'off' position routes the PA output through the 30 MHz LPF
- When KT1 is ON, it routes the PA output to KT2. Which is why you will see that the KT1 is on for the three other cases.
- When the KT1 is ON and KT2 is off, the off position of KT2 routes the PA output to 18 MHz LPF (That also works for 14 Mhz)
- When KT1 is On, KT2 is On, it routes the PA output to KT3
- KT3, when switched on selects the 7-10 Mhz filter
- KT3 when switched off selects the 3.5-5 Mhz filter

See the circuit to understand this

### Display (LCD)

The Raduino board is the size of a standard 16x2 LCD panel. It has three connectors:

First, is an 8 pin connector that provides +5v, GND and six analog input pins that can also be configured to be used as digital input or output pins. These are referred to as A0,A1,A2,A3,A6 and A7 pins. The A4 and A5 pins are missing from this connector as they are used to talk to the Si5351 over I2C protocol.

Second is a 16 pin LCD connector. This connector is meant specifically for the standard 16x2 LCD display in 4 bit mode. The 4 bit mode requires 4 data lines and two control lines to work: Lines used are : RESET, ENABLE, D4, D5, D6, D7

### PINS

There are two sets of completely programmable pins on the Raduino.

#### Top
First, on the top of the board, in line with the LCD connector is an 8-pin connector that is largely meant for analog inputs and front-panel control. It has a regulated 5v output, ground and six pins. Each of these six pins can be individually programmed either as an analog input, a digital input or a digital output.

The pins are assigned as follows (left to right, display facing you):
```
Pin 1 (Violet), A7, SPARE
Pin 2 (Blue),   A6, KEYER (DATA)
Pin 3 (Green),  +5v
Pin 4 (Yellow), Gnd
Pin 5 (Orange), A3, PTT
Pin 6 (Red),    A2, F BUTTON
Pin 7 (Brown),  A1, ENC B
Pin 8 (Black),  A0, ENC A
```
Note: ```A5```, ```A4``` are wired to the Si5351 as I2C interface!

#### Bottom

The second set of 16 pins on the Raduino's bottom connector are have the three clock outputs and the digital lines to control the rig.

This assignment is as follows :
```
Pin  1, GND
Pin  2, +5V
Pin  3, CLK0
Pin  4, GND
Pin  5, GND
Pin  6, CLK1
Pin  7, GND
Pin  8, GND
Pin  9, CLK2
Pin 10, GND
Pin 11, D2, CW_KEY
Pin 12, D3, TX_LPF_C
Pin 13, D4, TX_LPF_B
Pin 14, D5, TX_LPF_A
Pin 15, D6, CW_TONE
Pin 16, D7, TX_RX

```
- TX_RX: Switches between Transmit and Receive after sensing the PTT or the morse keyer
- CW_KEY: turns on the carrier for CW


## Used Libraries

### Wire.h
The Wire.h library is used to talk to the Si5351 and we also declare an instance of Si5351 object to control the clocks.

### EEPROM.h
The EEPROM library is used to store settings like the frequency memory, caliberation data, callsign etc .

### SI5351

We no longer use the standard SI5351 library because of its huge overhead due to many unused
features consuming a lot of program space. Instead of depending on an external library we now use
Jerry Gaffke's, KE7ER, lightweight standalone mimimalist "si5351bx" routines.

### LiquidCrystal.h

Used to drive the LCD in 4 bit mode.

## Software

### Overview

#### ubitx
Entry point.

#### ubitx_cat
Code to implement the CAT protocol. This is used by many radios to provide remote control to comptuers through the serial port.

#### ubitx_factory_alignment

#### ubitx_keyer

#### ubitx_menu

#### ubitx_si5351
An minimalist standalone set of Si5351 routines.

#### ubitx_ui
