


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
   save32 (54, minimum_freq); save32 (55, maximum_freq);
   //save32 (3, 1010000);
   save8(vfo_n, 0); //Active VFO A / B
   save8(mode_n, 0); //Mem mode
   save8(mem_ch_n, 0); //default mem channel
   save8(fine_tune_n, 0); //Fine - tuning display (disabled by default)
   save8(dial_n,0); //dial_accel / type 0 = disabled, 1 = type1
   save8(dcs_n, 15) ;
   save8(band_offset_n,0) ;
   save8(cat_mode_n, default_cat_mode) ;
   if (load8(baud_n) == 0xFF) save8(baud_n,0);
   save8(dummy_mode_n, 49) ;
   save8(id_enable_n,0) ;
   save8(savetimer_n, 1);
   //save_cache_vfo_f (700000) ;
   save_cache_mem_mode_f (700000) ;
   save8(band_n,(calc_band (700000))) ;
   #ifdef include_cb

   save8(cb_ch_n, 19); //CB ch 1 - 80 on default channel
   save8(cb_reg_n, 0); //CB region 0 - CEPT, 1 UK, 2 80CH (1 - 80) - CEPT 1 - 40, UK 41 - 80
   #endif

   save8(checkbyte_n, 1); //Check byte
#ifdef store_to_eeprom
      reset_cpu () ;
#endif
}

void load_values()
{
#ifdef store_to_eeprom
      min_freq = load32(54); max_freq = load32(55);
      active_vfo =  load8(vfo_n);
      mem_mode =   load8(mode_n);
      mem_channel =  load8(mem_ch_n);
      fine_tune =   load8(fine_tune_n);
      per_band_offset = load8(band_offset_n);
#ifdef include_cat
      cat_mode =   load8(cat_mode_n);
      baud_rate_n =  load8(baud_n);
      dummy_mode =  load8(dummy_mode_n);
      id_enable =   load8(id_enable_n);
#endif
      speed_dial =  load8(dial_n);
      dcs =    load8(dcs_n);
      band =    load8(band_n);
      savetimerON = load8(savetimer_n);
      #ifdef include_cb

      cb_region =   load8(cb_reg_n);
      cb_channel =  load8(cb_ch_n);

      #endif

#ELSE
      min_freq = minimum_freq;
      max_freq = maximum_freq;
      active_vfo =  0;
      mem_mode =   0;
      mem_channel =  0;
      fine_tune =   0;
      per_band_offset = 0;
      cat_mode =   default_cat_mode;
      baud_rate_n =  0;
#ifdef include_cat
      dummy_mode =  49;
#endif
      id_enable =   0;
      speed_dial =  0;
      dcs =    15;
      band =    calc_band (700000);
      savetimerON = 0;

      #ifdef include_cb

      cb_region =   0;
      cb_channel =  19;

      #endif

#endif

   set_dcs (dcs) ;

   switch (baud_rate_n)
   {
      case 0: set_uart_speed (4800); break;
      case 1: set_uart_speed (1200); break;
      case 2: set_uart_speed (2400); break;
      case 3: set_uart_speed (4800); break;
      case 4: set_uart_speed (9600); break;
      case 5: set_uart_speed (19200); break;
      case 6: set_uart_speed (38400); break;
      case 7: set_uart_speed (57600); break;
      case 8: set_uart_speed (115200); break;
   }
}

#ifdef include_cat

void swap_cat_mode()
{
   while(pb0){}
   while (true)
   {
      delay_ms (60) ;
      if (counter1 == 0) VFD_data (0xFF, 0xFF, (757) , 0xFF, 0, 0, 2, 0);
      if (counter1 == 20) VFD_data (0xFF, 0xFF, (140), 0xFF, 0, 0, 2, 0);
      k8 = 1; delay_us (1);
      ++counter1;
      if (counter1 > 40) {counter1 = 0; ++counter; }
      if (counter == 5) {save8(cat_mode_n,0); break;}
      if (pb0){while (pb0){} break; }
   }

   beep () ;

   if (counter <=  5)
   {
      if ( (counter1 > 0)&& (counter1 <= 20)) {save8(cat_mode_n,0); cat_mode = 0; }
      if ( (counter1 > 20) && (counter1 <= 40) ) {save8(cat_mode_n, 1); cat_mode = 1; }
   }

   counter1 = 0; counter = 0;
   k8 = 0;
}

