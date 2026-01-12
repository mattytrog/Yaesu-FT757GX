const int32 band_bank[11] = 
{
   50000, 180000, 350000, 700000, 1010000,
   1400000, 1800000, 2100000, 2400000, 2800000, 3000000
};

#ifndef include_custom_filter_selection
const int32 PLL_band_bank[30] =
{
   //lower limit, upper limit, result
   0, 149999, 1,
   150000, 249999, 1,
   250000, 399999, 2,
   400000, 749999, 3,
   750000, 1049999, 4,
   1050000, 1449999, 5,
   1450000, 1849999, 6,
   1850000, 2149999, 7,
   2150000, 2499999, 8,
   2500000, 9999999, 9
};
#else
int32 PLL_band_bank[30] =
{
   //lower limit, upper limit, result
   0, 149999, 1,
   150000, 249999, 1,
   250000, 399999, 2,
   400000, 749999, 3,
   750000, 1049999, 4,
   1050000, 1449999, 5,
   1450000, 1849999, 6,
   1850000, 2149999, 7,
   2150000, 2499999, 8,
   2500000, 9999999, 9
};
#endif

//!const int32 PLL_band_bank2[21] =
//!{
//!   //lower limit, upper limit, result
//!   0, 1,
//!   150000, 1,
//!   250000, 2,
//!   400000, 3,
//!   750000, 4,
//!   1050000, 5,
//!   1450000, 6,
//!   1850000, 7,
//!   2150000, 8,
//!   2500000, 9,
//!   3200000
//!};

#define cycles_delay 2

void strobe_Q64(INT8 val)
{
   PORTA = 0;
   delay_cycles(cycles_delay);
   switch(val)
   {
      case 0: PORTA = 0; delay_cycles(cycles_delay); break;
      case 1: PORTA = 8; delay_cycles(cycles_delay); break; //PLL1
      case 2: PORTA = 4; delay_cycles(cycles_delay); break; //counter preset
      case 3: PORTA = 12; delay_cycles(cycles_delay); break; //bpf data Q69/Q70
      case 4: PORTA = 2; delay_us(20); break; //beep
      case 5: PORTA = 10; delay_cycles(cycles_delay); break; //DL
      case 6: PORTA = 6; delay_cycles(cycles_delay); break; //inh
      case 7: PORTA = 14; delay_cycles(cycles_delay); break; //CAT TX
   } 
   PORTA = 0;
   delay_cycles(cycles_delay);
}

void default_Q64(INT8 val)
{
   switch(val)
   {
      default: PORTA = 0; break;
      case 1: PORTA = 10; break;
      case 2: PORTA = 14; break;
   }
}

void PLL1(int8 data){PORTB = data; strobe_Q64(1);}
void PLL2(int8 data){PORTB = data; BUS_DATA128 = 1; delay_cycles(cycles_delay); BUS_DATA128 = 0;}
void counter_preset_enable(){strobe_Q64(2);}
void banddata(){strobe_Q64(3);;}
void beep(){strobe_Q64(4);}
void errorbeep(INT8 beeps)
{
   for (INT8 i = 0; i < beeps; ++i)
   {beep(); delay_ms(200);}
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
   INT8 res = 0;
   if(pind4) res +=8;
   if(pind5) res +=4;
   if(pind6) res +=2;
   if(pind7) res +=1;
   RETURN res;
}

void load_10hz(INT8 val)
{
   INT8 loc100 = 112;
   PORTB = loc100 + val;
   counter_preset_enable();
}
void load_100hz(INT8 val)
{
   //save_port_b();
   INT8 loc100 = 112;
   PORTB = loc100 + val;
   //restore_port_b();
}

void quick_disp_flash(int8 beeps)
{
#ifdef include_display
   for (INT8 i = 0; i < beeps; ++i)
   {
   strobe_Q64(4); disp_buf[12] = mem_channel; delay_ms(100); 
   disp_buf[12] = 15; delay_ms(100);
   }

#endif
}

