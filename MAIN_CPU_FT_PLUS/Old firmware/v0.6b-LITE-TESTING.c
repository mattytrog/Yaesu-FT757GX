//Yaesu CPU firmware. (c)2024 Matthew Bostock

//Thanks to VK2TRP for the original PLL / LO diagrams. Saved me a ton of work!
// v0.6b - Lite

//Changelog:

// EEPROM now optional. If EEPROM disabled, memory backup is dependent on voltage/type of backup battery
// Removed CB just for now
// Hopefully simplified a lot of things now
// Very easy to modify this code now... Setup PLL, set frequency, update. Optional to update screen. Got ROM usage down to <25% from 80%!
// Dial acceleration moved to a function, with labelled defines to customise
// CB will add very little as all frequencies are in an array now
// 14 memory locations instead of 9(or 10) - A,B,C,D and E
// Offset setup not included for now. Can still be set though. Just do update_PLL(freq + offset). Investigating bootloaders so this can be updated over CAT
// moved to ints so we can support negative values in a sane way
// out-of-band TX inhibit removed for the moment... More arrays coming

// This is a stepping-stone release, as its all under-the-hood changes.
// Have a play, have a read

#include <18F452.h>
//#include <stdint.h>

//uncomment for bare minimum size. No mic buttons, dial acceleration, CB or CAT. I advise a bigger PIC!
//#define small_rom 

//remove bits and pieces if you wish, or problem is suspected
#ifndef small_rom
#define cat_included
#define mic_buttons
#define dial_accel
#define use_eeprom
#endif

//#define debug

#define min_freq 50000
#define max_freq 3000000

//added 2.7 brownout as parasitic power from serial will keep PIC running
#fuses HS,PUT, NOWDT,NOPROTECT,NOLVP, BORV42
#use delay(clock=20000000)

#use rs232(baud=4800, xmit=PIN_C6, rcv=PIN_C7, parity=N, stop=2, ERRORS)


//-----< Port location defines >-----
# byte PORTA = 0x0f80
# byte PORTB = 0x0f81
# byte PORTC = 0x0f82
# byte PORTD = 0x0f83
# byte PORTE = 0x0f84


# bit dial_clk=PORTA.0  //input  // from q66 counter input (normally high, pulses low for 50us, 1/10? of dial) interupt driven?
# bit Q64_4=PORTA.1  //output // cpu pin 4 bit 2 Q64 line decoder
# bit Q64_5=PORTA.2  //output // cpu pin 5 bit 1
# bit Q64_6=PORTA.3  //output // cpu pin 6 bit 0
# bit disp_int=PORTA.4  //output // display interupt
# bit k1=PORTA.5  //output // display bit 0
# bit pina6=PORTA.6
# bit pina7=PORTA.7



# bit CPU_29_BIT1=PORTB.0  //output // bus data bit 0 pll d0
# bit CPU_30_BIT2=PORTB.1  //output //          bit 1     d1
# bit CPU_31_BIT4=PORTB.2  //output //          bit 2     d2
# bit CPU_32_BIT8=PORTB.3  //output //          bit 3     d3
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


int BITSA,BITSB,BITSC,BITSD;
# bit   tmp_pin4=BITSA.0
# bit   tmp_pin5=BITSA.1 
# bit   tmp_pin6=BITSA.2 
# bit   tmp=BITSA.3
# bit   txi=BITSA.4 
# bit   gen_tx=BITSA.5 
# bit   dl=BITSA.6  
# bit   active_vfo=BITSA.7
# bit   vfo_mode=BITSB.0
# bit   mr_mode=BITSB.1 
# bit   cb_mode=BITSB.2 
# bit   sl=BITSB.3 
# bit   cl=BITSB.4 
//# bit   inactive_vfo=BITSB.5 
# bit   btn_down=BITSB.6 
# bit   count_toggle=BITSB.7 
# bit   dltmp=BITSC.0
# bit   tmp_vfo=BITSC.1
# bit   stored_vfo=BITSC.2
# bit   flip_direction=BITSC.3
# bit   dir=BITSC.4
# bit   negative_value = BITSC.5
# bit   force_refresh = BITSC.6
# bit   cb_on = BITSC.7
# bit   transmitting = BITSD.0
# bit   timerstart = BITSD.1
# bit   long_press = BITSD.2
# bit   acceleration = BITSD.3
# bit   update = BITSD.4

