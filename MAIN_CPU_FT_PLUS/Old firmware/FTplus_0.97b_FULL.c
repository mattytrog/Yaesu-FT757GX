//Yaesu CPU firmware v0.97b (c)2025
//Written by Matthew Bostock 
//Testing and research Siegmund Souza
//PLL diagrams and valuable info Daniel Keogh

//0.97b Changelog
//FIXES:
//More refinement to Kenwood mode. Missing FLRIG required commands added (FR, FT)

//0.96b Changelog
//NEW FEATURE: Kenwood CAT emulation
//Emulates a Kenwood TS140, enabling tuning of radio both ways. VFO wheel movements are reflected in the app of your choice.
//VFO A/B tuning supported, dummy mode change (just make sure you change the mode switch accordingly), memory modes "should" be supported too but cannot test as I'm only using FLRIG
//This requires a connection to the RS232 TX. You can either use the Mic Down pin (Pin 3 on MIC connector), or cut the wire of J11 pin 5 (AGC out) and connect to the TX header on the CPU.
//This will then give you serial TX/RX out of the CAT port. You will lose AGC on the cat port obviously, but who cares?
//Switch rig off... Hold VFO A/B... Switch on to change modes. The rig will beep until it is power cycled. Mode change complete. 

//0.95b Changelog
//NEW FEATURE: ZERO BIT SET (Off-frequency correction):
//Two modes possible. Hold VFO<>M to change these modes. 2 beeps = single offset, 3 beeps = per-band offset(default single offset)
//In single offset mode, there is one global offset across all bands, as we have been using.
//In Per-band offset mode, you can set different offsets per band. You can set these up PER MODE. eg you can align 7mhz based on LSB with one offset and 14mhz for another one
//
//IMPROVEMENTS: MEMORY MANAGEMENT
//Code completely rewritten for less ambiguity. Confirmation beeps and a blink of the chosen memory channel when saving
//When in VFO mode, you can change which channel you save to WITHOUT entering MR mode. Just hold down BAND UP or BAND DOWN until it beeps and the display flashes your new channel
//When in MR mode, you can treat the tuning dial as a third VFO(Starting from your saved frequency obviously). Tune up and down, without affecting VFO A or B. To change bands, hold BAND UP or BAND DOWN until it beeps and band changes
//VFO<>M, VFO>M and M>VFO all work regardless of mode now (apart from CB of course)
//
//SERIAL IMPROVEMENTS:
//Hold VFO A/B during power on (so hold VFO A/B THEN power on), to transmit a help screen over serial, outlining the command set
//Easy serial frequency setting option. Enter your frequency, ordinarily, followed by 0F...eg 070000000F to set 7mhz... 277812500F to set 2778125. Remember the 0F at the end and any zeroes if frequency is under 10mhz
//Easy memory programming. Same as easy serial, but instead of 0F at the end, put a value between 10 and 1E (0 and E). eg to save 7mhz to channel 7, you would enter 0700000017
//You can also type "help0" as ASCII to print the command set once again, (or power cycle the radio, holding VFO A/B7)
//FREQUENCIES MUST BE 8 BYTES TO BE ACCEPTED, PLUS THE COMMAND AT THE END, MAKING 10 BYTES.

#include <18F452.h>
#fuses HS,PUT, NOWDT,NOPROTECT,NOLVP, BORV42
#use delay(clock=20000000)
#use rs232(baud=38400, xmit=PIN_C6, rcv=PIN_C7, parity=N, stop=2, ERRORS)
//#org 0x7D00, 0x7FFF {} //For bootloader
//limits
#define min_freq 50000
#define max_freq 3199990
#define refresh 120
#define btnrefresh 16
//uncomment for bare minimum size. No mic buttons, dial acceleration, CB or CAT. I advise a bigger PIC!
//#define small_rom 

//remove bits and pieces if you wish, or problem is suspected
#ifndef small_rom
#define include_cat
#define include_mic_buttons
//#define include_mic_accel
#define include_dial_accel
#define include_cb
#define include_offset_programming
#define include_pms
#define store_to_eeprom
#endif

//PMS Settings
#define scan_pause_countVFO 200  // Pause after of squelch break
#define scan_pause_countMRCB 150

#define VFO_dwell_time 50       // Delay after tuning to next frequency. Lower numbers = faster scanning speed, though may overshoot if too fast
#define MR_dwell_time 10       
#define CB_dwell_time 1       

#define VFOflash 4000           //Blink rate of MR indicator during scanning. VFO flash is so high as there are effectively no delays between scanned frequencies
#define mrflash 40
#define cbflash 40

#define default_cat_mode 0       //0 = yaesu, 1 = kenwood
//#define debug
//#define serial_mode
//#define ncodes


# byte PORTA = 0x0f80
# byte PORTB = 0x0f81
# byte PORTC = 0x0f82
# byte PORTD = 0x0f83
# byte PORTE = 0x0f84

# bit dial_clk=PORTA.0  //input  // from q66 counter input (normally high, when counter = 0, goes low for uS). May use for TX output
# bit Q64_C=PORTA.1  //output // cpu pin 4 bit 2 Q64 line decoder
# bit Q64_B=PORTA.2  //output // cpu pin 5 bit 1
# bit Q64_A=PORTA.3  //output // cpu pin 6 bit 0
# bit disp_int=PORTA.4  //output // display interupt
# bit k1=PORTA.5  //output // display bit 0
# bit pina6=PORTA.6
# bit pina7=PORTA.7

# bit CPU_BIT1=PORTB.0  //output // bus data bit 0 pll d0
# bit CPU_BIT2=PORTB.1  //output //          bit 1     d1
# bit CPU_BIT4=PORTB.2  //output //          bit 2     d2
# bit CPU_BIT8=PORTB.3  //output //          bit 3     d3
# bit CPU_33_BIT16=PORTB.4  //output // pll a0
# bit CPU_34_BIT32=PORTB.5  //output //     a1
# bit CPU_35_BIT64=PORTB.6  //output //     a2
# bit CPU_36_BIT128=PORTB.7  //output // strobe for pll2  q42

# bit pb0=PORTC.0  //input  // keypad pb0
# bit pb1=PORTC.1  //input  // keypad pb1
# bit pb2=PORTC.2  //input  // keypad pb2   // also tells us when not to write to the display
# bit sw_500k=PORTC.3  //input  // 500khz step switch
# bit dial_dir=PORTC.4  //input  // Dial counting direction
# bit mic_up=PORTC.5  //input  // mic up button
# bit mic_dn=PORTC.6  //input  // mic down button
# bit pinc7=PORTC.7  //output // remote (CAT) wire (may use this for some sort of debugging output)

# bit sw_pms=PORTD.0  //input  // pms switch (mine is broken)
# bit mic_fast=PORTD.1  //input  // microphone fst (fast) button
# bit squelch_open=PORTD.2  //input  // Squelch open when high (for scanning)
# bit tx_mode=PORTD.3  //input  // PTT/On The Air (even high when txi set)
# bit pind4=PORTD.4  //input  // bcd counter sensing  bit 3
# bit pind5=PORTD.5  //input  // used for saveing vfo bit 2
# bit pind6=PORTD.6  //input  // even the undisplayed bit 1
# bit pind7=PORTD.7  //input  // 10hz component       bit 0

# bit k2=PORTE.0  //output // display bit 1   also these bits are for scanning the keypad
# bit k4=PORTE.1  //output // display bit 2
# bit k8=PORTE.2  //output // display bit 3


int8 BITSA,BITSB,BITSC,BITSD;
# bit   tmp_pin4=BITSA.0
# bit   tmp_pin5=BITSA.1 
# bit   tmp_pin6=BITSA.2 
# bit   tmp=BITSA.3
# bit   gen_tx=BITSA.4 
# bit   dl=BITSA.5  
# bit   active_vfo=BITSA.6
# bit   vfo_mode=BITSA.7
# bit   fine_tune_display=BITSB.0 
# bit   sl=BITSB.1 
# bit   cl=BITSB.2 
# bit   btn_down=BITSB.3 
# bit   dltmp=BITSB.4
# bit   dir=BITSB.5
# bit   pms = BITSB.6
# bit   per_band_offset = BITSB.7
# bit   timerstart = BITSC.0
# bit   long_press = BITSC.1
# bit   setup_offset = BITSC.2
# bit   force_update = BITSC.3
# bit   gtx = BITSC.4
# bit   stx = BITSC.5
# bit   ctx = BITSC.6
# bit   fine_tune = BITSC.7
# bit   AI = BITSD.0
# bit   cat_mode = BITSD.1
# bit   save_AI = BITSD.2

#ifdef include_cb
const int32 cb_channel_bank[80] = {
         2696500, 2697500, 2698500, 2700500, 2701500, 2702500, 2703500, 2705500, 2706500, 2707500,
         2708500, 2710500, 2711500, 2712500, 2713500, 2715500, 2716500, 2717500, 2718500, 2720500,
         2721500, 2722500, 2725500, 2723500, 2724500, 2726500, 2727500, 2728500, 2729500, 2730500,
         2731500, 2732500, 2733500, 2734500, 2735500, 2736500, 2737500, 2738500, 2739500, 2740500,

         2760125, 2761125, 2762125, 2763125, 2764125, 2765125, 2766125, 2767125, 2768125, 2769125,
         2770125, 2771125, 2772125, 2773125, 2774125, 2775125, 2776125, 2777125, 2778125, 2779125,
         2780125, 2781125, 2782125, 2783125, 2784125, 2785125, 2786125, 2787125, 2788125, 2789125,
         2790125, 2791125, 2792125, 2793125, 2794125, 2795125, 2796125, 2797125, 2798125, 2799125
};
int8 cb_channel;
int8 cb_region;
int8 channel_amount;
#endif
const int32 band_bank[11] = {
100000, 180000, 350000, 700000, 1010000,
1400000, 1800000, 2100000, 2400000, 2800000, 3000000
};

const int32 band_bank_edge[11] = {
50000, 200000, 380000, 720000, 1020000,
1435000, 1820000, 2145000, 2500000, 3000000, 3199990
};

const int32 PLL_band_bank[30] = {
//lower limit, upper limit, result
50000, 149999, 0,
150000, 249999, 1,
250000, 399999, 2,
400000, 749999, 3,
750000, 1049999, 4,
1050000, 1449999, 5,
1450000, 1849999, 6,
1850000, 2149999, 7,
2150000, 2499999, 8,
2500000, 3199999, 9
};

//VAL1  VAL2
//>VAL1 && <VAL2 = OK
//<VAL1 && >VAL2 = Out of band
const int32 blacklist[20]= {
150000, 200000,
350000, 400000,
700000, 750000,
1000000, 1050000,
1400000, 1450000,
1800000, 1850000,
2100000, 2150000,
2400000, 2550000,
2800000, 3000000
};

int32 frequency, vfo_frequency, mr_frequency, cb_frequency;
int32 disp_checksum, baud_rate;
int8 mem_channel, PLLband, band, mem_mode, dcs, speed_dial, dhz, speed1, speed2, speed3, speed4, mic_pressed, res1, res2, COMMAND_LENGTH, baud_rate_n;
int8 d1 = 0,d2 = 0,d3= 0,d4= 0,d5= 0,d6= 0,d7= 0,d8= 0,d9= 0,d10= 0, dummy_mode; 

#ifndef store_to_eeprom
//create ram store, follows eeprom-style if enabled
int32 ram_bank[32];
int8 var_bank[16];
#endif

void beep();
#ifdef include_cat
char temp_byte;
char buffer[30]; 
int1 command_received = 0;
int1 command_processed = 0;
int next_in = 0;
long serial_timer = 0;
int1 valid;

#INT_RDA
void  RDA_isr(void)
{
      timerstart = 1;
      
      if(command_processed)
      {
      for (int i = 0; i < COMMAND_LENGTH; ++i)
         {
         buffer[i] = '\0';
         }
      command_processed = 0;
      }
      if(kbhit()) 
      {
         temp_byte = getc();
         buffer[next_in]= temp_byte;
         next_in++;
         serial_timer = 0;
         
         if(!cat_mode)
         {
            if(next_in > (COMMAND_LENGTH - 1)) command_received = 1;
         }
         else
         {
            if(temp_byte == 0x3B)
            {
               if(next_in < (COMMAND_LENGTH - 1)) command_received = 1;
               else
               {
               next_in = 0;
               command_received = 0;
               command_processed = 1;
               while(kbhit()) { delay_ms(10);getc();}
               }
            }
         }
         
         if(command_received)
         {
            buffer[COMMAND_LENGTH] = '\0';             // terminate string with null   
            next_in = 0;
            timerstart = 0;
            while(kbhit()) { delay_ms(10);getc();}
            
         }
      } 
      

}
#endif

void load_10hz(int8 val10);

//only save if value different from current contents
void write32(int8 address, unsigned int32 data)
{
   int32 temp_data;
   int8 i;
   for(i = 0; i < 4; i++)
     *((int8 *)(&temp_data) + i) = read_eeprom(address + i);
   
   if(data == temp_data) return;
   for(i = 0; i < 4; i++)
   write_eeprom(address + i, *((int8 *)(&data) + i));
   //beep();
   //load_10hz(0);
}

unsigned int32 read32(int8 address)
{
   int8  i;
   int32 data;
   for(i = 0; i < 4; i++)
     *((int8 *)(&data) + i) = read_eeprom(address + i);
   return(data);
}

