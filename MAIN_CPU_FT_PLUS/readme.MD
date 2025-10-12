!! PLEASE PLEASE PLEASE READ THE SUPPLEMENT!! ALL THE INFO YOU NEED IS IN THERE, NOT HERE!
-

What is this?
-
This is a replacement CPU for the FT757GX transceiver by Yaesu.

ENSURE YOUR CPU IS FAULTY BEFORE REPLACING IT WITH THIS!!!

Changelogs go here:

v1.4.3b Changelog
-
Bugfix release. Offset calculation was changed to an incorrect value, due to out-of-band check actually changing global frequency, rather than just checking it and returning true/false.


v1.4.3a Changelog
-
Fixed VFO B being duplicated into VFO A (Flrig Kenwood mode)  
Fixed VFO B not updated from cold start Kenwood mode  
Fixed button tuning receiving changes over CAT  
Removed code duplication, saving 2% on hex size  
Accelerated dial adjusted for more instant speed and control  
Changed beep hold-high period to ensure compatibility  
  with PIC-based VFO dial boards  
Added and tested support for PIC18F4520  


