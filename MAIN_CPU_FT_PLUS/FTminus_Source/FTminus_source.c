//!FTMinus v1.0
//!INTENDED FOR SMALL CHIPS!!!
//!This is equivalent to a stock Yaesu CPU
//!
//!Implemented functions
//!Tuning
//!VFOs
//!Yaesu 8 Memories
//!Yaesu CAT
//!Yaesu PMS
//!Mic up/down/fast
//!
//!Unimplemented functions
//!Accelerated dial
//!Manual tuning
//!Kenwood CAT
//!Fine tuning
//!Offset mode
//!More memories
//!EEPROM storage 
//!   (easy to add, but LF chips may not need it, 
//!   if relying on battery in radio)
//!
//!No manual (Use the Yaesu one)
//!Bug reports welcome
//!


#define 18f452
//#define 16f877a
#ifdef 18f452
#include <18F452.h>
#endif
#ifdef 16f877a
#include <16F877A.h>
#endif
#ifdef 18f452
#fuses HS,PUT, NOWDT,NOPROTECT,NOLVP, BORV27
#endif
#ifdef 16f877a
#fuses HS,PUT, NOWDT,NOPROTECT,NOLVP
#endif
#use delay(clock=20000000)
#use rs232(baud=4800, xmit=PIN_C6, rcv=PIN_C7, parity=N, stop=2, ERRORS)
#ifdef 16f877a
# byte PORTA = 0x05
# byte PORTB = 0x06
# byte PORTC = 0x07
# byte PORTD = 0x08
# byte PORTE = 0x09
#endif
#ifdef 18f452
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
//# bit   fine_tune_display=BITSB.0 
# bit   sl=BITSB.1 
# bit   cl=BITSB.2 
# bit   btn_down=BITSB.3 
# bit   force_update = BITSB.4
# bit   dir=BITSB.5
# bit   pms = BITSB.6
# bit   command_received = BITSB.7
# bit   transmitting = BITSC.0
# bit   valid = BITSC.1
# bit   temp_mr = BITSC.2
# bit   stopped = BITSC.3
# bit   gtx = BITSC.4
# bit   stx = BITSC.5
# bit   ctx = BITSC.6
//# bit   fine_tune = BITSC.7
//# bit   AI = BITSD.0
//# bit   cat_mode = BITSD.1
//# bit   save_AI = BITSD.2
//# bit   cat_tx_request = BITSD.3
//# bit   cat_tx_transmitting = BITSD.4
//# bit   id_enable = BITSD.5
# bit   mic_down = BITSD.6
//# bit   dltmp = BITSD.7
#define refresh 500

int8 dmhz, dmhz1, dmhz2, d100k, d10k, d1k, d100, d10;
int8 tmp_dmhz;
char disp_buf[13] = {10,0,15,15,15,15,15,15,15,15,15,15,15};
int8 ram_store[60];
int8 ram_store1[60];
int8 ram_store2[60];
int8 mem_channel, band, dcs;

const INT16 PLL_band_bank[30] = 
{
   //lower limit, upper limit, result
   5, 14, 0,
   15, 24, 1,
   25, 39, 2,
   40, 74, 3,
   75, 104, 4,
   105, 144, 5,
   145, 184, 6,
   185, 214, 7,
   215, 249, 8,
   250, 300, 9
};

const INT16 band_bank[30] = 
{
   10, 17, 0,
   18, 34, 1,
   35, 69, 2,
   70, 100, 3,
   101, 139, 4,
   140, 179, 5,
   180, 209, 6,
   210, 239, 7,
   240, 279, 8,
   280, 300, 9
};

