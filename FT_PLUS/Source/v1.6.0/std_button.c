
void btn_dn_handler(int8 res)
{
   if (!res)
   {
      res = state;
      if((state == 1) || (state == 2))
      {
      if (sw_500k) res += 4;
      }
   }


   switch (res)
   {
      case 1: save_band_state_f (1, band1, storage_buffer[1]); if (band1 > 0) --band1; else band1 = 9; storage_buffer[1] = load_band_state_f (1, band1); break;
      case 2: save_band_state_f (2, band2, storage_buffer[2]); if (band2 > 0) --band2; else band2 = 9; storage_buffer[2] = load_band_state_f (2, band2); break;
      case 3: if (mem_channel > 0) --mem_channel; else mem_channel = 14; storage_buffer[3] = load_mem_ch_f (mem_channel); break;
#ifdef include_cb
      case 4: if(cb_channel > 1) --cb_channel; else cb_channel = 40; load_cb_state(0); break;
#endif
      case 5: storage_buffer[1] -= 50000; break;
      case 6: storage_buffer[2] -= 50000; break;
      default: 
   }
}

void btn_up_handler(int8 res)
{
   if (!res)
   {
      res = state;
      if((state == 1) || (state == 2))
      {
      if (sw_500k) res += 4;
      }
   }

   switch (res)
   {
      case 1: save_band_state_f (1, band1, storage_buffer[1]); if (band1 < 9) ++band1; else band1 = 0; storage_buffer[1] = load_band_state_f (1, band1); break;
      case 2: save_band_state_f (2, band2, storage_buffer[2]); if (band2 < 9) ++band2; else band2 = 0; storage_buffer[2] = load_band_state_f (2, band2); break;
      case 3: if (mem_channel < 14) ++mem_channel; else mem_channel = 0; storage_buffer[3] = load_mem_ch_f (mem_channel); break;
#ifdef include_cb
      case 4: if(cb_channel < 40) ++cb_channel; else cb_channel = 1; load_cb_state(0); break;
#endif
      case 5: storage_buffer[1] += 50000; break;
      case 6: storage_buffer[2] += 50000; break;
      
   }
}

void vfoab_handler()
{
   if (state == 1) state = 2;
   else
   if (state == 2) state = 1;
   write8 (state_n, state);
   vfo = state - 1;
}

void mvfo_handler()
{
   storage_buffer[state] = load_mem_ch_f (mem_channel);

   quick_disp_flash(2);
}

void vfom_handler()
{
   save_mem_ch_f (mem_channel, storage_buffer[state]);
   quick_disp_flash(2);
}

void mrvfo_handler()
{
   if (state == 3) state = vfo + 1; else state = 3;
   storage_buffer[3] = load_mem_ch_f (mem_channel);
   write8 (state_n, state);
}

void vfom_swap_handler()
{
   aux_buffer[0] = storage_buffer[vfo + 1];
   storage_buffer[vfo + 1] = storage_buffer[3];
   storage_buffer[3] = aux_buffer[0];
   aux_buffer[0] = 0;
   quick_disp_flash(2);
}

void dial_lock_button_handler()
{
   if ( ! dl) dl = 1; else dl =  0;
}

void clarifier_button_handler()
{
   if ( ! cl)
   {
      sl = 0; cl = 1;
      aux_buffer[2] = storage_buffer[state];
   }

   else
   {
      cl = 0;
      aux_buffer[2] = 0;
   }
}

void split_button_handler()
{
#ifdef include_cb
   if(state == 4) toggle_cb_region();
   else  
#endif
   if(state != 4)
   {
   if ( ! sl) {cl = 0; sl = 1; } else sl =  0;
   }
}

#ifdef include_fine_tuning
void micup()
{

   switch (state)
   {
      default: if (fine_tune_active){fine_tune_inh = 0; storage_buffer[state] += 1; } else storage_buffer[state] += 10; break;
      case 3: if (mem_channel < 14) ++mem_channel; else mem_channel = 0; storage_buffer[3] = load_mem_ch_f (mem_channel); break;
   }

}

void micdn()
{
   switch (state)
   {
      default: if (fine_tune_active){fine_tune_inh = 0; storage_buffer[state] -= 1; } else storage_buffer[state] -= 10; break;
      case 3: if (mem_channel > 0) --mem_channel; else mem_channel = 14; storage_buffer[3] = load_mem_ch_f (mem_channel); break;
   }
}
#else
void micup()
{

   switch (state)
   {
      default: storage_buffer[state] += 10; break;
      case 3: if (mem_channel < 14) ++mem_channel; else mem_channel = 0; storage_buffer[3] = load_mem_ch_f (mem_channel); break;
   }

}

