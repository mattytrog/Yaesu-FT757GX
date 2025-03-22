!! PLEASE PLEASE PLEASE READ THE SUPPLEMENT!! ALL THE INFO YOU NEED IS IN THERE, NOT HERE!   
!! BOOTLOADER FACILITY ADDED FROM v1.3.  
!! IF UPGRADING VIA BOOTLOADER, USE BOOTLOADER_VERSION  
!! IF PROGRAMMING WITH PICKIT, OR OTHER PROGRAMMER, USE PICKIT VERSION  
!! DIFFERENT BOOTLOADERS WITH DIFFERENT SPEEDS AVAILABLE.  

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

Based on a PIC18F452. Programmed using Pickit3. Hex file built with CCS.

Pickit header on CPU board for your cnvenience.

Only fit this if your CPU(s) are faulty. Yes, some new features are handy, but thats no reason to rip out a perfectly good chip.
In capitals: Ahem: IF YOU ARE THINKING OF DOING THIS BECAUSE OF LOW RECEIVE, TX PROBLEMS, NO POWER ETC... DON'T. 
IT WILL BE YOUR SWITCHING DIODES IN THE BPF THAT ARE FAULTY. THIS WILL NOT HELP YOU.


Disclaimer:
You have only yourself to blame. This is BETA software. Expect bugs and mistakes. If you kill your radio or diagnose the incorrect fault, not my problem.
But will still try to help.

