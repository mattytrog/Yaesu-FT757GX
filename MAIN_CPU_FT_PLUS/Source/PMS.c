

int8 program_mem_scan(int1 man_tune, INT32 curr_freq)
{
   INT8 curr_mem_ch, res, limit;
   INT1 counterstart, stopped, dir; 
   INT1 flash = 1;
   INT16 counter, counter1, counter2, counter2max;
   INT32 freq1, freq2, start_freq, end_freq;
   INT8 state;
   int8 scanstate;
   int8 incstate;
   int8 dwell;
   IF (man_tune) state = 0; else state = get_state ();
   IF (sw_500k) state += 10;
   INT8 vgrid = 15;
   
   switch(state)
   {
   case 0: scanstate = 0; break;
   case 1: scanstate = 1; break;
   case 2: scanstate = 1; break;
   case 3: scanstate = 3; break;
   case 4: scanstate = 4; break;
   case 11: scanstate = 2; break;
   case 12: scanstate = 2; break;
   case 13: scanstate = 5; break;
   case 14: scanstate = 4; break;
   }
   
   //gather frequencies based on rig state
   if(scanstate == 0)
   {
   if(!curr_freq) freq1 = 50000; else freq1 = curr_freq; 
   freq2 = max_freq;
   vgrid = 15;
   }
   if (scanstate == 1)
   {
         
      freq1 = band_bank[band];
      IF (band < 10)
      {
      freq2 = band_bank[band+1];
      }
      else 
      {
      freq2 = band_bank[0];
      }
      if(active_vfo == 0) vgrid = 1;
      if(active_vfo == 1) vgrid = 12;
      if(!curr_freq) curr_freq = freq1; 
      curr_mem_ch = 0;
      dwell = VFO_dwell_time;
   }
   IF (scanstate == 2)
      {
         freq1 = band_bank[band];
         freq2 = band_bank_edge[band];
         if(active_vfo == 0) vgrid = 1;
         if(active_vfo == 1) vgrid = 12;
         curr_freq = freq1;
         curr_mem_ch = 0;
         dwell = VFO_dwell_time;
      }
   if (scanstate == 3)
      {  
         curr_mem_ch = 0;
         freq1 = load_mem_ch_f (curr_mem_ch);
         if (curr_mem_ch >= 14) freq2 = load_mem_ch_f (0);
         else freq2 = load_mem_ch_f (curr_mem_ch + 1);
         curr_freq = freq1;
         vgrid = 2;
         dwell = VFO_dwell_time;
      }
   if (scanstate == 4)
      {
         if(cb_region == 2) limit = 80; else limit = 40;
         curr_mem_ch = cb_channel;
         freq1 = load_cb(curr_mem_ch, cb_region);
         freq2 = max_freq;
         curr_freq = freq1;
         vgrid = 2;
         dwell = CB_dwell_time;
      }
   if (scanstate == 5)
      {  
         limit = 14;
         curr_mem_ch = mem_channel;
         freq1 = load_mem_ch_f (curr_mem_ch);
         freq2 = max_freq;
         curr_freq = freq1;
         vgrid = 2;
         dwell = MR_dwell_time;
      }
   
   switch (scanstate)
   {
   
      case 0: incstate = 0; break;
      case 1: incstate = 1; break;
      case 2: incstate = 1; break;
      case 3: incstate = 1; break;
      case 4: incstate = 2; break;
      case 5: incstate = 3; break;
   }
   
   if(incstate !=0)
   {
   if (freq1 == freq2){pms = 0; errorbeep (2); RETURN 0; }
   }
   IF (freq1 < freq2){start_freq = freq1; end_freq = freq2;dir = 1; }
   IF (freq1 > freq2){start_freq = freq2; end_freq = freq1;dir = 0; }
   counter2max = 100;
   while(true)
   {
      if(sw_pms) { WHILE (sw_pms){} pms = 0; break;}
      if(incstate == 0) stopped = 0;
      else
      {
         if(squelch_open) stopped = 1;
      }
      
      IF (counterstart) ++counter;
      ++counter2; ++counter1;
      
      IF (counter2 > counter2max)
      {
         IF (flash) flash = 0; else flash = 1;
         counter2 = 0;
      }
      
      if(incstate == 0)
      {
         switch(flash)
         {
            case 1: VFD_data (0xFF, 0xFF, curr_freq, 0xFF, 1,0,0, 1) ; break;
            case 0: VFD_data (0xFF, 0xFF, 0, 15, 0,0,0, 1) ; break;
         }
      }
      
      if(incstate == 1)
      {
         switch(flash)
         {
            case 1: VFD_data (vgrid, 0xFF, curr_freq, 0xFF, 0,0,0, 1) ; break;
            case 0:  VFD_data (0xFF, 0xFF, curr_freq, 0xFF, 0,0,0, 1) ; break;
         }
      }
      
      if(incstate == 2)
      {
         switch(flash)
         {
            case 1: IF (flash) VFD_data (2, 0xFF, curr_mem_ch, 0xFF, 0,0,2, 1) ; break;
            case 0: VFD_data (0xFF, 0xFF, curr_mem_ch, 0xFF, 0,0,2, 1);  break; 
         }  
      }
      
      if(incstate == 3)
      {
         switch(flash)
         {
            case 1: VFD_data (2, 0xFF, curr_freq, curr_mem_ch, 0,0,0, 1) ; break;
            case 0: VFD_data (0xFF, 0xFF, curr_freq, curr_mem_ch, 0,0,0, 1) ; break;
         }
      }
      
      if(incstate == 2) curr_freq = load_cb(curr_mem_ch, cb_region);
      if(incstate == 3) curr_freq = load_mem_ch_f(curr_mem_ch);
      update_PLL(curr_freq);
      tx_oob_check(curr_freq);
      
      
      if(!stopped)
      {
         if(incstate == 0)
         {
            res = buttons (2);
            
            IF (res){ counter2 = 0; flash = 1; VFD_data (0xFF, 0xFF, curr_freq, 0xFF, 1,0,0, 1) ; }
            
            IF ( (res == 3)|| (res == 11)) curr_freq += 100;
            IF ( (res == 2)|| (res == 12)) curr_freq -= 100;
            if(curr_freq > max_freq) curr_freq = max_freq;
            if(curr_freq < min_freq) curr_freq = min_freq;
            IF ( (res == 5)|| (res == 6) || (res == 13)) {frequency = curr_freq; pms = 0; break; }
         
         }
         else
         {
            if(counter1 > dwell)
            {
               if(incstate == 1) 
               {
               if(dir) curr_freq += 10; 
               else curr_freq -= 10;
               if(curr_freq < start_freq) curr_freq = end_freq;
               else if (curr_freq > end_freq) curr_freq = start_freq;
               }
               if(incstate == 2) if(curr_mem_ch < limit) ++curr_mem_ch; else curr_mem_ch = 0;
               if(incstate == 3) if(curr_mem_ch < limit) ++curr_mem_ch; else curr_mem_ch = 0;
               counter1 = 0;
            }
            }
      }
      
      if(stopped)
      {
         IF (!squelch_open) counterstart = 1; else{counterstart = 0; counter = 0; }
         IF (counter > scan_pause_count) {counterstart = 0; stopped = 0; counter = 0; }
         res = buttons(2);
         IF ( (res == 3)|| (res == 11))
         {
            if(incstate == 1) curr_freq += 10;
            if(incstate == 2) if(curr_mem_ch < limit) ++curr_mem_ch; else curr_mem_ch = 0;
            if(incstate == 3) if(curr_mem_ch < limit) ++curr_mem_ch; else curr_mem_ch = 0;
            stopped = 0;
         }
         IF ( (res == 2)|| (res == 12))
         {
            if(incstate == 1) curr_freq -= 10;
            if(incstate == 2) if(curr_mem_ch > 0) --curr_mem_ch; else curr_mem_ch = limit;
            if(incstate == 3) if(curr_mem_ch > 0) --curr_mem_ch; else curr_mem_ch = limit;
            stopped = 0;
         }
         if(incstate == 1)
         {
         if(curr_freq < start_freq) curr_freq = end_freq;
         else if (curr_freq > end_freq) curr_freq = start_freq;
         }
         if(sw_pms) { WHILE (sw_pms){} pms = 0; break;}
      }
      
      
      if(pms == 0) break;
   }
   if(incstate == 0) frequency = curr_freq;
   if(squelch_open)
   {
   frequency = curr_freq;
   if(incstate == 2) {cb_channel = curr_mem_ch; }
   if(incstate == 3) {mem_channel = curr_mem_ch; }
   get_state();
   }
   return 1;
}
