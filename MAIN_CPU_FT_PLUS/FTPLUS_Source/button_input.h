

void mem_op(INT8 option)
{
   INT8 state = get_state ();
   INT1 blink = 0;
   
   IF (option == 1) //VFO A / B
   {
      beep();
      IF (active_vfo == 0){active_vfo = 1; state = 2; }
      ELSE{active_vfo = 0; state = 1; }
      save8(vfo_n,active_vfo);
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
#ifdef include_cb
      IF (state == 4) save_band_vfo_f (active_vfo, band, load_cb(cb_channel, cb_region)) ;
#endif
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

      save8(mode_n, mem_mode);
      blink = 0;
   }

   
   IF (blink) blink_vfo_display (0xFF, dcs, frequency, mem_channel);
   
   refresh_screen(refresh_all_state());
   while (( mic_fast) || (!mic_up) || (!mic_dn)){}
            mic_down = 0;
}


void mode_SWITCH(int8 mode)
{
   mem_mode = mode;
   save8(mode_n,mode);
}

void mode_SWITCH_kenwood(int8 mode)
{
   get_state();
   IF(mode == (48 + 0)){mem_mode = 0; active_vfo = 0; frequency = load_band_vfo_f(active_vfo, band); }
   IF(mode == (48 + 1)){mem_mode = 0; active_vfo = 1; frequency = load_band_vfo_f(active_vfo, band); }
   IF(mode == (48 + 2)){mem_mode = 1; frequency = load_mem_ch_f(mem_channel); }
   save8(mode_n, mem_mode); save8(vfo_n,active_vfo);
}

void lock_dial_kenwood(INT8 mode)
{
   IF(mode == (48 + 0)) dl = 0;
   IF(mode == (48 + 1)) dl = 1;
   dcs = get_dcs();
   save8(dcs_n,dcs);
}

void toggle_fine_tune_display()
{
   IF(fine_tune == 1) fine_tune = 0; else fine_tune = 1;
   errorbeep(1);
   save8(fine_tune_n,fine_tune);
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

   save8(band_offset_n,per_band_offset);
   RETURN;
}

void toggle_speed_dial()
{
   #ifdef include_dial_accel
   if(speed_dial) speed_dial = 0; else speed_dial = 1;
   errorbeep(speed_dial + 1);
   IF(speed_dial == 0) VFD_special_data(5);
   IF(speed_dial == 1) VFD_special_data(6);
   save8(dial_n,speed_dial);
   RETURN;

   #endif
}

#ifdef include_cb
void toggle_cb_mode()
{
   beep();
   sl = 0; cl = 0;
   dcs = get_dcs();
   int8 state = get_state();
   if(state < 4)
   {
      save8(cache_n,mem_mode);
      mem_mode = 2; save8(mode_n,mem_mode);
      read_all_state();
      cb_region = load8(cb_reg_n);
      frequency = update_PLL(frequency);
      VFD_data(0xFF, dcs, cb_channel, 0xFF,0,0, 2,0);
   }
   else
   IF(state == 4)
   {
      mem_mode = load8(cache_n); save8(mode_n,mem_mode);
      read_all_state();
      frequency = update_PLL(frequency);
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
      save8(cb_reg_n,cb_region);
   }

   RETURN;
}

#endif

void dial_lock_button()
{
   if(dl) dl = 0; else dl = 1;
   dcs = get_dcs();
}

void clarifier_button()
{
   if(mem_mode !=2)
   {
      if(cl) cl = 0; else {sl = 0; cl = 1;}
      dcs = get_dcs();
   }
   else
   {
      VFD_data(0xFF, 0xFF, frequency, 0xFF, 0, 0, 0, 0);
      delay_ms(1000);
   }
}

void split_button()
{
   if(mem_mode != 2)
   {
      if(sl) sl = 0; else {cl = 0; sl = 1;}
      dcs = get_dcs();
   }
#ifdef include_cb
   else
   {
      toggle_cb_region();
   }
#endif
}