const int32 cb_channel_bank[82] = {
0000000, 2696500, 2697500, 2698500, 2700500, 2701500, 2702500, 2703500, 2705500, 2706500, 2707500,
         2708500, 2710500, 2711500, 2712500, 2713500, 2715500, 2716500, 2717500, 2718500, 2720500,
         2721500, 2722500, 2725500, 2723500, 2724500, 2726500, 2727500, 2728500, 2729500, 2730500,
         2731500, 2732500, 2733500, 2734500, 2735500, 2736500, 2737500, 2738500, 2739500, 2740500,

         2760125, 2761125, 2762125, 2763125, 2764125, 2765125, 2766125, 2767125, 2768125, 2769125,
         2770125, 2771125, 2772125, 2773125, 2774125, 2775125, 2776125, 2777125, 2778125, 2779125,
         2780125, 2781125, 2782125, 2783125, 2784125, 2785125, 2786125, 2787125, 2788125, 2789125,
         2790125, 2791125, 2792125, 2793125, 2794125, 2795125, 2796125, 2797125, 2798125, 2799125
};

const int32 band_bank[10] = {
50000, 180000, 350000, 700000, 1010000,
1400000, 1800000, 2100000, 2400000, 2800000
};

//create ram store, follows eeprom-style if enabled
int32 ram_bank[16];



int32 frequency, offset, cb_channel;
int8 mem_channel, band, cb_channel_amount, cb_region;


#define BUFFER_SIZE 10
char temp_byte;//create 10 byte large buffer
char buffer[BUFFER_SIZE]; 
char garbage_data;
int Got_Data = 0;
int next_in = 1;
long ticktimer = 0;
#INT_RDA
void  RDA_isr(void)
{
timerstart = 1;
buffer[0] = '\0';
if((kbhit()) && (ticktimer > 10000)) {

 //{while (next_in >= 1) garbage_data = getc(); --next_in;}
for (int i = 0; i == sizeof buffer; ++i)
{
buffer[i] = '\0';
}
Got_Data = 0;
next_in = 1;
timerstart = 0;
ticktimer = 0;

return;
}
if((kbhit()) && (ticktimer < 10000)) {
   temp_byte = getc();
   buffer[next_in]= temp_byte;
   next_in++;
   ticktimer = 0;
   if((next_in == 6)){
      buffer[next_in] = '\0';             // terminate string with null   
      Got_Data = 1;
      next_in = 1;
      timerstart = 0;
   if(next_in > 6)  {while (next_in > 6) garbage_data = getc();}   
      
   }
}

                                           
}// INT_R

void beep();
//only save if value different from current contents
void savef(int8 address, unsigned int32 data)
{
   int32 temp_data;
   int8 i;
   for(i = 0; i < 4; i++)
     *((int8 *)(&temp_data) + i) = read_eeprom(address + i);
   
   if(data == temp_data) return;
   for(i = 0; i < 4; i++)
   write_eeprom(address + i, *((int8 *)(&data) + i));
}

unsigned int32 loadf(int8 address)
{
   int8  i;
   int32 data;
   for(i = 0; i < 4; i++)
     *((int8 *)(&data) + i) = read_eeprom(address + i);
   return(data);
}

void saveram(int8 base, int32 value)
{
//
#ifdef use_eeprom
savef (base * 4, value);
#else
ram_bank[base] = value;
#endif
}

int32 loadram(int8 base) 
{
int32 value;
#ifdef use_eeprom
value = loadf(base * 4);
#else
value = ram_bank[base];
#endif
return value;
}

void swapf(int src, int dst)
{
 int32 tmp = loadf (dst);
 savef(dst, (loadf(src)));
 savef(src, tmp);
 
}

void restore_pin_values()
{
 Q64_4 = tmp_pin4; 
 Q64_5 = tmp_pin5;
 Q64_6 = tmp_pin6;
}

void save_pin_values() 
{
 tmp_pin4 = Q64_4;
 tmp_pin5 = Q64_5;
 tmp_pin6 = Q64_6;
}

void beep()
{
 save_pin_values();
 Q64_4 = 1;
 Q64_5 = 0;
 Q64_6 = 0;
 restore_pin_values();
}

void errorbeep()
{
for (int i = 0; i <= 2; ++i)
{delay_ms(100);beep(); delay_ms(100);}
}

int set_dl(int dl_sw)
{
//
if (dl_sw == 1) //lock dial, return true
{
  dl = 1;
  Q64_5 = 0;
  Q64_6 = 1;
  Q64_4 = 1;
  return 1;
}
if (!dl_sw)
{
  dl = 0;
  Q64_5 = 0;
  Q64_4 = 0;
  Q64_6 = 0;
  return 0;
}

}

