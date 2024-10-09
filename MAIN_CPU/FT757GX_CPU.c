//Yaesu CPU firmware. (c)2024 Matthew Bostock

//Thanks to VK2TRP for the original PLL / LO diagrams. Saved me a ton of work!

#include <18F452.h>
#include <stdint.h>

//uncomment for bare minimum size. No mic buttons, dial acceleration, CB or CAT. I advise a bigger PIC!
//#define small_rom 

//remove bits and pieces if you wish, or problem is suspected
#ifndef small_rom
#define master_offset
#define cb_included
#define cat_included
#define mic_buttons
#define dial_accel
#endif

#ifdef dial_accel

//how long the dial must spin before it accelerated the counting. Master timer is overall timer.
//Sensitivity is divided by 6 by master timer, default they are the same. eg so if both are 300, dial sensitivity will
//be divided by 6. == 300 / 6.
//Sensitivity cannot be higher than timer
//A really low sensitivity will mean longer spins before anything is accelerated
#define dial_sensitivity 300
#define dial_master_timer 300
#endif

#define min_freq 50000
#define max_freq 3199990

//added 2.7 brownout as parasitic power from serial will keep PIC running
#fuses HS,PUT, NOWDT,NOPROTECT,NOLVP, BORV27
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


int BITSA,BITSB,BITSC;
# bit   tmp_pin4=BITSA.0
# bit   tmp_pin5=BITSA.1 
# bit   tmp_pin6=BITSA.2 
# bit   tmp=BITSA.3
# bit   txi=BITSA.4 
# bit   gen_tx=BITSA.5 
# bit   dl=BITSA.6 
# bit   brake=BITSA.7 
# bit   active_vfo=BITSB.0 
# bit   auto_update_VFD=BITSB.1 
# bit   freq_to_PLL=BITSB.2 
# bit   sl=BITSB.3 
# bit   cl=BITSB.4 
# bit   inactive_vfo=BITSB.5 
# bit   btn_down=BITSB.6 
# bit   cnt_toggle=BITSB.7 
# bit   dltmp=BITSC.0
# bit   tmp_vfo=BITSC.1
# bit   stored_vfo=BITSC.2
# bit   flip_direction=BITSC.3
# bit   dir=BITSC.4
# bit   negative_value = BITSC.5
uint32_t frequency, temp_offset; 
uint32_t freq_offset;
uint32_t cmhz1, cmhz2, cmhz, c100k, c10k, c1k, c100, c10;
uint32_t dmhz1, dmhz2, dmhz, d100k, d10k, d1k, d100, d10;
uint8_t mem_channel, mem_mode, band;
#ifdef cat_included
uint8_t cat_reply[76];
#endif

#ifdef cb_included
uint32_t cb_channel;
uint8_t cb_region;
#endif

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

//eeprom section
void loadf (int base)
{

 cmhz=read_eeprom(base);
 c100k=read_eeprom(base + 1);
 c10k=read_eeprom(base + 2);
 c1k=read_eeprom(base + 3);
 c100=read_eeprom(base + 4);
 c10=read_eeprom(base + 5); 
 frequency = ((c10) + (c100 * 10) + (c1k * 100) + (c10k * 1000) + (c100k * 10000) + (cmhz * 100000));
}

void savef( int base )
{
uint32_t checksum1 = ((read_eeprom(base+5) + (read_eeprom(base+4) * 10) + (read_eeprom(base+3) * 100)
                        + (read_eeprom(base+2) * 1000) + (read_eeprom(base+1) * 10000) + (read_eeprom(base) * 100000)));

if(checksum1 != frequency)
{
 write_eeprom(base,cmhz);
 write_eeprom(base + 1,c100k);
 write_eeprom(base + 2,c10k);
 write_eeprom(base + 3,c1k);
 write_eeprom(base + 4,c100);
 write_eeprom(base + 5,c10);
 //Q64(2);
}
}

void movef(int src, int dst)
{
 int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5;
 tmp0 = read_eeprom(src);
 tmp1 = read_eeprom(src + 1);
 tmp2 = read_eeprom(src + 2);
 tmp3 = read_eeprom(src + 3);
 tmp4 = read_eeprom(src + 4);
 tmp5 = read_eeprom(src + 5);
 write_eeprom(dst,tmp0);
 write_eeprom(dst + 1,tmp1);
 write_eeprom(dst + 2,tmp2);
 write_eeprom(dst + 3,tmp3);
 write_eeprom(dst + 4,tmp4);
 write_eeprom(dst + 5,tmp5);
 write_eeprom(src,0xFF);
 write_eeprom(src + 1,0xFF);
 write_eeprom(src + 2,0xFF);
 write_eeprom(src + 3,0xFF);
 write_eeprom(src + 4,0xFF);
 write_eeprom(src + 5,0xFF);
}

void swapf(int src, int dst)
{
 int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5;
 int tmp_e = 0xE0;
 tmp0 = read_eeprom(dst);
 tmp1 = read_eeprom(dst + 1);
 tmp2 = read_eeprom(dst + 2);
 tmp3 = read_eeprom(dst + 3);
 tmp4 = read_eeprom(dst + 4);
 tmp5 = read_eeprom(dst + 5);
 write_eeprom(tmp_e,tmp0);
 write_eeprom(tmp_e + 1,tmp1);
 write_eeprom(tmp_e + 2,tmp2);
 write_eeprom(tmp_e + 3,tmp3);
 write_eeprom(tmp_e + 4,tmp4);
 write_eeprom(tmp_e + 5,tmp5);
 write_eeprom(dst,(read_eeprom(src)));
 write_eeprom(dst + 1,(read_eeprom(src + 1)));
 write_eeprom(dst + 2,(read_eeprom(src + 2)));
 write_eeprom(dst + 3,(read_eeprom(src + 3)));
 write_eeprom(dst + 4,(read_eeprom(src + 4)));
 write_eeprom(dst + 5,(read_eeprom(src + 5)));
 write_eeprom(src, (read_eeprom(tmp_e)));
 write_eeprom(src + 1, (read_eeprom(tmp_e + 1)));
 write_eeprom(src + 2, (read_eeprom(tmp_e + 2)));
 write_eeprom(src + 3, (read_eeprom(tmp_e + 3)));
 write_eeprom(src + 4, (read_eeprom(tmp_e + 4)));
 write_eeprom(src + 5, (read_eeprom(tmp_e + 5)));
 write_eeprom(tmp_e,0);
 write_eeprom(tmp_e + 1,0);
 write_eeprom(tmp_e + 2,0);
 write_eeprom(tmp_e + 3,0);
 write_eeprom(tmp_e + 4,0);
 write_eeprom(tmp_e + 5,0);
 
}