void up_down(int1 updown, INT8 state)
{
   beep();
   IF(state  < 3)
   {
      IF( ! sw_500k)
      {
         save_band_vfo_f(active_vfo, band, frequency);
         IF( ! updown)
         {
            IF(band  == 0)band = 9;
            ELSE --band;
         }

         ELSE
         {
            IF(band  == 9)band = 0;
            ELSE ++band;
         }
         
         save8(band_n,band);
         frequency = load_band_vfo_f(active_vfo, band);
         cat_storage_buffer[0] = load_band_vfo_f(0, band);
         cat_storage_buffer[1] = load_band_vfo_f(1, band);
      }

      ELSE
      {
         IF( ! updown)
         {
         if (frequency > (min_freq)) frequency  -= jump_500k; else frequency = max_freq;
         }
         ELSE 
         {
         if (frequency < (max_freq)) frequency += jump_500k; else frequency = min_freq;
         }
      }
   }

   
   IF(state  == 3)
   {
      IF( ! updown)
      {
         IF(mem_channel  > 0)--mem_channel;
         ELSE mem_channel = 14;
      }

      ELSE
      {
         IF(mem_channel  < 14)++mem_channel;
         ELSE mem_channel = 0;
      }

      frequency = load_mem_ch_f(mem_channel);
   }

   #ifdef include_cb
   IF(state  == 4)
   {
      IF( ! updown)
      {
         IF(cb_channel  > 1)--cb_channel;
      }

      ELSE
      {
         IF(cb_channel  < channel_amount)++cb_channel;
      }
   }

   #endif
   get_state();
}

void LONG_up_down(int1 updown, int8 state)
{
   IF(state  < 3)
   {
      IF( ! updown)
      {
         IF(mem_channel  > 0)--mem_channel;
      }

      ELSE
      {
         LONG_press_up = 0;
         IF(mem_channel  < 14)++mem_channel;
      }

      blink_vfo_display(active_vfo, dcs, frequency, mem_channel);
   }

   
   
   IF(state  == 3)
   {
      IF( ! updown)
      {
         IF(band  == 0)band = 10;
         --band;
         frequency = band_bank[band];
         save_cache_mem_mode_f(frequency);
         LONG_press_down = 0;
      }

      ELSE
      {
         ++band;
         IF(band  > 9)band = 0;
         frequency = band_bank[band];
         save_cache_mem_mode_f(frequency);
         LONG_press_up = 0;
      }
   }

   get_state();
}


int8 micup(INT8 state, int8 increment)
{
  
         IF(state  < 3){frequency += increment; return 21;}
         IF(state  == 3)if(mem_channel  < 14)++mem_channel; else mem_channel = 0;
         IF(state  == 4){up_down(1, state); }
         
}

void micup_hold(int8 state)
{
         IF(state  < 3)frequency += 10;
         IF(state  == 4){up_down(1, state); }
}

int8 micdn(INT8 state, int8 increment)
{

         IF(state  < 3){frequency -= increment; return 21;}
         IF(state  == 3)if(mem_channel  > 0)--mem_channel; else mem_channel = 14;
         IF(state  == 4){up_down(0, state); }
       
}

void micdn_hold(int8 state)
{
         IF(state  < 3)frequency -= 10;
         IF(state  == 4){up_down(0, state); }
         BREAK;
}

void micfst(INT8 state)
{

               IF(state  < 3){beep(); up_down(1, state); }
               #ifdef include_cb
               IF(state  == 4){toggle_cb_region(); }
               #endif
}
void manual_adjust_frequency();
void micfst_hold(int8 state)
{
#ifdef include_manual_tuning
            IF(state  < 3)manual_adjust_frequency();
#endif
            IF(state  == 3){mem_op(5); }
            IF(state  == 4){mem_op(5); }
            
            
            while (( mic_fast) || (!mic_up) || (!mic_dn)){}
            mic_down = 0;
}


void micup_fst(INT8 state)
{
   IF(state  < 3)frequency += 100;
}

void micdn_fst(INT8 state)
{
   IF(state  < 3)frequency -= 100;
}

void micup_fst_hold(INT8 state)
{
   IF(state  < 3)frequency += 100;
}

void micdn_fst_hold(INT8 state)
{
   IF(state  < 3)frequency -= 100;
}

void program_offset();
void LONG_press_clarifier() {beep(); program_offset();}
void LONG_press_mvfo() {toggle_speed_dial();}


void LONG_press_dl()
{
   #ifdef include_manual_tuning
   IF(dl)manual_adjust_frequency();
   ELSE toggle_fine_tune_display();

   #ELSE
   toggle_fine_tune_display();

   #endif
}