void set_tx_inhibit(int res)
{
if(res == 1){
 txi = 1; 
 Q64_5 = 1;
 Q64_4 = 1;
 Q64_6 = 0;
}else if (res == 0)
{
txi = 0;
 Q64_4 = 0;
 Q64_5 = 0;
 Q64_6 = 0;
}
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

void display_driver(int8 vfo_grid, int8 dcs, int32 value, int8 channel_grid)
{
      int8 d1 = 15,d2 = 15,d3 = 15,d4 = 15,d5 = 15,d6 = 15,d7 = 15,d8 = 15,d9 = 15;
      
      if(vfo_grid != 0xFF) d1 = vfo_grid;
      
      if(dcs != 0xFF) d2 = dcs;
      
      if(value != 0x7FFFFFFF)
      {
      int32 tmp_value = value;
      if(value < 100000) d3 = 15;
      else {d3 = 0;while(tmp_value > 99999){tmp_value -= 100000; d3+=1;}}
      if(value < 10000) d4 = 15;
      else {d4 = 0;while(tmp_value > 9999){tmp_value -= 10000; d4+=1;}}
      if(value < 1000) d5 = 15;
      else {d5 = 0;while(tmp_value > 999){tmp_value -= 1000; d5+=1;}}
      if(value < 100) d6 = 15;
      else {d6 = 0;while(tmp_value > 99){tmp_value -= 100; d6+=1;}}
      if(value < 10) d7 = 15;
      else {d7 = 0;while(tmp_value > 9){tmp_value -= 10; d7+=1;}}
      if(value < 1) d8 = 15;
      else {d8 = 0;while(tmp_value > 0){tmp_value -= 1; d8+=1;}}
      }
      
      if(channel_grid != 0xFF) d9 = channel_grid;
      set_display_nibbles(d1,d2,d3,d4,d5,d6,d7,d8,d9);
}

void update_screen(active_vfo, dl, cl, sl, int32 frequency, int8 mem_channel)
{

      int8 d1 = 15,d2 = 15,d9 = 15;
      If(vfo_mode){
      if(active_vfo == 0) d1 = 1;
      else if(active_vfo == 1) d1 = 12;
      } else d1 = 15;
      
      //simple truth table to find out whats active for grid(digit 2)
      if      (!dl  && !cl && !sl) d2 = 15; //                 15  - GRID OFF
      else  if( dl  && !cl &&  sl) d2 = 0;  //                  0  - LOCK - SPLIT
      else  if( dl  && !cl && !sl) d2 = 1;  //            1(OR 7)  - LOCK
      else  if(!dl  &&  cl && !sl) d2 = 2;  //                  2  - CLAR
      else  if( dl  &&  cl && !sl) d2 = 3;  //          3 (OR 13)  - LOCK - CLAR
      else  if( dl  &&  cl &&  sl) d2 = 4;  //4(OR 5,6,8,9,10,11)  - LOCK - CLAR - SPLIT
      else  if(!dl  && !cl &&  sl) d2 = 12; //                 12  - SPLIT
      else  if(!dl  &&  cl &&  sl) d2 = 14; //                 14  - CLAR - SPLIT
      
      if(mr_mode) d9 = mem_channel;
      
      display_driver(d1, d2, frequency, d9);


}
//PLL update is fully portable fill in frequency and the output variables (for a display)
//if just want to tune 1 segment eg mhz, set freq_to_PLL to 0
//we actually have two "tuners" in this PLL update code. One for the display, one for the radio frequency.
//(C)cmhz etc is for the display, (D)dmhz etc is for the PLL. If master offset is disabled, the C values are copied to D

void update_PLL(int32 calc_frequency)
{

      static int32 result;
      if(force_refresh) { result = 0; force_refresh =0;}

      if(frequency == result) return;
      result = 0;
      int8 PLLband;
      int32 dmhz1, dmhz2, dmhz, d100k, d10k, d1k, d100, d10;
start:

      dltmp = dl;
      set_dl(1);
      Q64_4 = 0;
      Q64_5 = 0;
      Q64_6 = 0;

//split main frequency into mhz, 100k, 10k, 1k, 100, 10, 1 hz
//Its 32 bit number so we cannot just bitshift it, so we have to cascade it



      int32 tmp_frequency = calc_frequency;
      
      dmhz = 0;while(tmp_frequency > 99999){tmp_frequency -= 100000; dmhz+=1;}
      d100k = 0;while(tmp_frequency > 9999){tmp_frequency -= 10000; d100k+=1;}
      d10k = 0;while(tmp_frequency > 999){tmp_frequency -= 1000; d10k+=1;}
      d1k = 0;while(tmp_frequency > 99){tmp_frequency -= 100; d1k+=1;}
      d100 = 0;while(tmp_frequency > 9){tmp_frequency -= 10; d100+=1;}
      d10 = 0;while(tmp_frequency > 0){tmp_frequency -= 1; d10+=1;}
      
 


//LPF preselector. If this is not correct, you WILL be on the wrong band and PLL probably won't lock

//sum of MHz + 100khz. eg CB band 26,965 would be 269, which is PLL band 9
      int32 tmp_band_freq= ((dmhz * 10) + d100k);
      PLLband = 0;
      if ((tmp_band_freq > 5) && (tmp_band_freq <15))          {PLLband = 0;}          // 0.5 -> 1.5
      else if ((tmp_band_freq >= 15) && (tmp_band_freq <25))   {PLLband = 1;}          // 1.5 -> 2.5
      else if ((tmp_band_freq >= 25) && (tmp_band_freq <40))   {PLLband = 2;}          // 2.5 -> 3.99
      else if ((tmp_band_freq >= 40) && (tmp_band_freq <75))   {PLLband = 3;}          // 4.0 -> 7.499
      else if ((tmp_band_freq >= 75) && (tmp_band_freq <105))   {PLLband = 4;}          // 7.5 -> 10.499
      else if ((tmp_band_freq >= 105) && (tmp_band_freq <145))   {PLLband = 5;}          // 10.5 -> 14.499
      else if ((tmp_band_freq >= 145) && (tmp_band_freq <185))   {PLLband = 6;}          // 14.5 -> 18.499
      else if ((tmp_band_freq >= 185) && (tmp_band_freq <215))   {PLLband = 7;}          // 18.5 -> 21.499
      else if ((tmp_band_freq >= 215) && (tmp_band_freq <250))   {PLLband = 8;}          // 21.5 -> 24.999
      else if ((tmp_band_freq >= 250) && (tmp_band_freq <320))   {PLLband = 9;}          // 25 -> whatever

      PORTB = PLLband; Q64_4 = 0; Q64_5 = 1; Q64_6 = 1; delay_us(10);

      int PLL1_NCODE_L3 = 48; //empty latch
      int PLL1_NCODE_L2 = 0;
      int PLL1_NCODE_L1 = 0;
      int PLL1_NCODE_L0 = 0;
      
      int PLL2_NCODE_L3 = 48; //empty latch
      int PLL2_NCODE_L2 = 32; //empty latch
      int PLL2_NCODE_L1 = 0;
      int PLL2_NCODE_L0 = 0;
         
      int16 tmp_khz_freq = ((d100k * 100) + (d10k * 10) + (d1k));//sum of 100k + 10k + 1k eg 27781.25 = 781

      if(tmp_khz_freq >= 500) tmp_khz_freq -=500; 
      //if over 500 khz, take off 500 because we will calculate PLL2 to allow for the 500.
      //If we are at, say, 7499.9 PLL1 will be at Max. If we go to 7500.0, PLL1 will be at minimum, but PLL2 will
      //increase to the next 500 step.
      
      tmp_khz_freq +=560;
      //add on our 560 (start of N code ref is 560, so will count on from that. eg 0 = 560, 1 = 561 etc etc
   
      // tmp_khz_freq becomes NCODE- this is final NCODE for PLL1. eg 27.78125 = 781... - 500 = 281...281 + 560 = 841 in decimal.
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

       Q64_4 = 0; Q64_5 = 0; Q64_6 = 0;
       PORTB = PLL1_NCODE_L3;
       Q64_6 = 1; delay_us(10); Q64_6 = 0; delay_us(10);
       PORTB = 32 + PLL1_NCODE_L2;
       Q64_6 = 1; delay_us(10); Q64_6 = 0; delay_us(10);
       PORTB = 16 + PLL1_NCODE_L1;
       Q64_6 = 1; delay_us(10); Q64_6 = 0; delay_us(10);
       PORTB = PLL1_NCODE_L0;
       Q64_6 = 1; delay_us(10); Q64_6 = 0; delay_us(10);
       PLL2_NCODE_L3 = 48;
       PORTB = PLL2_NCODE_L3; // Prepare latch
       CPU_36_BIT128 = 1; delay_us(10); CPU_36_BIT128 = 0; delay_us(10);
       PLL2_NCODE_L2 = 32;
       PORTB = PLL2_NCODE_L2; // Prepare latch
       CPU_36_BIT128 = 1; delay_us(10); CPU_36_BIT128 = 0; delay_us(10);
       PORTB = 16 + PLL2_NCODE_L1;
       CPU_36_BIT128 = 1; delay_us(10); CPU_36_BIT128 = 0; delay_us(10);
       PORTB = PLL2_NCODE_L0;
       CPU_36_BIT128 = 1; delay_us(10); CPU_36_BIT128 = 0; delay_us(10);
      
      //load up 10hz + offset to send to BCD counter
      PORTB = (d100);
      delay_us(10);
      Q64_5 = 1;
      delay_us(10);
      Q64_5 = 0;
      
      //100hz just stays on the port
      //PORTB = (d100);
      
      //shush the Q64 CPU lines
      Q64_4 = 0;
      Q64_5 = 0;
      Q64_6 = 0;
      
      set_dl(dltmp);
      dl = dltmp;
      #ifdef debug
      puts("tuned PLL!");
      #endif
      result = frequency;
}

void down_button()
{
   if(vfo_mode &&  !sw_500k)
   {
   if (band > 0) --band;
   frequency = band_bank[band];
   }
   if(vfo_mode &&  sw_500k)
   {
   frequency -= 50000;
   }
   
   if(mr_mode)
   {
   if(mem_channel > 0) --mem_channel;
   frequency = loadram(mem_channel + 2);
   }

}

void up_button()
{
   if(vfo_mode &&  !sw_500k)
   {
   if (band < 9) ++band;
   frequency = band_bank[band];
   }
   if(vfo_mode &&  sw_500k)
   {
   frequency += 50000;
   }
   if(mr_mode)
   {
   if(mem_channel < 14) ++mem_channel;
   frequency = loadram(mem_channel + 2);
   }

}

void dial_lock_button()
{
   if(dl) dl = 0; else dl = 1;
   set_dl(dl);
}

void clarifier_button()
{
   if(cl) cl = 0;
   else 
   {
   cl = 1;
   saveram(20, frequency);
   }
}

void split_button()
{
   if(sl) sl = 0; else sl = 1;
}

void mvfo_button()
{
   if(vfo_mode)
   {
   frequency = loadram(mem_channel + 2);
   saveram(active_vfo, frequency);
   }
   if(mr_mode)
   {
   saveram(active_vfo, frequency);
   }
}

void vfom_button()
{
   if(vfo_mode)
   {
   saveram(mem_channel + 2, frequency);
   }
   if(mr_mode)
   {
   //frequency = loadram(active_vfo);
   saveram(mem_channel + 2, frequency);
   }
}

void vfoab_button()
{
   saveram(active_vfo, frequency);
   if(active_vfo == 0) active_vfo = 1;
   else active_vfo = 0;
   frequency = loadram(active_vfo);
}

void mrvfo_button()
{
   if(vfo_mode)
   {
      saveram(active_vfo, frequency); //save vfo
      frequency = loadram(mem_channel + 2);
      vfo_mode = 0; mr_mode = 1;
   }
   else
   if(mr_mode)
   {
      frequency = loadram(active_vfo);
      vfo_mode = 1; mr_mode = 0;
   }
}

void vfom_swap_button()
{
   if(vfo_mode)
   {
   frequency = loadram(active_vfo); //load vfo contents
   saveram(20, frequency); //save vfo to cache
   frequency = loadram(mem_channel + 2);//load mem channel
   saveram(active_vfo, frequency); //save mem channel to active vfo
   frequency = loadram(20); //load saved vfo
   saveram(mem_channel + 2, frequency); //save saved vfo to mem ch
   frequency = loadram(active_vfo); //load vfo contents
   return;
   }
   
   if(mr_mode)
   {
   frequency = loadram(mem_channel + 2);//load mem channel
   saveram(20, frequency); //save vfo to cache
   frequency = loadram(active_vfo); //load vfo contents
   saveram(mem_channel + 2, frequency); //save saved vfo to mem ch
   frequency = loadram(20); //load saved vfo
   saveram(active_vfo, frequency); //save mem channel to active vfo
   frequency = loadram(mem_channel + 2);//load mem channel
   return;
   }
}

#ifdef mic_buttons
void mic_button()
{
int res = 0;
static int mic_counter = 0;
if( !mic_fast && !mic_up) res = 1;
else if( !mic_fast && !mic_dn) res = 2;
else if(mic_fast && mic_up && mic_dn) {while (mic_fast && mic_up && mic_dn){} res = 10;}
else if(mic_fast && !mic_up) res = 11;
else if(mic_fast && !mic_dn) res = 12;

if(res)
{
   if(vfo_mode)
   {
   if(res == 1) frequency +=5;
   if(res == 2) frequency -=5;
   if(res == 11) frequency +=50;
   if(res == 12) frequency -=50;
   if(res == 10) vfoab_button();
   if(res == 20) mrvfo_button();
   }
   else if (mr_mode)
   {
   if(res == 1)
   {
   if(mem_channel < 14) ++mem_channel;
   while(mic_fast && !mic_up){}
   frequency = loadram(mem_channel + 2);
   }
   if(res == 2)
   {
   if(mem_channel > 0) --mem_channel;
   while(mic_fast && !mic_dn){}
   frequency = loadram(mem_channel + 2);
   }
   if(res == 10) mrvfo_button();
   
   }
   update = 1;
}
}

#endif

int buttons()
{
   if(pb2)
   {
      static int cycle = 0;
      ++cycle;
      if(cycle < 4) return;
      cycle = 0;
      disp_int = 0;// make sure display interupt low else we could corrupt display
      // pb0 = pb0, pb1 = pb1 pb2=pb2
      // sw_pms pms, sw_500k 500kc no need to scan
      
      k8 = 0;  k4 = 0;  k2 = 0;  k1 = 0;
      int res = 0, count = 0;
      while (pb2){}
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
      
      k1 = 0;
out:
      k8 = 0;  k4 = 0;  k2 = 0;  k1 = 0;
      if(res)
      {
         if(long_press) res+= 10;
         beep();
         
         //do what we need to do
         if       (res == 1) clarifier_button();
         else if  (res == 2) down_button();
         else if  (res == 3) up_button();
         else if  (res == 4) mvfo_button();
         else if  (res == 5) vfoab_button();
         else if  (res == 6) dial_lock_button();
         else if  (res == 7) vfom_button();
         else if  (res == 8) mrvfo_button();
         else if  (res == 9) split_button();
         else if  (res == 11) vfom_swap_button();
         else if  (res == 16) 
         {
            //toggle ports and counters to ensure we can TX
            for (int i = 0; i < 3; i++)
            {
            Q64_4 = 1;
            Q64_5 = 1;
            Q64_6 = 1;
            PORTA = 0;
            PORTB = 0;
            PORTC = 0;
            PORTD = 0;
            delay_ms(50);
            Q64_4 = 0;
            Q64_5 = 0;
            Q64_6 = 0;
            PORTA = 0;
            PORTB = 0;
            PORTC = 0;
            PORTD = 0;
            delay_ms(50);
            }
            #ifdef use_eeprom
            write_eeprom(0xFF, 0xFF);
            #endif
            reset_cpu();
         }
         //trigger display/PLL update 
         update = 1;
         }
      
    }
}

void transmit_check()
{
if(sl)
   {
      if(tx_mode)
      {
         if(!transmitting)
         {
            saveram(active_vfo, frequency);
            if(active_vfo == 0) {active_vfo = 1;}
            else if(active_vfo == 1) {active_vfo = 0;}
            frequency = loadram(active_vfo);
            transmitting = 1;
            update = 1;
         }
      }
      else
      {
         if(transmitting)
         {
            saveram(active_vfo, frequency);
            if(active_vfo == 0) {active_vfo = 1;}
            else if(active_vfo == 1) {active_vfo = 0;}
            frequency = loadram(active_vfo);
            transmitting = 0;
            update = 1;
         }
      }
   }
   
   if(cl)
   {
      if(tx_mode)
      {
         if(!transmitting)
         {
            saveram(21, frequency);
            frequency = loadram(20);
            transmitting = 1;
            update = 1;
         }         
      }
      else
      {
         if(transmitting)
         {
            saveram(20, frequency);
            frequency = loadram(21);
            transmitting = 0;
            update = 1;
         }
      }
   }
}

#ifdef cat_included
void parse_cat_command (char byte1, char byte2, char byte3, char byte4, char byte5)
{

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
int8 byte1_upper = ((byte1 >> 4) & 0xF);
int8 byte1_lower = byte1 & 0xF;
int8 byte2_upper = ((byte2 >> 4) & 0xF);
int8 byte2_lower = byte2 & 0xF;
int8 byte3_upper = ((byte3 >> 4) & 0xF);
int8 byte3_lower = byte3 & 0xF;
int8 byte4_upper = ((byte4 >> 4) & 0xF);
int8 byte4_lower = byte4 & 0xF;

frequency = ((byte4_lower * 1000000) + (byte3_upper * 100000) + (byte3_lower * 10000) + (byte2_upper * 1000) + (byte2_lower * 100) + (byte1_upper * 10) + byte1_lower);
}

else if (byte5 == 0xFF) reset_cpu();
update = 1;
}


