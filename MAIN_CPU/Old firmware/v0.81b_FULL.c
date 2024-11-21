//Yaesu CPU firmware v0.81b (c)2024
//Written by Matthew Bostock 
//Testing and research Siegmund Souza
//PLL diagrams and valuable info Daniel Keogh

//!0.81b Changelog
//!Initial PMS support. As follows:
//!With 500k pressed: Scanning of all saved MR channels. This is stock Yaesu behaviour I believe. How the clicking doesn't drive people mad though, I'll never know.
//!Press M>VFO or PMS to exit.
//!
//!
//!Without 500k pressed in:
//!If in VFO mode and you press PMS, scanning will take place between current band and next band.
//!If in MR mode, scanning will take place between current memory channel and next memory channel. If you scan from Channel E (last channel), the end channel will be channel 0
//!There will be a 3 second timeout after squelch closes before scanning continues
//!To skip current frequency the scanner has stopped on, nudge the VFO wheel to continue scanning
//!To switch off, press PMS switch again and the last scanned frequency will become VFO A
//!Program MR memories over serial
//!Fixed missing MR indicator in display
//!Fixed band data not being sent correctly. Incorrect band could have been selected, which would be on-frequency, or miles off.
//!Fixed incorrect line decoder command for the counter preset enable function. It was changing bands, rather than the counter.
//!A twat move by me that I had forgotten to fix. My bad. I guess...
//!All dials now sample the up/down counter now (apart from the CB channel dial, as its unnecessary)
//!Changed refresh of display from refreshing every cycle (when a change is detected), to every 120 cycles. This has zero effect
//!on the display and it permits us to increment the frequency in 10hz steps(yaesu spec), rather than 100hz. So in normal AND 
//!fine-tune modes, the steps are 10hz as original, with hopefully brilliant speed
//!Offset frequency setup is now far more sensitive and "accurate", being based of the up/down counter

//!0.8b Changelog
//!Fixed incorrect Hz being sent to the mixers. This was because the pin we use for strobing the band data was being strobed too 
//!quickly for the first digit, leading to almost an off-by-one (is that what its called?) error.
//!Dial sampling changed to directly sense the feedback from the BCD counter, rather than the old dial pulse. Using the dial pulse is OK, but
//!pulses are being missed. Even manually pulling the pin high didn`t help. Conclusion: The dial pulse is actually not used for counting in stock radios...
//!Its merely there to return a value on if the dial is spinning or not.
//!Result: the dial is far more sensitive now.
//!Removed type 2 dial. It was crap.
//!Fixed VFO not being saved, unless there was a pulse on the dial
//!Fixed TX inhibit vulnerability where you could change the frequency and transmit, thus defeating the switch behind front panel by just pressing MIC button


//!
//!Dial acceleration
//!=================
//!Choice of 2 dial types to use for main VFO tuning
//!0 = Standard non-accelerated dial
//!1 = Accelerated dial - timer / tug-of-war based. Gets faster the longer you spin
//!Change dial type by holding M>VFO button until you hear the two beeps
//!
//!Fine Tuning
//!===========
//!Fine tuning display(disabled by default)... If you require fine tuning, you can hold down D.Lock until the beeps. The display will move one 
//!digit to the left, to enable the "hidden" digit to be displayed. I have left this disabled because I didn't want to cause confusion as it can make
//!the display look a little confusing until you are used to it. Enable or disable by holding D.LOCK
//!
//!Software Frequency Alignment - Auto reference frequency compensation
//!====================================================================
//!Single tuning offset. Tune to any frequency and hold CLAR until the display shows 0. You can then tune up or down, until your frequency is correct.
//!Use of a known on-frequency signal and/or frequency counter (for transmitting tests) is HIGHLY recommended.
//!When happy, hold CLAR to save. To reset, just begin the procedure again and save at 0. Or hold down VFO A/B to reset EEPROM data
//!
//!
//!MR Mode
//!=======
//!
//!14 channels (0-9, plus A,B,C,D and E) - use just as normal MR channels
//!
//!CB Mode
//!=======
//!
//!Long-press MR/VFO to enable / disable
//!Short press SPLIT to change CB bands from GB to CEPT
//!Long press SPLIT to change to 80 ch mode (UK starts from 41)
//!
//!
//!Possible bugs
//!=============
//!
//!PLLs not locking should be solved now. Oops from me.
//!If any tuned frequency is VASTLY incorrect or jumps crazily amount, LET ME KNOW.
//!If you manage to get any weird characters or letters on the screen, LET ME KNOW.
//!
//!
//!Functional tests:
//!VFO tuning:
//!VFO A/B toggle:
//!MR Mode selection:
//!MR saving:
//!CB mode active/deactive:
//!CB region selection:
//!Microphone buttons:
//!CAT function test:
//!Offset setup
//!Fine tuning test
//Frequency accuracy

#include <18F452.h>
#fuses HS,PUT, NOWDT,NOPROTECT,NOLVP, BORV42
#use delay(clock=20000000)
#use rs232(baud=4800, xmit=PIN_C6, rcv=PIN_C7, parity=N, stop=2, ERRORS)
//#org 0x7D00, 0x7FFF {} //For bootloader
//limits
#define min_freq 50000
#define max_freq 3000000
#define refresh 120
//uncomment for bare minimum size. No mic buttons, dial acceleration, CB or CAT. I advise a bigger PIC!
//#define small_rom 

