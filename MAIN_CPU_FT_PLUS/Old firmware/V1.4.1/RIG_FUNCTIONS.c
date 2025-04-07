
int8 tx_oob_check(INT32 freq);

#define cycles_delay 2

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
//!Q7 - Kenwood CAT TX
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
void Q64(INT8 val)
{
   //Q64 is Octal (000 - 111) (0 - 7) (Bit 8, port A is actually dial clock)
   //Bits are reversed on the bus.
   //A0 is dial clock (or spare). A1 = LSB. Only 8 values. Hand code FOR speed.
   //eg Beep is binary 4... could just DO beep() = 2, but would not actually be
   //correct on schematic, though would work fine
   //Correction by Matthew 19/2/25
   IF(val == 0) PORTA = 0;
   else IF(val == 7) PORTA = 14; //0111 = 1110
   else IF(val == 6) PORTA = 6;  //0110 = 0110
   else IF(val == 5) PORTA = 10; //0101 = 1010
   else IF(val == 4) PORTA = 2;  //0100 = 0010
   else IF(val == 3) PORTA = 12; //0011 = 1100
   else IF(val == 2) PORTA = 4;  //0010 = 0100
   else IF(val == 1) PORTA = 8;  //0001 = 1000
   Q64_val = val;
}

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

void errorbeep(INT8 beeps)
{
   for (INT8 i = 0; i < beeps; ++i)
   {beep(); delay_ms(200);}
}

void set_dl()
{
   IF(dl  == 1)Q64(5);
   IF(dl  == 0)Q64(0);
}

void set_tx_inhibit(INT res)
{
   IF(res == 1)Q64(6);
   IF(res == 0)Q64(0);
}

int8 get_dcs()
{
   INT8 res = 15;
   for(INT8 i  = 0; i < 8; i++)
   {
      IF((dl == dcs_res[(i * 4) + 1])&&(cl == dcs_res[(i * 4) + 2])&&(sl == dcs_res[(i * 4) + 3])){res = dcs_res[i * 4]; break; }
   }

   set_dl();
   save_dcs_n(res);
   RETURN res;
}

void set_dcs(INT8 res)
{
   for (INT8 i = 0; i < 8; i++)
   {
      IF(res == dcs_res[(i * 4)]){dl = dcs_res[(i * 4) + 1]; cl = dcs_res[(i * 4) + 2]; sl = dcs_res[(i * 4) + 3]; break;}
   }

   set_dl();
   save_dcs_n(res);
}

void cat_transmit(INT1 tx_request)
{
   #ifdef debug_cat_tx

   IF (tx_request == 1) {Q64(5); dl = 1; dcs = get_dcs();} else {Q64(0); dl = 0; dcs = get_dcs();}
   #ELSE

   IF (tx_request == 1) Q64(7); else Q64(0);
   #endif

}

//===========================================================================================================================================================================================================
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
   INT8 res = 0;
   IF(pind4) res +=8;
   IF(pind5) res +=4;
   IF(pind6) res +=2;
   IF(pind7) res +=1;
   RETURN res;
}

void load_10hz(INT8 val)
{
   save_port_b();
   INT8 loc100 = 112;
   PORTB = loc100 + val;
   counter_preset_enable();
   restore_port_b();
}

void load_100hz(INT8 val)
{
   //save_port_b();
   INT8 loc100 = 112;
   PORTB = loc100 + val;
   //restore_port_b();
}

void PLL_REF()
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
   PORTB = (6<<4) + 0x5; PLL1();    //Latch6 = Bit2 = address + (5 in hex = 5). Could write this as 96 + 5
   PORTB = (5<<4) + 0xD; PLL1();    //Latch5 = Bit1 = address + (13 in hex = D)or 80 + 13
   PORTB = (4<<4) + 0xC; PLL1();    //Latch4 = Bit0 = address + (12 in hex = C)or 64 + 12
   //5DC in hex = 1500
   PORTB = (6<<4) + 0; PLL2();     //Latch6 = Bit2 address + (0 in hex = 0)or 96 + 0
   PORTB = (5<<4) + 0x1; PLL2();    //Latch5 = Bit1 address + (1 in hex = 1)or 80 + 1
   PORTB = (4<<4) + 0xE; PLL2();    //Latch4 = Bit0 address + (14 in hex = E)or 64 + 14
   //01E in hex = 30
}