void change_baud_rate()
{
   while(pb1){}
   INT1 upd = 0;
   INT8 res = 0;

   while (true)
   {
      delay_ms (60) ;

      switch (counter1)
      {
         case 0: baud_rate = 1200; upd = 1; break;
         case 20: baud_rate = 2400; upd = 1; break;
         case 40: baud_rate = 4800; upd = 1; break;
         case 60: baud_rate = 9600; upd = 1; break;
         case 80: baud_rate = 19200; upd = 1; break;
         case 100: baud_rate = 38400; upd = 1; break;
         case 120: baud_rate = 57600; upd = 1; break;
         case 140: baud_rate = 115200; upd = 1; break;
      }

      if (upd)
      {
         res += 1;
         VFD_data (0xFF, 0xFF, baud_rate, 0xFF,0,0, 1, 0);
         upd = 0;
      }

      k8 = 1; delay_us (1);
      ++counter1;
      if (counter1 > 160) {counter1 = 0; res = 0; ++counter; }
      if (counter == 5) {res = 0; break;}
      if (pb0){while (pb0){} break; }
   }

   beep () ;

   if (counter <=  5)
   {
      switch (res)
      {
         
         case 1: set_uart_speed (1200); break;
         case 2: set_uart_speed (2400); break;
         case 3: set_uart_speed (4800); break;
         case 4: set_uart_speed (9600); break;
         case 5: set_uart_speed (19200); break;
         case 6: set_uart_speed (38400); break;
         case 7: set_uart_speed (57600); break;
         case 8: set_uart_speed (115200); break;
         default: set_uart_speed (4800); break;
      }

      save8(baud_n, res) ;
      
      errorbeep (3) ;
   } ELSE save8(baud_n,0) ;

   k8 = 0;
}

void cycle_mode_speed()
{
   while(true)
   {
   if(cat_mode == 0) VFD_data (0xFF, 0xFF, (757) , 0xFF, 0, 0, 2, 0);
   if(cat_mode == 1) VFD_data (0xFF, 0xFF, (140) , 0xFF, 0, 0, 2, 0);
   k8 = 1;
   delay_ms(1000);
   switch (load8(baud_n))
         {
            case 1: baud_rate = 1200; break;
            case 2: baud_rate = 2400; break;
            case 3: baud_rate = 4800; break;
            case 4: baud_rate = 9600; break;
            case 5: baud_rate = 19200; break;
            case 6: baud_rate = 38400; break;
            case 7: baud_rate = 57600; break;
            case 8: baud_rate = 115200; break;
            default: baud_rate = 4800; break;
         }
   VFD_data (0xFF, 0xFF, baud_rate, 0xFF,0,0, 1, 0);
   k8 = 1;
   delay_ms(1000);
   if(!pb1) break;
   }
}

#endif

void toggle_savetimer()
{
if(savetimerON == 1) savetimerON = 0; else savetimerON = 1;
VFD_data (0xFF, 0xFF, savetimerON, 0xFF, 0,0, 1, 0);
errorbeep(savetimerON + 1);
save8(savetimer_n, savetimerON);
delay_ms(1000);
}

void set_min_max()
{
   int1 set_max = 0;
   int1 res = 0;
   int32 tmp_limit_low = min_freq;
   int32 tmp_limit_hi = max_freq;
   VFD_data (0xFF, 0xFF, tmp_limit_low, 1,0,0, 0, 0);
   while(true)
   {
      
      if(!set_max)
      {
         if(tmp_limit_low < 1) tmp_limit_low = 0;
         if(tmp_limit_low > 9999999) tmp_limit_low = 9999999;
         res = basic_dial (tmp_limit_low, 100);
         if(res) VFD_data (0xFF, 0xFF, tmp_limit_low, 1,0,0, 0, 0);
      }
      
      if(set_max)
      {
         if(tmp_limit_hi < 1) tmp_limit_hi = 0;
         if(tmp_limit_hi >= 9999999) tmp_limit_hi = 9999999;
         res = basic_dial (tmp_limit_hi, 100);
         if(res) VFD_data (0xFF, 0xFF, tmp_limit_hi, 2,0,0, 0, 0);
      }
      
      if(!res)
      {
      
         k4 = 1; delay_us (1);
         if(pb0)
         {
            while(pb0){}
            if(!set_max) 
            {
            
            set_max = 1;
            VFD_data (0xFF, 0xFF, tmp_limit_hi, 2,0,0, 0, 0);
            }
            else
            {
            
            set_max = 0;
            VFD_data (0xFF, 0xFF, tmp_limit_low, 1,0,0, 0, 0);
            }
            beep();
         }
            if(pb1) 
            {
            while(pb1){}
            k4 = 0; break;
            }
         k4 = 0;
      }
   
   }
   save32 (54, tmp_limit_low);
   save32 (55, tmp_limit_hi);
   reset_cpu();
}