void  display_send(int display_byte)
{
int res = 0;
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
 res = 1;
}


void VFD_send(uint8_t g1, uint8_t g2, uint8_t g3, uint8_t g4, uint8_t g5, uint8_t g6, uint8_t g7, uint8_t g8, uint8_t g9)
{
   
      
      while(pb2){} //wait for high PB2
      while(!pb2){} //and low again - then we send.
      display_send(10);   // preamble
      display_send(0);    // preamble
      display_send(15);   // preamble
      display_send(15);   // preamble
      display_send(g1);   // VFO
      display_send(g2);  // XTRA
      display_send(g3);   // D1
      display_send(g4);   // D2
      display_send(g5);   // D3
      display_send(g6);   // D4
      display_send(g7);   // D5
      display_send(g8);    //?
      display_send(g9);   // CH???
}

void update_display_mode01() //display update for VFO/MR mode
{
      uint8_t d1,d2,d3,d4,d5,d6,d7,d8,d9;
      
      if(mem_mode == 0)
      {
      d9 = 15;
      if (active_vfo == 0) d1 = 1;
      else if (active_vfo == 1) d1 = 12;
      }
      
      d2 = 15; //default
      
      //simple truth table to find out whats active for grid(digit 2)
            if( dl && !sl && !cl) d2 = 1;  //1 0 0
      else  if(!dl && !sl &&  cl) d2 = 2;  //0 0 1
      else  if( dl && !sl &&  cl) d2 = 3;  //1 0 1
      else  if( dl &&  sl &&  cl) d2 = 4;  //1 1 1
      else  if( dl &&  sl && !cl) d2 = 7;  //1 1 0
      else  if(!dl &&  sl && !cl) d2 = 12; //0 1 0
      else  if(!dl &&  sl &&  cl) d2 = 14; //0 1 1
       
      d3 = cmhz1; d4 = cmhz2; d5 = c100k; d6 = c10k; d7 = c1k; d8 = c100;
      
      if(d3 == 0) d3 = 15;
      if((d3 == 0) && (d4 == 0)) {d3 = 15; d4 = 15;}
      
      
      if(mem_mode == 1){d1 = 15; d9 = mem_channel;}
      VFD_send(d1,d2,d3,d4,d5,d6,d7,d8,d9);
      
}

void tmp_display_mode01(uint8_t tmp_vfo) //momentary display
{
      uint8_t d1,d2,d3,d4,d5,d6,d7,d8,d9;
      
      if(mem_mode == 0)
      {
      d9 = 15;
      if (tmp_vfo == 0) d1 = 1;
      else if (tmp_vfo == 1) d1 = 12;
      }
      
      d2 = 15; //default
      
            if( dl && !sl && !cl) d2 = 1;  //1 0 0
      else  if(!dl && !sl &&  cl) d2 = 2;  //0 0 1
      else  if( dl && !sl &&  cl) d2 = 3;  //1 0 1
      else  if( dl &&  sl &&  cl) d2 = 4;  //1 1 1
      else  if( dl &&  sl && !cl) d2 = 7;  //1 1 0
      else  if(!dl &&  sl && !cl) d2 = 12; //0 1 0
      else  if(!dl &&  sl &&  cl) d2 = 14; //0 1 1
       
      d3 = cmhz1; d4 = cmhz2; d5 = c100k; d6 = c10k; d7 = c1k; d8 = c100;
      
      if(d3 == 0) d3 = 15;
      if((d3 == 0) && (d4 == 0)) {d3 = 15; d4 = 15;}
      
      
      if(mem_mode == 1){d1 = 15; d9 = mem_channel;}
      VFD_send(d1,d2,d3,d4,d5,d6,d7,d8,d9);
      
}

#ifdef cb_included
void update_display_mode2()
{
      uint8_t d1 = 15,d2 = 15,d3 = 0,d4 = 0,d5 = 15,d6 = 15,d7 = 15,d8 = 15,d9 = 15;
      uint8_t tmp_mhz = cb_channel;
      //split Mhz into individual sections for each VFD grid
      while(tmp_mhz > 9){tmp_mhz -= 10; d3+=1;}
      while(tmp_mhz > 0){tmp_mhz -= 1; d4+=1;}
      
      VFD_send(d1,d2,d3,d4,d5,d6,d7,d8,d9);
      
}
#endif

