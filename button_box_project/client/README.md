# deej volume controller
This is a modified version of deej ([Original deej](https://github.com/omriharel/deej)). I always wanted some simple way of changing volume of different programs instead of using the mixer of Windows so I came across the deej project and decided to give it a try.
![Build](assets/deej_Controller_Build.jpg)
[Extra photos/video](https://imgur.com/a/3zxhjxF)

# Build process
 My first idea was to use an LCD (I2C), an rotary encoder and an Arduino pro micro to display the slider values on the screen in %. I wrote an simple Arduino code that maps the value from 0-100 to 0-1023 the code was working a bit but it wasn't accurate. When decreasing by 5% on the lcd, it decreased sometimes by 4 or 6 in Windows. With some help from other people in the deej Discord we tried to map correctly (the map function doesn't round the value). When achieving this the code still didn't work. So I moved on to modifying deej. After hours of trying to understand how deej works, I found some interessting lines in the serial.go file. In this file is the value read from the arduino transformed into a value Windows understands. I putted off the noise reduction, changed the mapping to 0-100 and removed the "normalize" feature. Created the modified build made my setup working.

# Parts needed
- 1 rotary encoder with pushbutton preferably a module so you do not need a extra filter circuit
- 1 Arduino pro micro (any other clone or Arduino board with an usb connection will do the job)
- 1 LCD (I used an I²C 16*2. When using an normal screen you need to change the library in the arduino code. If you modify the Arduino code you could use a bigger LCD)
- (if needed) filter circuit for rotary encoder
  - 4 10kohm resistors
  - 2 10nF (=0.01µF) ceramic capacitors
- (Extra) I²C EEPROM Module

![Parts used](assets/deej_Controller_Parts.jpg)
I used only 2 resistors since some where presoldered on the backside of the encoder board
![ExampleEEPROM](assets/EEPROM_Module.jpg)
This is an example of an external EEPROM module.

# Schematic
![Schematic](assets/deej_Controller_Schematic.png)
This is the schematic for the deej build without external EEPROM.
![SchematicExternalEEPROM](assets/deej_Controller_External_EEPROM_Schematic.png)
This is the schematic for the deej build with external EEPROM. I used an IC in the schematic but using een module would also work fine (without the resistors).

# Arduino code
The arduino code is based on the stock code from deej but is quite modified. Currently I have 2 versions of it, there is a simple difference between them, the support of EEPROM. EEPROM is some sort of storage that doesn't lose it's data when the power drops down. The values of the sliders get stored into the EEPROM regularly. So when rebooting the deej volume controller, the values from last time will be used. It is important to know that the EEPROM has about 100 000 write cycle and from than it becomes unstable. So use the EEPROM at your own risk. If you want to use EEPROM on your build I suggest going for an external EEPROM module since you can change the IC for cheap.

# 3D printing
As you can see in the picture my enclosure is 3D printed, but the quality and the design is far from good, so currently I'm not putting my design on Thingiverse since I would like to improve to a good design with good print quality.

# Building deej
This is a custom version of the deej and needs to be builded this can be done in the scripts folder.

# Contact
If you got any questions you can start a pull request, issue or contact me on [Discord deej server](https://discord.gg/nf88NJu).
