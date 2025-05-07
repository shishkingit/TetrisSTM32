# TetroBlue
STM32-Tetris project

"Scheme.png" you can see the schematics and pins

"Case.m3d" is Compas-3D(V14)-compatible 3D case for that project with USB-C for charging, SD-reader for .vex-format games (maybe later), and 4 pins for ST-link programmer

-------------------

Software you need:
-Arduino IDE
{
  Libraries for IDE:
  -Adafruit GFX Library 
  -Adafruit ILI9341
}

Board settings:
STM32 Core -> Generic STM32F1 Series -> STM32F103C8 

-------------------

Hardware you need:
-STM32F103C8T6 Blue Pill
-TFT 2,4" SPI ILI9341 240x320 Display (Later - 4,0" 320x480)
-X/Y Joystick
-6 Playable Swtitches + 1 ON/OFF switch
-1 speaker 
-ST-Link programmer
-a lot of wires
Optional: USB-C to 3,3v converter board + battery (1000mah in my variant)