//PLL update is fully portable fill in frequency and the output variables (for a display)
//if just want to tune 1 segment eg mhz, set freq_to_PLL to 0
//we actually have two "tuners" in this PLL update code. One for the display, one for the radio frequency.
//(C)cmhz etc is for the display, (D)dmhz etc is for the PLL. If master offset is disabled, the C values are copied to D
int update_pll(auto_update_VFD, freq_to_PLL)
{
start:

dltmp = dl;
set_dl(1);
Q64_4 = 0;
Q64_5 = 0;
Q64_6 = 0;

int res = 0;

//split main frequency into mhz, 100k, 10k, 1k, 100, 10, 1 hz
//Its 32 bit number so we cannot just bitshift it, so we have to cascade it
if(freq_to_PLL)
{
uint32_t tmp_frequency = frequency;
cmhz = 0;while(tmp_frequency > 99999){tmp_frequency -= 100000; cmhz+=1;}
c100k = 0;while(tmp_frequency > 9999){tmp_frequency -= 10000; c100k+=1;}
c10k = 0;while(tmp_frequency > 999){tmp_frequency -= 1000; c10k+=1;}
c1k = 0;while(tmp_frequency > 99){tmp_frequency -= 100; c1k+=1;}
c100 = 0;while(tmp_frequency > 9){tmp_frequency -= 10; c100+=1;}
c10 = 0;while(tmp_frequency > 0){tmp_frequency -= 1; c10+=1;}
}

else frequency = ((c10) + (c100 * 10) + (c1k * 100) + (c10k * 1000) + (c100k * 10000) + (cmhz * 100000));

//set some hard limits
   if(frequency < min_freq) {frequency = max_freq; brake = 1;}
   if(frequency > max_freq) {frequency = min_freq; brake = 1;}

     //split Mhz into individual sections for each VFD grid
uint8_t tmp_mhz = cmhz;
cmhz1 = 0;while(tmp_mhz > 9){tmp_mhz -= 10; cmhz1+=1;}
cmhz2 = 0;while(tmp_mhz > 0){tmp_mhz -= 1; cmhz2+=1;}

#ifdef master_offset
uint32_t tmp_frequency;
if(negative_value) tmp_frequency = frequency + freq_offset;// - 5000 ;
else tmp_frequency = frequency + freq_offset;
dmhz = 0;while(tmp_frequency > 99999){tmp_frequency -= 100000; dmhz+=1;}
d100k = 0;while(tmp_frequency > 9999){tmp_frequency -= 10000; d100k+=1;}
d10k = 0;while(tmp_frequency > 999){tmp_frequency -= 1000; d10k+=1;}
d1k = 0;while(tmp_frequency > 99){tmp_frequency -= 100; d1k+=1;}
d100 = 0;while(tmp_frequency > 9){tmp_frequency -= 10; d100+=1;}
d10 = 0;while(tmp_frequency > 0){tmp_frequency -= 1; d10+=1;}


tmp_mhz = dmhz;
dmhz1 = 0;while(tmp_mhz > 9){tmp_mhz -= 10; dmhz1+=1;}
dmhz2 = 0;while(tmp_mhz > 0){tmp_mhz -= 1; dmhz2+=1;}
#else
dmhz1 = cmhz1; dmhz2 = cmhz2; dmhz = cmhz; d100k = c100k; d10k = c10k; d1k = c1k; d100 = c100; d10 = c10;
#endif

//!   putc (dmhz);
//!   putc (d100k);
//!   putc (d10k);
//!   putc (d1k);
//!   putc (d100);
//!   putc (d10);

//LPF preselector. If this is not correct, you WILL be on the wrong band and PLL probably won't lock

//sum of MHz + 100khz. eg CB band 26,965 would be 269, which is PLL band 9
uint32_t tmp_band_freq = ((dmhz*10) + d100k);
uint8_t PLLband = 0;
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

uint32_t PLL1_NCODE_L3 = 48; //empty latch
uint32_t PLL1_NCODE_L2 = 0;
uint32_t PLL1_NCODE_L1 = 0;
uint32_t PLL1_NCODE_L0 = 0;

uint32_t PLL2_NCODE_L3 = 48; //empty latch
uint32_t PLL2_NCODE_L2 = 32; //empty latch
uint32_t PLL2_NCODE_L1 = 0;
uint32_t PLL2_NCODE_L0 = 0;

uint32_t tmp_d100k;
uint32_t tmp_dmhz;
tmp_d100k = d100k;
tmp_dmhz = dmhz1;
   
uint32_t tmp_khz_freq = ((tmp_d100k * 100) + (d10k * 10) + (d1k));//sum of 100k + 10k + 1k eg 27781.25 = 781

if(tmp_khz_freq >= 500) tmp_khz_freq -=500; //if over 500 khz, take off 500 because we will calculate PLL2 to allow for the 500.

//add on our 560 (start of N code ref is 560, so will count on from that. eg 0 = 560, 1 = 561 etc etc
tmp_khz_freq +=560;

// tmp_khz_freq - this is final NCODE for PLL1. eg 27.78125 = 781... - 500 = 281...281 + 560 = 841 in decimal.
//841 in hex is 349. So latch2 = 3, latch 1 = 4, latch 0 = 9;
PLL1_NCODE_L2 = (tmp_khz_freq >> 8); //first digit
PLL1_NCODE_L1 = ((tmp_khz_freq >> 4) & 0xF);//middle digit
PLL1_NCODE_L0 = (tmp_khz_freq & 0xF);//final digit


//PLL1 end

//PLL2. Do we need to tune for BandA or BandB? Band A = 0.5 - 14.4999... Band B = 14.5 = 30+

//eg 27.78125 we need mhz + 100k... Make a sum... So 27 * 10 = 270... Add our 7 = 277
//We have gone to nearest 5 - 275... Is band B so 275 / 5 = 55. 55+12 = 77... 77 - 30 = 37. Our NCODE is 37 37 >> 4 = 3... 37 & F = 7.
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
   PORTB = 112 + (d10);
   delay_us(10);
   Q64_5 = 1;
   delay_us(10);
   Q64_5 = 0;
   
   //100hz just stays on the port
   PORTB = 112 + (d100);
   
   //shush the Q64 CPU lines
   Q64_4 = 0;
   Q64_5 = 0;
   Q64_6 = 0;
   
   set_dl(dltmp);
   dl = dltmp;
   
   //is auto update active?
   if(auto_update_VFD) update_display_mode01();

   res = 1;
   
   return res;
}

//this emulates a CB radio (infact a double-banded CB radio). Meant for AM/FM use
//TODO; 80 ch mode if people want it
//Region 0 is UK, region 1 is CEPT / FCC
#ifdef cb_included
int update_cb_channel( uint32_t channel, int region)
{

if(channel < 2) cb_channel = 1;
if(channel > 39) cb_channel = 40;
int res=0;
int tmp_c10k;
if(region == 0) //uk
{
cmhz = 27;
if(channel <= 10) c100k = 6;
else if ((channel > 10) && (channel <= 20)) c100k = 7;
else if ((channel > 20) && (channel <= 30)) c100k = 8;
else if ((channel > 30) && (channel <= 40)) c100k = 9;

if(channel <= 10) tmp_c10k = channel - 1;
else if ((channel > 10) && (channel <= 20)) tmp_c10k = (channel - 1) - 10;
else if ((channel > 20) && (channel <= 30)) tmp_c10k = (channel - 1) - 20;
else if ((channel > 30) && (channel <= 40)) tmp_c10k = (channel - 1) - 30;
c10k = tmp_c10k;

c1k = 1; c100 = 2; c10 = 5;
savef(0);
write_eeprom(0xC3, channel);
update_pll(0,0);
update_display_mode2();
}
if(region == 1)
{
if(channel <= 3) {cmhz = 26; c100k = 9;}
else if ((channel > 3) && (channel <= 11)) {cmhz = 27; c100k = 0;}
else if ((channel > 11) && (channel <= 19)) {cmhz = 27; c100k = 1;}
else if ((channel > 19) && (channel <= 29)) {cmhz = 27; c100k = 2;}
else if ((channel > 29) && (channel <= 39)) {cmhz = 27; c100k = 3;}
else if ((channel == 40)) {cmhz = 27; c100k = 4;}

if(channel == 1) c10k = 6; 
else if(channel == 2) c10k = 7; 
else if(channel == 3) c10k = 8;
else if((channel > 3) && (channel <= 7)) c10k = channel - 4;
else if((channel > 7) && (channel <= 11)) c10k = channel - 3;
else if((channel > 11) && (channel <= 15)) c10k = channel - 12;
else if((channel > 15) && (channel <= 19)) c10k = channel - 11;
else if((channel > 19) && (channel <= 22)) c10k = channel - 20;
else if(channel == 23) c10k = 5;
else if(channel == 24) c10k = 3;
else if(channel == 25) c10k = 4;
else if((channel > 25) && (channel <= 29)) c10k = channel - 20;
else if((channel > 29) && (channel <= 39)) c10k = channel - 30;
else if ((channel == 40)) c10k = channel - 40;
c1k = 5; c100 = 0; c10 = 0;
savef(0);
write_eeprom(0xC3, channel);
update_pll(0,0);
update_display_mode2();
}
res = 1;
return res;
}
#endif