//remove bits and pieces if you wish, or problem is suspected
#ifndef small_rom
#define include_cat
#define include_mic_buttons
#define include_dial_accel
#define include_cb
#define include_offset_programming
#define store_to_eeprom
#endif

//#define debug
//#define ncodes


# byte PORTA = 0x0f80
# byte PORTB = 0x0f81
# byte PORTC = 0x0f82
# byte PORTD = 0x0f83
# byte PORTE = 0x0f84

# bit dial_clk=PORTA.0  //input  // from q66 counter input (normally high, pulses low for 50us, 1/10? of dial) interupt driven?
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


int8 BITSA,BITSB,BITSC;
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
# bit   timerstart = BITSC.0
# bit   long_press = BITSC.1
//# bit   speed_dial = BITSC.2
# bit   force_update = BITSC.3
# bit   gtx = BITSC.4
# bit   stx = BITSC.5
# bit   ctx = BITSC.6
# bit   fine_tune = BITSC.7

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
const int32 band_bank[10] = {
50000, 180000, 350000, 700000, 1010000,
1400000, 1800000, 2100000, 2400000, 2800000
};

int32 PLL_band_bank[30] = {
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
int32 blacklist[20]= {
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

int32 frequency;
int8 mem_channel, PLLband, band, mem_mode, dcs, speed_dial, dhz;

#ifndef store_to_eeprom
//create ram store, follows eeprom-style if enabled
int32 ram_bank[32];
int8 var_bank[16];
#endif

#ifdef include_cat
#define BUFFER_SIZE 10
char temp_byte, garbage_data;//create 10 byte large buffer and garbage zone
char buffer[BUFFER_SIZE]; 
int8 command_received = 0;
int next_in = 1;
long serial_timer = 0;

#INT_RDA
void  RDA_isr(void)
{
   timerstart = 1;
   buffer[0] = '\0';
   if((kbhit()) && (serial_timer > 10000)) 
   {
      for (int i = 0; i == sizeof buffer; ++i)
      {
      buffer[i] = '\0';
      }
      command_received = 0;
      next_in = 1;
      timerstart = 0;
      serial_timer = 0;
      return;
   }

   if((kbhit()) && (serial_timer < 10000)) 
   {
      temp_byte = getc();
      buffer[next_in]= temp_byte;
      next_in++;
      serial_timer = 0;
      if((next_in == 6))
      {
         buffer[next_in] = '\0';             // terminate string with null   
         command_received = 1;
         next_in = 1;
         timerstart = 0;
         if(next_in > 6)  {while (next_in > 6) garbage_data = getc();}      
      }
   }                                           
}

#endif


void beep();
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
write_eeprom(base + 0xF0, value);
#else
var_bank[base] = value;
#endif
}

int8 load8(int8 base) 
{
int32 value;
#ifdef store_to_eeprom
value = read_eeprom(base + 0xF0);
#else
value = var_bank[base];
#endif
return value;
}

void save_vfo_f(int8 vfo, int32 value) {save32 (vfo, value);}
int32 load_vfo_f(int8 vfo) {return (load32 (vfo));}

void save_mem_ch_f(int8 channel, int32 value) {save32 (channel+2, value);}
int32 load_mem_ch_f(int8 channel) {return (load32(channel+2));}

void save_clar_RX_f(int32 value) {save32(17, value);}
int32 load_clar_RX_f() {return (load32 (17));}

void save_clar_TX_f(int32 value) {save32(18, value);}
int32 load_clar_TX_f() {return (load32 (18));}

void save_cache_vfo_f(int32 value) {save32 (19, value);}
int32 load_cache_vfo_f() {return (load32 (19));}

void save_offset_f(int32 value) {save32 (20, value);}
int32 load_offset_f() {return (load32 (20));}

void save_checkbyte_n() {save8(15, 01);}
int8 load_checkbyte_n() {return (load8(15));}
void reset_checkbyte_n() {save8(15, 0xFF);}

void save_vfo_n(int8 res) {save8(14, res);}
int8 load_vfo_n() {return (load8(14));}

void save_mode_n(int8 res) {save8(13, res);}
int8 load_mode_n() {return (load8(13));}

void save_mem_ch_n(int8 res) {save8(12, res);}
int8 load_mem_ch_n() {return (load8(12));}

void save_fine_tune_n(int8 res) {save8(11, res);}
int8 load_fine_tune_n() {return (load8(11));}

void save_dial_n(int8 res) {save8(10, res);}
int8 load_dial_n() {return (load8(10));}

void save_cb_ch_n(int8 res) {save8(9, res);}
int8 load_cb_ch_n() {return (load8(9));}

void save_cb_reg_n(int8 res) {save8(8, res);}
int8 load_cb_reg_n() {return (load8(8));}

void save_dcs_n(int8 res) {save8(7, res);}
int8 load_dcs_n() {return (load8(7));}

void save_cache_n(int8 res) {save8(6, res);}
int8 load_cache_n() {return (load8(6));}

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
//!

void restore_pin_values()
{
 Q64_C = tmp_pin4; 
 Q64_B = tmp_pin5;
 Q64_A = tmp_pin6;
}

void save_pin_values() 
{
 tmp_pin4 = Q64_C;
 tmp_pin5 = Q64_B;
 tmp_pin6 = Q64_A;
}

void PLL1()
{
 save_pin_values();
 Q64_A = 1;
 delay_us(1);
 Q64_A = 0;
 restore_pin_values();
}

void PLL2()
{
 CPU_36_BIT128 = 1;
 delay_us(1);
 CPU_36_BIT128 = 0;
}

void counter_preset_enable()
{
save_pin_values();
Q64_C = 0;
Q64_B = 1; 
Q64_A = 0; 
delay_us(10);
restore_pin_values();
}

void banddata()
{ 
 save_pin_values();                
Q64_C = 0;
Q64_B = 1; 
Q64_A = 1;
 delay_us(10); 
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
{delay_ms(100);beep(); delay_ms(100);}
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
int8 loc10 = 0;
PORTB = loc10 + val10;
counter_preset_enable();
//printf("loaded 10");
}

void load_100hz(int8 val100)
{

int8 loc100 = 0;
PORTB = loc100 + val100;
}


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
      int8 d1,d2,d3,d4,d5,d6,d7,d8,d9,d10;
      
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
      //printf("counter %d\r\n", read_counter());
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
d3 = 0xFF;
}
if(mem_mode == 1) 
{
d1 = 2;
d3 = ch;
}
VFD_data(d1, d2, freq, d3, fine_tune_display);
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
      
      ///lets load any offset we may have...
