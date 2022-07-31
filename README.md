# ESCAPE_BOX
Wedding gift for friends that like escaperooms

# Project assumption
## Puzzles
1. Connect cables - connect image/text 8/10 sets, correct answer play sounds of birds 
2. Put bags with seeds in order from previous puzzle to light led matrix

| o | o |
|---|---|
| x | o |
| x | x |
| o | x |
3. Open presscode padlock (leds show correct pattern)

*other padlock time*
4. Some padlock opens pressure sensor box.
5. Blow sensor to open box(servo) with toy car
6. Ride good path over reedswitches to open another box(servo)
7. Smells from this box are hint for next puzzle
8. Pressing color pushbuttons in correct order plays woodpecker sounds
9. Knock woodpecker pattern to open last box

# BOM
- power supply
- DC jack socket
- dc/dc converter
- arduino mega
- banana plugs and sockets
- wires
- 3x servo sg90
- DFplayer
- sd card
- speaker
- MFRC522 rfid reader
- 5mm leds with bracket
- bmp280
- few reed switches
- 4 push buttons red/green/blue/yellow
- piezo knock sensor
 

# GPIO usage
- 8-10 Analogs for banaplugs
- 1 uart for DFplayer
- 1 spi for rfid
- 1 i2c for bmp280
- 8 GPIO OUTPUT for led matrix
- 8? GPIO INPUT for reed switches
- 3 PWM for servo
- 4 GPIO INPUT for pushbuttons
- 1 GPIO INPUT for knock sensor

## Summary (max)
- 10 Analogs
- 1 UART
- 1 SPI
- 1 $I^2C$
- 21 GPIO
- 3 PWM