#endif
//here we set our dial timer ( or triggers). As the timer decreases, it will pass these threasholds
//these dial timers can be anything lower than main dial timer. 
//set these to whatever you like. eg if you have a main timer of 2000, you could set timer 1 to 300, timer 2 to 250 etc.
//They need to descend because the main timer (dial_timer) descends

//basic tug-o-war between positive and negative pulses. Dial timers are in percentages / 10
//screen MUST be refreshed immediately. Set update_display flag ( as below)


#define dial_timer_max 2000 //max spin down-timer
//these are the trigger percentages. As dial_timer decreases, we will approach eg 80% of dialtimer, which is percent_of_d_timer_speed2.
//We can change these percentages so different speeds happen earlier or later. Defaults 80, 60, 40, 20, 10.
#define percent_of_d_timer_speed2 80
#define percent_of_d_timer_speed3 60
#define percent_of_d_timer_speed4 40
#define percent_of_d_timer_speed5 20
#define percent_of_d_timer_speed6 10

#define stop_reset_count 10 //stop spin check sampling count
#define negative_pulse_sample_rate 300 //how often to check for negative pulse to fight against the decrement
#define dial_timer_decrement 10 //overall timer decrement when spinning
#define dial_timer_pullback 15 //dial increment when negative (eg when slowing down spinning)