void LONG_press_vfom()
{
   errorbeep(3);
   for(INT i  = 30; i <= 40; i++)
   {save32(i, band_bank[i - 30]); }
   FOR(i  = 41; i <= 51; i++)
   {save32(i, band_bank[i - 41]); }
   reset_cpu();
}

void LONG_press_mrvfo()
{
   #ifdef include_cb
   toggle_cb_mode();

   #endif
}

void LONG_press_split()
{
   #ifdef include_cb
   IF(cb_region == 2)
   {
   cb_region = 0;
   VFD_special_data(2);
   }
   else
   {
   cb_region = 2;
   VFD_special_data(4);
   }
   errorbeep(cb_region + 1);
   
   save8(cb_reg_n, cb_region);

   #endif
}

void LONG_press_swap(){toggle_per_band_offset();}
int8 buttonaction (INT8 res, int8 increment)
{
   INT8 state = read_state();
   
   SWITCH(res)
   {
      CASE 1: beep(); clarifier_button(); break;
      CASE 2: up_down(0, state); break;
      CASE 3: up_down(1, state); break;
      CASE 4: mem_op(3); break; //MVFO
      CASE 5: beep(); mem_op(1); break; //VFOAB
      CASE 6: beep(); dial_lock_button(); break;
      CASE 7: mem_op(4); break; //VFOM
      CASE 8: beep(); mem_op(5); break; //MRVFO
      CASE 9: beep(); split_button(); break;
      CASE 10: mem_op(2); break; //VFOM SWAP
      CASE 11: res = micup(state, increment); break;
      CASE 12: res = micdn(state, increment); break;
      CASE 13: micfst(state); break;
      CASE 14: micup_fst(state); break;
      CASE 15: micdn_fst(state); break;
      case 16: micup_hold(state); break;
      case 17: micdn_hold(state); break;
      case 18: micfst_hold(state); break;
      case 19: micup_fst_hold(state); break;
      case 20: micdn_fst_hold(state); break;
      case 31: LONG_press_clarifier(); break;
      case 32: LONG_up_down(0, state); break;
      case 33: LONG_up_down(1, state); break;
      case 34: LONG_press_mvfo(); break;
      //case 35: LONG_press_vfoab(); break;
      case 36: LONG_press_dl(); break;
      case 37: LONG_press_vfom(); break;
      case 38: LONG_press_mrvfo(); break;
      case 39: LONG_press_split(); break;
      case 40: LONG_press_swap(); break;
      //default: RETURN 0; break;
   }
   force_update = 1;
   RETURN res;
}

