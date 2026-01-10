#INT_TIMER1
void t1_isr(void)
{
if(fl_cnt < 255) ++fl_cnt; else fl_cnt = 0;
if((fl_cnt) & 4) flash = 1; else flash = 0;
clear_interrupt(INT_TIMER1);
}

#define VFO_dwell_time 1                  // Delay after tuning to next frequency. Lower numbers = faster scanning speed, though may overshoot if too fast


#ifdef include_pms_basic

void pms_scan_basic()
{
   if(app_detected) {errorbeep(3); return;}
   temp_freq = storage_buffer[vfo + 1];
   int8 res;
   int1 upd_count = 0;
   int8 count = 0;
   while(true)
   {
      if(flash)
      {
         if(count < 255) 
         {
            if(!upd_count) ++count; upd_count = 1;
         }
         else count = 0;
#ifdef include_display
         VFD_data (vfo+1,15, temp_freq, mem_channel, 0);
#endif
      }
         else 
      {
#ifdef include_display
         VFD_data (15,15, temp_freq, mem_channel, 0);
#endif
         upd_count = 0;
      }
      update_PLL(temp_freq, 0xFF);
      res = buttons(1);
      if(squelch_open)
         {
            if(!stopped) {stopped = 1; count = 0;}
         }
         
      if(!stopped)
      {
         delay_us(VFO_dwell_time);
         temp_freq +=2;
         count = 0;
      }
      else
      {
         
         if((res == 11) || (res == 3)) temp_freq +=10;
         if((res == 12) || (res == 2)) temp_freq -=10;
         if(res == 33) temp_freq +=10;
         if(res == 32) temp_freq -=10;
         if((res) || (squelch_open)) count = 0;
         if(count > 3)
         {
            if(!squelch_open) stopped = 0;
            count = 4;
         }
      }
      if(temp_freq >= max_freq) temp_freq = min_freq;
      IF(!pms)
      {
         beep();
         delay_ms(200);
         
         break;
      } 
   }

   if(squelch_open) storage_buffer[vfo + 1] = temp_freq;


}
#endif

#ifdef include_pms_advanced

#define MR_dwell_time 200       
#define CB_dwell_time 50       

const int32 band_bank_edge[11] = 
{
   160000, 200000, 380000, 720000, 1020000,
   1435000, 1820000, 2145000, 2500000, 3000000, 3000000
};