//dial output counts(the value returned - the MAIN variable for eg frequency
   //COSMETICS:-
   //note the perculiar dial increments... They could just be nice round numbers like 5, 10, 100, 1000 etc...
   //However... Shifting the next-to-last digit lets all digits in the VFD change. If these were nice round numbers, only the digit being changed would appear to change.
   //This is only a cosmetic thing. Feel free to round them up or down as you like
#define default_out_speed 5 //Slowest
#define out_speed2 51      //Slightly faster
#define out_speed3 111     //Faster still
#define out_speed4 1111    //Super-fast
#define out_speed5 11111   //Ultra-fast

int8 dial(int32 &value, flip_direction, acceleration)
{
   static int16 count = 0;
   static int16 stop_count = 0;
   static int16 dial_timer = dial_timer_max;
   //if(reset) { dial_timer = 400; dial_increment = default_increment;}
   static int16 dial_timer1, dial_timer2, dial_timer3, dial_timer4, dial_timer5;
   
   #ifdef dial_accel
   static int16 dial_increment = 1;
   dial_timer1 = (dial_timer_max / 100) * percent_of_d_timer_speed2; //eg 80%
   dial_timer2 = (dial_timer_max / 100) * percent_of_d_timer_speed3;
   dial_timer3 = (dial_timer_max / 100) * percent_of_d_timer_speed4;
   dial_timer4 = (dial_timer_max / 100) * percent_of_d_timer_speed5;
   dial_timer5 = (dial_timer_max / 100) * percent_of_d_timer_speed6;
   #else
   static int16 dial_increment = 5;
   #endif
   int8 res = 0;
   if(!dial_clk)
   {
      tmp = dial_dir;
      while(!dial_clk )
      continue;
      if(tmp)
      {
      if(!flip_direction) value+=dial_increment; else value -=dial_increment;
      }
      else
      {
      if(!flip_direction) value-=dial_increment; else value +=dial_increment;
      }
      stop_count = 0;
      if(acceleration)
      {
      if(dial_timer <dial_timer_pullback) dial_timer = dial_timer_pullback;
      if(dial_timer >= dial_timer_max) dial_timer = dial_timer_max;
      dial_timer -= dial_timer_decrement;
      }
      res = 1;
      //display_driver(0,0,0,0,dial_timer,0);
      update = 1;
   }
   #ifdef dial_accel
   else
   {
   if(acceleration)
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
      if(stop_count >= stop_reset_count) dial_timer = dial_timer_max;
      }
      }
   }
      
   }
   ++count;
   if(count>negative_pulse_sample_rate) count = 0;
   //if (dial_timer < 300) dial_timer +=1;
   
   if(dial_timer > dial_timer1)                                   dial_increment = default_out_speed;
   if((dial_timer <= dial_timer1) && (dial_timer > dial_timer2))  dial_increment = out_speed2;
   if((dial_timer <= dial_timer2) && (dial_timer > dial_timer3))  dial_increment = out_speed3;
   if((dial_timer <= dial_timer3) && (dial_timer > dial_timer4))  dial_increment = out_speed4;
   if((dial_timer <= dial_timer4) && (dial_timer > dial_timer5))  dial_increment = out_speed5;
  #endif
}


