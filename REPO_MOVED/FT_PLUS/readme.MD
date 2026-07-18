!! PLEASE PLEASE PLEASE READ THE SUPPLEMENT!! ALL THE INFO YOU NEED IS IN THERE, NOT HERE!
-

What is this?
-
This is a replacement CPU for the FT757GX transceiver by Yaesu.

ENSURE YOUR CPU IS FAULTY BEFORE REPLACING IT WITH THIS!!!  

v1.4.7 Changelog
New display driver.
Responsiveness is improved. Flicker on original TMS chips should be gone now
Overall smoothness should be as good, or better than stock.
Display interrupt timing is completely different now
New adjustments for display driver within source

individual VFO band control
Now each VFO / buffer has its own band control, so EG VFO A could be on band 3 (7.0000mhz) and VFO B could be on 4(10.1000mhz)
When moving up and down bands, only current VFO will be changed. This applies to the extra mr mode VFO too!

Now using 4x frequency buffers to allow FTPlus to run more efficiently directly from RAM
Frequencies are no longer constantly rewritten to EEPROM, they are stored in RAM mostly
eg frequency swapping was done by copying a frequency from EEPROM to another EEPROM
slot, then rewriting. This is no longer done. All swaps occur within RAM and are flushed
to EEPROM when needed.



