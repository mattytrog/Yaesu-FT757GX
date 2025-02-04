![Alt text](https://github.com/mattytrog/Yaesu-FT757GX/blob/main/MAIN_CPU/readme.png?raw=true)
!(https://github.com/mattytrog/Yaesu-FT757GX/blob/main/MAIN_CPU/readme.png)

!!Compile with CCS compiler!!


0.95b Changelog

NEW FEATURE: ZERO BIT SET (Off-frequency correction):

Two modes possible. Hold VFO<>M to change these modes. 2 beeps = single offset, 3 beeps = per-band offset(default single offset)
In single offset mode, there is one global offset across all bands, as we have been using.
In Per-band offset mode, you can set different offsets per band. You can set these up PER MODE. eg you can align 7mhz based on LSB with one offset and 14mhz for another one

IMPROVEMENTS: MEMORY MANAGEMENT:

Code completely rewritten for less ambiguity. Confirmation beeps and a blink of the chosen memory channel when saving
When in VFO mode, you can change which channel you save to WITHOUT entering MR mode. Just hold down BAND UP or BAND DOWN until it beeps and the display flashes your new channel
When in MR mode, you can treat the tuning dial as a third VFO(Starting from your saved frequency obviously). Tune up and down, without affecting VFO A or B. To change bands, hold BAND UP or BAND DOWN until it beeps and band changes
VFO<>M, VFO>M and M>VFO all work regardless of mode now (apart from CB of course)

SERIAL IMPROVEMENTS:

Hold VFO A/B during power on (so hold VFO A/B THEN power on), to transmit a help screen over serial, outlining the command set
Easy serial frequency setting option. Enter your frequency, ordinarily, followed by 0F...eg 070000000F to set 7mhz... 277812500F to set 2778125. Remember the 0F at the end and any zeroes if frequency is under 10mhz
Easy memory programming. Same as easy serial, but instead of 0F at the end, put a value between 10 and 1E (0 and E). eg to save 7mhz to channel 7, you would enter 0700000017
You can also type "help0" as ASCII to print the command set once again, (or power cycle the radio, holding VFO A/B7)
FREQUENCIES MUST BE 8 BYTES TO BE ACCEPTED, PLUS THE COMMAND AT THE END, MAKING 10 BYTES.

=================================================================================================

0.9b Changelog
Happy 2025! Time this firmware had a name I think...
CHANGE: Made acceleration dial more predictable and less insane. Values can still be changed over serial
BUGFIX: Going in to offset programming the first time works as expected, however further attempts would result in incorrect garbage data and offsets. This was because of loading the saved offset without the direction marker
(which is offset value + 1 million... The direction of the offset wasn't being taken into account(eg removing of the 1 million marker). This is now fixed.
PMS further improved. Last scanned channel / mem channel / CB channel / frequency is stored to cache, rather than going back to the previous pre-PMS value
When in VFO mode, push in 500k, then press PMS. Current band will be scanned. Band plans are correct as of 25/01/25. eg on 40 meters, 7.0mhz to 7.2mhz will be scanned... 80 meters, 3.5 to 3.8 etc etc
For standard scan, eg from 7.0mhz to next yaesu "band", which is 10.1mhz press 500k again.

0.85b Changelog
All 0.84 additions
PMS system improved: (Without 500k button) CB scanning - Scan current programmed region 1-40... With 500k button - 80 channel CB scan (both regions)
Option for accelerate microphone in source. Disabled by default. It can be annoying

0.84a Experimental Changelog
New 10hz counting method. The whole CPU system now uses the up/down counter as a comparator, with the pulse as a sense to tell if dial is spinning.
This has resulted in a 10hz resolution possible on the microphone, which wasn't possible under this firmware, or firmware by others
Microphone steps have been changed to Yaesu stock (10hz), with a time-based acceleration function.
My bench frequency counter only has 25hz resolution for some reason, so ive tested it on the scope and it is working fine in my tests.

0.83b Changelog
Mostly a minor update. Reworked the mic button code.
In VFO mode:
Press FAST once to increase band (it loops)
Hold FAST until 2 beeps are heard, to change to MR mode

In MR mode:
Hold FAST until 2 beeps are heard, to change to CB mode (if using the CB version of course). If you are not using the CB version, holding FAST will put you back to VFO mode

In CB mode:
Press FAST once to change region (CE or B) - CEPT or British
Hold FAST until 2 beeps are heard, to go back to VFO mode

Up and down works as you would expect. Press FAST AND your up/down button to go fast. You will probably already be pressing up/down, so just press FAST too. No need to let go of anything.

Other stuff:
FINALLY fixed the 6.9999 bug. I thoght I'd fixed in in 0.82 but I was changing the wrong thing. It was thinking the dial had an input.

Normal tuning is 10khz per revolution. Fine tuning is 5khz per revolution.

0.82b Changelog
Fixed weird counter bug that would make the display show 6.9999 instead of 7.0000(the frequency was correct, it just automatically retarded the frequency)
Program dial speeds over serial (see graphic). Dial speeds also apply to microphone speeds too
Changed dial to 10khz(approx) per revolution. Before, it would do 10khz but only if you turned ultra-slowly. It was more like 5khz per revolution. Yaesu spec is 10khz per revolution.
Changed dial counter to 40000, instead of 20000. I felt the acceleration was kicking in too early.

0.81b Changelog
Initial PMS support. As follows:
	With 500k pressed: Scanning of all saved MR channels. This is stock Yaesu behaviour I believe. How the clicking doesn't drive people mad though, I'll never know.
	Press M>VFO or PMS to exit.
	Without 500k pressed in:
	If in VFO mode and you press PMS, scanning will take place between current band and next band.
	If in MR mode, scanning will take place between current memory channel and next memory channel. If you scan from Channel E (last channel), the end channel will be channel 0
	There will be a 3 second timeout after squelch closes before scanning continues
	If in CB mode, all CB channels will be scanned (1 - 80), so both FCC/CEPT (1-40) and British(41-80)
	To skip current frequency (in frequency scanning mode), nudge the VFO wheel to continue scanning
	To switch off, press PMS switch again (or open the squelch and press M>VFO as Yaesu stock) and the last scanned frequency will become VFO A

Program MR memories over serial
The memory command starts at E0 (for channel 0) to EE (channel 14)
Assemble your command as follows: <100hz><10hz><10khz><1khz><100khz><mhz(under 10)><0><mhz(over 10)>
Example:
To save 12.345.67 (1234567) to memory 4, command would be 67452301E4
To save 27.555.00 (2755500) to memory 8, command would be 00557502E8
To save 7.315.00 (731500) to memory 12, command would be 00157300EC

Fixed missing MR indicator in display
Fixed band data not being sent correctly. Incorrect band could have been selected, which would be on-frequency, or miles off.
Fixed incorrect line decoder command for the counter preset enable function. It was changing bands, rather than the counter.
A twat move by me that I had forgotten to fix. My bad. I guess...
All dials now sample the up/down counter now (apart from the CB channel dial, as its unnecessary)
Changed refresh of display from refreshing every cycle (when a change is detected), to every 120 cycles. This has zero effect
on the display and it permits us to increment the frequency in 10hz steps(yaesu spec), rather than 100hz. So in normal AND 
fine-tune modes, the steps are 10hz as original, with hopefully brilliant speed
Offset frequency setup is now far more sensitive and "accurate", being based of the up/down counter

========================================

0.8b Changelog
TESTED WITH REAL AND PIC BASED TMS2370 VFD DISPLAY DRIVER CHIPS
Fixed incorrect Hz being sent to the mixers. This was because the pin we use for strobing the band data was being strobed too 
quickly for the first digit, leading to almost an off-by-one (is that what its called?) error.
Dial sampling changed to directly sense the feedback from the BCD counter, rather than the old dial pulse. Using the dial pulse is OK, but
pulses are being missed. Even manually pulling the pin high didn`t help. Conclusion: The dial pulse is actually not used for counting in stock radios...
Its merely there to return a value on if the dial is spinning or not.
Result: the dial is far more sensitive now.
Removed type 2 dial. It was crap.
Fixed VFO not being saved, unless there was a pulse on the dial
Fixed TX inhibit vulnerability where you could change the frequency and transmit, thus defeating the switch behind front panel by just pressing MIC button

========================================

2/11/24 v0.75b hex and source added again
11:57PM I accidentally uploaded my debugging version earlier this evening. I have reuploaded the correct version now

Fine tune facility. display moves a digit to the left when incrementing slowly (disabled by default as could be annoying). Hold D.LOCK to try it.

Single offset, accessible from long press of CLAR. Hold CLAR to set, if your radio needs its frequency correcting. See below!
To remove the offset, just hold CLAR and ensure the number 0 is showing, then save. 0 offset!
========================================

2 different "dials" to try. 
Dial 0 is just a standard non-accelerated dial. 
Dial 1 is timer-based ( just counts up faster the faster you spin). 
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

