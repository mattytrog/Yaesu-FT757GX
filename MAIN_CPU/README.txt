!!Compile with CCS compiler!!
0.76b Changelog
Fixed weird bug where the CAT datastream digits would become misaligned (compiler optimisation doing it?)
Just changing all buffered bytes to int32s corrected it. Even though the bytes are obviously under int32 (they are int8),
they were becoming misaligned, resulting in the LSBs (bytes 2 & 1) of the CAT stream getting "lost" during the frequency multiplication calculation.
Looks like they were added elsewhere (or lost), resulting in lower frequency being displayed
========================================

2/11/24 v0.75b hex and source added again
11:57PM I accidentally uploaded my debugging version earlier this evening. I have reuploaded the correct version now

Fine tune facility. display moves a digit to the left when incrementing slowly (disabled by default as could be annoying). Hold D.LOCK to try it.

Single offset, accessible from long press of CLAR. Hold CLAR to set, if your radio needs its frequency correcting. See below!
To remove the offset, just hold CLAR and ensure the number 0 is showing, then save. 0 offset!
========================================

3 different "dials" to try. 
Dial 0 is just a standard non-accelerated dial. 
Dial 1 is timer-based ( just counts up faster the faster you spin). 
Dial 2 is sample-based. Permits you to give a sharp turn of the dial to jump a high amount, or just slowly turn for normal.
All selectable with long press of M>VFO
========================================

26/10/24 v0.72 hex/source added.
Another bugfix. Turning off in CB mode, under a temporary wideband would cause rig to be locked in CB, with TX inhibit on. Fixing would need an EEPROM reset. 
This is now fixed. One line of code! 
========================================

24/10/24 v0.71 hex/source added. Small bugfix release.
Improved 10hz counting (should be correct now)
Restored missing custom CAT commands
========================================

22/10/24 v0.7 hex and source added. Take a read.
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

Press SPLIT to change region from UK allocation to CEPT/FCC allocation (or mid block as we in the UK call it)
Long-press SPLIT to go into 80ch mode
"b" in the display when switching means "Britain"
"CE" in the display when switching means "America / Europe"
========================================

CAT is fully implemented (only the features on original design). Tested with FLdigi. PLEASE REPORT IF IT DOESN'T WORK FOR YOU.
To test, send 67 45 23 01 0A (in hex). Display should change to 123456

Extra CAT commands in addition to the original Yaesu ones:

XX XX XX XX FC Temporary wideband, regardless of front panel switch position (Until next reboot). See caution below regarding CB mode...
XX XX XX XX FD CB mode toggle. Wil only work when widebanded, either through switch or temporary FC command as above
XX XX XX XX FE Reset EEPROM and reboot
XX XX XX XX FF Reboot CPU
========================================

To reset EEPROM, hold VFO A/B at any time. Release. Listen for the beeps. Processor will reboot, resetting EEPROM. 
========================================


Only fit this if your CPU(s) are faulty. Yes, some new features are handy, but thats no reason to rip out a perfectly good chip.
In capitals: Ahem: IF YOU ARE THINKING OF DOING THIS BECAUSE OF LOW RECEIVE, TX PROBLEMS, NO POWER ETC... DON'T. 
IT WILL BE YOUR SWITCHING DIODES IN THE BPF THAT ARE FAULTY. THIS WILL NOT HELP YOU.


Disclaimer:
You have only yourself to blame. This is BETA software. Expect bugs and mistakes. If you kill your radio or diagnose the incorrect fault, not my problem.
But will still try to help.

