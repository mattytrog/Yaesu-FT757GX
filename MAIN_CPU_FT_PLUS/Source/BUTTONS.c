

void set_dl();
int8 get_dcs();

void mode_SWITCH(int8 mode)
{
   mem_mode = mode;
   save_mode_n(mode);
}

void mode_SWITCH_kenwood(int8 mode)
{
   get_state();
   IF(mode == (48 + 0)){mem_mode = 0; active_vfo = 0; frequency = load_band_vfo_f(active_vfo, band); }
   IF(mode == (48 + 1)){mem_mode = 0; active_vfo = 1; frequency = load_band_vfo_f(active_vfo, band); }
   IF(mode == (48 + 2)){mem_mode = 1; frequency = load_mem_ch_f(mem_channel); }
   save_mode_n(mem_mode); save_vfo_n(active_vfo);
}

void lock_dial_kenwood(INT8 mode)
{
   IF(mode == (48 + 0)) dl = 0;
   IF(mode == (48 + 1)) dl = 1;
   dcs = get_dcs();
   save_dcs_n(dcs);
}

void toggle_fine_tune_display()
{
   IF(fine_tune == 1) fine_tune = 0; else fine_tune = 1;
   errorbeep(1);
   save_fine_tune_n(fine_tune);
   RETURN;
}

void toggle_per_band_offset()
{
   IF(per_band_offset == 1)
   {
      per_band_offset = 0;
      errorbeep(2);
   }

   ELSE
   {
      per_band_offset = 1;
      errorbeep(3);
   }

   save_band_offset_n(per_band_offset);
   RETURN;
}

void toggle_speed_dial()
{
   #ifdef include_dial_accel
   if(speed_dial) speed_dial = 0; else speed_dial = 1;
   errorbeep(speed_dial + 1);
   IF(speed_dial == 0) VFD_special_data(5);
   IF(speed_dial == 1) VFD_special_data(6);
   save_dial_n(speed_dial);
   RETURN;

   #endif
}

#ifdef include_cb
void set_dcs(INT8 res);
int8 get_dcs();

void toggle_cb_mode()
{
   beep();
   sl = 0; cl = 0;
   dcs = get_dcs();
   int8 state = get_state();
   if(state < 4)
   {
      save_cache_n(mem_mode);
      mem_mode = 2; save_mode_n(mem_mode);
      read_all_state();
      cb_region = load_cb_reg_n();
      update_PLL(frequency);
      VFD_data(0xFF, dcs, cb_channel, 0xFF,0,0, 2,0);
   }
   else
   IF(state == 4)
   {
      mem_mode = load_cache_n(); save_mode_n(mem_mode);
      read_all_state();
      update_PLL(frequency);
      vfo_disp(active_vfo, dcs, frequency, mem_channel, 0);
   }
   
}

void toggle_cb_region()
{
   IF(mem_mode == 2)
   {
      IF(cb_region == 0)cb_region = 1; else cb_region = 0;
      errorbeep(cb_region + 1);
      IF(cb_region == 0)VFD_special_data(2);
      IF(cb_region == 1)VFD_special_data(3);
      
      VFD_data(0xFF, dcs, cb_channel, 0xFF, 0,0,2,0);
      save_cb_reg_n(cb_region);
   }

   RETURN;
}

#endif
