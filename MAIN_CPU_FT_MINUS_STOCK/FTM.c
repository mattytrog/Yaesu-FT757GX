//FTMinus - STOCK EDITION
//(c) 2025 Matthew Bostock M0WCA

//Refer to Yaesu manual. This is a completely stock inplementation

//Stock Features Missing:
//None

#define 18f452
//#define 16f877a
//#define 16f877


#ifdef 18f452
#include <18F452.h>
//#define eeprom_save_debug32
#endif

#ifdef 16f877a
#include <16F877A.h>
#endif

#ifdef 16f877
#include <16F877.h>
#endif



#ifndef 18f452
#fuses HS,PUT, NOWDT,NOPROTECT,NOLVP
#else
#fuses HS,PUT, NOWDT,NOPROTECT,NOLVP, BORV27
#endif

//#define debug

//#define invert_wideband_check
#define store_to_eeprom
#define refresh 250
#define min_freq 1000
#define max_freq 3200000
#define jump_500k 50000 //amt to jump when 500k button pressed. in kc * 10

#use delay(clock=20000000)
#use rs232(baud=4800, xmit=PIN_C6, rcv=PIN_C7, parity=N, stop=2, ERRORS)

#ifndef 18f452
# byte PORTA = 0x05
# byte PORTB = 0x06
# byte PORTC = 0x07
# byte PORTD = 0x08
# byte PORTE = 0x09

 #else
 
# byte PORTA = 0x0f80
# byte PORTB = 0x0f81
# byte PORTC = 0x0f82
# byte PORTD = 0x0f83
# byte PORTE = 0x0f84
#endif

# bit dial_clk=PORTA.0  //input  // from q66 counter input (normally high, when counter = 0, goes low FOR uS). unused. Using up/down counter = 0 which is same result. Use for TX
# bit disp_INT=PORTA.4  //output // display interupt
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
# bit CPU_36_BIT128=PORTB.7  //output // strobe FOR pll2  q42
# bit pb0=PORTC.0  //input  // keypad pb0
# bit pb1=PORTC.1  //input  // keypad pb1
# bit pb2=PORTC.2  //input  // keypad pb2   // also tells us when not to write to the display
# bit sw_500k=PORTC.3  //input  // 500khz step SWITCH
# bit dial_dir=PORTC.4  //input  // Dial counting direction
# bit mic_up=PORTC.5  //input  // mic up button
# bit mic_dn=PORTC.6  //input  // mic down button
# bit pinc7=PORTC.7  //output // remote (CAT) wire (may use this FOR some sort of debugging output)
# bit sw_pms=PORTD.0  //input 
# bit mic_fast=PORTD.1  //input  // microphone fst (fast) button
# bit squelch_open=PORTD.2  //input  // Squelch open when high (FOR scanning)
# bit tx_mode=PORTD.3  //input  // PTT/On The Air (even high when txi set)
# bit pind4=PORTD.4  //input  // bcd counter sensing  bit 3
# bit pind5=PORTD.5  //input  // used FOR saveing vfo bit 2
# bit pind6=PORTD.6  //input  // even the undisplayed bit 1
# bit pind7=PORTD.7  //input  // 10hz component       bit 0
# bit k2=PORTE.0  //output // display bit 1   also these bits are FOR scanning the keypad
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
# bit   mr_mode=BITSA.7 
# bit   sl=BITSB.0 
# bit   cl=BITSB.1 
# bit   btn_down=BITSB.2 
# bit   force_update = BITSB.3
# bit   dir=BITSB.4
# bit   pms = BITSB.5
# bit   command_received = BITSB.6
# bit   transmitting = BITSB.7
# bit   valid = BITSC.0
# bit   temp_mr = BITSC.1
# bit   stopped = BITSC.2
# bit   gtx = BITSC.3
# bit   stx = BITSC.4
# bit   ctx = BITSC.5
# bit   mic_down = BITSC.6
# bit   split_transmit = BITSC.7

int8 mem_channel, PLLband, band, mem_mode, dcs, res1, res2;
int32 frequency = 0;
int32 dmhz1, dmhz2, dmhz, d100k, d10k, d1k, d100, d10;
char disp_buf[13] = {10,0,15,15,15,15,15,15,15,15,15,15,15};

const int32 band_bank[11] = 
{
   100000, 180000, 350000, 700000, 1010000,
   1400000, 1800000, 2100000, 2400000, 2800000, 3000000
};

#ifdef include_pms
const int32 band_bank_edge[11] = 
{
   50000, 200000, 380000, 720000, 1020000,
   1435000, 1820000, 2145000, 2500000, 3000000, 3000000
};
#endif

const int16 PLL_band_bank[30] = 
{
   //lower limit, upper limit, result
   0, 14, 1,
   15, 24, 1,
   25, 39, 2,
   40, 74, 3,
   75, 104, 4,
   105, 144, 5,
   145, 184, 6,
   185, 214, 7,
   215, 249, 8,
   250, 999, 9
};

int8 dcs_res[32] = 
{
   //dcs dl cl sl
   15, 0, 0, 0,
   14, 0, 1, 1,
   12, 0, 0, 1,
   4, 1, 1, 1,
   3, 1, 1, 0,
   2, 0, 1, 0,
   1, 1, 0, 0,
   0, 1, 0, 1
};