//mode1 pms - VFO current band - start of next band
//mode2 pms + 500k curr band (between FCC freq)
//mode3 pms - MR - Curr channel to next channel, higher or lower
//mode4 pms + 500k MR - All MR channels (0 - 14)
//mode5 pms - CB Curr band
//mode6 pms - CB all bands
void pms_scan_advanced()
{

  if(app_detected) {errorbeep(3); return;}
  int32 freq1, freq2, curr_freq;
  int8 d_type, curr_vfo_band;
  int8 curr_channel;
  int8 pms_state = state;
  if(sw_500k) pms_state +=4;
  int8 count = 0;
  int8 old_region = 0;
  int8 old_channel = 0;
  int1 cb_upd = 0;
  int1 mr_upd = 0;
  int1 flash_en = 0;
  int1 blink = 0;
  switch(state)
  {
      case 1: curr_vfo_band = band1; break;
      case 2: curr_vfo_band = band2; break;   
  }

   if((pms_state == 1) || (pms_state == 2))
   {
      freq1 = band_bank[curr_vfo_band];
      if(curr_vfo_band == 9) freq2 = band_bank[0]; else freq2 = band_bank[curr_vfo_band + 1];
      curr_freq = storage_buffer[state];
      d_type = 1;
      flash_en = 1;
   }
   if(pms_state == 3)
   {
      freq1 = load_mem_ch_f(mem_channel);
      if(mem_channel == 14) freq2 = load_mem_ch_f(0); else freq2 = load_mem_ch_f(mem_channel+1);
      IF (freq1 < freq2) curr_freq = freq1;
      IF (freq1 > freq2) curr_freq = freq2;
      curr_channel = mem_channel;
      d_type = 1;
      flash_en = 1;
   }
#ifdef include_cb   
   if((pms_state == 4) || (pms_state == 8))
   {
      curr_channel = cb_channel;
      old_channel = cb_channel;
      old_region = cb_region;
      d_type = 2;
      curr_freq = load_cb(cb_channel, cb_region);
      flash_en = 1;
   }
#endif  
   if(pms_state == 7)
   {
      curr_channel = mem_channel;
      old_channel = mem_channel;
      d_type = 1;
      curr_freq = load_mem_ch_f(mem_channel);
      flash_en = 0;
   }
   
   
   if((pms_state == 5) || (pms_state == 6))
   {
      freq1 = band_bank[curr_vfo_band];
      freq2 = band_bank_edge[curr_vfo_band];
      curr_freq = storage_buffer[state];
      d_type = 1;
      flash_en = 1;
   }
      
   if (freq1 == freq2){pms = 0; errorbeep (2); RETURN; }
   IF (freq1 < freq2) dir = 1;
   IF (freq1 > freq2) dir = 0;
   
   int8 res;
   int1 upd_count = 0;
   
   while(true)
   {
      
      
      if(flash)
      {
         if(!upd_count) {++count; upd_count = 1;}
      }
      else 
      {
         if(count >= 255) count = 0;
         upd_count = 0;
      }
      if(flash_en) blink = flash; else blink = 1;
      switch(d_type)
      {
         case 1:
         switch(blink)
         {
            case 1: VFD_data (state, 0xFF, curr_freq, curr_channel, 0) ; break;
            case 0:  VFD_data (0xFF, 0xFF, curr_freq, curr_channel, 0) ; break;
         }
         break;
         
         case 2:
         switch(blink)
         {
            case 1: VFD_data (3, 0xFF, curr_channel, 15, 2) ; break;
            case 0:  VFD_data (0xFF, 0xFF, curr_channel, 15, 2) ; break;
         }
         break;
      }
      update_PLL(curr_freq, 0xFF);
      if(squelch_open) stopped = 1;
      res = buttons(1);
      if(!stopped)
      {
         switch(pms_state)
         {
            default:
            delay_us(VFO_dwell_time);
            if(dir) //freq1 is lower than freq2
            {
               
               if(curr_freq > freq2) curr_freq = freq1; else curr_freq += 2;
            }
            if(!dir) //freq2 is lower than freq1
            {
               
               if(curr_freq < freq2) curr_freq = freq1; else curr_freq -= 2;
            }
            break;
#ifdef include_cb            
            case 4:
            delay_ms(CB_dwell_time);
            if(curr_channel < 40) ++curr_channel; else curr_channel = 1;
            cb_upd = 1;
            break;
#endif            
            case 7:
            delay_ms(MR_dwell_time);
            if(curr_channel < 14) ++curr_channel; else curr_channel = 0;
            mr_upd = 1;
            break;
#ifdef include_cb            
            case 8:
            delay_ms(CB_dwell_time);
            if(curr_channel < 40) ++curr_channel; 
            else 
            { 
               if(cb_region < 7) cb_region +=1; else cb_region = 0;
               curr_channel = 1;
            }
            cb_upd = 1;
            break;
#endif            
            
         }
         
      }
      
      if(stopped)
      {
         
         if((res == 11) || (res == 3) || (res == 33)) 
         {
            switch(state)
            {
               default: curr_freq +=10; break;
               case 4: if(curr_channel < 40) ++curr_channel; else curr_channel = 1; cb_upd = 1; break;
               case 7: if(curr_channel < 14) ++curr_channel; else curr_channel = 0; mr_upd = 1; break;
#ifdef include_cb               
               case 8: if(curr_channel < 40) ++curr_channel;
               else 
               {
               if(cb_region < 7) cb_region +=1; else cb_region = 0;
               curr_channel = 1; 
               cb_upd = 1;
               }
               break;
#endif
            }
         }
         if((res == 12) || (res == 2) || (res == 32)) 
         {
            switch(state)
            {
               default: curr_freq -=10; break;
               case 4: if(curr_channel > 1) --curr_channel; else curr_channel = 40; cb_upd = 1; break;
               case 7: if(curr_channel > 0) --curr_channel; else curr_channel = 14; mr_upd = 1; break;
#ifdef include_cb
               case 8: if(curr_channel > 1) --curr_channel; 
               else 
               {
               if(cb_region > 0) cb_region -= 1; else cb_region = 7;
               curr_channel = 40; 
               cb_upd = 1;
               } 
               break;
#endif
            }
         }
         
         if((res) || (squelch_open)) count = 0;
         if(count > 3)
         {
            if(!squelch_open) stopped = 0;
            count = 4;
         }
      }
#ifdef include_cb
      if(cb_upd) {curr_freq = load_cb(curr_channel, cb_region); cb_upd = 0;}
#endif
      if(mr_upd) {curr_freq = load_mem_ch_f(curr_channel); mr_upd = 0;}
      IF(!pms)
      {
         beep();
         delay_ms(200);
         
         break;
      } 
      
      
      
   
   }
   if(squelch_open) 
   {
      if(state == 4) {cb_channel = curr_channel;}
      if(state == 3) {mem_channel = curr_channel;}
      storage_buffer[state] = curr_freq;
   }
   else
   {
      if(state == 4) {cb_channel = old_channel; cb_region = old_region; curr_freq = load_cb(cb_channel, cb_region);}
      if(state == 3) {mem_channel = old_channel; curr_freq = load_mem_ch_f(mem_channel);}
   }
   update_PLL(curr_freq, 0xFF);
}

#endif
