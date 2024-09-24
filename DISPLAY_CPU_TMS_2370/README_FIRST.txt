Original text by R. Kauls
Clarifications and additions by M. Bostock

Repairing the display unit at Yaesu FT757GX and 757GXII

1. How does it work?

The display unit of this radio consists of the following modules:

> a dc/dc converter to provide the voltages for the VFD tube and a negative voltage for the transceiver.
> a VFD tube
> a microcontroller TMS2370 to control the VFD and to generate interrupts for the transceiver cpu
> a decoder and some leds for displaying the mode (GXII only)
> some diodes and keys 
> a transistor for resetting the TMS2370

2. What is the trouble with your display?

Display dim?

Mostly the dc/dc converter has a problem. The transistor and the electrolytics are worn out often.
So the display may get more and more dim. Sometimes the transceiver stops receiving (the negative voltage is missing), too.
But the general function of this converter is easy to check:

> Connect the display board to 12V (reffer to the circuit diagram of your transceiver)
> Check the voltage at J01  . Should be -8V. There should be no ripple
> pin 17 display CPU should be -28V . 

If the voltages are tolerating more than 20 percent, you should check the transistor and the diodes.
I know, this is not so easy to do, sometime the displaytube is above the solder dots. This tube has to be handled 
with extreme care! It is obsolete and it is very difficult to replace it by an other one!

But beware : It is recommended to replace the electrolytics when you have to disassemble this board! Check the solder dots!
             Replace the keys if these were worn out!


Display dark or strange characters on it?

Ok, this is worse. But when the transceiver is working still, please check the dc/dc converter first!
If the converter is ok, then we have to look at the controller.

> Connect the pcb to a dc source 12V and 5V  (refer to the circuit diagram of your transceiver)
> Check display CPU pins 37 and 38 with a scope or meter. You should see a 500Khz signal.
If not, check
	> 5 V at pin 1 and pin 18 with respect to gnd
	> Rs, Cs and resonator (the yellow plastic part that looks like a strange capacitor)
	> D07 and C05
	If there is no faulty part, the controller is defective

Check the controllers reset circuit.
> Connect the pcb to a dc source 12V and 5V  (refer to the circuit diagram of your transceiver)
> Check display CPU pin 27 with a dvm. You should read abot -4V
> connect JP01 pin 12 RST (white wire) to Gnd.
> Recheck pin 27 with a dvm. You should read abot 5V

If not, check

	> Q03, R05, R09, R10, C05, D07

If this unit works, we have to do the last check:

Check VFD Tube:

> Connect the pcb to a dc source 12V and 5V  (reffer to the circuit diagram of your transceiver)

Touch the following pins to 5V (with a 100 ohm resistor for safety)...

> Display CPU pin 14 (Grid 0, R11) and Display CPU pin 26 (segment a). Segment should light.

Then connect pin 14 to 5v again, to segment b (different segment on SAME DIGIT should light).

Below is pin numbers for grids and segments (anodes)

GRID		DISPLAY CPU PIN
0		14
1		13
2		12
3		11
4		10
5		9
6		8
7		6
8		7

SEGMENT		DISPLAY CPU PIN
ANODES
A		26
B		25
C		24
D		23
E		22
F		21
G		20


You "can" do the test quicker by connecting all the above pins to 5v and every segment should light up.

The main local unit CPU (not the display CPU) is dependent on a 60hz signal created by the display CPU

Check this with a scope or a meter as follows

> Connect the pcb to a dc source 12V and 5V  (reffer to the circuit diagram of your transceiver)
> Check pin 16 display cpu (R13) OR PB2 (orange wire) on JP01, there should be a square signal, down to - 28V, up to 5V, about 60 cycles/second.

Without this, the main CPU cannot run correctly, or create the INT interrupt needed for the display.

Notes to builders of this project:

The pin pitch is 1.778mm(0.70 inches).
	Pin headers and sockets can be difficult to find. An alternative is use the pins only,
	out of a 2mm pitch header, soldering in place using the black plastic as a mounting. We then remove the pin from the mounting once soldered

If using the DIY PCB, a programming header is supplied for your convenience. This matches the Pickit 3. There is also a solder jumper that you must bridge
once programming is complete (this ties MCLR to GND). If fitting header pins to the programming header port, ensure they are mounted the LONGEST SIDE
on the same side as the PIC IC chip. The silkscreen is under the board, but please fit a header as I've described. The silkscreen is just a guide and I didn't
want it on the front of the PCB.

Be VERY careful removing the faulty TMS chip. These traces are easily damaged. Tying a bit of wire under the chip, to hold it like a "sling" while heat
is applied helps during removal. This allows you to apply pulling force to the faulty chip, GENTLY!!!

Remove the yellow resonator before applying heat. However if you MELT THE YELLOW RESONATOR, don't worry too much. It is only needed for original TMS chip.

Not for commercial use!!!

























	