const int32 blacklist[20]= 
{
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

#ifndef store_to_eeprom
   #ifdef 18f452
   int32 ram_bank[60];
   int8 var_bank[40];
   #else
      #define store_to_eeprom
   #endif
#endif

#define BUFFER_SIZE 10

char temp_byte;//create 10 byte large buffer
char buffer[BUFFER_SIZE]; 
int next_in = 0;
long ticktimer = 0;

#INT_RDA

void  RDA_isr(VOID)
{
   IF(kbhit())
   {
      temp_byte = getc();
      buffer[next_in] = temp_byte;
      next_in++;
      ticktimer = 0;

      IF((next_in == 5))
      {
         buffer[next_in] = '\0';   // terminate string with null
         next_in = 0;
         command_received = 1;
         
         WHILE(kbhit())getc();
      }
   }

}// INT_R

void beep();
void write32(int8 address, unsigned int32 data)
{
   int32 temp_data;
   int8 i;
   for (i = 0; i < 4; i++)
      * ( (int8 * ) (&temp_data) + i) = read_eeprom (address +  i);

   
   if (data == temp_data) return;
   for (i = 0; i < 4; i++)
      write_eeprom (address + i, * ( (int8 *) (&data) + i));
#ifdef eeprom_save_debug32
   beep () ;
#endif
   //load_10hz (0) ;
}

unsigned int32 read32(int8 address)
{
   int8 i;
   int32 data;
   for (i = 0; i < 4; i++)
      * ( (int8 * ) (&data) + i) = read_eeprom (address +  i);

   return (data) ;
}

void save32(int8 base, int32 value)
{
   //
#ifdef store_to_eeprom
   write32 (base * 4, value);
#ELSE
   ram_bank[base] = value;
#endif
}

int32 load32(int8 base) 
{
#ifdef store_to_eeprom
      return read32(base * 4);
#ELSE
      return ram_bank[base];
#endif
}

void save8(int8 base, int8 value)
{
   //
#ifdef store_to_eeprom
      int8 temp_data = read_eeprom(base + 0xE0);
      if(temp_data == value) return;
      write_eeprom(base + 0xE0, value);
      #ifdef eeprom_save_debug8
      beep();
      #endif
#ELSE
      var_bank[base] = value;
#endif
}

int8 load8(int8 base) 
{
#ifdef store_to_eeprom
      return read_eeprom(base + 0xE0);
#ELSE
      return var_bank[base];
      #endif
}


void save_band_vfo_f(int8 vfo, int8 band, int32 value)
{
   IF (vfo == 0)                                {save32 ((30 + band), value);}
   IF (vfo == 1)                                {save32 ((41 + band), value);}
}

int32 load_band_vfo_f(int8 vfo, int8 band) 
{
   if (vfo == 0)                                return (load32 (30 + band));
   if (vfo == 1)                                return (load32 (41 + band));
}

void save_clar_RX_f(int32 value)    {save32(52, value);}
int32 load_clar_RX_f()              {return (load32 (52));}
void save_clar_TX_f(int32 value)    {save32(53, value);}
int32 load_clar_TX_f()              {return (load32 (53));}



void save_mem_ch_f(int8 channel, int32 value)   {save32 (channel+4, value);}
int32 load_mem_ch_f(int8 channel)               {return (load32(channel+4));}
void save_cache_f(int32 value)                  {save32 (2, value);}
int32 load_cache_f()                            {return (load32 (2));}
void save_cache_mem_mode_f(int32 value)         {save32 (3, value);}
int32 load_cache_mem_mode_f()                   {return (load32 (3));}



#define vfo_n 0
#define band_n 1
#define band_offset_n 2
#define mode_n 3
#define mem_ch_n 4
#define dcs_n 5
#define cb_ch_n 6
#define cb_reg_n 7
#define fine_tune_n 8
#define dial_n 9
#define savetimer_n 10
#define cache_n 11
#define cat_mode_n 12
#define baud_n 13
#define dummy_mode_n 14
#define id_enable_n 15
#define checkbyte_n 31



void split_value(int32 value, INT8 &d3, int8 &d4, int8 &d5, int8 &d6, int8 &d7, int8 &d8, int8 &d9)
{
      INT32 tmp_value = value;
      d3  = 0; WHILE (tmp_value > 999999){tmp_value -= 1000000; d3 += 1; }
      d4  = 0; WHILE (tmp_value > 99999){tmp_value -= 100000; d4 += 1; }
      d5  = 0; WHILE (tmp_value > 9999){tmp_value -= 10000; d5 += 1; }
      d6  = 0; WHILE (tmp_value > 999){tmp_value -= 1000; d6 += 1; }
      d7  = 0; WHILE (tmp_value > 99){tmp_value -= 100; d7 += 1; }
      d8  = 0; WHILE (tmp_value > 9) {tmp_value -= 10; d8 += 1; }
      d9  = 0; WHILE (tmp_value > 0) {tmp_value -= 1; d9 += 1; }

   //dbuf[0] = d3; dbuf[1] = (d4 * 10) + d5; dbuf[2] = (d6 * 10) + d7;dbuf[2] = (d8 * 10) + d9;
}

void join_value(int32 &value, INT8 d3, int8 d4, int8 d5, int8 d6, int8 d7, int8 d8, int8 d9)
{
int32 temp_value = 0;
temp_value = (((int32)d3 * 1000000) + ((int32)d4 * 100000) + ((int32)d5 * 10000) + ((int32)d6 * 1000) + ((int32)d7 * 100)+((int32)d8 * 10) + ((int32)d9));
value = temp_value;
//!
}




int8 Q64_val, Q64_tmp;
void Q64(INT8 val)
{
   //Q64 is Octal (000 - 111) (0 - 7) (Bit 8, port A is actually dial clock)
   //Bits are reversed on the bus.
   //A0 is dial clock (or spare). A1 = LSB. Only 8 values. Hand code FOR speed.
   //eg Beep is binary 4... could just DO beep() = 2, but would not actually be
   //correct on schematic, though would work fine
   //Correction by Matthew 19/2/25
   
   switch(val)
   {
      case 0: PORTA = 0; break;
      case 1: PORTA = 8; break;
      case 2: PORTA = 4; break;
      case 3: PORTA = 12; break;
      case 4: PORTA = 2; break;
      case 5: PORTA = 10; break;
      case 6: PORTA = 6; break;
      case 7: PORTA = 14; break;
   
   }
   Q64_val = val;
}

#define cycles_delay 2

void PLL1()
{
   Q64_tmp = Q64_val;
   Q64(1);
   delay_cycles(cycles_delay);
   Q64(Q64_tmp);
}

void PLL2()
{
   CPU_36_BIT128 = 1;
   delay_cycles(cycles_delay);
   CPU_36_BIT128 = 0;
}

void counter_preset_enable()
{
   Q64_tmp = Q64_val;
   Q64(2);
   delay_cycles(cycles_delay);
   Q64(Q64_tmp);
}

void banddata()
{
   Q64_tmp = Q64_val;
   Q64(3);
   delay_cycles(cycles_delay);
   Q64(Q64_tmp);
}

void beep()
{
   Q64_tmp = Q64_val;
   Q64(4);
   delay_cycles(cycles_delay);
   Q64(Q64_tmp);
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

void load_100hz(INT8 val)
{
   //save_port_b();
   INT8 loc100 = 112;
   PORTB = loc100 + val;
   //restore_port_b();
}

void load_10hz(int8 val)
{

save_port_b();
int8 loc10 = 112;
PORTB = loc10 + val;
counter_preset_enable();

restore_port_b();
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

void set_dial_lock(int1 res)
{
   if(res  == 1)Q64(5);
   if(res  == 0)Q64(0);
}

void set_clarifier(int1 res)
{
   
   if(res  == 1){if(load_clar_TX_f() == 0) {save_clar_TX_f(frequency); cl = 1;}}
   if(res  == 0) {save_clar_TX_f(0); cl = 0;}
}

void set_split(int1 res)
{
   if(res  == 1) {if(!sl) sl = 1;}
   if(res  == 0) sl = 0;
}


void set_tx_inhibit(INT1 res)
{
   if(res == 1)Q64(6);
   if(res == 0)Q64(0);
}

void set_dial_lock(int1 res);
void set_clarifier(int1 res);
void set_split(int1 res);

int8 get_dcs()
{
   INT8 res = 15;
   for(INT8 i  = 0; i < 8; i++)
   {
      IF((dl == dcs_res[(i * 4) + 1])&&(cl == dcs_res[(i * 4) + 2])&&(sl == dcs_res[(i * 4) + 3])){res = dcs_res[i * 4]; break; }
   }

   if(dl) set_dial_lock(1); else set_dial_lock(0);
   if(cl) set_clarifier(1); else set_clarifier(0);
   if(sl) set_split(1); else set_split(0);
   save8(dcs_n,res);
   RETURN res;
}

void set_dcs(INT8 res)
{
   for (INT8 i = 0; i < 8; i++)
   {
      IF(res == dcs_res[(i * 4)]){dl = dcs_res[(i * 4) + 1]; cl = dcs_res[(i * 4) + 2]; sl = dcs_res[(i * 4) + 3]; break;}
   }
   
   if(dl) set_dial_lock(1); else set_dial_lock(0);
   if(cl) set_clarifier(1); else set_clarifier(0);
   if(sl) set_split(1); else set_split(0);
   save8(dcs_n,res);
}

int8 send_disp_buf(int1 fast_update)
{

   if(fast_update)
   {
   static int16 count = 0;
   ++count;
   if(count < refresh) return 0;
   count = 0;
   }
   WHILE(pb2){}
   WHILE( ! pb2){}//and low again - then we send.
   
   for(INT i = 0; i < 13; ++i)
   {     
      k8 =(disp_buf[i])&0xF;
      k4 =(disp_buf[i]>>1)&0xF;
      k2 =(disp_buf[i]>>2)&0xF;
      k1 =(disp_buf[i]>>3)&0xF;
      
      //display interrupt. Period is between 370 - 410uS between each nibble.
      //This needs testing with real TMS!!!
      disp_INT = 1;
      delay_us(30);
      disp_INT = 0;
      delay_us(370);      
   }

   k1 = 0; k2 = 0; k4 = 0; k8 = 0;
   return 1;
}

void VFD_data(INT8 vfo_grid, int8 dcs_grid, int32 value, int8 channel_grid, int1 zeroes)
{
   int8 g3,g4,g5,g6,g7,g8,g9;
   if(vfo_grid != 0xFF) disp_buf[4] = vfo_grid; else disp_buf[4] = 15;
   if(dcs_grid != 0xFF) disp_buf[5] = dcs_grid; else disp_buf[5] = 15; 
   if(channel_grid != 0xFF) disp_buf[12] = channel_grid; else disp_buf[12] = 15;
   
   split_value (value, g3, g4, g5, g6, g7, g8, g9);
   
   dmhz1 = g3; dmhz2 = g4; d100k = g5; d10k = g6; d1k = g7; d100 = g8; d10 = g9;
   dmhz = ((dmhz1 * 10) + (dmhz2));
   
   if(!zeroes)
   {
   IF (value < 1000000) g3 = 15;
   IF (value < 100000) g4 = 15;
   IF (value < 10000) g5 = 15;
   IF (value < 1000) g6 = 15;
   if (value < 100) g7 = 15;
   if (value < 10) g8 = 15;
   if (value < 1) g9 = 15;   
   }
   
   disp_buf[6] = g3;
    disp_buf[7] = g4;
     disp_buf[8] = g5;
      disp_buf[9] = g6;
       disp_buf[10] = g7;
        disp_buf[11] = g8;
}

void vfo_disp(INT8 vfo, int8 dcs, int32 freq, int8 ch)
{
   INT8 v1, v2, v10;
   
   v2 = dcs;

   IF (mem_mode == 0)
   {
      IF (vfo == 0)v1 = 1;
      IF (vfo == 1)v1 = 12;
      v10 = 15;
   }

   
   IF (mem_mode == 1)
   {
      v1 = 2;
      v10 = ch;
   }

   
   VFD_data (v1, v2, freq, v10, 0);
}

int8 dial_moved()
{
   if(res1 != res2)
   {
   return 1;
   }
   return 0;
}

int8 get_state()
{
   INT8 state = 0;

   IF (mem_mode == 0)
   {
      IF (active_vfo == 0) {save_band_vfo_f (0, band, frequency); state = 1; }
      IF (active_vfo == 1) {save_band_vfo_f (1, band, frequency); state = 2; }
   }

   IF (mem_mode == 1) {save_cache_mem_mode_f (frequency); save8(mem_ch_n,mem_channel); state = 3; }
#ifdef include_cb
   IF (mem_mode == 2) {save8(cb_ch_n, cb_channel); state = 4;}
#endif
   RETURN state;
}

int8 read_state()
{
   INT8 state = 0;

   IF (mem_mode == 0)
   {
      IF (active_vfo == 0) {state = 1; }
      IF (active_vfo == 1) {state = 2; }
   }

   IF (mem_mode == 1) {state = 3; }
   IF (mem_mode == 2) {state = 4; }
   RETURN state;
}

int8 read_all_state()
{
   INT8 state = 0;

   IF (mem_mode == 0)
   {
      IF (active_vfo == 0) {frequency = load_band_vfo_f (0, band); state = 1; }
      IF (active_vfo == 1) {frequency = load_band_vfo_f (1, band); state = 2; }
   }

   IF (mem_mode == 1) {frequency = load_cache_mem_mode_f (); mem_channel = load8(mem_ch_n); state = 3; }
   #ifdef include_cb
   IF (mem_mode == 2) {cb_channel = load8(cb_ch_n); state = 4; }
   #endif
   RETURN state;
}

int8 refresh_all_state()
{
   INT8 state = 0;

   IF (mem_mode == 0)
   {
      IF (active_vfo == 0) {frequency = load_band_vfo_f (0, band); state = 1; }
      IF (active_vfo == 1) {frequency = load_band_vfo_f (1, band); state = 2; }
   }

   IF (mem_mode == 1) { mem_channel = load8(mem_ch_n); frequency = load_mem_ch_f (mem_channel);state = 3; }
   #ifdef include_cb
   IF (mem_mode == 2) {cb_channel = load8(cb_ch_n); state = 4; }
   #endif
   RETURN state;
}




void dial_lock_button()
{
   if(dl) dl = 0; else dl = 1;
   dcs = get_dcs();
}

void clarifier_button()
{
      if(cl) cl = 0; else {sl = 0; cl = 1;}
      dcs = get_dcs();
}

void split_button()
{

      if(sl) sl = 0; else {cl = 0; sl = 1;}
      dcs = get_dcs();
}

void down_button()
{
   if(mem_mode == 0)
   {
      if(sw_500k)
      {
         if (frequency > (min_freq)) frequency  -= jump_500k; else frequency = max_freq;
      }
      else
      {
         save_band_vfo_f(active_vfo, band, frequency);
         IF(band  == 0)band = 9;
         ELSE --band;
         save8(band_n,band);
         frequency = load_band_vfo_f(active_vfo, band);
      }
   
   }
   
   if(mem_mode == 1)
   {
      IF(mem_channel  > 1)--mem_channel;
      ELSE mem_channel = 8;
      frequency = load_mem_ch_f(mem_channel);
   }
}

void up_button()
{
   if(mem_mode == 0)
   {
      if(sw_500k)
      {
         if (frequency < (max_freq)) frequency += jump_500k; else frequency = min_freq;
      }
      else
      {
         save_band_vfo_f(active_vfo, band, frequency);
         IF(band  == 9)band = 0;
         ELSE ++band;
         save8(band_n,band);
         frequency = load_band_vfo_f(active_vfo, band);
      }
   
   }
   
   if(mem_mode == 1)
   {
      IF(mem_channel  < 8) ++mem_channel;
      ELSE mem_channel = 1;
      frequency = load_mem_ch_f(mem_channel);
   }
}

void mvfo_button()
{
   get_state();
   switch(mem_mode)
   {
      case 0: save_band_vfo_f (active_vfo, band, load_mem_ch_f(mem_channel)); break;
      case 1: save_band_vfo_f (active_vfo, band, load_cache_mem_mode_f ()); break;
   }
   refresh_all_state();
}

void vfoab_button()
{
   get_state();
   switch(active_vfo)
   {
      case 0: active_vfo = 1; break;
      case 1: active_vfo = 0; break;
   }
   save8(vfo_n,active_vfo);
   refresh_all_state();
}

void vfom_button()
{
   get_state();
   switch(mem_mode)
   {
      case 0: save_mem_ch_f (mem_channel, load_band_vfo_f (active_vfo, band)); break;
      case 1: save_mem_ch_f (mem_channel, load_cache_mem_mode_f ()); break;
   }
   refresh_all_state();
}

void mrvfo_button()
{
   get_state();
   switch (mem_mode)
   {
      case 0: mem_mode = 1; break;
      case 1: mem_mode = 0; break;
   }
   save8(mode_n, mem_mode);
   refresh_all_state();
}

void vfom_swap_button()
{
   get_state();
   save_cache_f (load_band_vfo_f (active_vfo, band));
   save_band_vfo_f (active_vfo, band, load_mem_ch_f (mem_channel));
   save_mem_ch_f (mem_channel, load_cache_f ());
   refresh_all_state();
}

void micup()
{
  frequency +=10;
}

void micup_hold()
{
  frequency +=10;
}

void micdn()
{
  frequency -=10;
}

void micdn_hold()
{
   frequency -=10;         
}

void micfst()
{

}

void micfst_hold()
{
 
}


void micup_fst()
{
  frequency +=1110; 
}

void micdn_fst()
{
  frequency -=1110;
}

void micup_fst_hold()
{
  frequency +=1110;  
}

void micdn_fst_hold()
{
  frequency -=1110; 
}




int8 buttonaction (INT8 res)
{
   
   SWITCH(res)
   {
      CASE 1: beep(); clarifier_button(); break;
      CASE 2: beep(); down_button(); break;
      CASE 3: beep(); up_button(); break;
      CASE 4: beep(); mvfo_button(); break; //MVFO
      CASE 5: beep(); vfoab_button(); break; //VFOAB
      CASE 6: beep(); dial_lock_button(); break;
      CASE 7: beep(); vfom_button(); break; //VFOM
      CASE 8: beep(); mrvfo_button(); break; //MRVFO
      CASE 9: beep(); split_button(); break;
      CASE 10: beep(); vfom_swap_button(); break; //VFOM SWAP
      CASE 11: micup(); break;
      CASE 12: micdn(); break;
      CASE 13: micfst(); break;
      CASE 14: micup_fst(); break;
      CASE 15: micdn_fst(); break;
      case 16: micup_hold(); break;
      case 17: micdn_hold(); break;
      case 18: micfst_hold(); break;
      case 19: micup_fst_hold(); break;
      case 20: micdn_fst_hold(); break;
//!      case 31: LONG_press_clarifier(); break;
//!      case 32: LONG_up_down(0, state); break;
//!      case 33: LONG_up_down(1, state); break;
//!      case 34: LONG_press_mvfo(); break;
//!      //case 35: LONG_press_vfoab(); break;
//!      case 36: LONG_press_dl(); break;
//!      case 37: LONG_press_vfom(); break;
//!      case 38: LONG_press_mrvfo(); break;
//!      case 39: LONG_press_split(); break;
//!      case 40: LONG_press_swap(); break;
      //default: RETURN 0; break;
   }
   RETURN res;
}

#define ondelay 1
#define countdelay 10
#define holdcount 100
int8 buttons(INT8 option)
{
   
   STATIC INT8 btnres = 0;
   STATIC INT8 micres = 0;
   STATIC INT16 count = 0;
   STATIC INT16 mic_count = 0;
   STATIC INT16 holdcountmic;
   INT rtnres = 0;
   IF(pb2)
   {
       WHILE(pb2){}
      if(btn_down)
      {
      
         k1 = 1;k2 = 1; k4 = 1; k8 = 1; delay_us(ondelay);
         IF((pb2)||(pb1)||(pb0))
         {
            if(option == 2)
            {
            rtnres= btnres;
            btnres = 0;
            btn_down = 0;
            return rtnres;
            }

            IF(option == 1)
            {
               if(count < holdcount) {++count; delay_ms(countdelay);}
               IF(count >= holdcount)
               {
                  rtnres = btnres + 30;
                  btnres = 0;
                  
                  k1 = 0; k2 = 0; k4 = 0; k8 = 0;
                  
                  count = 0;
                  RETURN rtnres;
               }
              
            }
            
            IF(option == 3)
            {
               if(count < holdcount) {++count; delay_ms(countdelay); return 0xFF;}
               IF(count >= holdcount)
               {
                  rtnres = btnres + 30;
                  //btnres = 0;
                  
                  k1 = 0; k2 = 0; k4 = 0; k8 = 0;
                  k1 = 1;k2 = 1; k4 = 1; k8 = 1; delay_us(ondelay);
                  IF((!pb2)&&(!pb1)&&(!pb0)) count = 0;
                  k1 = 0; k2 = 0; k4 = 0; k8 = 0;
                  
                  RETURN rtnres;
               }
              
            }
         }
         else
         {
            
            btn_down = 0;
            rtnres = btnres;
            btnres = 0;
            k2 = 0; k4 = 0; k8 = 0;
            
            count = 0;
            RETURN rtnres;
         }

         k2 = 0; k4 = 0; k8 = 0;
      }
      
      if(mic_down)
      {
         if (( mic_fast) || (!mic_up) || (!mic_dn))
         {
            if(option == 2)
            {
               rtnres= micres;
               micres = 0;
               mic_down = 0;
               return rtnres;
            }
            
            IF(option == 1)
            {
               if(mic_count < holdcountmic) {++mic_count;}
               if(mic_count >= holdcountmic) 
               {
               rtnres = micres + 5;
               mic_down = 1;
               RETURN rtnres;
               }
            }
            
            IF(option == 3)
            {
               if(mic_count < holdcountmic) {++mic_count; return 0xFF;}
               if(mic_count >= holdcountmic) 
               {
               rtnres = micres + 5;
               mic_down = 1;
               RETURN rtnres;
               }
            }
         }
          else
         {
            mic_down = 0;
            mic_count = 0;
            rtnres = micres;
            micres = 0;
            RETURN rtnres;
         }
      
      } else mic_count = 0;
      
      
      //printf("%d\r\n", btn_down);
      
      IF( ! btn_down)
      {
         k4 = 0; k8 = 0; k1 = 0; k2 = 1; delay_us(ondelay);
         IF(pb2){btn_down  = 1; btnres = 1; }//(RESULT: 1)Clarifier
         IF(pb1){btn_down  = 1; btnres = 2; }//(RESULT: 2)Down
         IF(pb0){btn_down  = 1; btnres = 3; }//(RESULT: 3)Up
         
         k2 = 0; k4 = 1; delay_us(ondelay);
         IF(pb2){btn_down  = 1; btnres = 4; }//(RESULT: 4)M > VFO
         IF(pb1){btn_down  = 1; btnres = 5; }//(RESULT: 5)VFO A / B
         IF(pb0){btn_down  = 1; btnres = 6; }//(RESULT: 6)Dial lock
         
         k4 = 0; k8 = 1; delay_us(ondelay);
         IF(pb2){btn_down  = 1; btnres = 7; }//(RESULT: 7)VFO > M
         IF(pb1){btn_down  = 1; btnres = 8; }//(RESULT: 8)MR / VFO
         IF(pb0){btn_down  = 1; btnres = 9; }//(RESULT: 9)SPLIT
         
         k8 = 0; k1 = 1; delay_us(ondelay);
         IF(pb1){btn_down  = 1; btnres = 10; }//(RESULT: 11)VFO < > M
         k8 = 0; k4 = 0; k2 = 0; k1 = 0;
      
      }
   }
      
         if (!mic_fast&& ! mic_up) {mic_down = 1; micres = 11; holdcountmic = 25;}
         if (!mic_fast&& ! mic_dn) {mic_down = 1;micres = 12; holdcountmic = 25;}
         if (mic_fast&&mic_up&&mic_dn)  {mic_down = 1; micres  = 13;holdcountmic = 100;}
         if (mic_fast&& ! mic_up) {mic_down = 1; micres = 14; holdcountmic = 25;}
         if (mic_fast&& ! mic_dn) {mic_down = 1; micres = 15; holdcountmic = 25;}
     
   //if ( ( ! mic_fast) && (mic_up) && (mic_dn)) {mic_down = 0; miccnt =  0;}
      

      IF(sw_pms)
      {
         beep(); WHILE(sw_pms){}
         IF(pms)pms  = 0; else pms = 1;
      }


   
   
   
   RETURN 0;
}

void PLL_REF()
{
   //PORTB data looks like this, sent to the MC145145 PLLs:
   //0  16  32  48  64  80  96
   //LATCH0 LATCH1 LATCH2 LATCH3 LATCH4 LATCH5 LATCH6
   //NCODEBIT0 NCODEBIT1 NCODEBIT2 NCODEBIT3 REF_BIT0 REF_BIT1 REF_BIT2
   //Each bit is strobed to the PLLs every PLL update, this never changes though, so we only need to send this once, unless it gets overwritten.
   //Should probably add something to ensure it doesn't... Or should we send it every PLL update? I don't see the need...
   //Latch6 address = 96(or 6<<4)
   //Latch5 address = 80(or 5<<4)
   //Latch4 Address = 64(or 4<<4)
   //These are written to latches 6, 5 and 4.
   PORTB = (6<<4) + 0x5; PLL1(); //Latch6 = Bit2 = address + (5 in hex = 5). Could write this as 96 + 5
   PORTB = (5<<4) + 0xD; PLL1(); //Latch5 = Bit1 = address + (13 in hex = D)or 80 + 13
   PORTB = (4<<4) + 0xC; PLL1(); //Latch4 = Bit0 = address + (12 in hex = C)or 64 + 12
   //5DC in hex = 1500
   PORTB = (6<<4) + 0; PLL2(); //Latch6 = Bit2 address + (0 in hex = 0)or 96 + 0
   PORTB = (5<<4) + 0x1; PLL2(); //Latch5 = Bit1 address + (1 in hex = 1)or 80 + 1
   PORTB = (4<<4) + 0xE; PLL2(); //Latch4 = Bit0 address + (14 in hex = E)or 64 + 14
   //01E in hex = 30
}

//We have already sent latches 6,5 and 4, which are reference bits(look above at PLL_REF)
//This purely updates latches 3,2,1 and 0. Latch 3 isn't used, so we could put this in the PLL_REF graveyard IF we wanted to (ie sent once)
//Base frequency is loaded, then we load any offset. Once we know, we find out what LPF "band" should be selected and strobed. Original calc_frequency is unchanged remember
//Don't confuse these "bands" with the standard Yaesu bands we select when pressing BAND UP/DOWN

int1 update_PLL()
{
   // We only need to update, IF there is a new NCODE to send, ie when new frequency is requested. We don't touch ref latches(6,5,4), so no need to resend
   //IF PLL update is requested for the SAME frequency, you are sent on your way and bounced back whence you came lol
   if(frequency > max_freq) frequency = min_freq;
   else
   if(frequency < min_freq) frequency = max_freq;
   
   STATIC int16 old_khz_freq;
   STATIC int32 old_band_freq;
   STATIC int8 old_d100;
   STATIC int8 old_d10;
      
   
   INT16 tmp_band_freq =((dmhz * 10) + d100k);
   IF(tmp_band_freq != old_band_freq)
   {
      for(INT i  = 0; i < 10; i++)
      {
         IF((tmp_band_freq >= PLL_band_bank[(i * 3)])&&(tmp_band_freq <= PLL_band_bank[(i * 3) + 1])){ PLLband = (PLL_band_bank[(i * 3) + 2]); break; }
      }

      PORTB = PLLband; banddata();
   }
   
   INT16 tmp_khz_freq = ((d100k * 100) + (d10k * 10) + (d1k));
   IF(tmp_khz_freq >= 500)tmp_khz_freq -= 500;
   tmp_khz_freq += 560;
   
   INT PLL1_NCODE_L3 = 0; //empty latch 48
   INT PLL1_NCODE_L2 = 0; //   32
   INT PLL1_NCODE_L1 = 0; //   16
   INT PLL1_NCODE_L0 = 0; //   0
   
   IF(tmp_khz_freq != old_khz_freq)
   {
      //841 in hex is 349. So latch2 = 3, latch 1 = 4, latch 0 = 9;
      PLL1_NCODE_L2 = (tmp_khz_freq >> 8); //first digit
      PLL1_NCODE_L1 = ((tmp_khz_freq >> 4)&0xF); //middle digit
      PLL1_NCODE_L0 = (tmp_khz_freq&0xF); //final digit
      //PLL1 end
      PORTA = 0;
      PORTB = 48 + PLL1_NCODE_L3;
      PLL1();
      PORTB = 32 + PLL1_NCODE_L2;
      PLL1();
      PORTB = 16 + PLL1_NCODE_L1;
      PLL1();
      PORTB = 0 + PLL1_NCODE_L0;
      PLL1();
      old_khz_freq = tmp_khz_freq;
   }

   INT PLL2_NCODE_L3 = 0; //empty latch 48
   INT PLL2_NCODE_L2 = 0; //empty latch 32
   INT PLL2_NCODE_L1 = 0; //   16
   INT PLL2_NCODE_L0 = 0; //   0
   IF(tmp_band_freq != old_band_freq)
   {
      IF(PLLband < 6)
      {
         PLL2_NCODE_L1 = (((tmp_band_freq / 5) + 12)>> 4);
         PLL2_NCODE_L0 = (((tmp_band_freq / 5) + 12)&0xF);
      }

      ELSE
      {
         PLL2_NCODE_L1 = (((tmp_band_freq / 5) - 18)>> 4);
         PLL2_NCODE_L0 = (((tmp_band_freq / 5) - 18)&0xF);
      }

      
      
      
      PORTB = 48 + PLL2_NCODE_L3; // Prepare latch
      PLL2();
      PORTB = 32 + PLL2_NCODE_L2; // Prepare latch
      PLL2();
      PORTB = 16 + PLL2_NCODE_L1;
      PLL2();
      PORTB = 0 + PLL2_NCODE_L0;
      PLL2();
      old_band_freq = tmp_band_freq;
   }

   if(d100 != old_d100)
   {
      load_100hz(d100+1);
      old_d100 = d100;
   }
   
   if(d10 != old_d10)
   {
      load_10hz(d10);
      old_d10 = d10;
   }
   //res2 = d10;
   res2 = read_counter();
   

   
   #ifdef debug
   puts("tuned PLL ! ");
   printf("tmp_band_freq :  %ld\r\n", tmp_band_freq);
   printf("tmp_khz_freq :  %ld\r\n", tmp_khz_freq);

   #endif
   return 0;
}

int8 calc_band(INT32 frequency)
{
   for(INT i  = 0; i < 10; i++)
   {
      IF((frequency >= band_bank[i])&&(frequency < band_bank[i + 1]))break;
   }

   RETURN i;
}

int8 parse_cat ()
{
   INT32 byte4_upper = ((buffer[3] >> 4) & 0xF);
   INT32 byte4_lower = buffer[3] & 0xF;
   INT32 byte3_upper = ((buffer[2] >> 4) & 0xF);
   INT32 byte3_lower = buffer[2] & 0xF;
   INT32 byte2_upper = ((buffer[1] >> 4) & 0xF);
   INT32 byte2_lower = buffer[1] & 0xF;
   INT32 byte1_upper = ((buffer[0] >> 4) & 0xF);
   INT32 byte1_lower = buffer[0] & 0xF;

   SWITCH(buffer[4])
   {
      CASE 0x01: buttonaction(9); break;
      CASE 0x02: buttonaction(8); break;
      CASE 0x03: buttonaction(7); break;
      CASE 0x04: buttonaction(6); break;
      CASE 0x05: buttonaction(5); break;
      CASE 0x06: buttonaction(4); break;
      CASE 0x07: buttonaction(3); break;
      CASE 0x08: buttonaction(2); break;
      CASE 0x09: buttonaction(1); break;
      
      CASE 0x0A: frequency = ((int32)(byte4_lower * 1000000) + (int32)(byte3_upper * 100000) + (int32)(byte3_lower * 10000) + (int32)(byte2_upper * 1000) + (int32)(byte2_lower * 100) + (int32) (byte1_upper * 10) + (int32)byte1_lower); break;
      CASE 0x0B: buttonaction(11); break;
      
      CASE 0x0F: frequency = ((int32)(byte1_upper * 1000000) + (int32)(byte1_lower * 100000) + (int32)(byte2_upper * 10000) + (int32)(byte2_lower * 1000) + (int32)(byte3_upper * 100) + (int32)(byte3_lower * 10) + (int32)byte4_upper); update_pll(); break;
      CASE 0xFF: reset_cpu(); break;
   }

   command_received = 0;
   RETURN 1;
}

int8 tx_oob_check(INT32 freq)
{
   INT8 valid = 0;

   IF ( ! gen_tx)
   {
      IF (tx_mode)
      {
         IF ( ! gtx)
         {
            for (INT i = 0; i < 10; i++)
            {
               IF ( (freq >= blacklist[i * 2]) && (freq <= blacklist[ (i * 2) + 1])) {valid = 1; break; }
            }

            IF ( ! valid)
            {
               beep () ;
               WHILE (tx_mode) set_tx_inhibit (1);
            }

            gtx = 1;
         }
      }

      ELSE
      IF (gtx)
      {
         set_tx_inhibit (0) ;
         gtx = 0;
      }
   } ELSE valid = 1;

   RETURN valid;
}



int1 tx_override()
{

IF(sl)
   {
      IF (tx_mode)
      {
         IF ( ! stx)
         {
            save_band_vfo_f (active_vfo, band, frequency);
            IF (active_vfo == 0){active_vfo = 1;}
            else IF (active_vfo == 1){active_vfo = 0;}
            frequency = load_band_vfo_f (active_vfo, band);
            stx = 1;
            RETURN 1;
         }
      }

      ELSE
      {
         IF (stx)
         {
            save_band_vfo_f (active_vfo, band, frequency);
            IF (active_vfo == 0){active_vfo = 1;}
            else IF (active_vfo == 1){active_vfo = 0;}
            frequency = load_band_vfo_f (active_vfo, band);
            stx = 0;
            RETURN 1;
         }
      }
   }

IF (cl)
   {
      IF (tx_mode)
      {
         IF ( ! ctx)
         {
            save_clar_RX_f (frequency) ;
            frequency = load_clar_TX_f ();
            ctx = 1;
            RETURN 1;
         }
      }

      ELSE
      {
         IF (ctx)
         {
            save_clar_TX_f (frequency) ;
            frequency = load_clar_RX_f ();
            ctx = 0;
            RETURN 1;
         }
      }
   }
}

int1 pms_scanner()
      {
        int8 temp_mem_mode = mem_mode;
        mem_mode = 1;
        stopped = 0;
        while(true)
        {
         IF( ! stopped)
         {
            delay_ms(100);

            IF( ! squelch_open)
            {
               IF(mem_channel < 8)++mem_channel; else mem_channel = 1;
               frequency = load_mem_ch_f(mem_channel);
            }
         }

         vfo_disp (active_vfo, dcs, frequency, mem_channel);
         send_disp_buf(0);
         update_PLL();
         
         IF(squelch_open)
         {
            stopped = 1;
         }

         
         
         
         
         IF(stopped)
         {
            
            IF( ! squelch_open)
            {
               for(INT16 i = 0; i < 2000; ++i)
               {
                  

                  delay_ms(1);
               }

               stopped = 0;
            } ELSE stopped = 1;
         }
if(sw_pms) {WHILE(sw_pms){} break;}
      }
      
      mem_mode = temp_mem_mode;
      return 1;
}

void set_defaults()
{
   for (INT i = 0; i <= 18; i++)
   {save32 (i, 700000); }
   for (i = 19; i <= 29; i++)
   {save32 (i, 0); }
   for (i = 30; i <= 40; i++)
   {save32 (i, band_bank[i - 30]); }
   for (i = 41; i <= 51; i++)
   {save32 (i, band_bank[i - 41]); }
   for (i = 52; i <= 53; i++)
   {save32 (i, 0); }
   //save32 (54, minimum_freq); save32 (55, maximum_freq);
   //save32 (3, 1010000);
   save8(vfo_n, 0); //Active VFO A / B
   save8(mode_n, 0); //Mem mode
   save8(mem_ch_n, 1); //default mem channel
   save8(dcs_n, 15) ;
   save_cache_mem_mode_f (700000) ;
   save8(band_n,(calc_band (700000))) ;
   save8(checkbyte_n, 1); //Check byte
#ifdef store_to_eeprom
      reset_cpu () ;
#endif
}

void load_values()
{
      active_vfo =  load8(vfo_n);
      mem_mode =   load8(mode_n);
      mem_channel =  load8(mem_ch_n);
      dcs =    load8(dcs_n);
      band =    load8(band_n);


   set_dcs (dcs) ;
}

void main()
{
   setup_adc(ADC_OFF);
   set_tris_a(0b00001);
   set_tris_b(0b00000000);
   set_tris_c(0b11111111);
   set_tris_d(0b11111111);
   set_tris_e(0b000);
   Q64(0);
   delay_ms(200);
   k1 = 1; delay_us(1);
   #ifndef invert_wideband_check
   IF(pb0)gen_tx  = 0; // //widebanded?
   ELSE gen_tx = 1;
   #else
   IF(pb0)gen_tx  = 1; // //widebanded?
   ELSE gen_tx = 0;
   #endif
   k1 = 0;
   k4 = 1; delay_us(1);
   if(pb1) {write_eeprom (0xFF, 0xFF); beep(); delay_ms(1000); reset_cpu();}
   //if(pb0) {program_vfos(); beep(); delay_ms(1000); reset_cpu();}
   k4 = 0;
   
   PLL_REF();
   enable_interrupts(INT_rda); //toggle interrupts to ensure serial is ready
   enable_interrupts(global); //enable interrupts FOR CAT
  
   if (load8(checkbyte_n) != 1) {set_defaults (); load_values (); }
      else load_values () ;

      refresh_all_state();
      
   send_disp_buf(0);
   
   int16 count = 0;
   int8 res = 1;
   int1 txres = 0;
   stx = 0; ctx = 0;
   while(true)
   {
   
      if(sw_pms) {while(sw_pms){} res = pms_scanner();}
      
      if(count < 20000) ++count;
      res1 = read_counter();
      
      if(count == 20000) get_state();
         
      
      if(count >= 1000)
      {
       IF( ! res)
         {
            
            res = buttons(1);
            IF(res){ res = buttonaction(res); count = 1000;}
            
            
            
         }
      
      }
      
      if(!res)
            {
            if(command_received) {res = parse_cat(); count = 1000;}
            }
      
      if(!res)
      {
         if(dial_moved())
         {  
            if(dial_dir) frequency +=2; else frequency -=2;
            res = 1;
            count = 0;
         }
      }
      
      txres = tx_override();
      
      if((res) || (txres))
      {
         
         vfo_disp (active_vfo, dcs, frequency, mem_channel);
         update_pll();
         res = 0;
      }
      tx_oob_check(frequency);
      
      send_disp_buf(1);
      
      //


   }
}