void micdn()
{
   switch (state)
   {
      default: storage_buffer[state] -= 10; break;
      case 3: if (mem_channel > 0) --mem_channel; else mem_channel = 14; storage_buffer[3] = load_mem_ch_f (mem_channel); break;
   }
}
#endif

void micfst()
{
   switch (state)
   {
      case 1: btn_up_handler (1); break;
      case 2: btn_up_handler (2); break;
   }
}

#ifdef include_fine_tuning
void micup_fst(){fine_tune_inh = 1; storage_buffer[state] += 111;}
void micdn_fst(){fine_tune_inh = 1; storage_buffer[state] -= 111;}
void micup_hold(){ fine_tune_inh = 1; storage_buffer[state] += 10;}
void micdn_hold(){ fine_tune_inh = 1; storage_buffer[state] -= 10;}
void micfst_hold()
{
#ifdef include_manual_tuning
manual_adjust_frequency();
#endif
}
void micup_fst_hold(){fine_tune_inh = 1; storage_buffer[state] += 111;}
void micdn_fst_hold(){fine_tune_inh = 1; storage_buffer[state] -= 111;}

#else
void micup_fst(){storage_buffer[state] += 111;}
void micdn_fst(){storage_buffer[state] -= 111;}
void micup_hold(){storage_buffer[state] += 10;}
void micdn_hold(){storage_buffer[state] -= 10;}
void micfst_hold()
{
#ifdef include_manual_tuning
manual_adjust_frequency();
#endif
}
void micup_fst_hold(){storage_buffer[state] += 111;}
void micdn_fst_hold(){storage_buffer[state] -= 111;}
#endif

void toggle_fine_tune()
{
#ifdef include_fine_tuning
   if (fine_tune_active) fine_tune_active = 0; else fine_tune_active = 1;
   write8 (fine_tune_n, fine_tune_active);
   errorbeep (2) ;
#endif
}

void reset_eeprom()
{
   write8(checkbyte_n, 0xFF);
   while(true)
   {
   errorbeep(3); 
   delay_ms(1000);
   }
}

void set_fine_tune(){toggle_fine_tune();}
void set_reset_eeprom(){reset_eeprom();}

#ifdef include_enhanced_btn
void set_manual_tune(){beep(); manual_adjust_frequency();}
void set_alignment(){beep(); program_offset(0);}
void set_long_down(){LONG_dn();}
void set_long_up(){LONG_up();}
void set_dial_type(){toggle_speed_dial();}
void set_reset_bands(){reset_bands();}
void set_cb_mode(){toggle_cb_mode();}
void set_cat_pause(){toggle_cat_pause();}
void set_offset_type(){toggle_offset_type();}
#endif

int8 enhanced_option (int8 opt)
{
   switch (opt)
   {


      case 1: set_fine_tune(); break;

      case 2: set_reset_eeprom(); break;
#ifdef include_enhanced_btn
      case 3: set_manual_tune(); break;
      case 4: set_alignment(); break;
      case 5: set_long_down(); break;
      case 6: set_long_up(); break;
      case 7: set_dial_type(); break;
      case 8: set_reset_bands(); break;
      case 9: set_cb_mode(); break;
      case 10: set_cat_pause(); break;
      case 11: set_offset_type(); break;
#endif
   }
   
   return opt;
}



#ifdef include_enhanced_btn
void long_press_dial_lock_ndl()  {enhanced_option(lp_dial_lock_ndl);}
void long_press_vfoab()          {enhanced_option(lp_vfoab);}
void long_press_dial_lock_dl()   {enhanced_option(lp_dial_lock_dl);}
void long_press_clarifier()      {enhanced_option(lp_clarifier);}
void long_press_btn_dn()         {enhanced_option(lp_btn_dn);}
void long_press_btn_up()         {enhanced_option(lp_btn_up);}
void long_press_mvfo()           {enhanced_option(lp_mvfo);}
void long_press_vfom()           {enhanced_option(lp_vfom);}
void long_press_mrvfo()          {enhanced_option(lp_mrvfo);}
void long_press_split()          {enhanced_option(lp_split);}
void long_press_swap()           {enhanced_option(lp_swap);}
#else
void long_press_dial_lock_ndl()  {enhanced_option(1);}
void long_press_vfoab()          {enhanced_option(2);}
#endif