#ifdef store_to_eeprom
#ifdef include_offset_programming
      int32 offset = load_offset_f();
      if (offset >= 1000000) offset_frequency -= (offset - 1000000);
      else offset_frequency += offset;
#endif
#endif
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

       //
       //
       load_100hz(d100);
       
       
      oldcheck = newcheck;
      set_dl(dltmp);
      dl = dltmp;
      #ifdef debug
      puts("tuned PLL!");
      #endif
      //printf("%d%d%d\r\n", PLL1_NCODE_L2, PLL1_NCODE_L1, PLL1_NCODE_L0);
      //printf("%d%d\r\n", PLL2_NCODE_L1, PLL2_NCODE_L0 );
      
      //printf("%ld\r\n", offset_frequency);
      //printf("%d\r\n", PLLband);
      
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
   save_dcs_n(dcs);
   }

}

//frequency loader for each mode. For mode 0 (VFO mode), probably not used
void load_frequency(int8 mode)
{
if(mode == 0) frequency = load_vfo_f(active_vfo);
if(mode == 1) frequency = load_mem_ch_f(mem_channel);
#ifdef include_cb
if(mode == 2) 
   {
   if(cb_region == 0) {channel_amount = 40; frequency = cb_channel_bank[cb_channel - 1];}
   if(cb_region == 1) {channel_amount = 40; frequency = cb_channel_bank[cb_channel + 39];}
   if(cb_region == 2) {channel_amount = 80; frequency = cb_channel_bank[cb_channel - 1];}
   }
#endif
   
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

void down_button()
{
   if(!sw_500k)
   {
      if(mem_mode == 0)
      {
      if (band > 0) --band;
      frequency = band_bank[band];
      }
      if(mem_mode == 1)
      {
      if(mem_channel > 0) --mem_channel;
      load_frequency(1);
      }
#ifdef include_cb
      if(mem_mode == 2)
      {
      if(cb_channel > 1) --cb_channel;
      }
#endif
   }
   else
   {
   if(mem_mode == 0)
      {
      frequency -= 50000;
      }
   }
}

void up_button()
{
   if(!sw_500k)
   {
      if(mem_mode == 0)
      {
      if (band < 9) ++band;
      frequency = band_bank[band];
      }
      if(mem_mode == 1)
      {
      if(mem_channel < 14) ++mem_channel;
      load_frequency(1);
      }
#ifdef include_cb
      if(mem_mode == 2)
      {
      if(cb_channel < channel_amount) ++cb_channel;
      }
#endif
   }
   else
   {
   if(mem_mode == 0)
      {
      frequency += 50000;
      }
   }
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

void mvfo_button()
{
   if(mem_mode == 0)
   {
   frequency = load_mem_ch_f(mem_channel);
   save_vfo_f(active_vfo, frequency);
   }
   if(mem_mode == 1)
   {
   save_vfo_f(active_vfo, frequency);
   }
}

void vfom_button()
{
   if(mem_mode == 0)
   {
   save_mem_ch_f(mem_channel, frequency);
   }
   if(mem_mode == 1)
   {
   //frequency = load_value(active_vfo);
   save_mem_ch_f(mem_channel, frequency);
   }
}


void mrvfo_button()
{
if(mem_mode == 0)
{
save_cache_vfo_f(frequency);
mem_mode = 1;
save_mode_n(mem_mode);
load_frequency(mem_mode);
return;
}
if(mem_mode == 1)
{
frequency = load_cache_vfo_f();
mem_mode = 0;
save_mode_n(mem_mode);
return;
}
}

void vfoab_button()
{
   save_vfo_f(active_vfo, frequency);
   if(active_vfo == 0) active_vfo = 1;
   else active_vfo = 0;
   frequency = load_vfo_f(active_vfo);
   //load_values();
   force_update = 1;
}

void vfom_swap_button()
{
   if(mem_mode == 0)
   {
   frequency = load_vfo_f(active_vfo); //load vfo contents
   save_cache_vfo_f(frequency); //save vfo to cache
   frequency = load_mem_ch_f(mem_channel);//load mem channel
   save_vfo_f(active_vfo, frequency);
   frequency = load_cache_vfo_f(); //load cached vfo
   save_mem_ch_f(mem_channel, frequency); //save saved vfo to mem ch
   frequency = load_vfo_f(active_vfo); //load vfo contents
   return;
   }
   
   if(mem_mode == 1)
   {
   frequency = load_mem_ch_f(mem_channel);//load mem channel
   save_cache_vfo_f(frequency); //save vfo to cache
   frequency = load_vfo_f(active_vfo); //load vfo contents
   save_mem_ch_f(mem_channel, frequency); //save saved vfo to mem ch
   frequency = load_cache_vfo_f(); //load cached vfo
   save_vfo_f(active_vfo, frequency);
   frequency = load_mem_ch_f(mem_channel);//load mem channel
   return;
   }
}

void toggle_fine_tune_display()
{
if(fine_tune == 1) fine_tune = 0; else fine_tune = 1;
save_fine_tune_n(fine_tune);
errorbeep(1);
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
save_dial_n(speed_dial);
errorbeep(1);
#endif
}


#ifdef include_cb
void toggle_cb_mode()
{
if(mem_mode == 0 || mem_mode == 1)
{
save_cache_n(mem_mode);
mem_mode = 2;
save_mode_n(mem_mode);
return;
}
if(mem_mode == 2) mem_mode = load_cache_n();
load_frequency(mem_mode);
save_mode_n(mem_mode);
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
   save_cb_reg_n(cb_region);
   }
}
#endif

void program_offset();
int8 buttonaction (int8 res)
{

   if(!res) return 0;
   if       (res == 1) clarifier_button();
   else if  (res == 2) down_button();
   else if  (res == 3) up_button();
   else if  (res == 4) mvfo_button();
   else if  (res == 5) vfoab_button();
   else if  (res == 6) dial_lock_button();
   else if  (res == 7) vfom_button();
   else if  (res == 8) mrvfo_button();
#ifdef include_cb
   else if  (res == 9) {
   if(mem_mode !=2) split_button();
   else toggle_cb_region();
}
#else 
   else if  (res == 9) split_button();
#endif
   else if  (res == 11) vfom_swap_button();
#ifdef store_to_eeprom
   else if  (res == 21) program_offset();
#endif         
   else if  (res == 25) 
   {
      //toggle ports and counters to ensure we can TX
      for (int i = 0; i < 3; i++)
      {
      Q64_C = 1; Q64_B = 1; Q64_A = 1;
      PORTA = 0; PORTB = 0; PORTC = 0; PORTD = 0;
      delay_ms(50);
      Q64_C = 0; Q64_B = 0; Q64_A = 0;
      PORTA = 0; PORTB = 0; PORTC = 0; PORTD = 0;
      delay_ms(50);
      }
      #ifdef store_to_eeprom
      reset_checkbyte_n();
      #endif
      reset_cpu();
   }
   else if  (res == 24) toggle_speed_dial();
   else if  (res == 26) toggle_fine_tune_display();
#ifdef include_cb
   else if  (res == 28) {if(gen_tx) toggle_cb_mode();}
   else if (res == 29) 
   {
      if(gen_tx)
      {
         VFD_special_data(4);
         force_update = 1; VFD_data(0xFF, 0xFF, cb_channel, 0xFF, 2);
         cb_region = 2;
         save_cb_reg_n(cb_region);
      }
   }
#endif
force_update = 1;
return res;
}

int8 buttons()
{
      int res = 0;
      static int cycle = 0; ++cycle;
      if(cycle < 16) return res;
      cycle = 0; disp_int = 0;
      
      if(pb2)
      {
         while (pb2){}
         k8 = 0;  k4 = 0;  k2 = 0;  k1 = 0;
         int count = 0;
         
         if (btn_down)
         {
            k2 = 1; k4 = 1; k8 = 1;
            if (!pb2 && !pb1 && !pb0){res = 0; btn_down = 0; long_press = 0; goto out;}
            else {res = 0; long_press = 0; goto out;}
         }
         
         k4 = 0; k8 = 0; k1 = 0; k2 = 1; delay_us(1);
         if (pb2){while(pb2){++count;  delay_ms(100); if(count >=30) {long_press = 1; break;}} btn_down = 1; res = 1;  goto out;} //(RESULT: 1) Clarifier
         if (pb1){while(pb1){++count;  delay_ms(100); if(count >=30) {long_press = 1; break;}} btn_down = 1; res = 2;  goto out;} //(RESULT: 2) Down
         if (pb0){while(pb0){++count;  delay_ms(100); if(count >=30) {long_press = 1; break;}} btn_down = 1; res = 3;  goto out;} //(RESULT: 3) Up
         
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
         if(long_press) res+= 20;
         beep();
         }
      
    
    return res;
}

#ifdef include_dial_accel
//here we set our dial timer ( or triggers). As the timer decreases, it will pass these threasholds
//these dial timers can be anything lower than main dial timer. 
//set these to whatever you like. eg if you have a main timer of 2000, you could set timer 1 to 300, timer 2 to 250 etc.
//They need to descend because the main timer (dial_timer) descends

//basic tug-o-war between positive and negative pulses. Dial timers are in percentages / 10
//screen MUST be refreshed immediately. Set update_display flag ( as below)

#define dial_timer_max 20000 //max spin down-timer. All percentages are based of this. Default 1000
//these are the trigger percentages. As dial_timer decreases, we will approach eg 80% of dialtimer, which is percent_of_d_timer_speed2.
//We can change these percentages so different speeds happen earlier or later. Defaults 80, 60, 40, 20, 10.
#define percent_of_d_timer_speed2 80 //default 80%
#define percent_of_d_timer_speed3 60 //default 60%
#define percent_of_d_timer_speed4 40 //default 40%
#define percent_of_d_timer_speed5 20 //default 20%
#define percent_of_d_timer_speed6 10 //default 10%

#define stop_reset_count 5 //stop spin check sampling count. How fast dial reverts to lowest increment. Higher - longer, Lower, shorter. Default 25
#define negative_pulse_sample_rate 50 //how often to check for negative pulse to fight against the decrement. Higher number means easier acceleration. Lower is harder (eg faster and harder). Default 200
#define dial_timer_decrement 5 //overall timer decrement when spinning. How fast the main timker decreases. Default 10. Lower = longer spin before it speeds up. Not harder, just longer
#define dial_timer_pullback 15 //dial increment when negative (eg when slowing down spinning). How hard we fight against the decrement. Needs to be more than decrement. Default 15. Lower = acceleration increments kick in earlier

   //dial output counts(the value returned - the MAIN variable for eg frequency
   //COSMETICS:-
   //note the perculiar dial increments... They could just be nice round numbers like 5, 10, 100, 1000 etc...
   //However... Shifting the next-to-last digit lets all digits in the VFD change. If these were nice round numbers, only the digit being changed would appear to change.
   //This is only a cosmetic thing. Feel free to round them up or down as you like
//#define default_out_speed 5 //Slowest
#define out_speed2 11      //Slightly faster
#define out_speed3 51     //Faster still
#define out_speed4 123    //Super-fast
#define out_speed5 567   //Ultra-fast

int16 freq_dial_accel_type1(int32 &value, int16 start_increment)
{
   int8 res = 0;
   static int16 count = 0;
   static int16 stop_count = 0;  
   static int16 dial_timer = dial_timer_max;
   //if(reset) { dial_timer = 400; dial_increment = default_increment;}
   static int16 dial_timer1, dial_timer2, dial_timer3, dial_timer4, dial_timer5;
   
   
   static int16 dial_increment = start_increment;
   dial_timer1 = (dial_timer_max / 100) * percent_of_d_timer_speed2; //eg 80%
   dial_timer2 = (dial_timer_max / 100) * percent_of_d_timer_speed3;
   dial_timer3 = (dial_timer_max / 100) * percent_of_d_timer_speed4;
   dial_timer4 = (dial_timer_max / 100) * percent_of_d_timer_speed5;
   dial_timer5 = (dial_timer_max / 100) * percent_of_d_timer_speed6;
   
   
   int8 res1 = read_counter();
   static int8 res2;
   if(res1 != res2)
      {
      tmp = dial_dir;
      if(tmp) value +=dial_increment; else value -=dial_increment;
   
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
         if(dial_clk)
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
   if((dial_timer <= dial_timer1) && (dial_timer > dial_timer2))  dial_increment = out_speed2;
   if((dial_timer <= dial_timer2) && (dial_timer > dial_timer3))  dial_increment = out_speed3;
   if((dial_timer <= dial_timer3) && (dial_timer > dial_timer4))  dial_increment = out_speed4;
   if((dial_timer <= dial_timer4) && (dial_timer > dial_timer5))  dial_increment = out_speed5;

return res;
}
#endif

int16 freq_dial_basic(int32 &value, int16 dial_increment)
{
   int8 res = 0;
   int8 res1 = read_counter();
   static int res2;
   if(res1 != res2)
      {
         tmp = dial_dir;
         if(tmp) value+=dial_increment; else value-=dial_increment;
         res = 1;
         res2 = res1;
      }
      
   //if(res) putc(res);
   return res;
}


int8 misc_dial(int32 &value, int8 direction)
{
   int8 res1 = read_counter();
   static int res2;
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
   if(!dial_clk)
   {
      tmp = dial_dir;
      while(!dial_clk )
         continue;
      if(!direction)
      {
      if(tmp) value+=1; else value-=1;
      }
      else
      {
      if(tmp) value-=1; else value+=1;
      }
      return 1;
   } 
   else return 0;
}


#ifdef store_to_eeprom
void program_offset()
{
#ifdef include_offset_programming
save_offset_f(0);
update_PLL(frequency);
int32 test = load_offset_f();
int8 res = 0;
int8 btnres = 0;
int32 testfreq = 0;
force_update = 1;
 VFD_data(0xFF, 0xFF, 1, 0xFF, 5);
while(true)
{
if (misc_dial (test,dir)) res = 1;
if (test == -1) {test = 1; if(dir == 0) dir = 1; else dir = 0;}
if(test > 998) test = 999;
//if (test > 1000) {test = 1000; if(dir == 0) dir = 1; else dir = 0;}
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

btnres = buttons();
if(btnres == 1)
{
VFD_data(0xFF, 0xFF, frequency, 0xFF, 0);
delay_ms(1000);
btnres = 0;
}
else if (btnres == 6) break;
else if(btnres == 21) {
if(dir) save_offset_f(1000000 + test); 
if(!dir) save_offset_f(test);
force_update = 1;
break;
}
}
#endif
}
#endif

#ifdef include_cat
void parse_cat_command (char byte1, char byte2, char byte3, char byte4, char byte5)
{

int32 byte4_upper = ((byte4 >> 4) & 0xF);
int32 byte4_lower = byte4 & 0xF;
int32 byte3_upper = ((byte3 >> 4) & 0xF);
int32 byte3_lower = byte3 & 0xF;
int32 byte2_upper = ((byte2 >> 4) & 0xF);
int32 byte2_lower = byte2 & 0xF;
int32 byte1_upper = ((byte1 >> 4) & 0xF);
int32 byte1_lower = byte1 & 0xF;

if(byte5 == 1) {split_button();beep();}
else if(byte5 == 2) {mrvfo_button();beep();}
else if(byte5 == 3) {vfom_button();beep();}
else if (byte5 == 4) {dial_lock_button();beep();}
else if(byte5 == 5) {vfoab_button(); beep();}
else if(byte5 == 6) {mvfo_button();beep();}
else if (byte5 == 7) {up_button(); beep();}
else if (byte5 == 8) {down_button();beep();}
else if(byte5 == 11) {vfom_swap_button();beep();}

else if (byte5 == 10){


frequency = ((byte4_lower * 1000000) + (byte3_upper * 100000) + (byte3_lower * 10000) + (byte2_upper * 1000) + (byte2_lower * 100) + (byte1_upper * 10) + byte1_lower);

}
//quick access to saving mem channels
else if ((byte5 >= 0xE0) && (byte5 <= 0xEE)) {save_mem_ch_f((byte5 - 0xE0), (byte4_lower * 1000000) + (byte3_upper * 100000) + (byte3_lower * 10000) + (byte2_upper * 1000) + (byte2_lower * 100) + (byte1_upper * 10) + byte1_lower); errorbeep(3);}

else if (byte5 == 0xFC) gen_tx = 1;
#ifdef include_cb
else if (byte5 == 0xFD) {if(gen_tx) toggle_cb_mode();}
#endif
else if (byte5 == 0xFE) {buttonaction(25);}
else if (byte5 == 0xFF) reset_cpu();
force_update = 1;
}
#endif


void mic_button()
{
#ifdef include_mic_buttons
      static int cycle = 0;
      ++cycle;
      if(cycle < 8) return;
      cycle = 0;
int res = 0;
static int16 cnt = 0;
//static int mic_counter = 0;
if( !mic_fast && !mic_up) res = 1;
else if( !mic_fast && !mic_dn) res = 2;
else if(mic_fast && mic_up && mic_dn) {while (mic_fast && mic_up && mic_dn){} res = 10;}
else if(mic_fast && !mic_up) res = 11;
else if(mic_fast && !mic_dn) res = 12;
if(!res) {cnt = 0; return;}
force_update = 1;
if(mem_mode == 0)
{
   if(res == 1)
   {
      while(!mic_fast && !mic_up)
      {
      ++cnt; delay_ms(5);
      if(cnt >99) break;
      }
      if(cnt < 100) {frequency +=10; cnt = 0; beep();}
      if(cnt>99) 
      {
      ++cnt; delay_ms(5);
      frequency +=10;
      }
      if(cnt>399) 
      {
      ++cnt; delay_ms(5);
      frequency +=100;
      }
      if(cnt>=699) 
      {
      frequency +=500;
      }
   }
   
   if(res == 2)
   {
      while(!mic_fast && !mic_dn)
      {
      ++cnt; delay_ms(5);
      if(cnt >99) break;
      }
      if(cnt < 100) {frequency -=10; cnt = 0; beep();}
      if(cnt>99) 
      {
      ++cnt; delay_ms(5);
      frequency -=10;
      }
      if(cnt>399) 
      {
      ++cnt; delay_ms(5);
      frequency -=100;
      }
      if(cnt>=699) 
      {
      frequency -=500;
      }
   }
   if(res == 10) mrvfo_button();
   if(res == 11) frequency += 1000;
   if(res == 12) frequency -= 1000;
   return;
}

if(mem_mode == 1 || mem_mode == 2)
{
   if(res == 1)
   {
      while(!mic_fast && !mic_up) {}
      up_button();
   }
   if(res == 2)
   {
      while(!mic_fast && !mic_dn) {}
      down_button();
   }
   
   if(mem_mode == 1)
   {
   if(res == 10) mrvfo_button();
   }

  #ifdef include_cb
   if(mem_mode == 2)
   {
   if(res == 10) toggle_cb_region();
   }
   #endif
}
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

void program_mem_scan()
{
      
      int32 freq1, freq2, tmpfreq1, tmpfreq2;
      int16 counter1, counter2;
      int1 stopped = 0;
      int1 blink = 0;
      int8 tmp_cb_channel = load_cb_ch_n();
      if((!sw_500k) && ((mem_mode ==0) || (mem_mode == 1)))
      {
      if(mem_mode == 0)
      {
      freq1 = band_bank[band];
      freq2 = band_bank[band+1];
      }
      if(mem_mode == 1)
      {
      freq1 = load_mem_ch_f(mem_channel);
      if(mem_channel < 14) freq2 = load_mem_ch_f(mem_channel + 1);
      else freq2 = load_mem_ch_f(0);
      }
      if((freq1 == freq2) || (mem_mode == 2)) {errorbeep(2); pms = 0; return;}
      else
            {
            force_update = 1;
            counter1 = 0;
            counter2 = 0;
            
            
            if(freq2 < freq1)
            {
               tmpfreq1 = freq1; tmpfreq2 = freq2;
               //printf ("mem_channel: %d %ld %ld\r\n", mem_channel, tmpfreq1, tmpfreq2);
               while(true)
               {
                  
                  ++counter1;
                  if(counter1 >= 2500)
                  {
                  if(!blink) blink = 1; else blink = 0;
                  counter1 = 0;
                  }
                  if(!blink)
                  {
                  
                  if(mem_mode == 0) VFD_data(1, 0xFF, tmpfreq1, 0xFF, 0);
                  if(mem_mode == 1) VFD_data(2, 0xFF, tmpfreq1, 0xFF, 0);
                  }
                  else 
                  {
                  
                  VFD_data(0xFF, 0xFF, tmpfreq1, 0xFF, 0);
                  }
                  update_PLL(tmpfreq1);
                  force_update = 0;
                  
                  if(squelch_open) {counter2 = 0; stopped = 1;}
                  if(!dial_clk) stopped = 0;
                  if(sw_pms) break;
                  if(buttons() == 4) break;
                  if(stopped == 1)
                  {
                     if(!squelch_open) 
                     {
                     ++counter2;
                     if(counter2 == 50000) 
                     {
                        counter2 = 0; 
                        stopped = 0;
                     }
                     }
                  }
                  else
                  {
                   
                  tmpfreq1 -=1;
                  }
                  if(tmpfreq1 == tmpfreq2) tmpfreq1 = freq1;
                  
                  }
               
               }
               else
               {
                  tmpfreq1 = freq1; tmpfreq2 = freq2;
                  //printf ("mem_channel: %d %ld %ld\r\n", mem_channel, tmpfreq1, tmpfreq2);
                  while(true)
                  {
                     ++counter1;
                     if(counter1 >= 2500)
                     {
                     if(!blink) blink = 1; else blink = 0;
                     counter1 = 0;
                     }
                     if(!blink)
                     {
                     if(mem_mode == 0) VFD_data(1, 0xFF, tmpfreq1, 0xFF, 0);
                     if(mem_mode == 1) VFD_data(2, 0xFF, tmpfreq1, 0xFF, 0);
                     }
                     else
                     {
                     VFD_data(0xFF, 0xFF, tmpfreq1, 0xFF, 0);
                     }
                     update_PLL(tmpfreq1);
                     force_update = 0;
                     
              
                     if(squelch_open) {counter2 = 0; stopped = 1;}
                     if(!dial_clk) stopped = 0;
                     if(sw_pms) break;
                     if(buttons() == 4) break;
                     if(stopped == 1)
                     {
                        if(!squelch_open) 
                        {
                        ++counter2;
                        if(counter2 == 50000) 
                        {
                           counter2 = 0; 
                           stopped = 0;
                        }
                        }
                     }
                     else 
                     {
                 
                     tmpfreq1 +=1;
                     }
                     if(tmpfreq1 == tmpfreq2) tmpfreq1 = freq1;
                     
                  }
               
               }
            }
         
         
      }
      else
      {
      while(true)
         {
         if(mem_mode != 2)
         {
         tmpfreq1 = load_mem_ch_f(mem_channel);
         }
         else 
         {
         tmpfreq1 = cb_channel_bank[tmp_cb_channel];
         }
         ++counter1;
         if(counter1 >= 1200)
         {
         if(!blink) blink = 1; else blink = 0;
         counter1 = 0;
         }
         if(mem_mode !=2)
         {
         if(!blink) VFD_data(2, 0xFF, tmpfreq1, mem_channel, 0);
         else VFD_data(0xFF, 0xFF, tmpfreq1, mem_channel, 0);
         }
         else
         {
         //VFD_data(0xFF, 0xFF, tmpfreq1, mem_channel, 0);
         if(!blink) VFD_data(2, 0xFF, tmp_cb_channel, 0xFF, 2);
         else VFD_data(0xFF, 0xFF, tmp_cb_channel, 0xFF, 2);
         }
         update_PLL(tmpfreq1);
         force_update = 0;
         if(squelch_open) {counter2 = 0; stopped = 1;}
         if(!dial_clk) stopped = 0;
         if(sw_pms) break;
         if(buttons() == 4) break;
         if(stopped == 1)
         {
            if(!squelch_open) 
            {
            ++counter2;
            if(counter2 == 10000) 
            {
               counter2 = 0; 
               stopped = 0;
            }
            }
         }
         else
         if(mem_mode != 2)
         {
         if(counter1 == 0) ++mem_channel;
         if(mem_channel > 14) mem_channel = 0;
         }
         else
         {
         if(counter1 == 0) ++tmp_cb_channel;
         if(tmp_cb_channel > 80) tmp_cb_channel = 1;
         }
         }
      
      }
      if(mem_mode !=2)
      {
      frequency = tmpfreq1;
         save_vfo_f(active_vfo, frequency);
      }
            pms = 0;
         beep();
         while(sw_pms){}
            return;
}


void set_defaults()
{
      for (int i = 0; i < 20; i++)
      {save32(i, 700000);}
      
      for (i = 20; i < 32; i++)
      {save32(i, 0);}
      //save32(3, 1010000);
      save_vfo_n(0); //Active VFO A/B
      save_mode_n(0); //Mem mode
      save_mem_ch_n(0); //Default mem channel
      save_fine_tune_n(0); //Fine-tuning display (disabled by default)
      save_dial_n(0); //dial_accel/type 0 = disabled, 1 = type1, 2 = type2
#ifdef include_cb      
      save_cb_ch_n(19); //CB ch 1-80 on default channel
      save_cb_reg_n(0); //CB region 0-CEPT, 1 UK, 2 80CH (1-80) - CEPT 1-40, UK 41-80
#endif

      save_checkbyte_n(); //Check byte
      //reset_cpu();
}

void load_values()
{
   active_vfo =         load_vfo_n();
   mem_mode =           load_mode_n();
   mem_channel =        load_mem_ch_n();
   fine_tune =          load_fine_tune_n();
   speed_dial =         load_dial_n();
#ifdef include_cb
   cb_region =          load_cb_reg_n();
   cb_channel =         load_cb_ch_n();
#endif

   dcs = 15; process_dcs(1);
}

void main()
{
   delay_ms(100);
   setup_adc(ADC_OFF);
   set_tris_a(0b00001);
   set_tris_b(0b00000000);
   set_tris_c(0b11111111);
   set_tris_d(0b11111110);
   set_tris_e(0b000);
   PLL_REF();
   beep();
start:
   if(load_checkbyte_n() != 1) {set_defaults(); goto start;}
   if(load_checkbyte_n() == 1) load_values();

   k1 = 1; delay_us(1);
   if (pb0) gen_tx = 0;  // //widebanded?
   else gen_tx = 1;
   k1 = 0;
   
   pms = 0;
   #ifdef include_cat
   disable_interrupts(int_rda);
   enable_interrupts(int_rda); //toggle interrupts to ensure serial is ready
   enable_interrupts(global); //enable interrupts for CAT
   #endif
   //frequency = 700000;
   load_frequency(mem_mode);
   vfo_disp(active_vfo, dcs, frequency, mem_channel);
   update_PLL(frequency);
   //load_10hz(dhz);
   int8 dialres = 0;
   int8 counterstart = 0;
   int16 counter = 0;
   int16 counter2 = 0;
   while(true)
   {
   #ifdef include_cat
      if(timerstart){ ++serial_timer; delay_us(10);}
      if(command_received)
      {
      
         command_received = 0;
         parse_cat_command (buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
      }
  
   #endif
      ++counter2;
      if(counterstart) ++counter;
      //mic buttons
      
      buttonaction(buttons());
#ifdef include_mic_buttons
      mic_button();
#endif
      transmit_check();
      if(pms) program_mem_scan();
         if(mem_mode == 0 || mem_mode == 1) 
         {
    #ifdef include_dial_accel
         if(speed_dial == 1)
         {
         if (!fine_tune) dialres = freq_dial_accel_type1(frequency, 1);
         else dialres = freq_dial_accel_type1(frequency, 1);
         }
   //!      if(speed_dial == 2)
   //!      {
   //!      if (!fine_tune) dialres = freq_dial_accel_type2(frequency, 5);
   //!      else dialres = freq_dial_accel_type2(frequency, 1);
   //!      }
         if(speed_dial == 0)
         {
         if (!fine_tune) dialres = freq_dial_basic(frequency, 1);
         else dialres = freq_dial_basic(frequency, 1);
         }
            
    #else
          if (!fine_tune) dialres = freq_dial_basic(frequency, 1);
          else dialres = freq_dial_basic(frequency, 1);
    #endif
         
          int16 counterlimit;
          if(dialres !=0) counter2 = 0;
          if(dialres == 1) counterlimit = 10000;
          if(dialres > 1) counterlimit = 0;
          if(dialres == 1) 
          {
          counterstart = 1;counter = 0; if(fine_tune) fine_tune_display = 1;
          }
          if(counter > counterlimit)
          {
          counterstart = 0;
          counter = 0;
          fine_tune_display = 0;
          }
          
          if(counter2 > 10000) 
          {
          save_vfo_f(active_vfo, frequency);
          counter2 = 0;
          }
          check_limits();
          vfo_disp(active_vfo, dcs, frequency, mem_channel);
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
   }
}
