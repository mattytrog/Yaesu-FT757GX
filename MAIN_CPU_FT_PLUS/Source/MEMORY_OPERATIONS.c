
void mem_op(INT8 option)
{
   INT8 state = get_state ();
   INT1 blink = 0;
   
   IF (option == 1) //VFO A / B
   {
      beep();
      IF (active_vfo == 0){active_vfo = 1; state = 2; }
      ELSE{active_vfo = 0; state = 1; }
      save_vfo_n (active_vfo);
      blink = 0;
   }

   
   IF (option == 2) //VFOM SWAP
   {
      save_cache_f (load_band_vfo_f (active_vfo, band));
      save_band_vfo_f (active_vfo, band, load_mem_ch_f (mem_channel));
      save_mem_ch_f (mem_channel, load_cache_f ());
      blink = 1;
   }

   
   IF (option == 3) //MVFO
   {
      IF (state < 4) save_band_vfo_f (active_vfo, band, load_cache_mem_mode_f ());
      IF (state == 4) save_band_vfo_f (active_vfo, band, load_frequency (2)) ;
      blink = 1;
   }

   
   IF (option == 4) //VFOM
   {
      IF (state < 3) save_mem_ch_f (mem_channel, load_band_vfo_f (active_vfo, band));
      IF (state == 3) save_mem_ch_f (mem_channel, load_cache_mem_mode_f ()) ;
      blink = 1;
   }

   
   IF (option == 5) //MRVFO
   {
      IF (mem_mode == 0){state = 3; mem_mode = 1; }
      ELSE
      {
         mem_mode = 0;
         IF (active_vfo == 0) state = 1;
         IF (active_vfo == 1) state = 2;
      }

      save_mode_n (mem_mode);
      blink = 0;
   }

   
   IF (blink) blink_vfo_display (0xFF, dcs, frequency, mem_channel);
   
   refresh_screen(refresh_all_state());
   while (( mic_fast) || (!mic_up) || (!mic_dn)){}
            mic_down = 0;
}

