# TFT_MODULES_ESP32_EXAMPLES
Ejemplos para modulos TFT con ESP32

Features

Full support for ILI9341, ILI9488, ST7789V and ST7735 based TFT modules in 4-wire SPI mode. 


Connecting the display

ESP32 pin	Display module	Notes

T_IRQ:      -1
T_DO(miso): 22
T_DIN(mosi):23
T_CS:        5 (touch screen CS)
T_CLK:      18
sdo(mosi):  23
led:        32
sck:        18
sdi(miso):  19
DC:         16 (data/command)
RESET:      33
CS:         14 (display CS)
GND:	      GND	Power supply ground
Vcc:        3.3V	Power supply positive

Make shure the display module has 3.3V compatible interface, if not you must use level shifter!