const INT16 blacklist[20]= 
{
   15, 20,
   35, 40,
   70, 75,
   100, 105,
   140, 145,
   180, 185,
   210, 215,
   240, 255,
   280, 300
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

void load_mem_channel(INT8 base)
{
   dmhz = ram_store2[base];
   d100k = ram_store2[base+10];
   d10k = ram_store2[base+20];
   d1k = ram_store2[base+30];
   d100 = ram_store2[base+40];
}

void save_mem_channel(INT8 base)
{
   ram_store2[base] = dmhz;
   ram_store2[base+10] = d100k;
   ram_store2[base+20] = d10k;
   ram_store2[base+30] = d1k;
   ram_store2[base+40] = d100;
}

void save_clarifier()
{
   ram_store2[51] = dmhz;
   ram_store2[52] = d100k;
   ram_store2[53] = d10k;
   ram_store2[54] = d1k;
   ram_store2[55] = d100;
}

void load_clarifier()
{
   dmhz = ram_store2[51];
   d100k = ram_store2[52];
   d10k = ram_store2[53];
   d1k = ram_store2[54];
   d100 = ram_store2[55];
}

void load_vfos_a(INT8 base)
{
   dmhz = ram_store[base];
   d100k = ram_store[base+10];
   d10k = ram_store[base+20];
   d1k = ram_store[base+30];
   d100 = ram_store[base+40];
}

void save_vfos_a(INT8 base)
{
   ram_store[base] = dmhz;
   ram_store[base+10] = d100k;
   ram_store[base+20] = d10k;
   ram_store[base+30] = d1k;
   ram_store[base+40] = d100;
}

void load_vfos_b(INT8 base)
{
   dmhz = ram_store1[base];
   d100k = ram_store1[base+10];
   d10k = ram_store1[base+20];
   d1k = ram_store1[base+30];
   d100 = ram_store1[base+40];
}

void save_vfos_b(INT8 base)
{
   ram_store1[base] = dmhz;
   ram_store1[base+10] = d100k;
   ram_store1[base+20] = d10k;
   ram_store1[base+30] = d1k;
   ram_store1[base+40] = d100;
}

void save_vfo(INT8 res)
{
   IF(res == 0) save_vfos_a(band);
   IF(res == 1) save_vfos_b(band);
}

void load_vfo(INT8 res)
{
   IF(res == 0) load_vfos_a(band);
   IF(res == 1) load_vfos_b(band);
}

int8 Q64_val;

void Q64(INT8 val)
{
   //Q64 is Octal(000 - 111)(0 - 7)(Bit 8, port A is actually dial clock)
   //Bits are reversed on the bus.
   //A0 is dial clock(or spare). A1 = LSB. Only 8 values. Hand code FOR speed.
   //eg Beep is binary 4... could just DO beep() = 2, but would not actually be
   //correct on schematic, though would work fine
   //Correction by Matthew 19 / 2 / 25
   
   SWITCH(val)
   {
      CASE 0: PORTA = 0; break;
      CASE 1: PORTA = 8; break;
      CASE 2: PORTA = 4; break;
      CASE 3: PORTA = 12; break;
      CASE 4: PORTA = 2; break;
      CASE 5: PORTA = 10; break;
      CASE 6: PORTA = 6; break;
      CASE 7: PORTA = 14; break;
      DEFAULT: PORTA = 0; break;
   }

   Q64_val = val;
}

#define cycles_delay 2

int8 Q64_tmp;

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

void set_dl()
{
   IF(dl == 1)Q64(5);
   IF(dl == 0)Q64(0);
}

void set_cl()
{
   IF(cl == 1)save_clarifier();
}

void set_tx_inhibit(INT res)
{
   IF(res == 1)Q64(6);
   IF(res == 0)Q64(0);
}

int8 get_dcs()
{
   INT8 res = 15;
   for(INT8 i = 0; i < 8; i++)
   {
      IF((dl == dcs_res[(i * 4) + 1])&&(cl == dcs_res[(i * 4) + 2])&&(sl == dcs_res[(i * 4) + 3])){res = dcs_res[i * 4]; break; }
   }

   set_dl();
   RETURN res;
}

void set_dcs(INT8 res)
{
   dl = 0; cl = 0; sl = 0;
   for(INT8 i  = 0; i < 8; i++)
   {
      IF(res == dcs_res[(i * 4)]){dl = dcs_res[(i * 4) + 1]; cl = dcs_res[(i * 4) + 2]; sl = dcs_res[(i * 4) + 3]; break;}
   }

   set_dl();
}

void send_disp_buf()
{
   
   tmp_dmhz = dmhz;
   dmhz1 = 0; WHILE(tmp_dmhz >= 10){tmp_dmhz -= 10; dmhz1+= 1; }
   IF(dmhz1 == 0)dmhz1 = 15;
   dmhz2 = 0; WHILE(tmp_dmhz > 0){tmp_dmhz -= 1; dmhz2 += 1; }
   IF((dmhz1 == 15)&&(dmhz2 == 0))dmhz2 = 15;

   IF( ! mr_mode)
   {
      IF(active_vfo == 0)disp_buf[4] = 1;
      IF(active_vfo == 1)disp_buf[4] = 12;
      disp_buf[12] = 15;
   }

   ELSE
   {
      disp_buf[4] = 2;
      disp_buf[12] = mem_channel;
   }

   disp_buf[5] = dcs;
   disp_buf[6] = dmhz1;
   disp_buf[7] = dmhz2;
   disp_buf[8] = d100k;
   disp_buf[9] = d10k;
   disp_buf[10] = d1k;
   disp_buf[11] = d100;
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

   //printf(" % d % d\r\n", newcheck, oldcheck);
   force_update = 0;
   k1 = 0; k2 = 0; k4 = 0; k8 = 0;
}

void get_freq()
{
   d10 += 10; d100 += 10; d1k += 10; d10k+= 10; d100k += 10; dmhz +=  10;
   IF(dir)
   {
      
      IF(d10 > 19){d10 -= 10;d100 +=  1; }
      IF(d100 > 19){d100 -= 10;d1k +=  1; }
      IF(d1k > 19){d1k -= 10;d10k +=  1; }
      IF(d10k > 19){d10k -= 10;d100k +=  1; }
      IF(d100k > 19){d100k -= 10;dmhz +=  1; }
   }

   ELSE
   {
      IF(d10 < 10){d100 -= 1; d10 +=  10; }
      IF(d100 < 10){d1k -= 1; d100 +=  10; }
      IF(d1k < 10){d10k -= 1; d1k +=  10; }
      IF(d10k < 10){d100k -= 1; d10k +=  10; }
      IF(d100k < 10){dmhz -= 1; d100k +=  10; }
   }

   d10 -= 10; d100 -= 10; d1k -= 10; d10k-= 10; d100k -= 10; dmhz -=  10;
}

int8 buttonres(INT8 res)
{
   IF(res == 1)
   {
      IF(sl)sl  = 0;
      IF(cl)cl = 0; else cl = 1;
      set_cl();
   }

   
   IF( ! mr_mode)
   {
      IF(res == 23){dir = 1; d100k += 5; get_freq(); }
      IF(res == 22){dir = 0; d100k -= 5; get_freq(); }
   }

   IF(mr_mode)
   {
      IF(res == 2){if(mem_channel > 1) --mem_channel; }
      IF(res == 3){if(mem_channel < 8) ++mem_channel; }
      load_mem_channel(mem_channel);
   }

   ELSE
   {
      save_vfo(active_vfo);
      IF(res == 2){if(band > 0)-- band; }
      IF(res == 3){if(band < 9)++ band; }
      load_vfo(active_vfo);
   }

   IF(res == 4)
   {
      load_mem_channel(mem_channel);
      save_vfo(active_vfo);
   }

   IF(res == 5)
   {
      IF( ! mr_mode)
      {
         save_vfo(active_vfo);
         IF(active_vfo  == 0)active_vfo = 1; else active_vfo = 0;
         load_vfo(active_vfo);
      }
   }

   
   IF(res == 6)
   {
      IF( ! dl){dl = 1; }
      ELSE{dl  = 0; }
   }

   
   IF(res == 7)
   {
      IF(mr_mode)
      {
         load_vfo(active_vfo);
         save_mem_channel(mem_channel);
         load_mem_channel(mem_channel);
      }

      ELSE
      {
         save_mem_channel(mem_channel);
      }

   }

   IF(res == 8)
   {
      IF(mr_mode)
      {
         mr_mode = 0;
         load_vfo(active_vfo);
      }

      ELSE
      {
         mr_mode = 1;
         save_vfo(active_vfo);
         load_mem_channel(mem_channel);
      }
   }

   IF(res == 9)
   {
      IF(cl)cl  = 0;
      IF(sl)sl = 0; else sl = 1;
   }

   IF(res == 10)
   {
      load_vfo(active_vfo);
      save_mem_channel(0);
      load_mem_channel(mem_channel);
      save_vfo(active_vfo);
      load_mem_channel(0);
      save_mem_channel(mem_channel);
      IF(mr_mode)load_mem_channel(mem_channel);
      ELSE load_vfo(active_vfo);
   }

   IF(res)dcs  = get_dcs();
   RETURN res;
}

int8 buttons()
{

   
   
   STATIC int8 btnres = 0;
   INT8 rtnres = 0;

   IF(pb2)
   {
      WHILE(pb2){}
      

      IF(btn_down)
      {
         k1 = 1; k2 = 1; k4 = 1; k8 = 1; delay_us(1);
         IF(( ! pb0)&&( ! pb1)&&( ! pb2))
         {
            k1 = 0; k2 = 0; k4 = 0; k8 = 0;
            rtnres = 0;
            btnres = 0;
            btn_down = 0;
            RETURN 0;
         }

         ELSE
         {
            IF((pb0)||(pb1)||(pb2))btn_down  = 1;
            ELSE btn_down = 0;
            k1 = 0; k2 = 0; k4 = 0; k8 = 0;
            rtnres = btnres;
            btnres = 0;
            
            RETURN rtnres;
         }

      }

      IF( ! btn_down)
      {
         k4 = 0; k8 = 0; k1 = 0; k2 = 1; delay_us(1);
         IF(pb2){btn_down = 1; btnres = 1; }//(RESULT: 1)Clarifier
         IF(pb1)
         {
            btn_down = 1;
            IF( ! sw_500k)btnres = 2; else btnres = 22;
         }//(RESULT: 2)Down

         IF(pb0)
         {
            btn_down = 1;
            IF( ! sw_500k)btnres = 3; else btnres = 23;
         }//(RESULT: 3)Up

         
         k2 = 0; k4 = 1; delay_us(1);
         IF(pb2){btn_down = 1; btnres = 4; }//(RESULT: 4)M > VFO
         IF(pb1){btn_down = 1; btnres = 5; }//(RESULT: 5)VFO A / B
         IF(pb0){btn_down = 1; btnres = 6; }//(RESULT: 6)Dial lock
         
         k4 = 0; k8 = 1; delay_us(1);
         IF(pb2){btn_down = 1; btnres = 7; }//(RESULT: 7)VFO > M
         IF(pb1){btn_down = 1; btnres = 8; }//(RESULT: 8)MR / VFO
         IF(pb0){btn_down = 1; btnres = 9; }//(RESULT: 9)SPLIT
         
         k8 = 0; k1 = 1; delay_us(1);
         IF(pb1){btn_down = 1; btnres = 10; }//(RESULT: 11)VFO <  > M
         k8 = 0; k4 = 0; k2 = 0; k1 = 0;
      }
   }

   RETURN 0;
}

int8 mic_buttons()
{
   INT res = 0;
   STATIC int16 cnt = 0;
   //STATIC int mic_counter = 0;
   IF( !mic_fast && !mic_up) res = 1;
   else IF( !mic_fast && !mic_dn) res = 5;
   else IF(mic_fast && !mic_up) res = 13;
   else IF(mic_fast && !mic_dn) res = 14;
   if(!res) {cnt = 0; RETURN 0;}
   IF(res == 1)
   {
      WHILE( ! mic_fast&& ! mic_up)
      {
         IF(cnt < 500){++cnt; delay_ms(5); }
         IF(cnt >= 100){res = 2; break; }
      }

   }

   
   IF(res == 5)
   {
      WHILE( ! mic_fast&& ! mic_dn)
      {
         IF(cnt < 500){++cnt; delay_ms(5); }
         IF(cnt >= 100){res = 6; break; }
      }

   }

   
   
   IF((res == 1)||(res == 5))
   {
      IF(cnt > 99)res = 0;
   }

   
   IF( ! mr_mode)
   {
      if(res == 1){dir = 1; d100 += 1; get_freq(); cnt = 0; RETURN 1;}
      IF(res == 2){dir = 1; d100 += 1; get_freq(); }
      IF(res == 13){dir = 1; d1k += 1; get_freq(); }
      
      if(res == 5){dir = 0; d100 -= 1; get_freq(); cnt = 0; RETURN 1;}
      IF(res == 6){dir = 0; d100 -= 1; get_freq(); }
      IF(res == 14){dir = 0; d1k -= 1; get_freq(); }
   }

   
   ELSE
   {
      IF(res == 5){if(mem_channel > 1) --mem_channel; }
      IF(res == 1){if(mem_channel < 8) ++mem_channel; }
      load_mem_channel(mem_channel);
   }

   
   
   
   
   RETURN res;
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

int8 PLLband = 0;

void update_PLL()
{
   // We only need to update, IF there is a new NCODE to send, ie when new frequency is requested. We don't touch ref latches(6,5,4), so no need to resend
   //IF PLL update is requested for the SAME frequency, you are sent on your way and bounced back whence you came lol
   
   
   STATIC int16 old_khz_freq;
   STATIC int32 old_band_freq;
   STATIC int8 old_d100;
   
   INT16 tmp_band_freq =((dmhz * 10) + d100k);
   
   IF(tmp_band_freq != old_band_freq)
   {
      for(INT i  = 0; i < 10; i++)
      {
         IF((tmp_band_freq >= PLL_band_bank[(i * 3)])&&(tmp_band_freq <= PLL_band_bank[(i * 3) + 1])){ PLLband = (PLL_band_bank[(i * 3) + 2]); break; }
      }

      PORTB = PLLband; banddata();
   }

   //!      for (INT i = 0; i < 10; i++)
   // !
   {
      //!            IF((tmp_band_freq >= band_bank[(i * 3)]) && (tmp_band_freq <= band_bank[(i * 3) + 1])) { band = (band_bank[(i * 3) + 2]); break;}
      // !
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

   //res2 = d10;

   IF(d100 != old_d100)
   {
      load_100hz(d100+1);
      old_d100 = d100;
   }

   
   #ifdef debug
   puts("tuned PLL ! ");
   printf("display frequency :  % ld\r\n", calc_frequency);
   printf("tuned frequency :  % ld\r\n", offset_frequency);

   #endif
}

int8 parse_cat ()
{
   INT8 byte4_upper = ((buffer[3] >> 4) & 0xF);
   INT8 byte4_lower = buffer[3] & 0xF;
   INT8 byte3_upper = ((buffer[2] >> 4) & 0xF);
   INT8 byte3_lower = buffer[2] & 0xF;
   INT8 byte2_upper = ((buffer[1] >> 4) & 0xF);
   INT8 byte2_lower = buffer[1] & 0xF;
   INT8 byte1_upper = ((buffer[0] >> 4) & 0xF);
   INT8 byte1_lower = buffer[0] & 0xF;

   SWITCH(buffer[4])
   {
      CASE 0x01: buttonres(9); break;
      CASE 0x02: buttonres(8); break;
      CASE 0x03: buttonres(7); break;
      CASE 0x04: buttonres(6); break;
      CASE 0x05: buttonres(5); break;
      CASE 0x06: buttonres(4); break;
      CASE 0x07: buttonres(3); break;
      CASE 0x08: buttonres(2); break;
      CASE 0x09: buttonres(1); break;
      
      CASE 0x0A: dmhz = (byte4_lower * 10) + (byte3_upper); d100k = byte3_lower; d10k = byte2_upper; d1k = byte2_lower; d100 = byte1_upper; d10 = byte1_lower; load_10hz(d10);break;
      CASE 0x0B: buttonres(11); break;
      
      CASE 0x0F: dmhz = (byte1_upper * 10) + (byte1_lower); d100k = byte2_upper; d10k = byte2_lower; d1k = byte3_upper; d100 = byte3_lower; d10 = byte4_upper; break;
      CASE 0xFF: reset_cpu(); break;
   }

   command_received = 0;
   RETURN 1;
}

void max_check()
{
   IF((dmhz >= 30) && (d100k>= 0) && (d10k>= 0) && (d1k>= 0) && (d100>= 0)) {dmhz = 30; d100k = 0; d10k = 0; d1k = 0; d100 = 0; d10 = 0;}
   IF((dmhz == 0) && (d100k<=5)) {dmhz = 0; d100k = 5; d10k = 0; d1k = 0; d100 = 0; d10 = 0;}
}

int8 tx_check()
{
   INT8 res;

   IF(sl)
   {
      IF( ! transmitting)
      {
         IF(tx_mode)
         {
            transmitting = 1;
            buttonres(5);
            res = 1;
         }
      }

      
      IF(transmitting)
      {
         IF( ! tx_mode)
         {
            transmitting = 0;
            buttonres(5);
            res = 1;
         }
      }
   }

   
   IF(cl)
   {
      IF( ! transmitting)
      {
         IF(tx_mode)
         {
            save_vfo(active_vfo);
            transmitting = 1;
            load_clarifier();
            res = 1;
         }
      }

      
      IF(transmitting)
      {
         IF( ! tx_mode)
         {
            transmitting = 0;
            load_vfo(active_vfo);
            res = 1;
         }
      }
   }

   
   RETURN res;
}

int8 tx_oob_check(INT16 freq)
{
   INT8 valid = 0;

   IF( ! gen_tx)
   {
      IF(tx_mode)
      {
         IF( ! gtx)
         {
            for(INT i  = 0; i < 10; i++)
            {
               IF((freq  >= blacklist[i * 2])&&(freq <= blacklist[(i * 2) + 1])){valid = 1; break; }
            }

            IF( ! valid)
            {
               Q64(4); Q64(0);
               WHILE(tx_mode)set_tx_inhibit(1);
            }

            gtx = 1;
         }
      }

      ELSE
      IF(gtx)
      {
         set_tx_inhibit(0);
         gtx = 0;
      }
   } ELSE valid = 1;

   RETURN valid;
}

int8 dial_moved()
{
   static int8 res2;
   int8 res1;
   res1 = read_counter();
   if(res1 != res2)
   {
   res2 = res1;
   return 1;
   }
   return 0;
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
   IF(pb0)gen_tx  = 0; // //widebanded?
   ELSE gen_tx = 1;
   k1 = 0;
   enable_interrupts(INT_rda); //toggle interrupts to ensure serial is ready
   enable_interrupts(global); //enable interrupts FOR CAT
   PLL_REF();
   for(INT i  = 0; i < 10; ++i)
   {
      dmhz = (band_bank[i * 3] / 10); d100k = (band_bank[i * 3] -((band_bank[i * 3] / 10) * 10)); d10k = 0; d1k = 0; d100 = 0; d10 = 0;
      save_vfos_a(i); save_vfos_b(i);
   }

   FOR(i  = 0; i <= 8; ++i)
   {
      dmhz = 7; d100k = 0; d10k = 0; d1k = 0; d100 = 0; d10 = 0;
      save_mem_channel(i);
   }

   band = 3; active_vfo = 0; mr_mode = 0; mem_channel = 1;
   load_vfo(active_vfo);
   dl = 0; cl = 0; sl = 0; dcs = get_dcs(); pms = 0;
   send_disp_buf();
   update_pll();
   
   INT8 res = 0;
   int16 btnrefresh;
         
            
   WHILE(true)
   {
      IF(sw_pms)
      {
         WHILE(sw_pms){}
         
         IF(pms)
         {
            pms = 0;
            mr_mode = temp_mr;
         }

         
         ELSE
         {
            pms = 1;
            temp_mr = mr_mode;
            mr_mode = 1;
            res = 1;
         }

         
      }

      
      IF( ! pms)
      {
         
         if(!dial_moved()) ++btnrefresh; else btnrefresh = 0;
         IF( ! res)
         {
            IF(command_received){res  = parse_cat(); }
         }

         
         
         IF( ! res)
         {
            if(btnrefresh > refresh)
            {
               btnrefresh = 0;
               res = buttons();
               IF(res){Q64(4); q64(0); buttonres(res); }
            }
         }

         
         IF( ! res)
         {
            res = mic_buttons();
         }

         
         IF( ! res)
         {
            IF( ! dial_clk)
            {
               dir = dial_dir;
               
               IF(dial_dir)d100 += 1; else d100 -=  1;
               res = 1;
            }
         }

         
         IF( ! res)res = tx_check();
         
         tx_oob_check((dmhz * 10) + (d100k));
         
         
         IF(res)
         {
            get_freq();
            max_check();
            
            send_disp_buf();
            update_PLL();
            res = 0;
         }
      }

      ELSE
      {
         
         IF( ! stopped)
         {
            delay_ms(100);

            IF( ! squelch_open)
            {
               IF(mem_channel < 8)++mem_channel; else mem_channel = 1;
               load_mem_channel(mem_channel);
            }
         }

         
         send_disp_buf();
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
                  
                  IF(sw_pms)
                  {
                     WHILE(sw_pms){}
                     pms = 0;
                     i = 2000;
                  }

                  delay_ms(1);
               }

               stopped = 0;
            } ELSE stopped = 1;
         }

      }
   }
}

