9/10/2024 v0.5 hex and source added. Will still be messy in places
Compile with CCS compiler
========================================

Parts needed:

PIC18F452
2x22pF 0603 SMD or through hole capacitors
1x10k 0603 or through hole resistor
1x4K7 (4.7k) 0603 or through hole resistor
20mhz crystal
turned pins (2x20)

Schematic enclosed...

This is a replacement CPU for the FT757GX transceivers.
PLL calcs, firmware, gerbers and corrections by M. Bostock
Original PLL calcs and schematic by VK2TRP

What is this?

This is a replacement CPU for the FT757GX transceiver by Yaesu.

ENSURE YOUR CPU IS FAULTY BEFORE REPLACING IT WITH THIS!!!
THIS NEEDS TESTING WITH A REAL TMS2370 DISPLAY CPU!!! It should be fine. With my TMS replacement, its fine. Its easy to modify the code IF theres a problem on real TMS chips.

Based on a PIC18F452. Programmed using Pickit3. Hex file built with CCS.

Pickit header on CPU board for your cnvenience.

This is all new code.
Only one missing feature. PMS. What the hell is PMS? How does it work? How is it supposed to work? I might turn this into a frequency scanner.
========================================

Features implemented in software:
As stock:

VFO
Memories
Buttons
MIC buttons
CAT serial (4800 baud, 2 stop bits)
Out-of-band TX inhibit
M>VFO
VFO<M
M<>VFO
Split
Clarifier
Dial lock
Band / 500k UP/DOWN
Memory(MR) management

Additional features:

Compatibility with a PIC-based display CPU
VFO dial acceleration
CB mode
Offset fuction
Enhanced MIC control
+more
========================================
VFO dial acceleration - or "fuzzy logic" if you prefer:
The display counts faster the more the VFO dial is turned.
Slowly - 10 cycles
Faster - 1000
Faster still - 10000
Fastest - MHz
========================================

CB Mode
This places the transceiver in CB mode.

Long-press MR/VFO to activate or deactivate.
Channels 1-40 accessible, all pre-programmed.

Long-press SPLIT to change region from UK allocation to CEPT/FCC allocation (or mid block as we in the UK call it)
"b" in the display when switching means "Britain"
"A E" in the display when switching means "America / Europe"
========================================

CAT is fully implemented (only the features on original design). Tested with FLdigi. PLEASE REPORT IF IT DOESN'T WORK FOR YOU.
To test, send 67 45 23 01 0A (in hex). Display should change to 123456

Extra CAT commands in addition to the original Yaesu ones:

XX XX XX XX OE Return delay (currently useless as CAT tx isn't get implemented)
XX XX XX 00 10 Radio status(75 bytes returned) (as in FT757GXII but not currently working as I need a sample data stream to compare)
XX XX XX XX FC Temporary wideband, regardless of front panel switch position (Until next reboot)
XX XX XX XX FD CB mode toggle
XX XX XX XX FE Reset EEPROM and reboot
XX XX XX XX FF Reboot CPU
========================================

To reset EEPROM, hold VFO A/B at any time. Release. Listen for the beeps. Processor will reboot, resetting EEPROM. 
========================================

MIC buttons
These are enhanced over stock

In VFO mode...
UP/DOWN frequency up and down. Press up and down THEN hold fast to increase speed
Press FAST to swap between VFO A/B
Hold FAST for 3 seconds to change to MR mode (let go after the beeps)
Keep holding to flick into CB mode

In MR mode...
UP/DOWN channel up and down
Hold FAST for 3 seconds to swap to VFO mode
Keep holding to flick into CB mode

In CB mode
UP/DOWN, channel up/down
press FAST to swap between British and Cept / FCC bands (Muppets or mid-band)
Hold fast for 3 seconds to go back into VFO mode
========================================

MASTER OFFSET
This allows you to alter the displayed frequency vs actual frequency.
This is handy if your radio is slightly "off-centre" and you do not fancy opening it up and pot-twiddling.

To use:
Hold M>VFO for 3 seconds until screen goes blank. The radio will now be on 7000.0
Listen or transmit (with frequency counter ideally) and adjust dial. Value will be displayed (on the left for negative values, right for positive)
To test a different frequency, press BAND UP/DOWN to go up or down 1 mhz.

Once your desired offset is reached, press M>VFO again to save. You will hear beeps and radio will reboot.
Your display will now reflect your actual frequency. The N-code calculations do not change over the whole PLL range, so if you set it, it should be right everywhere.
NOTE: IMPORTANT: This will not help if your radio is damaged or has faulty diodes. To reset back to default values, reset EEPROM by holding VFOa/B for 3 seconds.
========================================

Not implemented yet:
RS232 CAT transmit. Is feature complete but not included for the moment as still deciding how to do it. I need a copy of a data stream from somebody please!

BUGS: There is bound to be some. Please open issue on here or find me on FB

Only fit this if your CPU(s) are faulty. Yes, some new features are handy, but thats no reason to rip out a perfectly good chip.
In capitals: Ahem: IF YOU ARE THINKING OF DOING THIS BECAUSE OF LOW RECEIVE, TX PROBLEMS, NO POWER ETC... DON'T. 
IT WILL BE YOUR SWITCHING DIODES IN THE BPF THAT ARE FAULTY. THIS WILL NOT HELP YOU.


Disclaimer:
You have only yourself to blame. This is BETA software. Expect bugs and mistakes. If you kill your radio or diagnose the incorrect fault, not my problem.
But will still try to help.

