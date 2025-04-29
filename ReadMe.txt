# TetroBlue
STM32-Tetris project with ArduinoUNO and C++
Later (maybe) IDE will be replaced with Clanq++ and Linker with ST-Link Utility.

"Scheme.png" you can see the schematics and pins

"Case.m3d" is Compad-3D-compatible 3D case for that project with USB-C for charging, SD-reader for .vex-format games (maybe later), and 4 pins for uploading a code on MCU

Software you need:
-Arduino UNO
  Libraries for UNO:
  -Adafruit GFX Library 
  -Adafruit ILI9341

Board settings:
STM32 Core -> Generic STM32F1 Series -> STM32F103C8 

Hardware you need:
-STM32F103C8T6 Blue Pill
-TFT 2,4" SPI ILI9341 240x320 Display (Later - 4,0" 320x480)
-X/Y Joystick
-6 Playable Swtitches + 1 ON/OFF switch
-1 speaker 
-ST-Link programmer
-a lot of wires
Optional: USB-C to 3,3v converter board + battery (2x1000mah in my variant)