#define ondelay 1
#define countdelay 10
#define holdcount 100
int8 buttons(INT8 option)
{
   
   STATIC INT8 btnres = 0;
   STATIC INT8 micres = 0;
   STATIC INT16 count = 0;
   STATIC INT16 mic_count = 0;
   STATIC INT16 holdcountmic;
   INT rtnres = 0;
   IF(pb2)
   {
       WHILE(pb2){}
      if(btn_down)
      {
      
         k1 = 1;k2 = 1; k4 = 1; k8 = 1; delay_us(ondelay);
         IF((pb2)||(pb1)||(pb0))
         {
            if(option == 2)
            {
            rtnres= btnres;
            btnres = 0;
            btn_down = 0;
            return rtnres;
            }

            IF(option == 1)
            {
               if(count < holdcount) {++count; delay_ms(countdelay);}
               IF(count >= holdcount)
               {
                  rtnres = btnres + 30;
                  btnres = 0;
                  
                  k1 = 0; k2 = 0; k4 = 0; k8 = 0;
                  
                  count = 0;
                  RETURN rtnres;
               }
              
            }
            
            IF(option == 3)
            {
               if(count < holdcount) {++count; delay_ms(countdelay); return 0xFF;}
               IF(count >= holdcount)
               {
                  rtnres = btnres + 30;
                  //btnres = 0;
                  
                  k1 = 0; k2 = 0; k4 = 0; k8 = 0;
                  k1 = 1;k2 = 1; k4 = 1; k8 = 1; delay_us(ondelay);
                  IF((!pb2)&&(!pb1)&&(!pb0)) count = 0;
                  k1 = 0; k2 = 0; k4 = 0; k8 = 0;
                  
                  RETURN rtnres;
               }
              
            }
         }
         else
         {
            
            btn_down = 0;
            rtnres = btnres;
            btnres = 0;
            k2 = 0; k4 = 0; k8 = 0;
            
            count = 0;
            RETURN rtnres;
         }

         k2 = 0; k4 = 0; k8 = 0;
      }
      
      if(mic_down)
      {
         if (( mic_fast) || (!mic_up) || (!mic_dn))
         {
            if(option == 2)
            {
               rtnres= micres;
               micres = 0;
               mic_down = 0;
               return rtnres;
            }
            
            IF(option == 1)
            {
               if(mic_count < holdcountmic) {++mic_count;}
               if(mic_count >= holdcountmic) 
               {
               rtnres = micres + 5;
               mic_down = 1;
               RETURN rtnres;
               }
            }
            
            IF(option == 3)
            {
               if(mic_count < holdcountmic) {++mic_count; return 0xFF;}
               if(mic_count >= holdcountmic) 
               {
               rtnres = micres + 5;
               mic_down = 1;
               RETURN rtnres;
               }
            }
         }
          else
         {
            mic_down = 0;
            mic_count = 0;
            rtnres = micres;
            micres = 0;
            RETURN rtnres;
         }
      
      } else mic_count = 0;
      
      
      //printf("%d\r\n", btn_down);
      
      IF( ! btn_down)
      {
         k4 = 0; k8 = 0; k1 = 0; k2 = 1; delay_us(ondelay);
         IF(pb2){btn_down  = 1; btnres = 1; }//(RESULT: 1)Clarifier
         IF(pb1){btn_down  = 1; btnres = 2; }//(RESULT: 2)Down
         IF(pb0){btn_down  = 1; btnres = 3; }//(RESULT: 3)Up
         
         k2 = 0; k4 = 1; delay_us(ondelay);
         IF(pb2){btn_down  = 1; btnres = 4; }//(RESULT: 4)M > VFO
         IF(pb1){btn_down  = 1; btnres = 5; }//(RESULT: 5)VFO A / B
         IF(pb0){btn_down  = 1; btnres = 6; }//(RESULT: 6)Dial lock
         
         k4 = 0; k8 = 1; delay_us(ondelay);
         IF(pb2){btn_down  = 1; btnres = 7; }//(RESULT: 7)VFO > M
         IF(pb1){btn_down  = 1; btnres = 8; }//(RESULT: 8)MR / VFO
         IF(pb0){btn_down  = 1; btnres = 9; }//(RESULT: 9)SPLIT
         
         k8 = 0; k1 = 1; delay_us(ondelay);
         IF(pb1){btn_down  = 1; btnres = 10; }//(RESULT: 11)VFO < > M
         k8 = 0; k4 = 0; k2 = 0; k1 = 0;
      
      }
   }
      
         if (!mic_fast&& ! mic_up) {mic_down = 1; micres = 11; holdcountmic = 25;}
         if (!mic_fast&& ! mic_dn) {mic_down = 1;micres = 12; holdcountmic = 25;}
         if (mic_fast&&mic_up&&mic_dn)  {mic_down = 1; micres  = 13;holdcountmic = 100;}
         if (mic_fast&& ! mic_up) {mic_down = 1; micres = 14; holdcountmic = 25;}
         if (mic_fast&& ! mic_dn) {mic_down = 1; micres = 15; holdcountmic = 25;}
     
   //if ( ( ! mic_fast) && (mic_up) && (mic_dn)) {mic_down = 0; miccnt =  0;}
      

      IF(sw_pms)
      {
         beep(); WHILE(sw_pms){}
         IF(pms)pms  = 0; else pms = 1;
      }


   
   
   
   RETURN 0;
}

int8 tx_oob_check(INT32 freq);