void  PLL_REF()
{
 //1500
 PORTB = 0b01100101; Q64_6 = 1; delay_us(10); Q64_6 = 0;//Latch2
 PORTB = 0b01011101; Q64_6 = 1; delay_us(10); Q64_6 = 0;//Latch1 
 PORTB = 0b01001100; Q64_6 = 1; delay_us(10); Q64_6 = 0;//Latch0

 //30
 PORTB = 0b01100000; CPU_36_BIT128 = 1; delay_us(10); CPU_36_BIT128 = 0;//Latch2
 PORTB = 0b01010001; CPU_36_BIT128 = 1; delay_us(10); CPU_36_BIT128 = 0;//Latch1 
 PORTB = 0b01001110; CPU_36_BIT128 = 1; delay_us(10); CPU_36_BIT128 = 0;//Latch0
}


void main()
{
   //We aren't in fastIO mode so do we need this???
   setup_adc(ADC_OFF);
   set_tris_a(0b00001);
   set_tris_b(0b00000000);
   set_tris_c(0b11111111);
   set_tris_d(0b11111110);
   set_tris_e(0b000);
   PLL_REF();
   
   //clear screen
   display_driver (0xFF, 0xFF, 0x7FFFFFFF, 0xFF); delay_ms(500);
   
   //welcome beep
   beep();
   
   //are we using eeprom or RAM for storage?
   //set ram store to some defaults
   #ifdef use_eeprom
   if(read_eeprom(0xFF) == 0xFF)
   {      
      for (int i = 0; i < 20; i++)
      {saveram(i, 700000);}
      write_eeprom(0xFF, 0);
      write_eeprom(0xFE, 0);
      write_eeprom(0xFD, 1);
      write_eeprom(0xFC, 0);
      write_eeprom(0xFB, 0);
   }
   #else
   for (int i = 0; i < 20; i++)
      {saveram(i, 700000);}
   #endif
   
   
   band = 3;
   #ifdef use_eeprom
   active_vfo = read_eeprom(0xFE);
   vfo_mode = read_eeprom(0xFD);
   mr_mode = read_eeprom(0xFC);
   mem_channel = read_eeprom(0xFB);
   frequency = loadram(active_vfo);
   #else
   vfo_mode = 1; mr_mode = 0;
   active_vfo = 0;
   mem_channel = 0;
   frequency = band_bank[band];
   #endif
   update = 1;
   dl = 0; cl = 0; sl = 0;
   
   int32 count = 0;
   #ifdef cat_included
   disable_interrupts(int_rda);
   enable_interrupts(int_rda); //toggle interrupts to ensure serial is ready
   enable_interrupts(global); //enable interrupts for CAT
   #endif
   
   while(true)
   {
   #ifdef cat_included
   if(timerstart){
   ++ticktimer;
   delay_us(10);
   }
   if(Got_Data){
         Got_Data = 0;
         
         //print_value(buffer[5], 500); delay_ms(500);
         parse_cat_command (buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
   }
  
   #endif
   buttons();
   #ifdef mic_buttons
   mic_button();
   #endif
   dial(frequency,0,1);//change frequency value, normal direction, acceleration on
   transmit_check();
   //impose hard limits prior to tuning and sending to display
   if(frequency < min_freq) frequency = max_freq;
   if(frequency > max_freq) frequency = min_freq;
   
   ++count;
   
   if(count == 5000)
   {
   if(vfo_mode && !update)
   {
   saveram(active_vfo, frequency);
#ifdef use_eeprom
   if(read_eeprom(0xFE) != active_vfo) write_eeprom(0xFE, active_vfo);
   if(read_eeprom(0xFD) != vfo_mode) write_eeprom(0xFD, vfo_mode);
   if(read_eeprom(0xFC) != mr_mode) write_eeprom(0xFC, mr_mode);
   if(read_eeprom(0xFB) != mem_channel) write_eeprom(0xFB, mem_channel);
#endif
   }
   
   count = 0;
   }
   //we need to check if something elsewhere (eg dial or button) has triggered an update
   //dont refresh all the time as dial will slow down
   if(update) 
      {
      update_screen(active_vfo,dl,cl,sl,(frequency/10), mem_channel);
      update_PLL((frequency + 70)); //70 is a small offset personal to my radio
      update = 0;
      count = 0;
      }
   
    
   }
}