void save32(int8 base, int32 value)
{
//
#ifdef store_to_eeprom
write32 (base * 4, value);
#else
ram_bank[base] = value;
#endif
}

int32 load32(int8 base) 
{
int32 value;
#ifdef store_to_eeprom
value = read32(base * 4);
#else
value = ram_bank[base];
#endif
return value;
}

void save8(int8 base, int8 value)
{
//
#ifdef store_to_eeprom
int8 temp_data = read_eeprom(base + 0xE0);
if(temp_data == value) return;
write_eeprom(base + 0xE0, value);
#else
var_bank[base] = value;
#endif
}

int8 load8(int8 base) 
{
int32 value;
#ifdef store_to_eeprom
value = read_eeprom(base + 0xE0);
#else
value = var_bank[base];
#endif
return value;
}
// 0 1
void save_vfo_f(int8 vfo, int32 value) {save32 (vfo, value);}
int32 load_vfo_f(int8 vfo) {return (load32 (vfo));}

// 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16
void save_mem_ch_f(int8 channel, int32 value) {save32 (channel+2, value);}
int32 load_mem_ch_f(int8 channel) {return (load32(channel+2));}

//20 21 22 23 24 25 26 27 28 29 30 31
void save_offset_f(int32 value)
{
if(per_band_offset) save32 ((PLLband + 21), value);
else save32 (20, value);
}
int32 load_offset_f() 
{
if(per_band_offset) return (load32 (PLLband + 21));
else return (load32 (20));
}

//32
void save_cache_vfo_f(int32 value) {save32 (32, value);}
int32 load_cache_vfo_f() {return (load32 (32));}

//33
void save_cache_mem_mode_f(int32 value) {save32 (33, value);}
int32 load_cache_mem_mode_f() {return (load32 (33));}

//34
void save_clar_RX_f(int32 value) {save32(34, value);}
int32 load_clar_RX_f() {return (load32 (34));}

//35
void save_clar_TX_f(int32 value) {save32(35, value);}
int32 load_clar_TX_f() {return (load32 (35));}

//single bytes. Start at 0xE0
void save_checkbyte_n(int8 res) {save8(31, res);}
int8 load_checkbyte_n() {return (load8(31));}
void reset_checkbyte_n() {save8(31, 0xFF);}

void save_dummy_mode_n(int8 res) {save8(16, res);}
int8 load_dummy_mode_n() {return (load8(16));}


void save_baud_rate_n(int8 res) {save8(15, res);}
int8 load_baud_rate_n() {return (load8(15));}

void save_cat_mode_n(int8 res) {save8(14, res);}
int8 load_cat_mode_n() {return (load8(14));}

void save_band_offset_n(int8 res) {save8(13, res);}
int8 load_band_offset_n() {return (load8(13));}

void save_vfo_n(int8 res) {save8(12, res);}
int8 load_vfo_n() {return (load8(12));}

void save_mode_n(int8 res) {save8(11, res);}
int8 load_mode_n() {return (load8(11));}

void save_mem_ch_n(int8 res) {save8(10, res);}
int8 load_mem_ch_n() {return (load8(10));}

void save_fine_tune_n(int8 res) {save8(9, res);}
int8 load_fine_tune_n() {return (load8(9));}

void save_dial_n(int8 res) {save8(8, res);}
int8 load_dial_n() {return (load8(8));}

void save_cb_ch_n(int8 res) {save8(7, res);}
int8 load_cb_ch_n() {return (load8(7));}

void save_cb_reg_n(int8 res) {save8(6, res);}
int8 load_cb_reg_n() {return (load8(6));}

void save_dcs_n(int8 res) {save8(5, res);}
int8 load_dcs_n() {return (load8(5));}

void save_cache_n(int8 res) {save8(4, res);}
int8 load_cache_n() {return (load8(4));}

void save_speed1_n(int8 res) {save8(3, res);}
int8 load_speed1_n() {return (load8(3));}

void save_speed2_n(int8 res) {save8(2, res);}
int8 load_speed2_n() {return (load8(2));}

void save_speed3_n(int8 res) {save8(1, res);}
int8 load_speed3_n() {return (load8(1));}

void save_speed4_n(int8 res) {save8(0, res);}
int8 load_speed4_n() {return (load8(0));}

void save_all_n()
{
save_vfo_n(active_vfo);
save_mem_ch_n(mem_channel);
save_cb_ch_n(cb_channel);
save_cb_reg_n(cb_region);
save_mode_n(mem_mode);
save_dcs_n(dcs);
save_dial_n(speed_dial);
save_fine_tune_n(fine_tune);
save_band_offset_n(per_band_offset);
}

//!BCD Decoder Q64
//!C - 12, B - 13, A - 10
//!C - CPU4, B - CPU5, A - CPU6
//!Q0 - NC
//!Q1 - PLL1
//!Q2 - Q65 (1)
//!Q3 - Q69 (5)
//!Q4 - Beep
//!Q5 - Dial lock
//!Q6 - TX Inhibit
//!Q7 - N/C
//!Q8 - NC
//!Q9 - NC

//!C  B  A
//!0  0  1     Q1 PLL1
//!0  1  0     Q2 Q65(1)
//!0  1  1     Q3 Q69(5)
//!1  0  0     Q4 Beep
//!1  0  1     Q5 Dial Lock
//!1  1  0     Q6 TX Inhibit
//!1  1  1     SPARE

void restore_pin_values()
{
 Q64_C = tmp_pin4; 
 Q64_B = tmp_pin5;
 Q64_A = tmp_pin6;
delay_cycles(2);
}

void save_pin_values() 
{
 tmp_pin4 = Q64_C;
 tmp_pin5 = Q64_B;
 tmp_pin6 = Q64_A;
 delay_cycles(2);
}

void PLL1()
{
 save_pin_values();
 Q64_A = 1;
 delay_cycles(2);
 Q64_A = 0;
 restore_pin_values();
}

void PLL2()
{
 CPU_36_BIT128 = 1;
 delay_cycles(2);
 CPU_36_BIT128 = 0;
}

void counter_preset_enable()
{
save_pin_values();
Q64_C = 0;
Q64_B = 1; 
Q64_A = 0; 
delay_cycles(2);
restore_pin_values();

}

void banddata()
{ 
 save_pin_values();                
Q64_C = 0;
Q64_B = 1; 
Q64_A = 1;
 delay_cycles(2);
restore_pin_values();
}

void transmit()
{
save_pin_values();
 Q64_C = 1;
 Q64_B = 1;
 Q64_A = 1;
 restore_pin_values();
}

void beep()
{
 save_pin_values();
 Q64_C = 1;
 Q64_B = 0;
 Q64_A = 0;
 restore_pin_values();
}

void errorbeep(int8 beeps)
{
for (int8 i = 0; i < beeps; ++i)
{beep(); delay_ms(200);}
}

void set_dl(dl)
{
   if (dl == 1) //lock dial, return true
   {
      Q64_B = 0;
      Q64_A = 1;
      Q64_C = 1;
   }
   if (dl == 0)
   {
      Q64_B = 0;
      Q64_C = 0;
      Q64_A = 0;
   }
}

void set_tx_inhibit(int res)
{
   if(res == 1)
   {
      Q64_B = 1;
      Q64_C = 1;
      Q64_A = 0;
   }
   if (res == 0)
   {
      Q64_C = 0;
      Q64_B = 0;
      Q64_A = 0;
   }
}

int8 tmp_port_b = 0;
void save_port_b()
{
tmp_port_b = PORTB;
}

void restore_port_b()
{
PORTB = tmp_port_b;
tmp_port_b = 0;
}

int8 read_counter()
{
int8 res = 0;
if(pind4) res +=8;
if(pind5) res +=4;
if(pind6) res +=2;
if(pind7) res +=1;
return res;
}


void load_10hz(int8 val10)
{

save_port_b();
int8 loc100 = 112;
PORTB = loc100 + val10;
counter_preset_enable();

restore_port_b();
}


void load_100hz(int8 val100)
{
//save_port_b();
int8 loc100 = 112;
PORTB = loc100 + val100;
//restore_port_b();
}

//we can combine dial lock, clarifier and split to one code, rather than 3. We will use the display truth table
void process_dcs(int8 dcs_in)
{
if(dcs_in)
   {
      if(dcs == 15)        {dl = 0; cl = 0; sl = 0;}
      else if(dcs == 0)    {dl = 1; cl = 0; sl = 1;}
      else if(dcs == 1)    {dl = 1; cl = 0; sl = 0;}
      else if(dcs == 2)    {dl = 0; cl = 1; sl = 0;}
      else if(dcs == 3)    {dl = 1; cl = 1; sl = 0;}
      else if(dcs == 4)    {dl = 1; cl = 1; sl = 1;}
      else if(dcs == 12)   {dl = 0; cl = 0; sl = 1;}
      else if(dcs == 14)   {dl = 0; cl = 1; sl = 1;}
   
   }
   else
   {
      if      (!dl  && !cl && !sl) dcs = 15; //                 15  - GRID OFF
      else  if( dl  && !cl &&  sl) dcs = 0;  //                  0  - LOCK - SPLIT
      else  if( dl  && !cl && !sl) dcs = 1;  //            1(OR 7)  - LOCK
      else  if(!dl  &&  cl && !sl) dcs = 2;  //                  2  - CLAR
      else  if( dl  &&  cl && !sl) dcs = 3;  //          3 (OR 13)  - LOCK - CLAR
      else  if( dl  &&  cl &&  sl) dcs = 4;  //4(OR 5,6,8,9,10,11)  - LOCK - CLAR - SPLIT
      else  if(!dl  && !cl &&  sl) dcs = 12; //                 12  - SPLIT
      else  if(!dl  &&  cl &&  sl) dcs = 14; //                 14  - CLAR - SPLIT   
   }

}

void VFD_special_data(int8 option);
void VFD_data(int8 vfo_grid, int8 dcs_grid, int32 value, int8 channel_grid, int8 display_type);
void vfo_disp(int8 vfo, int8 dcs, int32 freq, int8 ch);

//frequency loader for each mode. For mode 0 (VFO mode), probably not used
void load_frequency(int8 mode)
{

vfo_frequency = load_cache_vfo_f();
mr_frequency = load_mem_ch_f(mem_channel);

#ifdef include_cb
if(cb_region == 0) {channel_amount = 40; cb_frequency = cb_channel_bank[cb_channel - 1];}
if(cb_region == 1) {channel_amount = 40; cb_frequency = cb_channel_bank[cb_channel + 39];}
if(cb_region == 2) {channel_amount = 80; cb_frequency = cb_channel_bank[cb_channel - 1];}
if(mode == 2) frequency = cb_frequency;
#endif

if(mode == 0) frequency = vfo_frequency;
if(mode == 1) frequency = mr_frequency;
}


void check_limits()
{
if(mem_mode == 0 || mem_mode == 1)
{
if (frequency < min_freq) frequency = max_freq;
if (frequency > max_freq) frequency = min_freq;
if(mem_channel > 14) mem_channel = 14;
if(mem_channel < 1) mem_channel = 0;
}
#ifdef include_cb
if(mem_mode == 2)
{
if(cb_channel < 1) cb_channel = channel_amount;
if(cb_channel > channel_amount) cb_channel = 1;
}
#endif
}

int1 long_press_down, long_press_up;
void down_button()
{
   
   if(mem_mode == 0)
   {
   if(!long_press_down)
      {
         if(!sw_500k)
         {
         if(band == 0) band = 10;
         --band;
         frequency = band_bank[band];
         }
         else
         {
         frequency -= 50000;
         }
      }
      else
      {
      if(mem_channel > 0) --mem_channel;
      for (int i = 0; i < 2; i++){force_update = 1; vfo_disp(active_vfo, dcs, frequency, (mem_channel + 20)); beep(); delay_ms(50); vfo_disp(active_vfo, dcs, frequency, mem_channel); delay_ms(150);}
      long_press_down = 0;
      }
   }
   if(mem_mode == 1)
   {
      if(!long_press_down)
      {
         if(mem_channel > 0) --mem_channel;
         load_frequency(1);
      }
      else
      {
      if(band == 0) band = 10;
      --band;
      frequency = band_bank[band];
      long_press_down = 0;
      }
   }
#ifdef include_cb
   if(!long_press_down)
   {
      if(mem_mode == 2)
      {
         if(cb_channel > 1) --cb_channel;
      }
   }
   else
   {
      if(mem_channel > 0) --mem_channel;
      for (int i = 0; i < 2; i++){force_update = 1; VFD_data(0xFF, 0xFF, cb_channel, mem_channel, 2); beep(); delay_ms(50); VFD_data(0xFF, 0xFF, cb_channel, 0xFF, 2); delay_ms(150);}
      long_press_down = 0;
   }
#endif
   
}