void program_offset()
{
   #ifdef include_offset_programming

   setup_offset = 1;
   INT32 offset_val = load_offset_f();
   INT32 testfreq = frequency;
   
   IF(offset_val >= 1000000)
   {
      offset_val -= 1000000;
      dir = 1;
   }

   ELSE
   {
      dir = 0;
   }

   INT8 res = 1;
   int8 btnres;
   //GOTO start;

   WHILE(true)
   {
      //IF (offset_val > 1000) {offset_val = 1000; if(dir == 0) dir = 1; else dir = 0;}
      start:
      IF (res == 1)
      {
         IF(offset_val)
         {
            IF(dir)
            {
               testfreq = frequency - offset_val;
               VFD_data(0xFF, 0xFF, offset_val, 0xFF, 1,0,3,0);
            }

            ELSE
            {
               testfreq = frequency + offset_val;
               VFD_data(0xFF, 0xFF, offset_val, 0xFF, 1,0,4,0);
            }
         }

         ELSE VFD_data(0xFF, 0xFF, 1, 0xFF, 0,0,5,0);
         testfreq = update_PLL(testfreq);
         res = 0;
      }

      btnres = buttons(1);
      IF (offset_val == -1) {offset_val = 1; if(dir == 0) dir = 1; else dir = 0;}
      IF(!dir)
      {
         IF((btnres == 2) || (btnres == 12)) {res = 1; offset_val -=1;}
         else IF((btnres == 3) || (btnres == 11)){res = 1; offset_val +=1;}
      }

      ELSE
      {
         IF((btnres == 2) || (btnres == 12)) {res = 1; offset_val +=1;}
         else IF((btnres == 3) || (btnres == 11)) {res = 1; offset_val -=1;}
      }

      IF(btnres == 1)
      {
         beep();
         VFD_data(0xFF, 0xFF, frequency, 0xFF, 0,0,0,0);
         delay_ms(1000);
         btnres = 0;
         res = 1;
         GOTO start;
      }

      IF(btnres == 6)
      {
         beep();
         save_offset_f(0);
         VFD_data(0xFF, 0xFF, 1, 0xFF, 0,0,5,0);
         delay_ms(1000);
         BREAK;
      }

      IF((btnres == 31) || (btnres == 13))
      {
         IF(dir) save_offset_f(1000000 + offset_val);
         IF(!dir) save_offset_f(offset_val);
         setup_offset = 0;
         BREAK;
      }

      res1 = read_counter();
      IF (misc_dial (offset_val,dir)) res = 1;
      IF (offset_val == -1) {offset_val = 1; if(dir == 0) dir = 1; else dir = 0;}
      IF(offset_val > 998) offset_val = 999;
      tx_oob_check(testfreq);
   }

   beep();

   #endif

}


#ifdef include_manual_tuning