void down_btn_parse()
{

if(mem_mode == 0)
   {
   if (sw_500k) frequency -=50000;
   else
   {
   if (band < 1) band = 0;
   else band -=1;
   write_eeprom(0xC4, band);
   loadf((band * 8) + 32);
   savef(active_vfo*8);
   }
   update_pll(1,1);
   }
else 
if (mem_mode == 1)
   {
   if (mem_channel > 0 ) mem_channel -= 1;
   else mem_channel = 0;
   write_eeprom(0xC2, mem_channel);
   loadf((mem_channel * 8) + 0x70);
   update_pll(1,1);
   }
#ifdef cb_included
else
if(mem_mode == 2)
   {
   if(cb_channel <2) cb_channel = 1; else cb_channel -=1;
   update_cb_channel(cb_channel, cb_region);
   }
#endif
}
  
void up_btn_parse()
  {
   if(mem_mode == 0)
      {
      if (sw_500k) frequency +=50000;
      else
      {
      if(band > 8) band = 9;
      else band +=1;
      write_eeprom(0xC4, band);
      loadf((band * 8) + 32);
      savef(active_vfo*8);
      }
      update_pll(1,1);
      }
   else 
   if (mem_mode == 1)
      {
      if (mem_channel < 9 ) mem_channel += 1;
      else mem_channel = 9;
      write_eeprom(0xC2, mem_channel);
      loadf((mem_channel * 8) + 0x70);
      update_pll(1,1);
      }
#ifdef cb_included
   else
   if(mem_mode == 2)
      {
      if(cb_channel > 39) cb_channel = 40; else cb_channel +=1;
      update_cb_channel(cb_channel, cb_region);
      }
#endif
  }

//quick routine to cancel split and clarifier if a different / incompatible mode is selected
void cancel_cs()
{
   if(cl)
   {
   cl = 0;
   write_eeprom (0xC6, 0); //CL Active (0 off, 1 on)
   }
   if(sl)
   sl = 0;
   write_eeprom (0xC7, 0); //Spl Active (0 off, 1 on)
}
   
   
   
void toggle_mvfo()
{
loadf((mem_channel * 8) + 112);
savef(active_vfo * 8);
mem_mode = 0;
write_eeprom(0xC0, mem_mode);
update_pll(1,1);
}

void toggle_vfom()
{
loadf(active_vfo * 8);
savef((mem_channel * 8) + 112);
mem_mode = 1;
write_eeprom(0xC0, mem_mode);
update_pll(1,1);
}

void toggle_vfom_swap()
{
swapf(active_vfo * 8, (mem_channel * 8) + 112);
loadf(active_vfo * 8);
//if(mem_mode == 0) mem_mode = 1;
//if(mem_mode == 1) mem_mode = 0;
//write_eeprom(0xC0, mem_mode);
update_pll(1,1);
}

void toggle_mrvfo(int override)
{
cancel_cs();
if(!override)
   {
#ifdef cb_included
      if(mem_mode == 2) 
      {
      VFD_send(15,15,cmhz1,cmhz2,c100k,c10k,c1k,c100,15);
      delay_ms(500);
      update_display_mode2();
      return;
      }
      else
#endif
      if (mem_mode == 1)
      {
      write_eeprom(0xC2, mem_channel);
      mem_mode = 0;
      write_eeprom(0xC0, mem_mode);
      active_vfo = read_eeprom(0xC1);
      if(active_vfo == 0) inactive_vfo = 1;
      if(active_vfo == 1) inactive_vfo = 0;
      loadf(active_vfo * 8);
      update_pll(1,1);
      }
      else
      if(mem_mode == 0)
      {
      savef(active_vfo * 8);
      write_eeprom(0xC1, active_vfo);
      mem_mode = 1;
      write_eeprom(0xC0, mem_mode);
      mem_channel = read_eeprom(0xC2);
      loadf((mem_channel * 8) + 0x70);
      update_pll(1,1);
      }
   }
#ifdef cb_included
   else
   {
      if(mem_mode !=2)
      {
      movef (0, 0xE0);
      movef(8, 0xE8);//we arent in CB
         write_eeprom(0xC9, mem_mode);  //store last used MR/VFO to eep for safekeeping
         mem_mode = 2;
         write_eeprom(0xC0, mem_mode);

         cb_channel = read_eeprom(0xc3);
         cb_region = read_eeprom(0xC8);
         update_cb_channel(cb_channel, cb_region);

      }
      else 
      if(mem_mode == 2)
      {
      movef(0xE0, 0);
      movef(0xE8, 8);
      active_vfo = read_eeprom(0xC1);
      if(active_vfo == 0) inactive_vfo = 1;
      if(active_vfo == 1) inactive_vfo = 0;
      loadf(active_vfo * 8);
      mem_mode = read_eeprom(0xC9);
      write_eeprom(0xC0, mem_mode);
      if(mem_mode == 1)
      {
      mem_channel = read_eeprom(0xC2);
      loadf((mem_channel * 8) + 0x70);
      
      }
      update_pll(1,1);
      }
   }
#endif

}

void toggle_vfoab()
{
if ((mem_mode == 1) || (mem_mode == 2)) {errorbeep(); return;}
savef(active_vfo * 8);
if(active_vfo == 0) {active_vfo = 1; inactive_vfo = 0;}
else if(active_vfo == 1) {active_vfo = 0; inactive_vfo = 1;}
loadf(active_vfo * 8);
write_eeprom(0xC1, active_vfo);
update_pll(1,1);

}

void toggle_dial_lock()
{
if (dl) set_dl(0); else set_dl(1);
write_eeprom (0xc5, dl);
update_display_mode01();
}

void toggle_clarifier()
{
if (!sl)
{
if(cl) cl = 0; else
{
savef(0xE8);
cl = 1;
}
write_eeprom (0xc6, cl);
update_display_mode01();
} else errorbeep();
}

