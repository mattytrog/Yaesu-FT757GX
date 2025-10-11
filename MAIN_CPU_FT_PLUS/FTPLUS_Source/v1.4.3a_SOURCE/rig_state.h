int8 get_state()
{
   INT8 state = 0;

   IF (mem_mode == 0)
   {
      IF (active_vfo == 0) {save_band_vfo_f (0, band, frequency); state = 1; }
      IF (active_vfo == 1) {save_band_vfo_f (1, band, frequency); state = 2; }
   }

   IF (mem_mode == 1) {save_cache_mem_mode_f (frequency); save8(mem_ch_n,mem_channel); state = 3; }
#ifdef include_cb
   IF (mem_mode == 2) {save8(cb_ch_n, cb_channel); state = 4;}
#endif
   RETURN state;
}

int8 read_state()
{
   INT8 state = 0;

   IF (mem_mode == 0)
   {
      IF (active_vfo == 0) {state = 1; }
      IF (active_vfo == 1) {state = 2; }
   }

   IF (mem_mode == 1) {state = 3; }
   IF (mem_mode == 2) {state = 4; }
   RETURN state;
}

int8 read_all_state()
{
   INT8 state = 0;

   IF (mem_mode == 0)
   {
      cat_storage_buffer[0] = load_band_vfo_f (0, band);
      cat_storage_buffer[1] = load_band_vfo_f (1, band);
      IF (active_vfo == 0) {frequency = cat_storage_buffer[0]; state = 1; }
      IF (active_vfo == 1) {frequency = cat_storage_buffer[1]; state = 2; }
   }

   IF (mem_mode == 1) {frequency = load_cache_mem_mode_f (); mem_channel = load8(mem_ch_n); state = 3; }
   #ifdef include_cb
   IF (mem_mode == 2) {cb_channel = load8(cb_ch_n); state = 4; }
   #endif
   RETURN state;
}

int8 refresh_all_state()
{
   INT8 state = 0;

   IF (mem_mode == 0)
   {
      cat_storage_buffer[0] = load_band_vfo_f (0, band);
      cat_storage_buffer[1] = load_band_vfo_f (1, band);
      IF (active_vfo == 0) {frequency = cat_storage_buffer[0]; state = 1; }
      IF (active_vfo == 1) {frequency = cat_storage_buffer[1]; state = 2; }
      
   }

   IF (mem_mode == 1) { mem_channel = load8(mem_ch_n); frequency = load_mem_ch_f (mem_channel);state = 3; }
   #ifdef include_cb
   IF (mem_mode == 2) {cb_channel = load8(cb_ch_n); state = 4; }
   #endif
   RETURN state;
}

#ifdef include_cb
int32 load_cb(int8 &cb_channel, int8 cb_region)
{
      IF (cb_region == 0){channel_start = 1; channel_amount = 40;} 
      IF (cb_region == 1){channel_start = 1; channel_amount = 40;} 
      IF (cb_region == 2){channel_start = 1; channel_amount = 80;}
      if(cb_channel < channel_start) cb_channel = channel_start;
      if(cb_channel > (channel_start + (channel_amount - 1))) cb_channel = (channel_start + (channel_amount - 1));
      if(cb_region == 0) cb_frequency = cb_channel_bank[cb_channel - 1];
      if(cb_region == 1) cb_frequency = cb_channel_bank[cb_channel + 39];
      if(cb_region == 2) cb_frequency = cb_channel_bank[cb_channel - 1];
      
      return cb_frequency;
}
#endif