int8 check_cat();
//!void manual_adjust_frequency()
//!   {
//!   get_state();
//!   INT32 temp_freq = frequency;
//!   STATIC int old_temp_freq;
//!   VFD_data(0xFF,0xFF,frequency,0xFF,1,0,0,0);
//!   INT8 res = 0;
//!#ifdef include_cat
//!   INT8 catres = 0;
//!#endif   
//!   beep();
//!   INT8 current_digit = 1;
//!   while (( mic_fast) || (!mic_up) || (!mic_dn)){current_digit = 0;}
//!   
//!   WHILE(true)
//!   {
//!      res = buttons(3);
//!      if(res == 0xFF) flash_freq_data(current_digit + 10,temp_freq);
//!      IF((res  == 6) ||(res == 13))
//!      {
//!         
//!         IF(current_digit  < 7)++current_digit;else current_digit  = 1;
//!      }
//!  
//!      flash_freq_data(current_digit,temp_freq);
//!      start:
//!      tx_oob_check(temp_freq);
//!
//!      if(!dial_clk)
//!      {
//!      if(dial_dir) res = 3; else res = 2;
//!      
//!      }
//!
//!      
//!      IF((res  == 3) ||(res == 11) || (res == 33) || (res == 16))
//!      {
//!         
//!         switch (current_digit)
//!         {
//!         case 1: IF(d3 > 3) {d3 = 0; temp_freq += 1000000;} else {d3 += 1; temp_freq += 1000000;} break;
//!         case 2: IF(d4 > 9) {d4 = 0; temp_freq += 100000;} else {d4 += 1; temp_freq += 100000;} break;
//!         case 3: IF(d5 > 9) {d5 = 0; temp_freq += 10000;} else {d5 += 1; temp_freq += 10000;} break;
//!         case 4: IF(d6 > 9) {d6 = 0; temp_freq += 1000;} else {d6 += 1; temp_freq += 1000;} break;
//!         case 5: IF(d7 > 9) {d7 = 0; temp_freq += 100;} else {d7 += 1; temp_freq += 100;} break;
//!         case 6: IF(d8 > 9) {d8 = 0; temp_freq += 10;} else {d8 += 1; temp_freq += 10;} break;
//!         case 7: IF(d9 > 9) {d9 = 0; temp_freq += 1;} else {d9 += 1; temp_freq += 1;} break;
//!         }
//!         
//!         
//!         if((res == 33) || (res == 16)) 
//!         {
//!         res = buttons(3);
//!         if ((res == 33) || (res == 16)) 
//!         {
//!         flash_freq_data(current_digit + 10,temp_freq);
//!         temp_freq = update_PLL(temp_freq);
//!         goto start;
//!         }
//!         }
//!      }
//!
//!      ELSE
//!      IF((res  == 2) ||(res == 12) || (res == 32) || (res == 17))
//!      {
//!      
//!         switch (current_digit)
//!         {
//!         case 1: IF(d3 < 1) {d3 = 9; temp_freq -= 1000000;} else {d3 -= 1; temp_freq -= 1000000;} break;
//!         case 2: IF(d4 < 1) {d4 = 9; temp_freq -= 100000;} else {d4 -= 1; temp_freq -= 100000;} break;
//!         case 3: IF(d5 < 1) {d5 = 9; temp_freq -= 10000;} else {d5 -= 1; temp_freq -= 10000;} break;
//!         case 4: IF(d6 < 1) {d6 = 9; temp_freq -= 1000;} else {d6 -= 1; temp_freq -= 1000;} break;
//!         case 5: IF(d7 < 1) {d7 = 9; temp_freq -= 100;} else {d7 -= 1; temp_freq -= 100;} break;
//!         case 6: IF(d8 < 1) {d8 = 9; temp_freq -= 10;} else {d8 -= 1; temp_freq -= 10;} break;
//!         case 7: IF(d9 < 1) {d9 = 9; temp_freq -= 1;} else {d9 -= 1; temp_freq -= 1;} break;
//!         }
//!
//!
//!         
//!        
//!         if((res == 32) || (res == 17)) 
//!         {
//!         res = buttons(3);
//!         if ((res == 32) || (res == 17)) 
//!         {
//!         flash_freq_data(current_digit + 10,temp_freq);
//!         temp_freq = update_PLL(temp_freq);
//!         goto start;
//!         }
//!         }
//!      }
//!
//!#ifdef include_cat      
//!      if(!res) catres = check_cat();
//!      if(catres == 1) temp_freq = frequency;
//!#endif     
//!      
//!      IF(temp_freq != old_temp_freq)
//!      {
//!         temp_freq = update_PLL(temp_freq);
//!         old_temp_freq = temp_freq;
//!         
//!      }
//!      
//!      
//!      IF((res  == 5) || (res == 18)) { break;}   
//!   }
//!
//!   beep();
//!   frequency = temp_freq;
//!   save_band_vfo_f(active_vfo,band,frequency);
//!   refresh_screen(read_all_state());
//!   while (( mic_fast) || (!mic_up) || (!mic_dn)){}
//!            mic_down = 0;
//!   }


