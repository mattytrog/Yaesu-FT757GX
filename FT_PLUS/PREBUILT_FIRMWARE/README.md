## FTPlus FAQ
I've soldered my FTPlus in, but the screen is blank when I power on.
 - Double-check soldering. Double check you have 2 x resistors, 2 x capacitors, 1 x crystal fitted.
 
I cannot get CAT working
 - Double-check connections. Pressing Mic down button should make your serial TX light(if fitted) light up.
 - Check baud rate. To start with, baud rate is 4800, stop bits 2, try switching off flow control too.
 - Hold M>VFO on power-on to see your current baud rate and use up/down buttons or the dial to change. Press M>VFO again to save.
 
I have an original display chip fitted. There is a slight flicker on the display when CAT is running
 - Normal behaviour. Everything in FTPlus is dealt with using software and timer interrupts...
 - Writing data to Cat TX will cause a very slight pause of display data, which is seen as a momentary increase in brightness.
 - PIC-based display CPUs are unaffected by this
 
What is the refresh rate of the VFD?
 - On original chips, it is between 54-58 Hz. On PIC-based display CPUs, it is 60Hz.
 
Configurator isn't working
 - Ensure nothing else is using CAT. Close any rig control apps down.
 
I cannot access bootloader
 - Are you holding PMS as you turn on rig? You should be. In the updater app, press Check Bootloader and it should help
 - Have you flashed an old HEX file? You could have erased the bootloader. Easily fixable with a Pickit. Send to me and I'll fix it.
 - If using a PICKIT, only flash hex files from v1.6.0 onwards if possible. These include the bootloader already. Some earlier files 
	may have it included, but I cannot remember which. Use CAT and the updater app, with BLHEX files to update.
 
I've held VFO A/B to reset all memories, but my radio is constantly beeping
 - Correct behaviour. Power off and on again to complete the reset.

I can update via PIC no problem and there is no signal transmission. What about your giant warning in the supplement???
 - Oh yeah. I fixed that. It was transmitting because when the PIC enters programming mode (MCLR high), a few instructions
	are ran. One of these was tuning the PLL circuits. So I've removed the initial tune until radio is fully booted.