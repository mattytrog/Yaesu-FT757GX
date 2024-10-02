Parts needed:

PIC18F452
2x22pF 0603 SMD or through hole capacitors
1x10k 0603 or through hole resistor
1x4K7 (4.7k) 0603 or through hole resistor
20mhz crystal
turned pins (2x20)

Schematic enclosed...

This is a replacement CPU for the FT757GX transceivers.
Original PLL calcs and schematic by VK2TRP
Firmware, gerbers and corrections by M. Bostock

What is this?

This is a replacement CPU for the FT757GX transceiver by Yaesu.

ENSURE YOUR CPU IS FAULTY BEFORE REPLACING IT WITH THIS!!!

Based on a PIC18F452. Programmed using CCS + Pickit3.

Pickit header on CPU board for your cnvenience.

Building on earlier work by VK2TRP, this is my new version.

Features implemented in software:

As stock:

VFO
Memories
Buttons
MIC buttons
CAT serial (4800 baud, 2 stop bits)


Additional features:

Compatibility with a PIC-based display CPU
VFO dial acceleration
CB mode

VFO dial acceleration - or "fuzzy logic" if you prefer:
The display counts faster the more the VFO dial is turned.
Slowly - 10 cycles
Faster - 1000
Faster still - 10000
Fastest - MHz

CB Mode
This places the transceiver in CB mode.

Long-press MR/VFO to activate or deactivate.
Channels 1-40 accessible, all pre-programmed.

Long-press SPLIT to change region from UK allocation to CEPT/FCC allocation (or mid block as we in the UK call it)
"b" in the display when switching means "Britain"
"A E" in the display when switching means "America / Europe"

CAT is fully implemented (only the features oin original design). Tested with FLdigi. PLEASE REPORT IF IT DOESN'T WORK FOR YOU.
To test, send 67 45 23 01 0A (in hex). Display should change to 123456

To reset EEPROM, hold VFO A/B when powering on. Release. Listen for the beeps. Processor will reboot, resetting EEPROM. 


Not implemented yet:

TX Inhibit
TX inhibit (will be added, thus, rig will be widebanded by default. The switch on the front panel will have no effect for now)
RS232 CAT transmit. Is feature complete but not included for the moment as still deciding how to do it. Choices are:

RS232
Bidirectional single wire (this requires modification of your USB-serial adapter - just a diode adding)
TX/RX separate - the RC3 and RD0 pins have been broken out (these are the 500k & PMS switch). We use one of these pins for RS232 TX 
(as long as the button is not pressed obviously)

BUGS: There is bound to be some. Please open issue on here or find me on FB

Only fit this if your CPU(s) are faulty. Yes, some new features are handy, but thats no reason to rip out a perfectly good chip.
In capitals: Ahem: IF YOU ARE THINKING OF DOING THIS BECAUSE OF LOW RECEIVE, TX PROBLEMS, NO POWER ETC... DON'T. 
IT WILL BE YOUR SWITCHING DIODES IN THE BPF THAT ARE FAULTY. THIS WILL NOT HELP YOU.


Disclaimer:
You have only yourself to blame. This is BETA software. Expect bugs and mistakes. If you kill your radio or diagnose the incorrect fault, not my problem.
But will still try to help.