void manual_adjust_frequency()
   {
      INT32 temp_freq = frequency;
      STATIC int32 old_temp_freq;
      VFD_data(0xFF,0xFF,temp_freq,0xFF,1,0,0,0);
      beep();
      
      int16 counter = 0;
      int8 current_digit = 1;
      int8 res = 0;
      while (( mic_fast) || (!mic_up) || (!mic_dn)){current_digit = 0;}
      
      while(true)
      {
         
         if(counter < vfo_button_tuning_flash_speed) ++counter;
         else
         {
            if(flash) flash = 0; else flash = 1;
            counter = 0;
         }
start:      
         IF (current_digit == 7)
         {
            switch(flash)
            {
            case 1: VFD_data (0xFF, 0xFF, temp_freq, 0xFF, 1,current_digit, 1, 0) ; break;
            case 0: VFD_data (0xFF, 0xFF, temp_freq, 0xFF, 1, 0, 1, 0) ; break;
            }
         }
      
         else
         {
            switch(flash)
            {
            case 1: VFD_data (0xFF, 0xFF, temp_freq, 0xFF, 1,current_digit, 0, 0) ; break;
            case 0: VFD_data (0xFF, 0xFF, temp_freq, 0xFF, 1, 0, 0, 0) ; break;
            }
         }
         
         res = buttons(3);
         if(res) {flash = 0; counter = 0;}
      
//!         if(current_digit  < 7)
//!         {
//!            res1 = read_counter();
//!            if(dial_moved())
//!            {
//!            if(dial_dir) res = 3; else res = 2;
//!            flash = 0; counter = 0;
//!            }
//!         }
         
         IF((res  == 6) ||(res == 13))
         {
            IF(current_digit  < 7)++current_digit;else current_digit  = 1;
         }
         
         IF((res  == 3) ||(res == 11) || (res == 33) || (res == 16))
         {
         
            switch (current_digit)
            {
            case 1: IF(d3 > 3) {d3 = 0; temp_freq += 1000000;} else {d3 += 1; temp_freq += 1000000;} break;
            case 2: IF(d4 > 9) {d4 = 0; temp_freq += 100000;} else {d4 += 1; temp_freq += 100000;} break;
            case 3: IF(d5 > 9) {d5 = 0; temp_freq += 10000;} else {d5 += 1; temp_freq += 10000;} break;
            case 4: IF(d6 > 9) {d6 = 0; temp_freq += 1000;} else {d6 += 1; temp_freq += 1000;} break;
            case 5: IF(d7 > 9) {d7 = 0; temp_freq += 100;} else {d7 += 1; temp_freq += 100;} break;
            case 6: IF(d8 > 9) {d8 = 0; temp_freq += 10;} else {d8 += 1; temp_freq += 10;} break;
            case 7: IF(d9 > 9) {d9 = 0; temp_freq += 1;} else {d9 += 1; temp_freq += 1;} break;
            }
            
            if((res == 33) || (res == 16)) 
               {
               res = buttons(3);
               if ((res == 33) || (res == 16)) 
               {
               flash = 0; counter = 0;
               temp_freq = update_PLL(temp_freq);
               goto start;
               }
               }
         }
         
         IF((res  == 2) ||(res == 12) || (res == 32) || (res == 17))
         {
      
            switch (current_digit)
            {
            case 1: IF(d3 < 1) {d3 = 9; temp_freq -= 1000000;} else {d3 -= 1; temp_freq -= 1000000;} break;
            case 2: IF(d4 < 1) {d4 = 9; temp_freq -= 100000;} else {d4 -= 1; temp_freq -= 100000;} break;
            case 3: IF(d5 < 1) {d5 = 9; temp_freq -= 10000;} else {d5 -= 1; temp_freq -= 10000;} break;
            case 4: IF(d6 < 1) {d6 = 9; temp_freq -= 1000;} else {d6 -= 1; temp_freq -= 1000;} break;
            case 5: IF(d7 < 1) {d7 = 9; temp_freq -= 100;} else {d7 -= 1; temp_freq -= 100;} break;
            case 6: IF(d8 < 1) {d8 = 9; temp_freq -= 10;} else {d8 -= 1; temp_freq -= 10;} break;
            case 7: IF(d9 < 1) {d9 = 9; temp_freq -= 1;} else {d9 -= 1; temp_freq -= 1;} break;
            }
   
            if((res == 32) || (res == 17)) 
            {
            res = buttons(3);
            if ((res == 32) || (res == 17)) 
            {
            flash = 0; counter = 0;
            temp_freq = update_PLL(temp_freq);
            goto start;
            }
            }
         }
         
#ifdef include_cat      
         if(!res)
         {
         if(check_cat() == 1) {flash = 0; counter = 0; temp_freq = frequency;}
         }
#endif     
         tx_oob_check(temp_freq);
         IF(temp_freq != old_temp_freq)
         {
         temp_freq = update_PLL(temp_freq);
         old_temp_freq = temp_freq;
         }
         
         IF((res  == 5) || (res == 18)) { break;}
               

      
      }
      
   beep();
   frequency = temp_freq;
   save_band_vfo_f(active_vfo,band,frequency);
   refresh_screen(read_all_state());
   while (( mic_fast) || (!mic_up) || (!mic_dn)){}
            mic_down = 0;
   }
#endif