void up_button()
{
   
   if(mem_mode == 0)
   {
   if(!long_press_up)
      {
         if(!sw_500k)
         {
         ++band;
         if(band > 9) band = 0;
         frequency = band_bank[band];
         }
         else
         {
         frequency += 50000;
         }
      }
      else
      {
         if(mem_channel < 14) ++mem_channel;
         for (int i = 0; i < 2; i++){force_update = 1; vfo_disp(active_vfo, dcs, frequency, (mem_channel + 20)); beep(); delay_ms(50); vfo_disp(active_vfo, dcs, frequency, mem_channel); delay_ms(150);}
         long_press_up = 0;
      }
   }
   if(mem_mode == 1)
   {
   if(!long_press_up)
      {
      if(mem_channel < 14) ++mem_channel;
      load_frequency(1);
      }
      else
      {
      ++band;
      if(band > 9) band = 0;
      frequency = band_bank[band];
      long_press_up = 0;
      }
      
   }
#ifdef include_cb
   if(!long_press_up)
      {
         if(mem_mode == 2)
         {
            if(cb_channel < channel_amount) ++cb_channel;
         }
      }
      else
      {
         if(mem_channel < 14) ++mem_channel;
         for (int i = 0; i < 2; i++){force_update = 1; VFD_data(0xFF, 0xFF, cb_channel, mem_channel, 2); beep(); delay_ms(50); VFD_data(0xFF, 0xFF, cb_channel, 0xFF, 2); delay_ms(150);}
         long_press_up = 0;
      }
#endif
   
}

void dial_lock_button()
{
   if(dl) dl = 0; else dl = 1;
   set_dl(dl);
   process_dcs(0);
}

void clarifier_button()
{
   if(cl) cl = 0;
   else 
   {
   cl = 1;
   save_clar_TX_f(frequency);
   }
   process_dcs(0);
}

void split_button()
{
   if(sl) sl = 0; else sl = 1;
   process_dcs(0);
}

void mode_switch(int mode)
{
   
   if(mode == 1)
   {
      if(mem_mode == 0) save_vfo_f(active_vfo, load_cache_vfo_f());
      frequency = load_mem_ch_f(mem_channel);     
   }
   if(mode == 0)
   {
      if(mem_mode == 0) save_vfo_f(active_vfo, load_cache_vfo_f());
      frequency = load_vfo_f(active_vfo);
   }
mem_mode = mode;
save_mode_n(mode);
}

void mode_switch_kenwood(int mode)
{
   
   if(mode == (48+0)) {mem_mode = 0; active_vfo = 0; frequency = load_vfo_f(active_vfo);}
   if(mode == (48+1)) {mem_mode = 0; active_vfo = 1; frequency = load_vfo_f(active_vfo);}
   if(mode == (48+2)) {mem_mode = 1; frequency = load_mem_ch_f(mem_channel);}
}

void lock_dial_kenwood(int mode)
{
if(mode == (48 + 0)) dl = 0;
if(mode == (48 + 1)) dl = 1;
process_dcs(0);
save_dcs_n(dcs);
}

void mrvfo_button()
{
if(mem_mode == 0) mode_switch(1);
else mode_switch(0);
}

void vfom_button()
{  int8 temp_mode = mem_mode;
   mode_switch(0);
   if(temp_mode == 0) save_mem_ch_f(mem_channel, load_cache_vfo_f());
   if(temp_mode == 1) save_mem_ch_f(mem_channel, load_cache_mem_mode_f());
   frequency = load_mem_ch_f(mem_channel);
   for (int i = 0; i < 2; i++){force_update = 1; vfo_disp(0xFF, dcs, frequency, (mem_channel + 20)); beep(); delay_ms(50); vfo_disp(0xFF, dcs, frequency, mem_channel); delay_ms(150);}
   mode_switch(temp_mode);
}

void mvfo_button()
{
   int8 temp_mode = mem_mode;
   mode_switch(0);
   if(temp_mode != 2)
   {
   save_cache_vfo_f(load_mem_ch_f(mem_channel));
   save_vfo_f(active_vfo, load_mem_ch_f(mem_channel));
   for (int i = 0; i < 2; i++){force_update = 1; vfo_disp(0xFF, dcs, load_mem_ch_f(mem_channel), (mem_channel + 20)); beep(); delay_ms(50); vfo_disp(0xFF, dcs, load_mem_ch_f(mem_channel), mem_channel); delay_ms(150);}
   }
   else
   {
   save_cache_vfo_f(cb_frequency);
   save_vfo_f(active_vfo, cb_frequency);
   for (int i = 0; i < 2; i++){force_update = 1; VFD_data(0xFF, 0xFF, cb_frequency, mem_channel, 0); beep(); delay_ms(50); VFD_data(0xFF, 0xFF, cb_frequency, 0xFF, 0); delay_ms(150);}
   }
   
   mode_switch(temp_mode);
}

void vfom_swap_button()
{
   int8 temp_mode = mem_mode;
   save_cache_mem_mode_f(load_mem_ch_f(mem_channel));
   save_vfo_f(active_vfo, load_cache_vfo_f());
   save_mem_ch_f(mem_channel, load_cache_mem_mode_f());
   mode_switch(0);  
   save_cache_vfo_f(load_mem_ch_f(mem_channel));
   save_cache_mem_mode_f(load_vfo_f(active_vfo));
   save_vfo_f(active_vfo, load_cache_vfo_f());
   save_mem_ch_f(mem_channel, load_cache_mem_mode_f());
   if(temp_mode == 0)
   {
   for (int i = 0; i < 2; i++){force_update = 1; vfo_disp(0xFF, dcs, load_cache_mem_mode_f(), (mem_channel + 20)); beep(); delay_ms(50); vfo_disp(0xFF, dcs, load_cache_mem_mode_f(), mem_channel); delay_ms(150);} 
   }
   if(temp_mode == 1)
   {
   for (int i = 0; i < 2; i++){force_update = 1; vfo_disp(0xFF, dcs, load_vfo_f(active_vfo), (mem_channel + 20)); beep(); delay_ms(50); vfo_disp(0xFF, dcs, load_vfo_f(active_vfo), mem_channel); delay_ms(150);} 
   }
   mode_switch(temp_mode);
   
   
}

void vfoab_button()
{
   if(mem_mode == 0)
   {
      if(active_vfo == 0)
      {
      save_vfo_f(0, load_cache_vfo_f());
      active_vfo = 1;
      }
      else
      {
      save_vfo_f(1, load_cache_vfo_f());
      active_vfo = 0;
      }
      frequency = load_vfo_f(active_vfo);
      force_update = 1;
   }
}

void vfo_a()
{
      if(active_vfo == 1)
      {
      save_vfo_f(1, load_cache_vfo_f());
      active_vfo = 0;
      }
      force_update = 1;
}

void vfo_b()
{
      if(active_vfo == 0)
      {
      save_vfo_f(0, load_cache_vfo_f());
      active_vfo = 1;
      }
      force_update = 1;
}

void toggle_fine_tune_display()
{
if(fine_tune == 1) fine_tune = 0; else fine_tune = 1;
errorbeep(1);
return;
}

void toggle_per_band_offset()
{
if(per_band_offset == 1) 
{
per_band_offset = 0; 
errorbeep(2);
}
else
{
per_band_offset = 1;
errorbeep(3);
}

return;
}




void toggle_speed_dial()
{
#ifdef include_dial_accel
++speed_dial;
if(speed_dial > 1) speed_dial = 0;
if(speed_dial == 0) VFD_special_data(5);
if(speed_dial == 1) VFD_special_data(6);
if(speed_dial == 2) VFD_special_data(7);
force_update = 1;
errorbeep(1);
return;
#endif
}


#ifdef include_cb
void toggle_cb_mode()
{
if(mem_mode == 0 || mem_mode == 1)
{
save_cache_n(mem_mode);
mem_mode = 2;
VFD_data(0xFF, 0xFF, cb_channel, 0xFF, 2);
return;
}
if(mem_mode == 2) mem_mode = load_cache_n();
load_frequency(mem_mode);
vfo_disp(active_vfo, dcs, frequency, mem_channel);
return;
}

