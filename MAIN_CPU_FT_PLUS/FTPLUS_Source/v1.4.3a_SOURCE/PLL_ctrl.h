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



int32 update_PLL(INT32 calc_frequency)
{
// We only need to update, if there is a new NCODE to send, ie when new frequency is requested. We don't touch ref latches(6,5,4), so no need to resend
//if PLL update is requested for the SAME frequency, you are sent on your way and bounced back whence you came lol
      
      int32 offset_frequency; //preserve original frequency incase we need it late
      if(calc_frequency < min_freq) {calc_frequency = max_freq;}
      if(calc_frequency > max_freq) {calc_frequency = min_freq;}
      
      offset_frequency = calc_frequency;
      STATIC int16 old_khz_freq;
      STATIC int32 old_band_freq;
      STATIC int8 old_PLLband;
      STATIC int8 old_d100h;
      STATIC int8 old_d10h;
      static int32 oldcheck;
      int32 newcheck = calc_frequency;
      
      if(newcheck == oldcheck) return newcheck;
      
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
      
      
      dmhz = 0;while(tmp_frequency > 99999){tmp_frequency -= 100000; dmhz+=1;}
      d100k = 0;while(tmp_frequency > 9999){tmp_frequency -= 10000; d100k+=1;}
      d10k = 0;while(tmp_frequency > 999){tmp_frequency -= 1000; d10k+=1;}
      d1k = 0;while(tmp_frequency > 99){tmp_frequency -= 100; d1k+=1;}
      d100h = 0;while(tmp_frequency > 9){tmp_frequency -= 10; d100h+=1;}
      d10h = 0;while(tmp_frequency > 0){tmp_frequency -= 1; d10h+=1;}
      
      int32 tmp_band_freq= (offset_frequency / 10000);  
      int16 tmp_khz_freq = ((d100k * 100) + (d10k * 10) + (d1k));

      if(tmp_khz_freq >= 500) tmp_khz_freq -=500; 
      tmp_khz_freq +=560;
      
      int PLL1_NCODE_L3 = 0; //empty latch 48
      int PLL1_NCODE_L2 = 0; //            32
      int PLL1_NCODE_L1 = 0; //            16
      int PLL1_NCODE_L0 = 0;//              0
      int PLL2_NCODE_L3 = 0; //empty latch 48
      int PLL2_NCODE_L2 = 0; //empty latch 32
      int PLL2_NCODE_L1 = 0; //            16
      int PLL2_NCODE_L0 = 0; //             0
      
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
       if(old_d100h != d100h)
       {
       if(old_d100h < d100h) load_100hz(d100h+1);
       if(old_d100h > d100h) load_100hz(d100h+1);
       old_d100h = d100h;
       }
       
       if(old_d10h != d10h)
       {
       if(old_d10h < d10h) load_10hz(d10h);
       if(old_d10h > d10h) load_10hz(d10h+1);
       old_d10h = d10h;
       }
       res2 = read_counter();
       
      oldcheck = newcheck;
      #ifdef debug
      puts("tuned PLL!");
      printf ("display frequency : %ld\r\n", calc_frequency);
      printf ("tuned frequency : %ld\r\n", offset_frequency);
      #endif
      return calc_frequency;
}
