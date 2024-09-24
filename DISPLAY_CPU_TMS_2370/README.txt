READ THE README_FIRST FILE NOW PLEASE!!!

Firmware by R. Kauls
PCB gerbers by M. Bostock

TL;DR:- 

Parts needed:

PIC16F737 microcontroller
17 x DTB113Z digital transistors. These are pre-biased for you.
17 x 100K 0603 SMD resistors
1 x 100nF Capacitor
Header pins to fit (I use 2mm pitch pins and remove the plastic "holder".
Pickit 3 Programmer or other programmer

Easiest to build is the SOIC version. Use the DIY gerber if you want to flash the chip after you have built the complete project.
SSOP and SSOP inverted are slightly more difficult but is a lot neater.
The SSOP inverted board is in case you want to keep the original chip fitted. You might have to disconnect some pins to old chip, eg power... But hides the custom PCB and keeps it looking "original"
Both SSOP boards have flashing / programming capability as standard.
LOOK AT THE PNG PICTURES IN EACH FOLDER!!!
==========================================


This project has a SLIM PITCH of 0.070 inches(1.778mm) Standard 2.54mm (0.100) pitch pins will NOT FIT.
I made and tested an adapter board but it was in the way of the morse keyer switch PCB, so has been discarded.

You can use either a SOIC-28 or a SSOP-28 PIC controller with this. Just make sure you use the right gerber for the right size chip!!!
Gerbers (ZIP) and diagram (PNG) are organised into folders as follows:

FT757GX_Display_TMS_SOIC_VERSION - Soic version. Uses both sides of PCB. Easiest to build due to SOIC being bigger (maybe)?
FT757GX_Display_TMS_SSOP_STANDARD - SSOP version. Fits the standard way in place of old chip (I personally use this in my lab-rat 757)
FT757GX_Display_TMS_SOIC_INVERTED - SSOP version but back to front, so a 0.6mm board can be built and it will fit under the VFD tube (some OLD pins will need to be flattened flush, but thats no big deal is it?)


Programming pins are as follows on all boards that have programming header:

MCLR
5V
GND
PGD
PGC

This matches the Pickit 3 which is the cheapest and most versatile programmer for these. Differences between "OEM" and "DIY" are highlighted below.

OEM gerbers: No programming header or pads. It is assumed programming will be handled by external clip or pre-programmed prior to fitting
DIY gerbers: Pickit 3 compatible header (2 sided boards/NON TSSOP) or pads (TSSOP version) supplied for your convenience. There is a little LNK jumper to link after programming (MCLR-GND)

DIY flush mount board added. This uses a PIC16F737-I/SS TSSOP package. Can be surface mounted to existing through-holes, pins not required (just fill the holes with solder)
    Fit to same side as original IC.

TESTING: Inverted DIY flush board added. This is the same as the flush board apart from everything is inverted. This is designed slim enough to fit underneath the VFD display!
    Do not attempt to fit this to the NON-DISPLAY SIDE with the components facing you! It will not work. If fitting this in place of chip, components face the front of radio.

TESTING: The TMS1_TH gerber is for a through-hole version of the PIC16F737 IC (PIC16F737-I/SP)



Disclaimer: You have only yourself to blame. Not for commercial use. All info and technical drawings correct at time of upload. E+OE.

READ THE README_FIRST FILE NOW PLEASE!!!