void toggle_split()
{
if (!cl)
{
if(sl) sl = 0; else sl = 1;
write_eeprom (0xc7, sl);
update_display_mode01();
} else errorbeep();
}

#ifdef cb_included
void toggle_cb_region()
{
if(mem_mode !=2) {errorbeep(); return;}
cb_region = read_eeprom(0xC8);
if (cb_region == 0)
{
cb_region = 1;
VFD_send(15, 15, 10, 14, 15, 15, 15, 15, 15); delay_ms(500);
}
else
{
cb_region = 0;
VFD_send(15, 15, 15, 11, 15, 15, 15, 15, 15); delay_ms(500);
}
//if (cb_region == 0) print_value ( 700044, 500); delay_ms(1000);
//if (cb_region == 1) print_value ( 700045, 500); delay_ms(1000);
write_eeprom (0xc8, cb_region);
update_cb_channel(cb_channel, cb_region);
}

#endif


//Generic dial function. Value is output, increment is the increment, flip_direction is to change to counter-clockwise increase
//which we use in master offset setup
int get_dial(uint32_t &value, uint16_t increment, flip_direction)
{
int res = 0;
if(!dial_clk)
   {
   tmp = dial_dir;
   while(!dial_clk )
   continue;
   if(tmp)
   {
   if(!flip_direction) value+=increment; else value -=increment;
   }
   else
   {
   if(!flip_direction) value-=increment; else value +=increment;
   }
   res = 1;
   }
   return res;
}

//If button on front panel isn't active, beep if out of band
int freq_blacklist()
{
if(
(((cmhz >= 1) &&  (c100k >= 5)) && (cmhz < 2)) ||
(((cmhz >= 3) &&  (c100k >= 5)) && (cmhz < 4)) ||
((cmhz >= 7) && ((cmhz <= 7) &&  (c100k < 5))) ||
((cmhz >= 10) && ((cmhz <= 10) &&  (c100k < 5))) ||
((cmhz >= 14) && ((cmhz <= 14) &&  (c100k < 5))) ||
((cmhz >= 18) && ((cmhz <= 18) &&  (c100k < 5))) ||
((cmhz >= 21) && ((cmhz <= 21) &&  (c100k < 5))) ||
(((cmhz >= 24) &&  (c100k >= 5)) && (cmhz < 25)) ||
(((cmhz >= 28) &&  (cmhz < 30)))
) return 0;
else return 1;
}


//split == rx and tx different vfos
//clar == transmit on first frequency, tune anywhere for rx, tx on first frequency

//deal with clar and split when TX active and update display
void txrx()
{
stored_vfo = active_vfo; //save for later
if(tx_mode)
   {
   if(sl) // split
   {
      if(stored_vfo == 0) tmp_vfo = 1;
      else if (stored_vfo == 1) tmp_vfo = 0;
      if(tx_mode)
      {
      //set_tx_inhibit(1); //inhibit tx while loading temp vfo
      loadf(tmp_vfo * 8);
      update_pll(0,0);
      tmp_display_mode01(tmp_vfo);
      //set_tx_inhibit(0); //release inhibit
      while(tx_mode){}
      loadf(stored_vfo * 8);
      update_pll(1,0);
      }
   }
   if(cl)
   {
      if(tx_mode)
      {
      loadf(0xE8);
      update_pll(1,0);
      while(tx_mode);
      loadf(stored_vfo * 8);
      update_pll(1,0);
      }
   
   }
   }
}

//mic buttons
void mic_buttonaction(uint8_t res)
{
#ifdef mic_buttons
if(mem_mode == 0)
{
if(res == 1) {frequency += 1; update_pll(1,1);}
if(res == 2) {frequency -= 1; update_pll(1,1);}
if(res == 11) {frequency += 100; update_pll(1,1);}
if(res == 12) {frequency -= 100; update_pll(1,1);}
if(res == 10) toggle_vfoab();
}
else
if((mem_mode == 1) || (mem_mode == 2))
{
if(res == 1) {up_btn_parse(); while(!mic_up){}}
if(res == 2) {down_btn_parse(); while(!mic_dn){}}
//if(res == 12) {frequency -= 100;}
}
#ifdef cb_included
if(mem_mode == 2)
{
if(res == 10) toggle_cb_region();
}
#endif
if(res == 20)
{
if(mem_mode == 0 || mem_mode == 1) toggle_mrvfo(0);
else if(mem_mode == 2) toggle_mrvfo(1);
}
if(res == 21) toggle_mrvfo(1);
#endif
}

int get_micbuttons()
{
int res = 0;
#ifdef mic_buttons
static int mic_counter = 0;
if( !mic_fast && !mic_up) res = 1;
else if( !mic_fast && !mic_dn) res = 2;
else if(mic_fast && mic_up && mic_dn)
   {
      while(mic_fast)
      {
      delay_ms(500);
      mic_counter +=1;
      if(mic_counter == 6)
      {
      errorbeep();
      res = 20;
      }
      else if(mic_counter >= 10)
      {
      res = 21;
      break;
      }
      else res = 10;
      }
      mic_counter = 0;
   }
else if(mic_fast && !mic_up) res = 11;
else if(mic_fast && !mic_dn) res = 12;
#endif
return res;

}


