READ THE README_FIRST FILE NOW PLEASE!!!

Firmware by R. Kauls
PCB gerbers by M. Bostock

TL;DR:- 
Easiest to build is the SOIC version. Use the DIY gerber if you want to flash the chip after you have built the complete project.
SSOP and SSOP inverted are slightly more difficult but is a lot neater.
The SSOP inverted board is in case you want to keep the original chip fitted. You might have to disconnect some pins to old chip, eg power... But hides the custom PCB and keeps it looking "original"
Both SSOP boards have flashing / programming capability as standard.
LOOK AT THE PNG PICTURES IN EACH FOLDER!!!


This project has a SLIM PITCH of 0.070 inches(1.778mm) Standard 2.54mm (0.100) pitch pins will NOT FIT.
I made and tested an adapter board but it was in the way of the morse keyer switch PCB, so has been discarded.

You can use either a SOIC-28 or a SSOP-28 PIC controller with this. Just make sure you use the right berber for the riht size chip!!!
Gerbers and diagram are organised into SSOP, SOIC folders etc... Take a look

OEM gerbers: No programming header or pads. It is assumed programming will be handled by external clip or pre-programmed prior to fitting
DIY gerbers: Pickit 3 compatible header (2 sided boards/NON TSSOP) or pads (TSSOP version) supplied for your convenience. There is a little LNK jumper to link after programming (MCLR-GND)

DIY flush mount board added. This uses a PIC16F737-I/SS TSSOP package. Can be surface mounted to existing through-holes, pins not required (just fill the holes with solder)
    Fit to same side as original IC.

TESTING: Inverted DIY flush board added. This is the same as the flush board apart from everything is inverted. This is designed slim enough to fit underneath the VFD display!
    Do not attempt to fit this to the NON-DISPLAY SIDE with the components facing you! It will not work. If fitting this in place of chip, components face the front of radio.

TESTING: The TMS1_TH gerber is for a through-hole version of the PIC16F737 IC (PIC16F737-I/SP)



Disclaimer: You have only yourself to blame. Not for commercial use. All info and technical drawings correct at time of upload. E+OE.

READ THE README_FIRST FILE NOW PLEASE!!!
