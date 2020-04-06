# chambrePousse

Arduino code for an automated bread rise chamber.
It controls a cold thing (e.g. a fridge) and a warm thing (e.g. a lightbulb).
The idea is to put your dough in the fridge overnight, and get it to proof while you sleep, to get perfectly proofed bread/brioche/etc. when you get up.

## Why the name

a "Chambre de Pousse" is french for a rise chamber. It litterally means "grow room".

## Hardware needed

- Two 4-digits 7 segments displays with their controller (TM1637):
  - one for the time
  - one for the temperature
- Three buttons:
  - one to set the time
  - one to set the temperature when keeping cold
  - one to set the proofing temperature
- Three LEDs and their corresponding resistor
  - one to show when the cold is on
  - one to show when the warmth is on
  - one to show when the chamber is proofing
- A clicky encoder
- An RTC module (I'm using a DS3231)
- A DallasTemperature-compatible temperature probe
- Two 5v relays
  - one to trigger the cold thing
  - one to trigger the warm thing

## How to use

On first start (and every time the RTC module looses the time), turning the encoder will set the time.
While pressing both temperature setting buttons (low temperature and high temperature) the encoder sets the displays brightness.