//if radio is slightly out of tune, you can make a cosmetic adjustment here.
//Use a known on-frequency radio or freq counter
//Use up and down band buttons to change up and down frequencies. If wideband switch isn't active
//(radio not widebanded), it will beep.
#ifdef master_offset
void set_master_offset()
{
   frequency = 700000;
   uint32_t offset = 0;
   write_eeprom (0xD1, 0); //master offset byte2
   write_eeprom (0xD2, 0); //master offset byte3
   write_eeprom (0xD3, 1); //is offset negative? 1 = yes, 0 = no
   uint32_t tmp_offset;
   uint32_t dir_offset = 0;
   uint8_t d1,d2,d3;
   dir = 0;
   int res = 0;
    //clear screen
   VFD_send(15, 15, 15, 15, 15, 15, 15, 15, 15); delay_ms(500);
   //VFD_send(15, 15, 15, 15, 15, 5, 0, 0, 15);
   //loop this until value chosen
   while(true)
   {
      dir_offset = offset;
      res = get_dial(offset, 1,dir);
      if(res)
      {
         tmp_offset = offset;
         if(offset > 998) offset = 1;
         if(offset < 1) {
         if(dir == 1) dir = 0; 
         else dir = 1;
         offset = 1;}
         d1 = 0;while(tmp_offset > 99){tmp_offset -= 100; d1+=1;}
         d2 = 0;while(tmp_offset > 9){tmp_offset -= 10; d2+=1;}
         d3 = 0;while(tmp_offset > 0){tmp_offset -= 1; d3+=1;}
         if (dir == 0) {VFD_send(15, 15, 15, 15, 15, d1, d2, d3, 15); negative_value = 0;}
         if (dir == 1) {VFD_send(15, 15, d1, d2, d3, 15, 15, 15, 15); negative_value = 1;}
         res = 0;
      }
  
        if (pb2)
        {
      while (pb2)
      continue;    
       
        if (btn_down){ k2 = 1; k4 = 1; k8 = 1; if (!pb2 && !pb1 && !pb0) btn_down = 0;}
            
        k2 = 0; k4 = 1; delay_us(1);
        if (pb2 ){
        btn_down = 1; while(pb2){}
        errorbeep();
        
        write_eeprom (0xD3, negative_value);
        write_eeprom (0xD0, d1); //master offset byte1
        write_eeprom (0xD1, d2); //master offset byte2
        write_eeprom (0xD2, d3); //master offset byte3
        break;
        }
        k4 = 0; k2 = 1; delay_us(1);
        if (pb1){btn_down = 1; while(pb1){} frequency -= 100000;update_pll (1,1);}
        
        k2 = 1; delay_us(1);
        if (pb0){btn_down = 1; while(pb0){} frequency += 100000;update_pll (1,1);}
        }
        
        k2 = 0; k4 = 1; delay_us(1);
        if (pb0 ){
        while(pb0){}
        beep();
        if(dir == 1) dir = 0; else dir = 1;
        offset = 1;
        }
         k4 = 0;
        if(tx_mode)
        {
        freq_offset = (offset);
        update_pll (1,1);
         while(tx_mode){}
         if (dir == 0) {VFD_send(15, 15, 15, 15, 15, d1, d2, d3, 15);  negative_value = 0;}
         if (dir == 1) {VFD_send(15, 15, d1, d2, d3, 15, 15, 15, 15);  negative_value = 1;}
        }
   }
   reset_cpu();
}
#endif

//call functions for button actions. Long-press adds 10 to the res(result)
void buttonaction(uint8_t res)
{
if(res == 1) toggle_clarifier();
else if(res == 2) toggle_mvfo();
else if(res == 3) toggle_vfom();
else if(res == 4) toggle_vfom_swap();
else if(res == 5) down_btn_parse();
else if(res == 6) toggle_vfoab();
else if(res == 7) toggle_mrvfo(0);
else if(res == 8) up_btn_parse();
else if(res == 9) toggle_dial_lock();
else if(res == 10) toggle_split();
#ifdef master_offset
else if(res == 12) set_master_offset();
#endif
else if(res == 16) 
{

errorbeep();
delay_ms(1000);
errorbeep();
delay_ms(1000);
write_eeprom(0xFF, 0xFF);
delay_ms(1000);
reset_cpu();
}
else if (res == 17) toggle_mrvfo(1);
#ifdef cb_included
else if (res == 20) toggle_cb_region();
#endif
//!
//!while(pb2){}
//!return res;
}

//scan buttons by alternating k1=k8 lines and listening on PB lines for response
int get_buttons()
{
disp_int = 0; 
 
 k4 = 0;
 k8 = 0;
 k1 = 0;
 int res = 0;
 int count = 0;
 int long_press = 0;
 if (pb2)
 {
while (pb2)
continue;    
 
  if (btn_down){k2 = 1; k4 = 1; k8 = 1; if (!pb2 && !pb1 && !pb0) btn_down = 0;}

  if (! btn_down)
  {
  k2 = 1; delay_us(1);
  if (pb2){while(pb2){++count; delay_ms(100);if(count >=30) {long_press = 1; break;}} btn_down = 1; res = 1; goto out;}
   
  k2 = 0; k4 = 1; delay_us(1);
  if (pb2 ){while(pb2){++count;  delay_ms(100);if(count >=30) {long_press = 1; break;}} btn_down = 1; res = 2; goto out;}
  
  k4 = 0; k8 = 1; delay_us(1);
  if (pb2){while(pb2){++count;  delay_ms(100);if(count >=30) {long_press = 1; break;}} btn_down = 1; res = 3; goto out;}
   
  k8 = 0; k1 = 1; delay_us(1);
  if (pb1){while(pb1){++count;  delay_ms(100);if(count >=30) {long_press = 1; break;}} btn_down = 1; res = 4; goto out;}
  
  k1 = 0; k2 = 1; delay_us(1);
  if (pb1){while(pb1){++count;  delay_ms(100);if(count >=30) {long_press = 1; break;}} btn_down = 1; res = 5; goto out;}
   
  k2 = 0; k4 = 1; delay_us(1);
  if (pb1 ){while(pb1){++count;  delay_ms(100);if(count >=30) {long_press = 1; break;}} btn_down = 1; res = 6; goto out;}
  
  k4 = 0; k8 = 1; delay_us(1);
  if (pb1){while(pb1){++count;  delay_ms(100);if(count >=30) {long_press = 1; break;}} btn_down = 1; res = 7; goto out;}
   
  k8 = 0; k2 = 1; delay_us(1);
  if (pb0){while(pb0){++count; delay_ms(100); if(count >=30) {long_press = 1; break;}} btn_down = 1; res = 8; goto out;}
   
  k2 = 0; k4 = 1; delay_us(1);
  if (pb0 ){while(pb0){++count;  delay_ms(100);if(count >=30) {long_press = 1; break;}} btn_down = 1; res = 9; goto out;}
  
  k4 = 0; k8 = 1; delay_us(1);
  if (pb0){while(pb0){++count;  delay_ms(100);if(count >=30) {long_press = 1; break;}} btn_down = 1; res = 10;goto out;}
  k8 = 0;
  }
}
out:
   if(res)
   {
      if(long_press)
      {
      res+= 10;
      }
   beep();
   return res;
   }
   else return 0;
}