void toggle_cb_region()
{
if(mem_mode == 2)
   {
   if(cb_region == 0) cb_region = 1; else cb_region = 0;
   if(cb_region == 0) VFD_special_data(2);
   if(cb_region == 1) VFD_special_data(3);
   force_update = 1; VFD_data(0xFF, 0xFF, cb_channel, 0xFF, 2);
   }
   return;
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void  display_send(int display_byte)
{
 k1 = 0 ; // bit 8
 k2 = 0 ; // bit 4
 k4 = 0 ; // bit 2
 k8 = 0 ; // bit 1
 disp_int = 0;
 
 if ((display_byte & 1)==1) k8=1;
 if ((display_byte & 2)==2) k4=1;
 if ((display_byte & 4)==4) k2=1;
 if ((display_byte & 8)==8) k1=1;

 // display interrupt. Period is between 370 - 410uS between each nibble.
 // This needs testing with real TMS!!!
 disp_int = 1;
 delay_us(30);
 disp_int = 0;
 delay_us(370);

 k1 = 0;  // bit 8
 k2 = 0;  // bit 4
 k4 = 0;  // bit 2
 k8 = 0;  // bit 1
}

void set_display_nibbles(int8 d1,int8 d2,int8 d3,int8 d4,int8 d5,int8 d6,int8 d7,int8 d8,int8 d9)
{
      while(pb2){} //wait for high PB2
      while(!pb2){} //and low again - then we send.
      display_send(10);   // preamble
      display_send(0);    // preamble
      display_send(15);   // preamble
      display_send(15);   // preamble
      display_send(d1);   // VFO
      display_send(d2);  // XTRA
      display_send(d3);   // D1
      display_send(d4);   // D2
      display_send(d5);   // D3
      display_send(d6);   // D4
      display_send(d7);   // D5
      display_send(d8);    //?
      display_send(d9);   // CH???
}

void split_value(int32 value, int8 &d3, int8 &d4, int8 &d5, int8 &d6, int8 &d7, int8 &d8, int8 &d9)
{
int32 tmp_value = value;
      if(value < 1000000) d3 = 15;
      else {d3 = 0;while(tmp_value > 999999){tmp_value -= 1000000; d3+=1;}}
      if(value < 100000) d4 = 15;
      else {d4 = 0;while(tmp_value > 99999){tmp_value -= 100000; d4+=1;}}
      if(value < 10000) d5 = 15;
      else {d5 = 0;while(tmp_value > 9999){tmp_value -= 10000; d5+=1;}}
      if(value < 1000) d6 = 15;
      else {d6 = 0;while(tmp_value > 999){tmp_value -= 1000; d6+=1;}}
      if(value < 100) d7 = 15;
      else {d7 = 0;while(tmp_value > 99){tmp_value -= 100; d7+=1;}}
      if(value < 10) d8 = 15;
      else {d8 = 0;while(tmp_value > 9){tmp_value -= 10; d8+=1;}}
      if(value < 1) d9 = 0;
      else {d9 = 0;while(tmp_value > 0){tmp_value -= 1; d9+=1;}}
}
int8 read_counter();
void load_10hz(int8 val10);
void VFD_data(int8 vfo_grid, int8 dcs_grid, int32 value, int8 channel_grid, int8 display_type)
{    

      if(!force_update)
      {
      static int cycle = 0; ++cycle;
      if(cycle < refresh) return;
      cycle = 0;
      }
      
      static int32 oldcheck;
      int32 newcheck = (vfo_grid+2) + dcs_grid + value + channel_grid+1 + display_type + 1;
      
      if(!force_update)
      {
      if(newcheck == oldcheck) return;
      }
      
      
      if(vfo_grid != 0xFF) d1 = vfo_grid; else d1 = 15;
      
      if(dcs_grid != 0xFF) d2 = dcs_grid; else d2 = 15;
      
      if((value == 0) && (display_type != 2)) set_display_nibbles(d1,d2,15,15,15,15,15,0,d10); //zero display
      else 
      if(value == 0x7FFFFFFF) {d3 = 15; d4 = 15; d5 = 15; d6 = 15; d7 = 15; d8 = 15; d9 = 15;}
      else split_value(value, d3, d4, d5, d6, d7, d8, d9);
      
      if(channel_grid != 0xFF) d10 = channel_grid; else d10 = 15;
      if ((value > 1000) && (display_type > 1)) display_type = 1;               //change to type 1 if out of type 2 limits
         
      if(display_type == 0) set_display_nibbles(d1,d2,d3,d4,d5,d6,d7,d8,d10); //eg 27.781.2
      if(display_type == 1) set_display_nibbles(d1,d2,d4,d5,d6,d7,d8,d9,d10); //eg 77.812.5
      if(display_type == 2) set_display_nibbles(d1,d2,15,15,d7,d8,d9,15,d10); //eg 000 or 123 central
      if(display_type == 3) set_display_nibbles(d1,d2,d7,d8,d9,15,15,15,d10); //eg 000 or 123 to the left
      if(display_type == 4) set_display_nibbles(d1,d2,15,15,d7,d8,d9,15,d10); //eg 000 or 123 to the right
      if(display_type == 5) set_display_nibbles(d1,d2,15,15,15,0 ,15,15,d10); //eg 000 or 123 to the right
      
      oldcheck = (vfo_grid+2) + dcs_grid + value + channel_grid+1 + display_type + 1;
      //if(read_counter() != dhz) load_10hz(dhz);
      gtx = 0;
      #ifdef debug
      puts("updated display!");
      #endif
}

void VFD_special_data(int8 option)
{
if(option == 0) set_display_nibbles(15,15,15,15, 8, 0,12,15,15);//80c
if(option == 1) set_display_nibbles(15,15,15,15, 4, 0,12,15,15);//40c
if(option == 2) set_display_nibbles(15,15,15,15,15,12,14,15,15);//CE
if(option == 3) set_display_nibbles(15,15,15,15,15,15,11,15,15);//B
if(option == 4) set_display_nibbles(15,15,10,1 ,1 ,15,12,11,15);//ALL CB
if(option == 5) set_display_nibbles(15,15,13,1 ,10,15,0,15,15);//DIA O
if(option == 6) set_display_nibbles(15,15,13,1 ,10,15,1,15,15);//DIA 1
if(option == 7) set_display_nibbles(15,15,13,1 ,10,15,2,15,15);//DIA 2
delay_ms(500);
}

void vfo_disp(int8 vfo, int8 dcs, int32 freq, int8 ch)
{

int8 d1,d2,d3;

d2 = dcs;
if(mem_mode == 0) 
{

if(vfo == 0) d1 = 1;
if(vfo == 1) d1 = 12;
if(ch >= 20) d3 = ch - 20;
else d3 = 0xFF;
}
if(mem_mode == 1) 
{
d1 = 2;
d3 = ch;
}
disp_checksum = d1+d2+freq+d3+fine_tune_display;
VFD_data(d1, d2, freq, d3, fine_tune_display);
}

void cls()
{
set_display_nibbles(15,15,15,15,15,15,15,15,15);
}

void  PLL_REF()
{
//PORTB data looks like this, sent to the MC145145 PLLs:
//0          16          32          48          64          80          96
//LATCH0     LATCH1      LATCH2      LATCH3      LATCH4      LATCH5      LATCH6
//NCODEBIT0  NCODEBIT1   NCODEBIT2   NCODEBIT3   REF_BIT0    REF_BIT1    REF_BIT2

//Each bit is strobed to the PLLs every PLL update, this never changes though, so we only need to send this once, unless it gets overwritten.
//Should probably add something to ensure it doesn't... Or should we send it every PLL update? I don't see the need...

//Latch6 address = 96 (or 6<<4)
//Latch5 address = 80 (or 5<<4)
//Latch4 Address = 64 (or 4<<4)

//These are written to latches 6,5 and 4.
 PORTB = (6<<4) + 0x5; PLL1();                  //Latch6 = Bit2 = address + (5 in hex = 5). Could write this as 96+5
 PORTB = (5<<4) + 0xD; PLL1();                  //Latch5 = Bit1 = address + (13 in hex = D) or 80+13
 PORTB = (4<<4) + 0xC; PLL1();                  //Latch4 = Bit0 = address + (12 in hex = C) or 64+12
//5DC in hex = 1500

 PORTB = (6<<4) + 0; PLL2();                    //Latch6 = Bit2 address + (0 in hex = 0) or 96+0
 PORTB = (5<<4) + 0x1; PLL2();                  //Latch5 = Bit1 address + (1 in hex = 1) or 80+1
 PORTB = (4<<4) + 0xE; PLL2();                  //Latch4 = Bit0 address + (14 in hex = E) or 64+14
//01E in hex = 30
}

//We have already sent latches 6,5 and 4, which are reference bits(look above at PLL_REF)
//This purely updates latches 3,2,1 and 0. Latch 3 isn't used, so we could put this in the PLL_REF graveyard if we wanted to (ie sent once)
//Base frequency is loaded, then we load any offset. Once we know, we find out what LPF "band" should be selected and strobed. Original calc_frequency is unchanged remember
//Don't confuse these "bands" with the standard Yaesu bands we select when pressing BAND UP/DOWN

void update_PLL(int32 calc_frequency)
{
// We only need to update, if there is a new NCODE to send, ie when new frequency is requested. We don't touch ref latches(6,5,4), so no need to resend
//if PLL update is requested for the SAME frequency, you are sent on your way and bounced back whence you came lol
      
      if(!force_update)
      {
      static int cycle = 0; ++cycle;
      if(cycle < refresh) return;
      cycle = 0;
      }
      static int32 oldcheck;
      int32 newcheck = calc_frequency;
      
      if(force_update) {oldcheck = 0;}
      if(newcheck == oldcheck) return;
      
      //lock dial whilst tuning
      dltmp = dl; set_dl(1);
      
      
      
      int32 offset_frequency = calc_frequency; //preserve original frequency incase we need it later
      
//!      ///lets load any offset we may have...
//!#ifdef store_to_eeprom
//!#ifdef include_offset_programming
//!      if(!setup_offset)
//!      {
//!      int32 offset = load_offset_f();
//!      if (offset >= 1000000) offset_frequency -= (offset - 1000000);
//!      else offset_frequency += offset;
//!      }
//!
//!#endif
//!#endif
      //LPF preselector section
      //work out what band to strobe to PORTB. PLL_band_bank[i] = lower limit / upper limit / result
      for (int i = 0; i < 10; i++)
      {
         if((offset_frequency >= PLL_band_bank[(i * 3)]) && (offset_frequency <= PLL_band_bank[(i * 3) + 1])) { PLLband = (PLL_band_bank[(i * 3) + 2]); break;}
      }
      PORTB = PLLband; banddata(); 

      //might as well check the stock band bank, so we dont jump stupid amounts if pressing band button after being on VFO
      for (i = 0; i < 10; i++)
      {
      if((offset_frequency >= band_bank[i]) && (offset_frequency < band_bank[i + 1])) {band = i; break;}
      }

       ///lets load any offset we may have...
#ifdef store_to_eeprom
#ifdef include_offset_programming
      if(!setup_offset)
      {
      int32 offset = load_offset_f();
      if (offset >= 1000000) offset_frequency -= (offset - 1000000);
      else offset_frequency += offset;
      }

#endif
#endif     
      


      //split main frequency into mhz, 100k, 10k, 1k, 100, 10, 1 hz
      //Its in base10 so we cannot just bitshift it, so we have to cascade it into chunks
      //Changing to hex, bit shifting and going back to decimal is slow and a waste of time
      
      int32 tmp_frequency = offset_frequency;
      
      int32 dmhz, d100k, d10k, d1k, d100, d10;
      dmhz = 0;while(tmp_frequency > 99999){tmp_frequency -= 100000; dmhz+=1;}
      d100k = 0;while(tmp_frequency > 9999){tmp_frequency -= 10000; d100k+=1;}
      d10k = 0;while(tmp_frequency > 999){tmp_frequency -= 1000; d10k+=1;}
      d1k = 0;while(tmp_frequency > 99){tmp_frequency -= 100; d1k+=1;}
      d100 = 0;while(tmp_frequency > 9){tmp_frequency -= 10; d100+=1;}
      d10 = 0;while(tmp_frequency > 0){tmp_frequency -= 1; d10+=1;}
      dhz = d10;
      
      int32 tmp_band_freq= (offset_frequency / 10000);  
      int16 tmp_khz_freq = ((d100k * 100) + (d10k * 10) + (d1k));//sum of 100k + 10k + 1k eg 27781.25 = 781

      if(tmp_khz_freq >= 500) tmp_khz_freq -=500; 
      //if over 500 khz, take off 500 because we will calculate PLL2 to allow for the 500.
      //If we are at, say, 7499.9 PLL1 will be at Max. If we go to 7500.0, PLL1 will be at minimum, but PLL2 will
      //increase to the next 500 step.
      
      if(dmhz > 9) tmp_khz_freq +=560; //305 minimum - 1305 maximum lower and higher. lowest end of 500kc will go to 1306. highest end of 500kc will go to 304
      else tmp_khz_freq +=560;
      //add on our 560 (start of N code ref is 560, so will count on from that. eg 0 = 560, 1 = 561 etc etc
      //305 + 256 = 561 actually
      // tmp_khz_freq becomes NCODE- this is final NCODE for PLL1. eg 27.78125 = 781... - 500 = 281...281 + 560 = 841 in decimal.
      
      //Lets set our NCODE addresses for both PLLs
      
      int PLL1_NCODE_L3 = 0; //empty latch 48
      int PLL1_NCODE_L2 = 0; //            32
      int PLL1_NCODE_L1 = 0; //            16
      int PLL1_NCODE_L0 = 0;//              0
      
      int PLL2_NCODE_L3 = 0; //empty latch 48
      int PLL2_NCODE_L2 = 0; //empty latch 32
      int PLL2_NCODE_L1 = 0; //            16
      int PLL2_NCODE_L0 = 0; //             0
      
      //841 in hex is 349. So latch2 = 3, latch 1 = 4, latch 0 = 9;
      PLL1_NCODE_L2 = (tmp_khz_freq >> 8); //first digit
      PLL1_NCODE_L1 = ((tmp_khz_freq >> 4) & 0xF);//middle digit
      PLL1_NCODE_L0 = (tmp_khz_freq & 0xF);//final digit
      //PLL1 end

      //PLL2. Do we need to tune for BandA or BandB? Band A = 0.5 - 14.4999... Band B = 14.5 = 30+

      //eg 27.78125 we need mhz + 100k... Make a sum... So 27 * 10 = 270... Add our 7 = 277
      //We have gone to nearest 500khz (see PLL1) - 275... Is band B so 275 / 5 = 55. 55+12 = 77... 77 - 30 = 37. Our NCODE is 37...
      //We then split this for each latch... 37 >> 4 = 3... 37 & F = 7.
      
      //Identify which PLL mode to be in. If in band B, we take off 30 off our code
      if(PLLband < 6){
      //this is BAND A calculations, if we are below band 6 (see LPF preselector)
      PLL2_NCODE_L1 = (((tmp_band_freq / 5) + 12) >> 4); 
      PLL2_NCODE_L0 = (((tmp_band_freq / 5) + 12) & 0xF);
      }
      else
      {
      PLL2_NCODE_L1 = ((((tmp_band_freq / 5) + 12)-30) >> 4);
      PLL2_NCODE_L0 = ((((tmp_band_freq / 5) + 12)-30) & 0xF);
      }

       Q64_C = 0; Q64_B = 0; Q64_A = 0;
       PORTB = 48 + PLL1_NCODE_L3;
       PLL1();
       PORTB = 32 + PLL1_NCODE_L2;
       PLL1();
       PORTB = 16 + PLL1_NCODE_L1;
       PLL1();
       PORTB = 0 + PLL1_NCODE_L0;
       PLL1();
       
       PORTB = 48 + PLL2_NCODE_L3; // Prepare latch
       PLL2();
       PORTB = 32 + PLL2_NCODE_L2; // Prepare latch
       PLL2();
       PORTB = 16 + PLL2_NCODE_L1;
       PLL2();
       PORTB = 0 + PLL2_NCODE_L0;
       PLL2();

       //res2 = d10;
       load_100hz(d100);
       if(((read_counter()) > (d10)) || ((read_counter()) < (d10))) load_10hz(d10);
       res2 = read_counter();
       
       
       
      oldcheck = newcheck;
      set_dl(dltmp);
      dl = dltmp;
      #ifdef debug
      puts("tuned PLL!");
      printf ("display frequency : %ld\r\n", calc_frequency);
      printf ("tuned frequency : %ld\r\n", offset_frequency);
      #endif
      //printf("%d%d%d\r\n", PLL1_NCODE_L2, PLL1_NCODE_L1, PLL1_NCODE_L0);
      //printf("%d%d\r\n", PLL2_NCODE_L1, PLL2_NCODE_L0 );
      
      //printf("Counter: %d d10: %d\r\n", read_counter(), dhz);
      //printf("%ld\r\n", oldcheck);
      //printf("%d\r\n", PLLband);
      
}

unsigned int8 compute_stat_flags()
{
unsigned int8 flag = 0;
if(gen_tx) flag += (1 << 6);
if(tx_mode) flag += (1 << 5);
if(mem_mode !=0) flag += (1 << 4);
if(active_vfo == 1) flag += (1 << 3);
if(cl) flag += (1 << 2);
if(sl) flag += (1 << 1);
if(dl) flag += (1 << 0);
return flag;
}

void program_offset();
int8 buttonaction (int8 res)
{

   if(!res) return 0;
   force_update = 1;
#ifdef include_cb
   if       (res == 1) 
   {
   beep();
   if(mem_mode != 2) clarifier_button();
   else
   {
   force_update = 1;
   VFD_data(0xFF, 0xFF, frequency, 0xFF, 0);
   delay_ms(1000);
   }
   }
#else
   if       (res == 1) {beep(); clarifier_button();}
#endif
   else if  (res == 2)     {beep(); down_button();}
   else if  (res == 3)     {beep(); up_button();}
   else if  (res == 4)     {mvfo_button();}
   else if  (res == 5)     {beep();vfoab_button();}
   else if  (res == 6)     {beep(); dial_lock_button();}
   else if  (res == 7)     {vfom_button();}
   else if  (res == 8)     {beep();mrvfo_button();}
#ifdef include_cb
   else if  (res == 9) {
   if(mem_mode !=2)        {beep(); split_button();}
   else                    {beep(); toggle_cb_region();}
}
#else 
   else if  (res == 9)     {beep(); split_button();}
#endif
   else if  (res == 11)    {vfom_swap_button();}
#ifdef store_to_eeprom
   else if  (res == 21)    {
                           beep();
                           program_offset();
                           }
   else if  (res == 31)    toggle_per_band_offset();
#endif         
   else if  (res == 25) 
   {
      //toggle ports and counters to ensure we can TX
      for (int i = 0; i < 1; i++)
      {
      Q64_C = 1; Q64_B = 1; Q64_A = 1;
      PORTA = 0; PORTB = 0; PORTC = 0; PORTD = 0;
      delay_ms(50);
      Q64_C = 0; Q64_B = 0; Q64_A = 0;
      PORTA = 0; PORTB = 0; PORTC = 0; PORTD = 0;
      delay_ms(50);
      }
      load_10hz(0);
      #ifdef store_to_eeprom
      reset_checkbyte_n();
      #endif
      reset_cpu();
   }
   else if  (res == 22)       {beep(); long_press_down = 1; down_button();}
   else if  (res == 23)       {beep(); long_press_up = 1; up_button();}
   else if  (res == 24)       {beep(); toggle_speed_dial();}
   else if  (res == 26)       {beep(); toggle_fine_tune_display();}
#ifdef include_cb
   else if  (res == 28)       {
                              if(gen_tx) 
                                 {
                                 beep();
                                 toggle_cb_mode();
                                 }
                              }
   else if (res == 29) 
   {
      if(gen_tx)
      {
      beep();
         VFD_special_data(4);
         force_update = 1; VFD_data(0xFF, 0xFF, cb_channel, 0xFF, 2);
         cb_region = 2;
         save_cb_reg_n(cb_region);
      }
   }
#endif
save_all_n();
force_update = 1;

disp_int = 0; k2 = 1; k4 = 1; k8 = 1;
while(pb2 || pb1 || pb0){}
k2 = 0; k4 = 0; k8 = 0;
return res;
}

int8 buttons()
{
      int res = 0;
      static int cycle = 0; ++cycle;
      if(cycle < btnrefresh) return res;
      cycle = 0; disp_int = 0;
      
      if(pb2)
      {
         while (pb2){}
        
         
    
         int count = 0;
         
         k4 = 0; k8 = 0; k1 = 0; k2 = 1; delay_us(1);
         if (pb2){while(pb2){++count;  delay_ms(100); if(count >=30) {long_press = 1; break;}} btn_down = 1; res = 1;  goto out;} //(RESULT: 1) Clarifier
         if (pb1){while(pb1){++count;  delay_ms(100); if(count >=10) {long_press = 1; break;}} btn_down = 1; res = 2;  goto out;} //(RESULT: 2) Down
         if (pb0){while(pb0){++count;  delay_ms(100); if(count >=10) {long_press = 1; break;}} btn_down = 1; res = 3;  goto out;} //(RESULT: 3) Up
         
         k2 = 0; k4 = 1; delay_us(1);
         if (pb2){while(pb2){++count;  delay_ms(100); if(count >=30) {long_press = 1; break;}} btn_down = 1; res = 4;  goto out;} //(RESULT: 4) M>VFO
         if (pb1){while(pb1){++count;  delay_ms(100); if(count >=30) {long_press = 1; break;}} btn_down = 1; res = 5;  goto out;} //(RESULT: 5) VFO A/B
         if (pb0){while(pb0){++count;  delay_ms(100); if(count >=30) {long_press = 1; break;}} btn_down = 1; res = 6;  goto out;} //(RESULT: 6) Dial lock
         
         k4 = 0; k8 = 1; delay_us(1);
         if (pb2){while(pb2){++count;  delay_ms(100); if(count >=30) {long_press = 1; break;}} btn_down = 1; res = 7;  goto out;} //(RESULT: 7) VFO>M
         if (pb1){while(pb1){++count;  delay_ms(100); if(count >=30) {long_press = 1; break;}} btn_down = 1; res = 8;  goto out;} //(RESULT: 8) MR/VFO
         if (pb0){while(pb0){++count;  delay_ms(100); if(count >=30) {long_press = 1; break;}} btn_down = 1; res = 9;  goto out;} //(RESULT: 9) SPLIT
             
         k8 = 0; k1 = 1; delay_us(1);
         if (pb1){while(pb1){++count;  delay_ms(100); if(count >=30) {long_press = 1; break;}} btn_down = 1; res = 11; goto out;} //(RESULT: 11) VFO<>M
         k8 = 0;  k4 = 0;  k2 = 0;  k1 = 0;
         
         
      }
      if(sw_pms)
      {
      beep();
      while(sw_pms){}
      if(pms) pms = 0; else pms = 1;
      }
      
out:
      
      if(res)
      {
      
      if(long_press) {long_press = 0; res+= 20;}
      }
      
    
    return res;
}

int1 dial_clock()
{
static int8 clk2;
static int8 cnt;
int8 clk1 = read_counter();
if(clk2 != clk1)
{
if (clk1 == 0) ++cnt;
clk2 = clk1;
}
if(cnt == 10) {cnt = 0; return 0;}
return 1;

}

   //dial output counts(the value returned - the MAIN variable for eg frequency
   //COSMETICS:-
   //note the perculiar dial increments... They could just be nice round numbers like 5, 10, 100, 1000 etc...
   //However... Shifting the next-to-last digit lets all digits in the VFD change. If these were nice round numbers, only the digit being changed would appear to change.
   //This is only a cosmetic thing. Feel free to round them up or down as you like
   
   //outspeed1 = 2 = 10khz approx per revolution. 1 = 5khz approx per revolution
#define out_speed1 2       //Slowest. Required for all dials...

#ifdef include_dial_accel
#define out_speed2 10      //Slightly faster
#define out_speed3 25     //Faster still
#define out_speed4 50     //Super-fast

//here we set our dial timer ( or triggers). As the timer decreases, it will pass these threasholds
//these dial timers can be anything lower than main dial timer. 
//set these to whatever you like. eg if you have a main timer of 2000, you could set timer 1 to 300, timer 2 to 250 etc.
//They need to descend because the main timer (dial_timer) descends

//basic tug-o-war between positive and negative pulses. Dial timers are in percentages / 10
//screen MUST be refreshed immediately. Set update_display flag ( as below)

#define dial_timer_max 20000 //max spin down-timer. All percentages are based of this. Default 40000
//these are the trigger percentages. As dial_timer decreases, we will approach eg 80% of dialtimer, which is percent_of_d_timer_speed2.
//We can change these percentages so different speeds happen earlier or later. Defaults 80, 60, 40, 20, 10.

#define percent_of_d_timer_speed1 90 //default 80%
#define percent_of_d_timer_speed2 60 //default 80%
#define percent_of_d_timer_speed3 40 //default 60%
#define percent_of_d_timer_speed4 20 //default 40%
#define percent_of_d_timer_speed5 0 //default 20%


#define stop_reset_count 5 //stop spin check sampling count. How fast dial reverts to lowest increment. Higher - longer, Lower, shorter. Default 25
#define negative_pulse_sample_rate 50 //how often to check for negative pulse to fight against the decrement. Higher number means easier acceleration. Lower is harder (eg faster and harder). Default 50
#define dial_timer_decrement 10 //overall timer decrement when spinning. How fast the main timker decreases. Default 5. Lower = longer spin before it speeds up. Not harder, just longer
#define dial_timer_pullback 20 //dial increment when negative (eg when slowing down spinning). How hard we fight against the decrement. Needs to be more than decrement. Default 20. Lower = acceleration increments kick in earlier

int16 freq_dial_accel_type1(int32 &value, int16 start_increment)
{
   int8 res = 0;
   static int16 count = 0;
   static int16 stop_count = 0;  
   static int16 dial_timer = dial_timer_max;
   //if(reset) { dial_timer = 400; dial_increment = default_increment;}
   static int16 dial_timer1, dial_timer2, dial_timer3, dial_timer4, dial_timer5;
   
   
   static int16 dial_increment = start_increment;
   dial_timer1 = (dial_timer_max / 100) * percent_of_d_timer_speed1; //eg 80%
   dial_timer2 = (dial_timer_max / 100) * percent_of_d_timer_speed2;
   dial_timer3 = (dial_timer_max / 100) * percent_of_d_timer_speed3;
   dial_timer4 = (dial_timer_max / 100) * percent_of_d_timer_speed4;
   dial_timer5 = (dial_timer_max / 100) * percent_of_d_timer_speed5;
  
   if(res1 != res2)
      {
      tmp = dial_dir;
      if(tmp) 
      {
      value +=dial_increment;
      }
      else 
      {
      value -=dial_increment;
      }
      
      stop_count = 0;

      if(dial_timer <dial_timer_pullback) dial_timer = dial_timer_pullback;
      if(dial_timer >= dial_timer_max) dial_timer = dial_timer_max;
      dial_timer -= dial_timer_decrement;
      res2 = res1;
      res = dial_increment;
      //VFD_data(0xFF, 0xFF, dial_timer, 0xFF, 0);
      
   }
   else
   {
      if(count == negative_pulse_sample_rate)
      {
         if(dial_timer <dial_timer_pullback) dial_timer = dial_timer_pullback;
         if(dial_timer >= dial_timer_max) dial_timer = (dial_timer_max - dial_timer_pullback);
         dial_timer +=dial_timer_pullback;
         //display_driver(0,0,0,0,dial_timer,0);
         if(dial_clock())
         {
            ++stop_count;
            if(stop_count >= stop_reset_count) 
            {
               dial_timer = dial_timer_max;
               res = 0;
               
            }
         }
      }
      
   }
   ++count;
   if(count>negative_pulse_sample_rate) count = 0;
   //if (dial_timer < 300) dial_timer +=1;
   
   if(dial_timer > dial_timer1)                                   dial_increment = start_increment; 
   if((dial_timer <= dial_timer1) && (dial_timer > dial_timer2))  dial_increment = speed1;
   if((dial_timer <= dial_timer2) && (dial_timer > dial_timer3))  dial_increment = speed2;
   if((dial_timer <= dial_timer3) && (dial_timer > dial_timer4))  dial_increment = speed3;
   if((dial_timer <= dial_timer4) && (dial_timer >= dial_timer5))  dial_increment = speed4;
return res;
}
#endif

int16 freq_dial_basic(int32 &value, int16 dial_increment)
{
   int8 res = 0;
   static int8 temp_count = 0;
   if(res1 != res2) temp_count +=1;
      
   if (temp_count > 9)
      {
         temp_count = 0;
         tmp = dial_dir;
         if(tmp) value+=dial_increment;
         else value-=dial_increment;
         res = 1;
         res2 = res1;
      }
      
   //if(res) putc(res);
   return res;
}


int8 misc_dial(int32 &value, int8 direction)
{
   if(res1 != res2)
      {
      tmp = dial_dir;
      if(!direction)
      {
         if(tmp) value+=1; else value-=1;
      }  
      else
      {
         if(tmp) value-=1; else value+=1;
      }
      res2 = res1;
      return 1;
   } 
   else return 0;
}

int8 misc_dial8(int8 &value, int8 direction)
{
   static int8 temp_count = 0;
   if(res1 != res2)
      {
      temp_count +=1;
      if(temp_count == 10)
      {
         tmp = dial_dir;
         if(!direction)
      {
         if(tmp) value+=1; else value-=1;
      }  
      else
      {
         if(tmp) value-=1; else value+=1;
      }
      temp_count = 0;
      }
      res2 = res1;
      return 1;
      } 
   else return 0;
}

int1 dial_moved()
{
static int8 counter2;
int8 counter1 = read_counter();
if(counter2 != counter1)
{
counter2 = counter1;
return 1;
}
else return 0;
}

#ifdef store_to_eeprom
void program_offset()
{
#ifdef include_offset_programming
//save_offset_f(0);
setup_offset = 1;
int32 test = load_offset_f();
if(test >= 1000000)
{
test -= 1000000;
dir = 1;
}
else 
{
dir = 0;
}
int8 res = 1;
int8 btnres = 0;
int32 testfreq = frequency;

cls();
//goto start;
while(true)
{

//if (test > 1000) {test = 1000; if(dir == 0) dir = 1; else dir = 0;}
start:
if (res == 1)
{
if(test)
{
   if(dir) {
   testfreq = frequency - test;
   VFD_data(0xFF, 0xFF, test, 0xFF, 3);
   }
   else
   {
   testfreq = frequency + test;
   VFD_data(0xFF, 0xFF, test, 0xFF, 4);
   }
}
else VFD_data(0xFF, 0xFF, 1, 0xFF, 5);
update_PLL(testfreq);
res = 0;
}
res1 = read_counter();
if (misc_dial (test,dir)) res = 1;
if (test == -1) {test = 1; if(dir == 0) dir = 1; else dir = 0;}
if(test > 998) test = 999;
btnres = buttons();
if(btnres == 1)
{
VFD_data(0xFF, 0xFF, frequency, 0xFF, 0);
delay_ms(1000);
btnres = 0;
res = 1;
goto start;
}
else if(btnres == 6)
{
save_offset_f(0);
break;
}
else if(btnres == 21) {
if(dir) save_offset_f(1000000 + test); 
if(!dir) save_offset_f(test);
setup_offset = 0;
force_update = 1;
break;
}
}
beep();
vfo_disp(active_vfo, dcs, frequency, mem_channel);
#endif
}
#endif

#ifdef include_cat
void parse_cat_command_yaesu (char byte1, char byte2, char byte3, char byte4, char byte5)
{

int32 byte4_upper = ((byte4 >> 4) & 0xF);
int32 byte4_lower = byte4 & 0xF;
int32 byte3_upper = ((byte3 >> 4) & 0xF);
int32 byte3_lower = byte3 & 0xF;
int32 byte2_upper = ((byte2 >> 4) & 0xF);
int32 byte2_lower = byte2 & 0xF;
int32 byte1_upper = ((byte1 >> 4) & 0xF);
int32 byte1_lower = byte1 & 0xF;

switch(byte5){
   case 0x01: split_button(); beep(); break;
   case 0x02: mrvfo_button(); beep(); break;
   case 0x03: vfom_button(); beep(); break;
   case 0x04: dial_lock_button(); beep(); break;
   case 0x05: vfoab_button(); beep(); break;
   case 0x06: mvfo_button(); beep(); break;
   case 0x07: up_button(); beep(); break;
   case 0x08: down_button(); beep(); break;
   case 0x09: clarifier_button(); beep(); break;
   case 0x0A: frequency = ((byte4_lower * 1000000) + (byte3_upper * 100000) + (byte3_lower * 10000) + (byte2_upper * 1000) + (byte2_lower * 100) + (byte1_upper * 10) + byte1_lower); break;
   case 0x0B: vfom_swap_button(); beep(); break;
   case 0x0F: frequency = ((byte1_upper * 1000000) + (byte1_lower * 100000) + (byte2_upper * 10000) + (byte2_lower * 1000) + (byte3_upper * 100) + (byte3_lower * 10) + byte4_upper); break;
   case 0xFC: gen_tx = 1; break;
   case 0xFE: buttonaction(25); break;
   case 0xFF: reset_cpu(); break;
#ifdef include_cb
   case 0xFD: if(gen_tx) toggle_cb_mode(); break;
#endif   
   case 0xFB:
      int8 count = 0;
      if ((byte1_upper * 16) + (byte1_lower) != 0) {save_speed1_n((byte1_upper * 16) + byte1_lower); ++count;}
      if ((byte2_upper * 16) + (byte2_lower) != 0) {save_speed2_n((byte2_upper * 16) + byte2_lower); ++count;}
      if ((byte3_upper * 16) + (byte3_lower) != 0) {save_speed3_n((byte3_upper * 16) + byte3_lower); ++count;}
      if ((byte4_upper * 16) + (byte4_lower) != 0) {save_speed4_n((byte4_upper * 16) + byte4_lower); ++count;}
      errorbeep(count);
      reset_cpu();
      break;
}

if ((byte5 >= 0xE0) && (byte5 <= 0xEE)) 
   {
   save_mem_ch_f
   ((byte5 - 0xE0), 
   (byte4_lower * 1000000) + 
   (byte3_upper * 100000) + 
   (byte3_lower * 10000) + 
   (byte2_upper * 1000) + 
   (byte2_lower * 100) + 
   (byte1_upper * 10) + 
   (byte1_lower));
   errorbeep(3);
   }

force_update = 1;
}

char ifbuf[38] = {
'I', 'F', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
'0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', ';',
};

int8 t1 = 0, t2= 0, t3= 0, t4= 0, t5= 0, t6= 0, t7= 0;
int32 i1 = 0, i2= 0, i3= 0, i4= 0, i5= 0, i6= 0, i7= 0;

int i;

void calc_if()
{
   int32 temp_value = frequency;
   split_value(temp_value, t1, t2, t3, t4, t5, t6, t7);
   if(t1 == 15) ifbuf[5] = 48 + 0; else ifbuf[5] = 48 + t1; //Mhz10
   if(t2 == 15) ifbuf[6] = 48 + 0; else ifbuf[6] = 48 + t2; //Mhz1
   ifbuf[7] = 48 + t3; //Khz100
   ifbuf[8] = 48 + t4; //Khz10
   ifbuf[9] = 48 + t5; //Khz1
   ifbuf[10] = 48 + t6; //Hz100
   ifbuf[11] = 48 + t7; //Hz10   
   ifbuf[26] = ('0'); //Mem CH first digit (eg 0) in ASCII not HEX
   ifbuf[27] = ('0'); //Mem CH second digit (eg 8) in ASCII not HEX
   if(tx_mode) ifbuf[28] = ('1'); else ifbuf[28] = ('0'); //TX/RX (0 RX, 1 TX) in ASCII
   ifbuf[29] = dummy_mode; //Mode 1 = LSB, 2 = USB, 3 = CW, 4 = FM, 5 = AM, 6 = FSK, 7 = CWN. All dummy values. Should reflect mode change in application
   if(active_vfo == 0) {ifbuf[30] = ('0'); ifbuf[32] = ('0');}//VFO0 (VFO A)
   if(active_vfo == 1) {ifbuf[30] = ('1'); ifbuf[32] = ('1');}//VFO1 (VFO B)
   ifbuf[31] = ('0'); //Scan (0 = off, 1 = on)
   
}

void send_if()
{
   //calc_if();
   for (i = 0; i < 38; i++)
   {
   putc(ifbuf[i]);
   }
}

const char cat_comm[200] = {
//1st char, 2nd char, location of terminator, res
'A', 'I', 3, 1,  //AI0 = SET. NO ANSWER. IF off.
'A', 'I', 3, 2,  //AI1 = SET. ANSWER. IF on. Answer IF
'D', 'N', 2, 3,  // DN = SET. NO ANSWER. DOWN. No reply. Action up button
'U', 'P', 2, 4,  // UP = SET. NO ANSWER. UP. No reply. Action down button
'F', 'A', 2, 5,  // FA = READ. ANSWER. VFOA Answer (FA000) (Freq) (0) (;)
'F', 'B', 2, 6,  // FB = READ. ANSWER. VFOB Answer (FB000) (Freq) (0) (;)
'F', 'A', 13, 7, // FA = SET. ANSWER. VFOA Set & Answer (FA000) (freq) (0) (;)
'F', 'B', 13, 8, // FB = SET. ANSWER. VFOB Set & Answer (FB000) (freq) (0) (;)
'F', 'N', 3, 9,  // FN = SET. NO ANSWER. (0;) = VFOA, (1;) = VFOB, (2;) = MR,
'F', 'R', 3, 27,  // FR = SET. ANSWER. (FR) (0;) = VFOA, (1;) = VFOB, Answer FR (VFO)(;)
'F', 'R', 2, 28,  // FR = READ. ANSWER. (FR) (0;) = VFOA, (1;) = VFOB
'F', 'T', 3, 29,  // FT = SET. ANSWER. (FT) (0;) = VFOA, (1;) = VFOB, Answer FT (VFO)(;)
'F', 'T', 2, 30  // FT = READ. ANSWER. (FT) (0;) = VFOA, (1;) = VFOB
'I', 'F', 2, 10, // IF = READ. ANSWER. Answer IF
'L', 'K', 2, 11, // LK = READ. ANSWER. Answer LK0; or LK1; UNLOCK OR LOCK
'L', 'K', 3, 12, // LK = SET. ANSWER. LK0; Lock off. LK1; Lock on....Answer LK0; / LK1; UNLOCK/LOCK
'M', 'C', 5, 14, // MC = SET. NO ANSWER. MC Memory channel. (MC)(0)(CH). eg MC002; = mem 2
'M', 'D', 3, 15, // MD = SET. NO ANSWER. MD; MODE - Fake mode 1 = LSB, 2 = USB, 3 = CW, 4 = FM, 5 = AM, 7 = CWN
'M', 'R', 6, 16, // MR = READ. ANSWER. (MR) (0) (0) (memch) (;). ANSWER (MR) (0) (0) (mem ch. 2 digits) (000) (Frequency. + 0) (dummy mode) (0) (0) (00) (0) ;
'M', 'W', 23, 17,// MW = SET. NO ANSWER (MW) (0) (0) (mem ch. 2 digits) (000) (Frequency. + 0) (dummy mode) (0) (0) (00) (0) ;
'R', 'C', 2, 18, // RC = SET. NO ANSWER. Clarifier offset = 0.
'R', 'D', 2, 19, // RD = SET. NO ANSWER. Clarifier freq decrease.
'R', 'U', 2, 20, // RU = SET. NO ANSWER.  Clarifier freq increase.
'R', 'T', 3, 21, // RT = SET. NO ANSWER. RT0; = Clar off. RT1 = Clar on.
'R', 'X', 2, 22, // RX = SET. NO ANSWER. RX; mode.
'T', 'X', 2, 23, // TX = SET. NO ANSWER. TX; mode.
'S', 'C', 3, 24, // SC = SET. NO ANSWER. SC0; PMS off...SC1; PMS on.
'S', 'P', 3, 25, // SP = SET. NO ANSWER. SP0; split off...SP1 split on.
'I', 'D', 2, 26, // ID = READ. ANSWER.  Answer (ID006;)


};

char cat_ans[96] = {
'F', 'A', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', ';', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
'F', 'B', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', ';', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
'L', 'K', '0', ';', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
'M', 'R', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', ';'
};

void send_cat_ans(int8 ans)
{
for (int i = 0; i < 24; i++)
{
putc(cat_ans[((ans * 24) + i)]);
}
}

void parse_cat_command_kenwood ()
{

int32 temp_value;

int8 i; 
//AI DNUP FAFB FN ID IF LK MC MD MR MW RC RDRU RT RXTX SC SP
int8 res = 0;
   for (i = 0; i < 26; i++)
   {
   if ((buffer[0] == cat_comm[(i*4)]) && (buffer[1] == cat_comm[(i * 4) + 1]) && (buffer[cat_comm[(i*4) + 2]] == ';')) 
      {res = cat_comm[(i * 4) + 3]; break;}
   }

switch (res) {
//all no answer
case 1: AI = 0; break;
case 2: AI = 1; break;
case 3: down_button(); break;
case 4: up_button(); break;
case 5: temp_value = load_vfo_f(0); break;
case 6: temp_value = load_vfo_f(1); break;
case 7: temp_value = load_vfo_f(0); break;
case 8: temp_value = load_vfo_f(1); break;
case 9: mode_switch_kenwood(buffer[2]); break;

case 11: send_cat_ans(2); break; //LK;
case 12: lock_dial_kenwood(buffer[2]); break; //LK+0 or 1;
case 16: temp_value = load_cache_vfo_f(); break;
case 18: break; //Clear clar freq
case 19: break; //Clar freq - 1 or 10
case 20: break; //Clar freq + 1 or 10
case 21: break; //toggle clar on off
case 22: break; //set rx mode
case 23: break; //set tx mode
case 24: break; //PMS on/off
case 25: break; //split on/off
case 26: puts("ID006;"); break; //ID
}

if((res == 5) || (res == 7)) 
{
   split_value(temp_value, t1, t2, t3, t4, t5, t6, t7);
   if (res == 5)
      {
      
      if(t1 == 15) cat_ans[5] = 48; else cat_ans[5] = (48 + t1);
      if(t2 == 15) cat_ans[6] = 48; else cat_ans[6] = (48 + t2);
      if(t3 == 15) cat_ans[7] = 48; else cat_ans[7] = (48 + t3);
      if(t4 == 15) cat_ans[8] = 48; else cat_ans[8] = (48 + t4);
      if(t5 == 15) cat_ans[9] = 48; else cat_ans[9] = (48 + t5);
      if(t6 == 15) cat_ans[10] = 48; else cat_ans[10] = (48 + t6);
      if(t7 == 15) cat_ans[11] = 48; else cat_ans[11] = (48 + t7);
      }
      
   if(res == 7)
      {
      
      cat_ans[5] = buffer[5];
      cat_ans[6] = buffer[6];
      cat_ans[7] = buffer[7];
      cat_ans[8] = buffer[8];
      cat_ans[9] = buffer[9];
      cat_ans[10] = buffer[10];
      cat_ans[11] = buffer[11];
      
      i1 = cat_ans[5] - 48; //10mhz
      i2 = cat_ans[6] - 48; //1mhz
      i3 = cat_ans[7] - 48; //100khz
      i4 = cat_ans[8] - 48; //10khz
      i5 = cat_ans[9] - 48; //1khz
      i6 = cat_ans[10] - 48; //100hz
      i7 = cat_ans[11] - 48;
    
      
      
      temp_value = ((i1 * 1000000) + (i2 * 100000) + (i3 * 10000) + (i4 * 1000) + (i5 * 100) + (i6 * 10) + (i7));
      if(temp_value >= 3199990) temp_value = 3199990;
      if(temp_value < 50000) temp_value = 50000;
      if (active_vfo == 0) {frequency = temp_value; save_cache_vfo_f(frequency); save_vfo_f(0, temp_value);}
      else save_vfo_f(0, temp_value);
      
      
      }
      
   send_cat_ans(0);
   calc_if();
}

if((res == 6) || (res == 8)) 
{
   split_value(temp_value, t1, t2, t3, t4, t5, t6, t7);
   if(res == 6)
   {
   
   if(t1 == 15) cat_ans[5 + 24] = 48; else cat_ans[5 + 24] = (48 + t1);
   if(t2 == 15) cat_ans[6 + 24] = 48; else cat_ans[6 + 24] = (48 + t2);
   if(t3 == 15) cat_ans[7 + 24] = 48; else cat_ans[7 + 24] = (48 + t3);
   if(t4 == 15) cat_ans[8 + 24] = 48; else cat_ans[8 + 24] = (48 + t4);
   if(t5 == 15) cat_ans[9 + 24] = 48; else cat_ans[9 + 24] = (48 + t5);
   if(t6 == 15) cat_ans[10 + 24] = 48; else cat_ans[10 + 24] = (48 + t6);
   if(t7 == 15) cat_ans[11 + 24] = 48; else cat_ans[11 + 24] = (48 + t7);
   }
   
   if(res == 8)
      {
      cat_ans[5 + 24] = buffer[5];
      cat_ans[6 + 24] = buffer[6];
      cat_ans[7 + 24] = buffer[7];
      cat_ans[8 + 24] = buffer[8];
      cat_ans[9 + 24] = buffer[9];
      cat_ans[10 + 24] = buffer[10];
      cat_ans[11 + 24] = buffer[11];
      
      i1 = cat_ans[5 + 24] - 48; //10mhz
      i2 = cat_ans[6 + 24] - 48; //1mhz
      i3 = cat_ans[7 + 24] - 48; //100khz
      i4 = cat_ans[8 + 24] - 48; //10khz
      i5 = cat_ans[9 + 24] - 48; //1khz
      i6 = cat_ans[10 + 24] - 48; //100hz
      i7 = cat_ans[11 + 24] - 48;
    
      temp_value = ((i1 * 1000000) + (i2 * 100000) + (i3 * 10000) + (i4 * 1000) + (i5 * 100) + (i6 * 10) + (i7));
      if(temp_value >= 3199990) temp_value = 3199990;
      if(temp_value < 50000) temp_value = 50000;
      if (active_vfo == 1) {frequency = temp_value; save_cache_vfo_f(frequency); save_vfo_f(1, temp_value);}
      else save_vfo_f(1, temp_value);
      }
      
   send_cat_ans(1);
   calc_if();
}

if(res == 10) {calc_if(); send_if();}

if(res == 15)
{
dummy_mode = (buffer[2]);
save_dummy_mode_n(dummy_mode);
}

if(res == 27)
{
force_update = 1;
if(buffer[2] == '0')
   {
   active_vfo = 0; save_vfo_n(0);
   frequency = load_vfo_f(active_vfo);
   puts("FR0;\0");
   }
if(buffer[2] == '1')
   {
   active_vfo = 1; save_vfo_n(1);
   frequency = load_vfo_f(active_vfo);
   puts("FR1;\0");
   }  
   
}

if(res == 28)
{
   if(active_vfo == 0) puts("FR0;\0");
   if(active_vfo == 1) puts("FR1;\0");
}
   

if(res == 29)
{
force_update = 1;
if(buffer[2] == '0')
   {
   active_vfo = 0; save_vfo_n(0);
   frequency = load_vfo_f(active_vfo);
   puts("FT0;\0");
   }
if(buffer[2] == '1')
   {
   active_vfo = 1; save_vfo_n(1);
   frequency = load_vfo_f(active_vfo);
   puts("FT1;\0");
   }  
   
}

if(res == 30)
{
   if(active_vfo == 0) puts("FT0;\0");
   if(active_vfo == 1) puts("FT1;\0");
}

res = 0;
}


#endif


int8 mic_button(int16 increment)
{
#ifdef include_mic_buttons
      static int cycle = 0;
      ++cycle;
      if(cycle < btnrefresh) return 0;
      cycle = 0;
int res = 0;
static int16 cnt = 0;
//static int mic_counter = 0;
if( !mic_fast && !mic_up) res = 1;
else if( !mic_fast && !mic_dn) res = 5;
else if(mic_fast && mic_up && mic_dn) res = 9;
else if(mic_fast && !mic_up) res = 13;
else if(mic_fast && !mic_dn) res = 14;
if(!res) {cnt = 0; return 0;}
force_update = 1;

if(res == 1)
   {
      while(!mic_fast && !mic_up)
      {
      if(cnt < 500) {++cnt; delay_ms(5);}
      if(cnt >= 499) {res = 4; break;}
      else if(cnt >= 300) {res = 3; break;}
      else if(cnt >= 100) {res = 2; break;}
      }
      
   }
   
 if(res == 5)
   {
      while(!mic_fast && !mic_dn)
      {
      if(cnt < 500) {++cnt; delay_ms(5);}
      if(cnt >= 499) {res = 8; break;}
      else if(cnt >= 300) {res = 7; break;}
      else if(cnt >= 100) {res = 6; break;}
      }
      
   }
   
   if(res == 9)
   {
      while (mic_fast && mic_up && mic_dn)
      {
      if(cnt <= 750) {++cnt; delay_ms(5);}
      if(cnt == 750) {res = 12; break;}
      else if(cnt == 500) {res = 11; break;}
      else if(cnt == 250) {res = 10; break;}
      }  
   }
   
   if((res == 1) || (res == 5) || (res == 9))
   {
   if(cnt > 99) res = 0;
   }
   
   if(mem_mode == 0)
   {
      if(res == 1) {
      if(increment == 1) {frequency += 1; cnt = 0; beep(); return 1;}
      else {frequency += speed2; cnt = 0; beep(); return 1;}
      }
      if(res == 2) {frequency += speed2;}
#ifdef include_mic_accel
      if(res == 3) {frequency += speed3;}
      if(res == 4) {frequency += speed4;}
      if(res == 13) {frequency += 1234;}
#else
      if(res == 3) {frequency += speed2;}
      if(res == 4) {frequency += speed2;}
      if(res == 13) {frequency += 100;}
#endif
      
      if(res == 5) {
      if(increment == 1) {frequency -= 1; cnt = 0; beep(); return 1;}
      else {frequency -= speed2; cnt = 0; beep(); return 1;}
      }
      if(res == 6) {frequency -= speed2;}
#ifdef include_mic_accel
      if(res == 7) {frequency -= speed3;}
      if(res == 8) {frequency -= speed4;}
      if(res == 14) {frequency -= 1234;}
#else
      if(res == 7) {frequency -= speed2;}
      if(res == 8) {frequency -= speed2;}
      if(res == 14) {frequency -= 100;}
#endif
   }
   
   if((mem_mode == 1) || (mem_mode == 2))
   {
      if(res == 1) {cnt = 0; beep(); up_button();}
      if(res == 2) {up_button();}
      if(res == 3) {up_button();}
      if(res == 4) {up_button();}
      
      if(res == 5) {cnt = 0; beep(); down_button();}
      if(res == 6) {down_button();}
      if(res == 7) {down_button();}
      if(res == 8) {down_button();}

   }
   
   if(res ==  9)
   {
   errorbeep(1);
   if(mem_mode == 0) up_button();
   #ifdef include_cb
   if(gen_tx)
   {
   if(mem_mode == 2) toggle_cb_region();
   }
   #endif
   }
   if(res == 10)
   {
   errorbeep(2);
   if(mem_mode == 0) {mrvfo_button(); return res;}
   #ifdef include_cb
   if(gen_tx)
   {
      if(mem_mode == 1) {toggle_cb_mode(); return res;}
      if(mem_mode == 2) {toggle_cb_mode(); mrvfo_button(); return res;}
   }
   #else
   if(mem_mode == 1) {mrvfo_button(); return res;}
   #endif
   }
   if(res == 11) errorbeep(3);
   if(res == 12) errorbeep(4);
   
   

return res;
#endif
}

void transmit_check()
{
if(!force_update)
      {
      static int cycle = 0; ++cycle;
      if(cycle < refresh) return;
      cycle = 0;
      }
int8 valid = 0;
if(!gen_tx)
{
   if(tx_mode)
   {
      if(!gtx)
      {
         for (int i = 0; i < 10; i++)
         {
            if((frequency >= blacklist[i * 2]) && (frequency <= blacklist[(i * 2)+1])) {valid = 1; break;}
         }
         if(!valid)
         {
         set_tx_inhibit(1);
         beep();
         
         }
         gtx = 1;
      }
   }
   else
   if(gtx)
   {
   set_tx_inhibit(0);
   gtx = 0;
   }
} else valid = 1;

if(sl)
   {
      if(tx_mode)
      {
         if(!stx)
         {
            save_vfo_f(active_vfo, frequency);
            if(active_vfo == 0) {active_vfo = 1;}
            else if(active_vfo == 1) {active_vfo = 0;}
            frequency = load_vfo_f(active_vfo);
            stx = 1;
            force_update = 1;
         }
      }
      else
      {
         if(stx)
         {
            save_vfo_f(active_vfo, frequency);
            if(active_vfo == 0) {active_vfo = 1;}
            else if(active_vfo == 1) {active_vfo = 0;}
            frequency = load_vfo_f(active_vfo);
            stx = 0;
            force_update = 1;
         }
      }
   }
   
   if(cl)
   {
      if(tx_mode)
      {
         if(!ctx)
         {
            save_clar_RX_f(frequency);
            frequency = load_clar_TX_f();
            ctx = 1;
            force_update = 1;
         }         
      }
      else
      {
         if(ctx)
         {
            save_clar_TX_f(frequency);
            frequency = load_clar_RX_f();
            ctx = 0;
            force_update = 1;
         }
      }
   }
}



void pms_vfo(int32 freq1, int32 freq2)
{
//set some flags
   int1 stopped = 0;
   int1 blink = 0;
   int1 direction = 0;

   //variables
   int32 hifreq, lofreq, tmpfreq1;
   int16 counter1, counter2, counter3;
   
   if(freq1 == freq2) 
   {
      errorbeep(2); pms = 0; return;
   }
   if(freq1 < freq2)
   {
   direction = 1;
   lofreq = freq1; hifreq = freq2;
   }
   else
   {
   direction = 0;
   lofreq = freq2; hifreq = freq1;
   }
   tmpfreq1 = load_vfo_f(active_vfo); 
   while(true)
   {
      if(!stopped)
      {
         force_update = 0;
         ++counter3;
        if(counter3 > VFOflash)
        {
        if (blink) blink = 0; else blink = 1;
        counter3 = 0;
        }
         if(blink) VFD_data(1, 0xFF, tmpfreq1, 0xFF, 0);
         else VFD_data(12, 0xFF, tmpfreq1, 0xFF, 0);
         update_PLL(tmpfreq1);
         ++counter1;
         if(counter1 >= VFO_dwell_time)
         {
            if(direction) tmpfreq1+=10; else tmpfreq1-=10;
            counter1 = 0;
         }
         if(squelch_open)
         {
         force_update = 1;
         VFD_data(0xFF, 0xFF, tmpfreq1, 0xFF, 0);
         stopped = 1;
         }
         if(sw_pms) break;
      } 
   
      if(stopped)
      {
         while(squelch_open)
         {
            force_update = 1;
            if(!dial_clock())
            {
               while(!dial_clock()){}
               if(!dial_dir) {tmpfreq1-=10; direction = 0;}
               else if(dial_dir) {tmpfreq1+=10; direction = 1;}
               VFD_data(0xFF, 0xFF, tmpfreq1, 0xFF, 0);
               update_PLL(tmpfreq1);
             }
             if(sw_pms) break;
         }
         delay_ms(10);
         ++counter2;
         
         if(counter2 > scan_pause_countVFO) 
         {
         stopped = 0;
         counter2 = 0;
         }
       
      }

   if(tmpfreq1 < lofreq) tmpfreq1 = hifreq;
   else
   if(tmpfreq1 > hifreq) tmpfreq1 = lofreq;
   if(sw_pms) break; 
  }
  
   if (squelch_open)
   {
      frequency = tmpfreq1;
      save_vfo_f(active_vfo, frequency);
   }
}


void pms_mr()
{
int32 tmpfreq1;
int8 start = 0; int8 limit = 14;
int1 stopped = 0;
int1 blink;
int16 counter1, counter2, counter3;

while(true)
  {
     if(!stopped)
     {
        tmpfreq1 = load_mem_ch_f(mem_channel);
        if(counter1 > MR_dwell_time)
        {
         counter1 = 0;
         if(mem_channel < limit) ++mem_channel; 
                     else mem_channel = start;
         
        }
        
        force_update = 1;
        ++counter3;
        if(counter3 > mrflash)
        {
        if (blink) blink = 0; else blink = 1;
        counter3 = 0;
        }
        if(blink) VFD_data(0xFF, 0xFF, tmpfreq1, mem_channel, 0);
        else VFD_data(2, 0xFF, tmpfreq1, mem_channel, 0);
        update_PLL(tmpfreq1);
        if(sw_pms) break;
        if(squelch_open) stopped = 1;
        delay_ms(1);
        ++counter1;
        
        
     }
     
     if(stopped)
     {
      VFD_data(2, 0xFF, tmpfreq1, mem_channel, 0);
      delay_ms(10);
      if(dial_moved()) stopped = 0;
      ++counter2;
      if(counter2 > scan_pause_countMRCB) 
      {
         counter2 = 0;
         if (squelch_open) stopped = 1; else stopped = 0;
         
      }
      if(sw_pms) break;
      }
     
  }
}

void pms_cb()
{
#ifdef include_cb
int8 tmp_cb_channel = cb_channel;
int8 disp_cb_channel = 0;
int8 start; int8 limit;
int1 blink;
int32 tmpfreq1;
if(cb_region == 0)
      {
      if((tmp_cb_channel < 1) || (tmp_cb_channel > 39)) tmp_cb_channel = 0;
         limit = 39; start = 0;
         disp_cb_channel = tmp_cb_channel + 1;
      }
      if(cb_region == 1)
      {
      if((tmp_cb_channel < 40) || (tmp_cb_channel > 79)) tmp_cb_channel = 40;
         limit = 79; start = 40;
         disp_cb_channel = tmp_cb_channel - 39;
      }
      if(cb_region == 2)
      {
      if((tmp_cb_channel < 1) || (tmp_cb_channel > 79)) tmp_cb_channel = 0;
         limit = 79; start = 0;
         disp_cb_channel = tmp_cb_channel + 1;
      } 

int1 stopped = 0;
int16 counter1, counter2, counter3;

while(true)
  {
     if(!stopped)
     {
        
        tmpfreq1 = cb_channel_bank[tmp_cb_channel];
        if((cb_region == 0) || (cb_region == 2)) disp_cb_channel = tmp_cb_channel + 1;
        else disp_cb_channel = tmp_cb_channel - 39;
        force_update = 1;
        ++counter3;
        if(counter3 > cbflash)
        {
        if (blink) blink = 0; else blink = 1;
        counter3 = 0;
        }
        if(blink) VFD_data(0xFF, 0xFF, disp_cb_channel, 0xFF, 2);
        else VFD_data(2, 0xFF, disp_cb_channel, 0xFF, 2);
        update_PLL(tmpfreq1);
        if((counter1 > CB_dwell_time) || (dial_moved()))
        {
         counter1 = 0;
         if(tmp_cb_channel < limit) ++tmp_cb_channel;
         else tmp_cb_channel = start;
        }
        
        if(sw_pms) break;
        if(squelch_open) stopped = 1;
        delay_ms(1);
        ++counter1;
        
        
     }
     
     if(stopped)
     {
     if(dial_moved()) stopped = 0;
      VFD_data(2, 0xFF, disp_cb_channel, 0xFF, 2);
      delay_ms(10);
      ++counter2;
      
      if(counter2 > scan_pause_countMRCB) 
      {
         counter2 = 0;
         if (squelch_open) stopped = 1; else stopped = 0;
         
      }
      if(sw_pms) break;
      }
     
  }
if (squelch_open)
{
cb_channel = disp_cb_channel;
}
#endif
}

void program_mem_scan()
{
#ifdef include_pms
   int8 res;
   if(mem_mode == 0) res = 1;
   if(mem_mode == 1) res = 2;
   if(mem_mode == 2) res = 3;
   if(sw_500k) res +=3;
   
   if(res == 1) pms_vfo(band_bank[band], band_bank[band+1]);
   if(res == 2) 
   {
   if (mem_channel < 14) pms_vfo(load_mem_ch_f(mem_channel), load_mem_ch_f(mem_channel + 1));
   else pms_vfo(load_mem_ch_f(mem_channel), load_mem_ch_f(0));
   }
   
   if(res == 4) pms_vfo(band_bank[band], band_bank_edge[band]);
   if(res == 5) pms_mr();
   if((res == 3) || (res == 6)) pms_cb();
   res = 0;
   pms = 0;
   beep();
   while(sw_pms){}
#endif
}


void serial_operation()
{
static int32 old_disp_checksum;
if(old_disp_checksum != disp_checksum)
{
if(mem_mode == 0)
{
if(active_vfo == 0) printf ("VFO A ");
if(active_vfo == 1) printf ("VFO B ");
}
if(mem_mode == 1)
{
printf ("MEM %d ", mem_channel);
}
printf ("%ld\n\r", frequency);
old_disp_checksum = disp_checksum;
}
}

void swap_cat_mode()
{
if(cat_mode == 0) {save_cat_mode_n (1); cat_mode = 1;}
else {save_cat_mode_n (0); cat_mode = 0;}
force_update = 1;
VFD_data(0xFF, 0xFF, cat_mode, 0xFF, 2);
}

void change_baud_rate()
{
   int8 btnres;
   
   baud_rate_n = load_baud_rate_n();
   if((baud_rate_n < 1) || (baud_rate_n > 8))baud_rate_n = 0;
   
   while(true)
   {
   
   switch (baud_rate_n)
   {
   case 0: baud_rate = 4800; break; 
   case 1: baud_rate = 1200; break;
   case 2: baud_rate = 2400; break;
   case 3: baud_rate = 4800; break;
   case 4: baud_rate = 9600; break;
   case 5: baud_rate = 19200; break;
   case 6: baud_rate = 38400; break;
   case 7: baud_rate = 57600; break;
   case 8: baud_rate = 115200; break;
   }
   VFD_data(0xFF, 0xFF, (baud_rate * 10), 0xFF, 0);
   save_baud_rate_n(baud_rate_n);
   btnres = buttons();
   if(btnres == 9) {beep(); ++baud_rate_n;}
   if(btnres == 29) {cls(); errorbeep(3); break;}
   if(baud_rate_n > 8) baud_rate_n = 1;
   }
}

void set_defaults()
{
      save32(0, 700000);
      for (int i = 1; i < 20; i++)
      {save32(i, 700000);}
      
      for (i = 20; i < 40; i++)
      {save32(i, 0);}
      //save32(3, 1010000);
      save_vfo_n(0); //Active VFO A/B
      save_mode_n(0); //Mem mode
      save_mem_ch_n(0); //Default mem channel
      save_fine_tune_n(0); //Fine-tuning display (disabled by default)
      save_dial_n(0); //dial_accel/type 0 = disabled, 1 = type1
      save_dcs_n(15);
      save_band_offset_n(0);
      save_cat_mode_n(default_cat_mode);
      save_baud_rate_n(0);
      save_dummy_mode_n(49);
      save_cache_vfo_f(700000);
      save_cache_mem_mode_f(700000);
      save_speed1_n(out_speed1);
      
#ifdef include_dial_accel
      save_speed2_n(out_speed2);
      save_speed3_n(out_speed3);
      save_speed4_n(out_speed4);
#endif
#ifdef include_cb      
      save_cb_ch_n(19); //CB ch 1-80 on default channel
      save_cb_reg_n(0); //CB region 0-CEPT, 1 UK, 2 80CH (1-80) - CEPT 1-40, UK 41-80
#endif
      save_checkbyte_n(1); //Check byte
#ifdef store_to_eeprom
      reset_cpu();
#endif
}

void load_values()
{
#ifdef store_to_eeprom
   active_vfo =         load_vfo_n();
   mem_mode =           load_mode_n();
   mem_channel =        load_mem_ch_n();
   fine_tune =          load_fine_tune_n();
   per_band_offset =    load_band_offset_n();
   cat_mode =           load_cat_mode_n();
   baud_rate_n =        load_baud_rate_n();
   dummy_mode =         load_dummy_mode_n();
   speed_dial =         load_dial_n();
   speed1 =             load_speed1_n();
   dcs =                load_dcs_n();
#ifdef include_dial_accel
   speed2 =             load_speed2_n();
   speed3 =             load_speed3_n();
   speed4 =             load_speed4_n();
#endif
#ifdef include_cb
   cb_region =          load_cb_reg_n();
   cb_channel =         load_cb_ch_n();
#endif

#else
   active_vfo =         0;
   mem_mode =           0;
   mem_channel =        0;
   fine_tune =          0;
   per_band_offset =    0;
   cat_mode =           default_cat_mode;
   speed_dial =         0;
   dcs =                15;
   speed1 =             out_speed1;
#ifdef include_dial_accel
   speed2 =             out_speed2;
   speed3 =             out_speed3;
   speed4 =             out_speed4;
#endif
#ifdef include_cb
   cb_region =          0;
   cb_channel =         19;
#endif
#endif
    process_dcs(1);
    switch (baud_rate_n)
   {
   case 0: set_uart_speed (4800); break; 
   case 1: set_uart_speed (1200); break;
   case 2: set_uart_speed (2400); break;
   case 3: set_uart_speed (4800); break;
   case 4: set_uart_speed (9600); break;
   case 5: set_uart_speed (19200); break;
   case 6: set_uart_speed (38400); break;
   case 7: set_uart_speed (57600); break;
   case 8: set_uart_speed (115200); break;
   }
}

void main()
{

   delay_ms(200);
   setup_adc(ADC_OFF);
   set_tris_a(0b00001);
   set_tris_b(0b00000000);
   set_tris_c(0b11111111);
   set_tris_d(0b11111111);
   set_tris_e(0b000);
start:
#ifdef include_cat
   disable_interrupts(int_rda);
   enable_interrupts(int_rda); //toggle interrupts to ensure serial is ready
   enable_interrupts(global); //enable interrupts for CAT
#endif
   
   PLL_REF();
   
   mic_pressed = 0;
   pms = 0;
   AI = 0;
   cls();
   int8 micres = 0;
   int8 dialres = 0;
   int8 btnres = 0;
   int8 counterstart = 0;
   int16 counter = 0;
   int32 counter1 = 0;
   int16 counter2 = 0;
   fine_tune_display = 0;
   k1 = 1; delay_us(1);
   if (pb0) gen_tx = 0;  // //widebanded?
   else gen_tx = 1;
   k1 = 0;
   k4 = 1; delay_us(1);
   if (pb1)
   {
   
   swap_cat_mode();  
   while(pb1){}
   delay_ms(2000);
   goto start;
   } else beep();
   if(cat_mode == 0)  COMMAND_LENGTH = 5; else COMMAND_LENGTH = 26;
   k4 = 0;
   k8 = 1; delay_us(1);
   if(cat_mode == 1)
      {
         if(pb0)
         {
         errorbeep(3);
         while(pb0){}
         change_baud_rate();
         k8 = 1; delay_us(1);
         while(pb0){}
         }
      }
   
   k8 = 0;
#ifdef store_to_eeprom
   if(load_checkbyte_n() != 1) {set_defaults();}
   else load_values();
   load_frequency(mem_mode);
#else
   set_defaults();
   load_values();
#endif
   res1 = read_counter();
   res2 = res1;
   //
   force_update = 1;
   vfo_disp(active_vfo, dcs, frequency, mem_channel);
   update_PLL(frequency);
   
   while(true)
   {
   res1 = read_counter();
   #ifdef include_cat
      if(cat_mode == 0)
      {
         if(command_received)
         {
            
            command_received = 0;
            parse_cat_command_yaesu (buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]);
            command_processed = 1;
         }
      }
  
  
      if(cat_mode == 1)
      {
         if((!dialres) && (command_received))
         {
         
            command_received = 0;
            parse_cat_command_kenwood ();
            command_processed = 1;
         }
      }
   #endif
      ++counter2; ++counter1;
      if(counterstart) ++counter;
      //mic buttons
      btnres = buttons();
      if(btnres) buttonaction(btnres);
#ifdef include_mic_buttons
      if(!fine_tune) micres = mic_button(speed1);
      else micres = mic_button(1);
      
#endif
      transmit_check();
      if(pms) program_mem_scan();
         if(mem_mode == 0 || mem_mode == 1) 
         {
    #ifdef include_dial_accel
         if((!dialres) || (force_update))
         {
    
         if(speed_dial == 1)
         {
         if (!fine_tune) dialres = freq_dial_accel_type1(frequency, speed1);
         else dialres = freq_dial_accel_type1(frequency, 1);
         }
   //!      if(speed_dial == 2)
   //!      {
   //!      if (!fine_tune) dialres = freq_dial_accel_type2(frequency, 5);
   //!      else dialres = freq_dial_accel_type2(frequency, 1);
   //!      }
         if(speed_dial == 0)
         {
         if (!fine_tune) dialres = freq_dial_basic(frequency, speed1);
         else dialres = freq_dial_basic(frequency, 1);
         }
         } 
    #else
          if((!dialres) || (force_update))
          {
          if (!fine_tune) dialres = freq_dial_basic(frequency, speed1);
          else dialres = freq_dial_basic(frequency, 1);
          }
    #endif
         
          int16 counterlimit;
          if(dialres !=0) counter2 = 0;
          if(dialres == 1) counterlimit = 6000;
          if(dialres > 1) counterlimit = 0;
          if(dialres == 1 || micres == 1) 
          {
          counterstart = 1;counter = 0; if(fine_tune) fine_tune_display = 1;
          }
          if(counter > counterlimit)
          {
          counterstart = 0;
          counter = 0;
          fine_tune_display = 0;
          }
          
   
          if(counter2 > 5000) 
          {
          if (!mem_mode) save_cache_vfo_f(frequency);
          else save_cache_mem_mode_f(frequency);
          counter2 = 0;
          }
   #ifdef include_cat      
//!          if((AI) && (counter1 > 150000))
//!          {
//!             if(!dialres) calc_if();
//!             else counter1 = 0;
//!          }
   #endif
          check_limits();
          vfo_disp(active_vfo, dcs, frequency, mem_channel);
          }
          if((micres) || (btnres))
          {
          if(mem_mode == 0) save_vfo_f(active_vfo, load_cache_vfo_f());
          //if(mem_mode == 1) save_vfo_f(active_vfo, load_cache_vfo_f());
          }
   #ifdef include_cb     
         if(mem_mode == 2)
         {
         dialres = misc_dial8(cb_channel,0);
         check_limits();
         load_frequency(2);
         //if((count & 1) == 1) buttonaction(buttons());
         
         VFD_data(0xFF, 0xFF, cb_channel, 0xFF, 2);
         }
   #endif     
   
      update_PLL(frequency);
      //!
      dialres = 0;
      if(force_update) { force_update = 0;}
      //if(check == 2) {frequency = 700000; save_checkbyte_n(1); check = 1;}
   }
}