//We have already sent latches 6,5 and 4, which are reference bits(look above at PLL_REF)
//This purely updates latches 3,2,1 and 0. Latch 3 isn't used, so we could put this in the PLL_REF graveyard IF we wanted to (ie sent once)
//Base frequency is loaded, then we load any offset. Once we know, we find out what LPF "band" should be selected and strobed. Original calc_frequency is unchanged remember
//Don't confuse these "bands" with the standard Yaesu bands we select when pressing BAND UP/DOWN
int8 calc_band(INT32 frequency)
{
   for(INT i  = 0; i < 10; i++)
   {
      IF((frequency >= band_bank[i])&&(frequency < band_bank[i + 1]))break;
   }

   RETURN i;
}

void update_PLL(INT32 calc_frequency)
{
// We only need to update, if there is a new NCODE to send, ie when new frequency is requested. We don't touch ref latches(6,5,4), so no need to resend
//if PLL update is requested for the SAME frequency, you are sent on your way and bounced back whence you came lol
      STATIC int16 old_khz_freq;
      STATIC int32 old_band_freq;
      STATIC int8 old_PLLband;
      //STATIC int8 old_d100;
      static int32 oldcheck;
      int32 newcheck = calc_frequency;
      
      if(newcheck == oldcheck) return;
      int32 offset_frequency = calc_frequency; //preserve original frequency incase we need it late
      for (int i = 0; i < 10; i++)
      {
         if((offset_frequency >= PLL_band_bank[(i * 3)]) && (offset_frequency <= PLL_band_bank[(i * 3) + 1])) { PLLband = (PLL_band_bank[(i * 3) + 2]); break;}
      }
      if(PLLband != old_PLLband)
      {
      PORTB = PLLband; banddata(); 
      old_PLLband = PLLband;
      }

#ifdef include_offset_programming
      if(!setup_offset)
      {
      int32 offset = load_offset_f();
      if (offset >= 1000000) offset_frequency -= (offset - 1000000);
      else offset_frequency += offset;
      }

#endif    

      int32 tmp_frequency = offset_frequency;
      
      int32 dmhz, d100k, d10k, d1k, d100, d10;
      dmhz = 0;while(tmp_frequency > 99999){tmp_frequency -= 100000; dmhz+=1;}
      d100k = 0;while(tmp_frequency > 9999){tmp_frequency -= 10000; d100k+=1;}
      d10k = 0;while(tmp_frequency > 999){tmp_frequency -= 1000; d10k+=1;}
      d1k = 0;while(tmp_frequency > 99){tmp_frequency -= 100; d1k+=1;}
      d100 = 0;while(tmp_frequency > 9){tmp_frequency -= 10; d100+=1;}
      d10 = 0;while(tmp_frequency > 0){tmp_frequency -= 1; d10+=1;}
      
      int32 tmp_band_freq= (offset_frequency / 10000);  
      int16 tmp_khz_freq = ((d100k * 100) + (d10k * 10) + (d1k));

      if(tmp_khz_freq >= 500) tmp_khz_freq -=500; 
      tmp_khz_freq +=560;
      
      int PLL1_NCODE_L3 = 0; //empty latch 48
      int PLL1_NCODE_L2 = 0; //            32
      int PLL1_NCODE_L1 = 0; //            16
      int PLL1_NCODE_L0 = 0;//              0
      
      if(tmp_khz_freq != old_khz_freq)
      {
      //841 in hex is 349. So latch2 = 3, latch 1 = 4, latch 0 = 9;
      PLL1_NCODE_L2 = (tmp_khz_freq >> 8); //first digit
      PLL1_NCODE_L1 = ((tmp_khz_freq >> 4) & 0xF);//middle digit
      PLL1_NCODE_L0 = (tmp_khz_freq & 0xF);//final digit
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

      int PLL2_NCODE_L3 = 0; //empty latch 48
      int PLL2_NCODE_L2 = 0; //empty latch 32
      int PLL2_NCODE_L1 = 0; //            16
      int PLL2_NCODE_L0 = 0; //             0


      if(tmp_band_freq != old_band_freq)
      {
         if(PLLband < 6)
         {
         PLL2_NCODE_L1 = (((tmp_band_freq / 5) + 12) >> 4); 
         PLL2_NCODE_L0 = (((tmp_band_freq / 5) + 12) & 0xF);
         }
         else
         {
         PLL2_NCODE_L1 = (((tmp_band_freq / 5) -18) >> 4);
         PLL2_NCODE_L0 = (((tmp_band_freq / 5) -18) & 0xF);
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
       load_100hz(d100);
       load_10hz(d10);
       res2 = read_counter();
       
      oldcheck = newcheck;
      #ifdef debug
      puts("tuned PLL!");
      printf ("display frequency : %ld\r\n", calc_frequency);
      printf ("tuned frequency : %ld\r\n", offset_frequency);
      #endif
      
}

void program_offset()
{
   #ifdef include_offset_programming

   setup_offset = 1;
   INT32 offset_val = load_offset_f();

   IF(offset_val >= 1000000)
   {
      offset_val -= 1000000;
      dir = 1;
   }

   ELSE
   {
      dir = 0;
   }

   INT8 res = 1;
   INT8 btnres = 0;
   INT32 testfreq = frequency;
   cls();
   //GOTO start;

   WHILE(true)
   {
      //IF (offset_val > 1000) {offset_val = 1000; if(dir == 0) dir = 1; else dir = 0;}
      start:
      IF (res == 1)
      {
         IF(offset_val)
         {
            IF(dir)
            {
               testfreq = frequency - offset_val;
               VFD_data(0xFF, 0xFF, offset_val, 0xFF, 0,0,3,0);
            }

            ELSE
            {
               testfreq = frequency + offset_val;
               VFD_data(0xFF, 0xFF, offset_val, 0xFF, 0,0,2,0);
            }
         }

         ELSE VFD_data(0xFF, 0xFF, 1, 0xFF, 0,0,5,0);
         update_PLL(testfreq);
         res = 0;
      }

      btnres = buttons(1);
      IF (offset_val == -1) {offset_val = 1; if(dir == 0) dir = 1; else dir = 0;}
      IF(!dir)
      {
         IF((btnres == 2) || (btnres == 12)) {res = 1; offset_val -=1;}
         else IF((btnres == 3) || (btnres == 11)){res = 1; offset_val +=1;}
      }

      ELSE
      {
         IF((btnres == 2) || (btnres == 12)) {res = 1; offset_val +=1;}
         else IF((btnres == 3) || (btnres == 11)) {res = 1; offset_val -=1;}
      }

      IF(btnres == 1)
      {
         VFD_data(0xFF, 0xFF, frequency, 0xFF, 0,0,0,0);
         delay_ms(1000);
         btnres = 0;
         res = 1;
         GOTO start;
      }

      IF(btnres == 6)
      {
         save_offset_f(0);
         BREAK;
      }

      IF((btnres == 31) || (btnres == 13))
      {
         IF(dir) save_offset_f(1000000 + offset_val);
         IF(!dir) save_offset_f(offset_val);
         setup_offset = 0;
         BREAK;
      }

      res1 = read_counter();
      IF (misc_dial (offset_val,dir)) res = 1;
      IF (offset_val == -1) {offset_val = 1; if(dir == 0) dir = 1; else dir = 0;}
      IF(offset_val > 998) offset_val = 999;
      tx_oob_check(testfreq);
   }

   beep();

   #endif

}