//cat (PC-radio only at the moment) as I don't know how the reply byte sequence is built
#ifdef cat_included
void parse_cat_command (char byte1, char byte2, char byte3, char byte4, char byte5)
{
int return_delay = 0;
//int requested = 76;

if(byte5 == 1) {toggle_split();beep();}
else if(byte5 == 2) {toggle_mrvfo(0);beep();}
else if(byte5 == 3) {toggle_vfom();beep();}
else if (byte5 == 4) {toggle_dial_lock(); beep();}
else if(byte5 == 5) {toggle_vfoab();beep();}
else if(byte5 == 6) {toggle_mvfo();beep();}
else if (byte5 == 7) {frequency +=50000; beep();}
else if (byte5 == 8) {frequency -=50000; beep();}
else if(byte5 == 11) {toggle_vfom_swap();beep();}
else if(byte5 == 12) {toggle_mrvfo(1);beep();}

else if (byte5 == 10){
uint8_t byte1_upper = ((byte1 >> 4) & 0xF);
uint8_t byte1_lower = byte1 & 0xF;
uint8_t byte2_upper = ((byte2 >> 4) & 0xF);
uint8_t byte2_lower = byte2 & 0xF;
uint8_t byte3_upper = ((byte3 >> 4) & 0xF);
uint8_t byte3_lower = byte3 & 0xF;
//uint8_t byte4_upper = ((byte4 >> 4) & 0xF);
uint8_t byte4_lower = byte4 & 0xF;



uint8_t tmp_cmhz = 0;
uint8_t tmp_c100k = 0;
tmp_cmhz = ((byte4_lower) * 10) + byte3_upper;
tmp_c100k = byte3_lower;
putc (tmp_cmhz);
putc(tmp_c100k);
if (((tmp_cmhz <=0) && (tmp_c100k < 5)) || (tmp_cmhz > 31)) beep();
else
{
cmhz = tmp_cmhz;
c100k = tmp_c100k;
c10k = byte2_upper;
c1k = byte2_lower;
c100 = byte1_upper;
c10 = byte1_lower;
}

}
//cat reply(not working. Goes to RC6 or MIC/down)
else if (byte5 == 0x10 && byte4 == 0x00){


for (int i = 0; i < 75; ++i)
{
delay_ms(return_delay);
putc(cat_reply[i]);
}
//memset(cat_reply, '\0', sizeof cat_reply);
}
else if (byte5 == 0x0E) {return_delay = byte4; byte5 = 0x10;}
else if (byte5 == 0xFC) {gen_tx = 1; errorbeep();}
else if (byte5 == 0xFD) {if(gen_tx == 1) toggle_mrvfo(1); else errorbeep();}
else if (byte5 == 0xFE) { write_eeprom(0xFF, 0xFF); reset_cpu();}
else if (byte5 == 0xFF) reset_cpu();
//!//puts("thank-you");
//!

delay_ms (100);
savef(active_vfo * 8);
update_pll(1,0);
//while(!update_display(active_vfo));
}
#endif

 
//EEPROM check byte is stored at 0xFF. If this location is 0xFF, EEPROM is blank
void EEPROM_setup(int write)
{
if(write)
{
cmhz = 7; c100k = 0; c10k = 0; c1k = 0; c100 = 0; c10 = 0; //start freq
//store above to VFO A (0) and B (8)
savef(0) ; savef(8) ; 

//store above to mem channels (0-9)
savef(0x70); savef(0x78); savef(0x80); savef(0x88); savef(0x90);
savef(0x98); savef(0xa0); savef(0xa8); savef(0xb0); savef(0xb8);

//save stock yaesu bands
cmhz = 0; c100k = 5; savef(0x20); // band 0
cmhz = 1; c100k = 8; savef(0x28); // band 1
cmhz = 3; c100k = 5; savef(0x30); // band 2
cmhz = 7; c100k = 0; savef(0x38); // band 3
cmhz = 10; c100k = 1; savef(0x40); // band 4
cmhz = 14; c100k = 0; savef(0x48); // band 5
cmhz = 18; c100k = 0; savef(0x50); // band 6
cmhz = 21; c100k = 0; savef(0x58); // band 7
cmhz = 24; c100k = 0; savef(0x60); // band 8
cmhz = 28; c100k = 0; savef(0x68); // band 9

// global defaults upon fresh flash or EEPROM reset
write_eeprom(0xC0,0); //mode (0 VFO, 1 MR, 2 CB)
write_eeprom(0xC1,0); //Active VFO(0-1)
write_eeprom (0xC2,0); //Mem ch in use (0-7) 
write_eeprom (0xC3,19); //cb ch in use (def 19 lol)
write_eeprom(0xC4,3); //band = 3(70000)
write_eeprom (0xC5,0); //DL Active (0 off, 1 on)
write_eeprom (0xC6, 0); //CL Active (0 off, 1 on)
write_eeprom (0xC7, 0); //Spl Active (0 off, 1 on)
write_eeprom (0xC8,0); // region. 0 = UK, 1 = CEPT/USA, 3 = 80CH (not implemented)
write_eeprom (0xC9,3); //was we in VFO or MR before CB mode? def unset 3
write_eeprom (0xD0, 0); //master offset byte1
write_eeprom (0xD1, 0); //master offset byte2
write_eeprom (0xD2, 0); //master offset byte3
write_eeprom (0xD3, 1); //is offset negative? 1 = yes, 0 = no
write_eeprom (0xFF,0); //eeprom check byte
reset_cpu();
}
else
{
//get active VFO (probably 0)
active_vfo = read_eeprom(0xC1);

//load start freq into current eeprom
loadf(active_vfo * 8);

//load mem channel and mode
mem_channel = read_eeprom(0xC2);
mem_mode = read_eeprom(0xC0);

#ifdef cb_included
//load CB channel and region
cb_channel = read_eeprom(0xC3);
cb_region = read_eeprom(0xC8);
#endif
//load band
band = read_eeprom(0xC4);
//load dial lock, clarifier and split status
dl = read_eeprom(0xC5); cl = read_eeprom(0xC6); sl = read_eeprom(0xC7);

temp_offset = ((read_eeprom(0xD0) * 100) +
              (read_eeprom(0xD1) * 10)  +
              (read_eeprom(0xD2)));
negative_value = read_eeprom(0xD3);
freq_offset = (temp_offset);
}
}

//PLL initial setup divisors
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


//dial acceleration setup
uint8_t tick = 0;
#ifdef dial_accel
uint16_t dial_increment = 1;
#else
uint16_t dial_increment = 10;
#endif
#ifdef dial_accel
uint16_t dial_timer = dial_master_timer;
#endif

//interrupt for CAT serial. Typical int_rda - nothing exciting

#ifdef cat_included
#define BUFFER_SIZE 10
char temp_byte;//create 10 byte large buffer
char buffer[BUFFER_SIZE]; 
char garbage_data;
int data_received = 0;
int next_in = 1;
long ticktimer = 0;
int timerstart = 0;
#INT_RDA
void  RDA_isr(void)
{
timerstart = 1;
buffer[0] = '\0'; //null out first byte as a "start" bit
if((kbhit()) && (ticktimer > 10000)) {

 //{while (next_in >= 1) garbage_data = getc(); --next_in;}
for (int i = 0; i == sizeof buffer; ++i)
{
buffer[i] = '\0';
}
data_received = 0;
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
      data_received = 1;
      next_in = 1;
      timerstart = 0;
   if(next_in > 6)  {while (next_in > 6) garbage_data = getc();}   
      
   }
}

                                           
}// INT_R
#endif