#ifdef include_custom_PLL_REF
//for future app support
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
   PLL1((6<<4) + (PLL1REF >> 8));    //Latch6 = Bit2 = address + (5 in hex = 5). Could write this as 96 + 5
   PLL1((5<<4) + ((PLL1REF >> 4) & 0xF));    //Latch5 = Bit1 = address + (13 in hex = D)or 80 + 13
   PLL1((4<<4) + (PLL1REF & 0xF));    //Latch4 = Bit0 = address + (12 in hex = C)or 64 + 12
   //5DC in hex = 1500
   PLL2((6<<4) + (PLL2REF >> 8));     //Latch6 = Bit2 address + (0 in hex = 0)or 96 + 0
   PLL2((5<<4) + ((PLL2REF >> 4) & 0xF));    //Latch5 = Bit1 address + (1 in hex = 1)or 80 + 1
   PLL2((4<<4) + (PLL2REF & 0xF));    //Latch4 = Bit0 address + (14 in hex = E)or 64 + 14
   //01E in hex = 30
}
#else
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
   PLL1((6<<4) + 0x5);    //Latch6 = Bit2 = address + (5 in hex = 5). Could write this as 96 + 5
   PLL1((5<<4) + 0xD);    //Latch5 = Bit1 = address + (13 in hex = D)or 80 + 13
   PLL1((4<<4) + 0xC);    //Latch4 = Bit0 = address + (12 in hex = C)or 64 + 12
   //5DC in hex = 1500
   PLL2((6<<4) + 0);     //Latch6 = Bit2 address + (0 in hex = 0)or 96 + 0
   PLL2((5<<4) + 0x1);    //Latch5 = Bit1 address + (1 in hex = 1)or 80 + 1
   PLL2((4<<4) + 0xE);    //Latch4 = Bit0 address + (14 in hex = E)or 64 + 14
   //01E in hex = 30
}
#endif

