# TFT_28_ILI9143_ESP32_EXAMPLES
Ejemplos para modulos TFT con ESP32

LIBRARIES

- LOVYANGRFX

- ARDUINO GFX


Full support for ILI9341 based TFT modules in 4-wire SPI mode. 

------------------------

Connecting the display

Display module: ESP32 PIN 
T_IRQ:          -1

T_DO(miso):     22

T_DIN(mosi):    23

T_CS:            5 (touch screen CS)

T_CLK:          18

sdo(mosi):      23

led:            32

sck:            18

sdi(miso):      23

DC:             27 (data/command)

RESET:          33

CS:             14 (display CS)

GND:	          GND	Power supply ground

Vcc:            3.3V	Power supply positive

Make shure the display module has 3.3V compatible interface, if not you must use level shifter!