void main()
{
//We aren't in fastIO mode so do we need this???
setup_adc(ADC_OFF);
set_tris_a(0b00001);
set_tris_b(0b00000000);
set_tris_c(0b11111111);
set_tris_d(0b11111110);
set_tris_e(0b000);

//first, check if radio is widebanded
k1 = 1; delay_us(1);
if (pb0) gen_tx = 0;  // //widebanded?
else gen_tx = 1;
k1 = 0;

PLL_REF();

//lets deal with the EEPROM. If EEPROM is blank, fill it with some values. If not blank, load them.
if(read_eeprom(0xFF) == 0xFF) {EEPROM_setup(1);  reset_cpu();} else { EEPROM_setup(0);}

//here we set our dial timer ( or triggers). As the timer decreases, it will pass these threasholds
//these dial timers can be anything lower than main dial timer. These are divided by 6 as an example,
//then multiplied, so we get 5/6ths, 4/6ths etc
//set these to whatever you like. eg if you have a main timer of 600, you could set timer 1 to 300, timer 2 to 250 etc.
//They need to descend because the main timer (dial_timer) descends
#ifdef dial_accel
uint16_t dial_timer1, dial_timer2, dial_timer3, dial_timer4, dial_timer5;
dial_timer1 = (dial_sensitivity / 6) * 5;
dial_timer2 = (dial_sensitivity / 6) * 4;
dial_timer3 = (dial_sensitivity / 6) * 3;
dial_timer4 = (dial_sensitivity / 6) * 2;
dial_timer5 = (dial_sensitivity / 6);
#endif
//welcome

//clear display
VFD_send(15, 15, 15, 15, 15, 15, 15, 15, 15); delay_ms(500);
if(mem_mode == 0 || mem_mode == 1)
{
//update_display_mode01(); delay_ms(1000);
beep();
update_pll(1,1);
}

#ifdef cb_included
if(mem_mode == 2)
{
VFD_send(15, 15, 12, 11, 15, 15, 15, 15, 15); delay_ms(1000); //CB
beep();
update_cb_channel(cb_channel, cb_region);
}
#endif

#ifdef cat_included
memset(cat_reply, '\0', sizeof cat_reply);
disable_interrupts(int_rda);
enable_interrupts(int_rda); //toggle interrupts to ensure serial is ready
enable_interrupts(global); //enable interrupts for CAT
#endif

int dialres = 0;
int buttonres = 0;
int mic_buttonres = 0;
brake = 0;
//our main loop, just like the Armoured-dildo IDE
   
   while(true)
   {
   
#ifdef cat_included
   //this is the CAT timeout. If incorrect or short command is entered, if timer goes over value and data invalid, data discarded
   if(timerstart){
   ++ticktimer;
   delay_us(10);
   }
   if(data_received){
   data_received = 0;
   parse_cat_command (buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);  
   }
#endif
   
   if(mem_mode == 0 || mem_mode == 1)
   {
   if(get_dial(frequency, dial_increment,0))
   {
   //dial is turning, add small delay for "debounce" so we can actually decrease the timer, or dialres would go back to 0 too quickly
   //because optical encoders are fast. And so are PICs. We will retune radio constantly and update the display at same time
   dialres = 1;
#ifdef dial_accel
   if(brake) {dial_timer = dial_master_timer; brake = 0;}
   if(dial_timer > 0) dial_timer-=1; else dial_timer = 1;
#endif   
   while(!update_pll(1,1));
   delay_us(500);
   }
   
   //check our new dial timer against the other ones we set earlier. If passed the threashold, increase the increment for next cycle
   //adjust dial timer and sensitivity to change how long you spin before these get triggered. Sensitivity cannot be more than timer or it will just lock you to a faster speed
#ifdef dial_accel
   if(dial_timer > dial_timer1)                                   dial_increment = 5;//50hz resolution. Can be lower but whats the point?       Slowest
   if((dial_timer <= dial_timer1) && (dial_timer > dial_timer2))  dial_increment = 10; //100hz resolution.                                      Slower
   if((dial_timer <= dial_timer2) && (dial_timer > dial_timer3))  dial_increment = 100; //1khz resolution.                                      Slow
   if((dial_timer <= dial_timer3) && (dial_timer > dial_timer4))  dial_increment = 1000; //10khz resolution.                                    Not Slow
   if((dial_timer <= dial_timer4) && (dial_timer > dial_timer5))  dial_increment = 10000; //100khz resolution. I reckon this is too fast.       Definitely Not Slow
#endif
   //dial complete. Just need to do final check later to see if its stopped turning. If it has, reset timer
   //end of dial
   }
#ifdef cb_included
   if(mem_mode == 2)
   {
   dial_increment = 1;
   if(get_dial(cb_channel, dial_increment,0))
   {
   while(!update_cb_channel(cb_channel, cb_region));
   delay_us(500);
   }
   }
#endif

   //deal with TX. Are we in band? Are we split? Is clarifier on? etc
   txrx();
   
   //now we need to deal with everything else. Buttons, mic buttons
   //microphone buttons
   //
   mic_buttonres = get_micbuttons();
   if(mic_buttonres) 
   {mic_buttonaction(mic_buttonres); 
   mic_buttonres = 0;
   
   }
   
   
      if (tick == 4)  // We only need to update these every 1/4 cycle
         {
         
         buttonres = get_buttons();
         if(buttonres) {buttonaction(buttonres); buttonres = 0;}
         
         //!   //if dial isnt moving, reset timer
         if(!get_dial(frequency, dial_increment, 0))
         {
         if(mem_mode == 0 || mem_mode == 1)
         {
            if(dialres == 1)
            {
            savef(active_vfo * 8);
#ifdef dial_accel
            dial_timer = dial_sensitivity;
#endif
            dialres = 0;
            }
         }
#ifdef cb_included
         if(mem_mode == 2)
         {
            if(dialres == 1)
            {
            write_eeprom (0xC3,cb_channel);
            dialres = 0;
            }
         }
#endif
         }
         tick = 0;
         //int 
         }
         else
         {
         if( pb2 && (! cnt_toggle)) { cnt_toggle = 1; tick = tick + 1;}
         if ((!pb2) && cnt_toggle) cnt_toggle = 0;
         }
   }
}