int32 oob_check_freq;
void set_PLL(INT32 offset_frequency, int1 force, int8 force_BPF)
{
// We only need to update, if there is a new NCODE to send, ie when new frequency is requested. We don't touch ref latches(6,5,4), so no need to resend
//if PLL update is requested for the SAME frequency, you are sent on your way and bounced back whence you came lol
       //if(calc_frequency > max_freq) calc_frequency = max_freq;
      
      oob_check_freq = offset_frequency;
      STATIC int16 old_khz_freq;
      STATIC int16 old_band_freq;
      STATIC int8 old_PLLband;
      STATIC int8 old_d10h;
      
      int16 tmp_band_freq = 0;
      int16 tmp_khz_freq = 0;
      int8 d100h, d10h;
      
      if(force)
      {
      old_khz_freq = 0;
      old_band_freq = 0;
      old_PLLband = 0;

      old_d10h = 0;
      }
      
      //The correct filtering is selected by having a value on PORTB (1 - 9), strobed to Q64, which in turn strobes to Q69, which then decodes the value and sends to Q70
      //This is left as an option to be changed in Update_PLL. Default value is 0xFF which is automatic. Or can be forced by replacing with a value (1-9), <actually 0-9>
      //Pointless going higher than 9 because all inputs of Q70 will be high if value >9, leading to no filter being selected and loss of tx/rx
      if(force_BPF == 0xFF)
      {
         for (int i = 0; i < 10; i++)
         {
            #ifndef include_custom_filter_selection
            if((offset_frequency >= PLL_band_bank[(i * 3)]) && (offset_frequency <= PLL_band_bank[(i * 3) + 1])) { PLLband = (PLL_band_bank[(i * 3) + 2]); break;}
            //if((offset_frequency >= PLL_band_bank2[(i * 2)]) && (offset_frequency <= (PLL_band_bank2[(i * 2) + 2]) - 1)) { PLLband = (PLL_band_bank2[(i * 2) + 1]); break;}
            #else
            if((offset_frequency >= PLL_band_bank[(i * 3)]) && (offset_frequency <= PLL_band_bank[(i * 3) + 1])) { PLLband = read8(i + 0xD0); break;}
            #endif

         }
         if(PLLband != old_PLLband)
         {
         old_d10h = 0;
         PORTB = PLLband; banddata(); 
         old_PLLband = PLLband;
         }
      }
      else
      {
         PLLband = force_BPF;
         if(PLLband != old_PLLband)
         {
         old_d10h = 0;
         PORTB = PLLband; banddata(); 
         old_PLLband = PLLband;
         }
      }

#ifdef include_offset_programming
      if(!setup_offset)
      {
      offset = load_offset_f();
      if (offset >= 10000) offset_frequency += (offset - 10000);
      else offset_frequency -= offset;
      }

#endif    

      int32 tmp_frequency = offset_frequency;
      while(tmp_frequency > 99999)     {tmp_frequency -= 100000;  tmp_band_freq += 10;}
      while(tmp_frequency > 9999)      {tmp_frequency -= 10000;   tmp_band_freq +=1;   tmp_khz_freq += 100;}
      while(tmp_frequency > 999)       {tmp_frequency -= 1000;    tmp_khz_freq += 10;}
      while(tmp_frequency > 99)        {tmp_frequency -= 100;     tmp_khz_freq += 1;}
      d100h = 0;  while(tmp_frequency > 9)         {tmp_frequency -= 10;      d100h+=1;}
      d10h = 0;   while(tmp_frequency > 0)         {tmp_frequency -= 1;       d10h+=1;}

      if(tmp_khz_freq >= 500) tmp_khz_freq -=500; 
      tmp_khz_freq +=560;
      
      int8 PLL1_NCODE_L3 = 0; //empty latch 48
      int8 PLL1_NCODE_L2 = 0; //            32
      int8 PLL1_NCODE_L1 = 0; //            16
      int8 PLL1_NCODE_L0 = 0;//              0
      int8 PLL2_NCODE_L3 = 0; //empty latch 48
      int8 PLL2_NCODE_L2 = 0; //empty latch 32
      int8 PLL2_NCODE_L1 = 0; //            16
      int8 PLL2_NCODE_L0 = 0; //             0
      
      if(tmp_khz_freq != old_khz_freq)
      {
      //841 in hex is 349. So latch2 = 3, latch 1 = 4, latch 0 = 9;
      PLL1_NCODE_L2 = (tmp_khz_freq >> 8); //first digit
      PLL1_NCODE_L1 = ((tmp_khz_freq >> 4) & 0xF);//middle digit
      PLL1_NCODE_L0 = (tmp_khz_freq & 0xF);//final digit
      //PLL1 end
      PLL1((3<<4) + PLL1_NCODE_L3);
      PLL1((2<<4) + PLL1_NCODE_L2);
      PLL1((1<<4) + PLL1_NCODE_L1);
      PLL1(0 + PLL1_NCODE_L0);
      old_khz_freq = tmp_khz_freq;
      }

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
   
          
          
          PLL2((3<<4) + PLL2_NCODE_L3);
          PLL2((2<<4) + PLL2_NCODE_L2); // Prepare latch
          PLL2((1<<4) + PLL2_NCODE_L1);
          PLL2(0 + PLL2_NCODE_L0);
          
          old_band_freq = tmp_band_freq;
       }
       //res2 = d10;
       
       

       if(old_d10h != d10h)
       {
       load_10hz(d10h);
       old_d10h = d10h;
       }

       load_100hz(d100h);

       res1 = read_counter();
       res2 = res1;
      
}

int8 calc_band(INT32 frequency)
{
   for(INT i  = 0; i < 10; i++)
   {
      IF((frequency >= band_bank[i])&&(frequency < band_bank[i + 1]))break;
   }

   RETURN i;
}

#ifdef include_app_selectable_minmax
void update_PLL(INT32 &offset_frequency, int8 force_BPF)
{

if(offset_frequency > maximum_freq) offset_frequency = maximum_freq;
if(offset_frequency < minimum_freq) offset_frequency = minimum_freq;
set_PLL (offset_frequency, 0, force_BPF);

}
#else
void update_PLL(INT32 &offset_frequency, int8 force_BPF)
{

if(offset_frequency > max_freq) offset_frequency = max_freq;
if(offset_frequency < min_freq) offset_frequency = min_freq;
set_PLL (offset_frequency, 0, force_BPF);

}
#endif
