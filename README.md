# Testing-RX5808-module
  Testing RX5808 receiver module

  Data in registers and RSSI value you can see on Serial monitor.

  created 03.01.2022 
  by Vlad Lesnov
  
  Amended by Martin Glass 31.03.2022, for use with RotorHazard (and similar) projects.

If you want to test whether the SPI bus on your RX5808 is working - here's a utility App that runs on the Nodes, which reads the RX5808 internal registers via SPI.

You can use the Arduino IDE (or other) to load this App onto an Arduino Nano or STM32 Blue Pill board and use it to test the RX5808 modules inside your Timer box. Note: you also need to connect your PC/Arduino IDE via the USB Port on the Nano board to view the results on the Serial monitor.
The App simply cycles through the registers and prints out the readings on the Serial monitor, for different set frequencies.
If you get good readouts from the RX5808, the data in the registers will be shown, and your SPI bus is working.
If you get zeros (or 0xFF) then the SPI link has failed.
