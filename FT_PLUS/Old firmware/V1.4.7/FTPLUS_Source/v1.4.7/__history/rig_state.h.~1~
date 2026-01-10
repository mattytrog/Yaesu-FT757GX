

void save_f_state(int8 state)
{
   switch(state)
   {
   case 1: save_band_vfo_f (0, band, frequency);  break;
   case 2: save_band_vfo_f (1, band, frequency);  break;
   case 3: save8(mem_ch_n,mem_channel);  break;
   #ifdef include_cb
   case 4: save8(cb_ch_n, cb_channel); save8(cb_reg_n, cb_region); break;
   #endif
   }
}

int8 get_state(int1 save_freq)
{
   INT8 state = 0;

   IF (mem_mode == 0)
   {
      IF (active_vfo == 0) state = 1;
      IF (active_vfo == 1) state = 2;
   }

   IF (mem_mode == 1) state = 3;
#ifdef include_cb
   IF (mem_mode == 2) state = 4;
#endif
   if(save_freq) save_f_state(state);
   RETURN state;
}

void set_state(int8 state)
{
   if(state == 1) {mem_mode = 0; active_vfo = 0; save_vfo(0); save_mode(0);}
   if(state == 2) {mem_mode = 0; active_vfo = 1; save_vfo(1); save_mode(0);}
   if(state == 3) {mem_mode = 1; save_mode(1);}
   if(state == 4) {mem_mode = 2; save_mode(2);}
   if(state == 5) {mem_mode = 0; active_vfo = 0;}
   if(state == 6) {mem_mode = 0; active_vfo = 1;}
}

int8 read_all_state()
{
   INT8 state = get_state(0);

   IF (state == 1)frequency = load_band_vfo_f (0, band);
   IF (state == 2)frequency = load_band_vfo_f (1, band);
   if (state == 3) {mem_channel = load8(mem_ch_n); frequency = load_mem_ch_f (mem_channel);  }
   #ifdef include_cb
   IF (state == 4) frequency = load_cb(cb_channel, cb_region);
   #endif
   RETURN state;
}







void update_buffers(int32 value)
{
   
   if((mem_mode == 0) && (active_vfo == 0)) {storage_buffer[0] = value; storage_buffer[1] = load_band_vfo_f (1, band);}
   if((mem_mode == 0) && (active_vfo == 1)) {storage_buffer[1] = value; storage_buffer[0] = load_band_vfo_f (0, band);}
   if(mem_mode == 1) storage_buffer[2] = value;
   if(mem_mode == 2) storage_buffer[3] = value;
}

