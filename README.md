# RB50-Button-Testing

This branch hosts the code used to test and obtain data from the Kaipule RB50 BLE button. Documentation of the advertising data bytes and structure can be found here: https://docs.google.com/document/u/1/d/1BD5jUhtNEsc0ZPJ6Qv0R4ZflRWnQhX5IUBeMDvWbsEk/edit?usp=sharing. 

The purpose of this code is to collect data from the RB50 button (with BLE), parse, and publish it to the Particle console. 

Files to flash:
  - kaipuleB-button.ino
  - kaipule-button.cpp (auto generated)

The only current configuration needed is to add the BLE Adress of the RB50 button at the top of the .ino file as a global variable.

