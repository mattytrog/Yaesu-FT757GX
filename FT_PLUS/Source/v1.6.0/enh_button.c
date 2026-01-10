


#ifdef include_enhanced_btn

int8 perform_cat_action();
int8 buttons(INT1 option);

void manual_adjust_frequency()
   {
#ifdef include_manual_tuning
if (app_detected) return;
      temp_freq = storage_buffer[get_state()];
      STATIC int32 old_temp_freq;
      VFD_data(get_state(),0xFF,temp_freq,0xFF,0);
      beep();
      int8 d3,d4,d5,d6,d7,d8,d9;
      split_value(temp_freq, d3,d4,d5,d6,d7,d8,d9);
      int8 current_digit = 1;
      int8 res = 0;
      while (( mic_fast) || (!mic_up) || (!mic_dn)){current_digit = 1;}
      
      while(true)
      {
         
         
         IF (current_digit == 7)
         {
            switch(flash)
            {
            case 1: VFD_data (state, 0xFF, temp_freq, 0xFF, current_digit + 10) ; break;
            case 0: VFD_data (state, 0xFF, temp_freq, 0xFF, 10) ; break;
            }
         }
      
         else
         {
            switch(flash)
            {
            case 1: VFD_data (state, 0xFF, temp_freq, 0xFF, current_digit + 10) ; break;
            case 0: VFD_data (state, 0xFF, temp_freq, 0xFF, 9) ; break;
            }
         }
         //if(res) VFD_data (state, 0xFF, res, 0xFF, 1) ;
         res = buttons(1);
         if(res) {flash = 0;}
         
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
  
         }
         
         

         IF(temp_freq != old_temp_freq)
         {
         storage_buffer[get_state()] = temp_freq;
         update_PLL(temp_freq, 0xFF);
         old_temp_freq = temp_freq;
         }
         
         IF((res == 18) || (res == 36)) { break;}
               

      
      }
      
   errorbeep(3);
   delay_ms(1000);
   while (( mic_fast) || (!mic_up) || (!mic_dn)){}
   btn_down = 0; mic_down = 0;
   storage_buffer[get_state()] = temp_freq;
#endif
   }

void program_offset(int16 offset_val)
{
   #ifdef include_offset_programming
   if(app_detected) {errorbeep(3); return;}
   setup_offset = 1;
   
   if(!offset_val) offset = load_offset_f(); else offset = offset_val;
   if(offset >= 20000) {offset = 0; save_offset_f(0);}
   temp_freq = storage_buffer[state];
   
   IF(offset >= 10000)
   {
      offset -= 10000;
      dir = 0;
   }

   ELSE
   {
      dir = 1;
   }

   INT8 res = 1;
   int8 btnres;
   int1 ch_up = 0;
   int1 ch_dn = 0;
   while(true)
   {

      IF (res == 1)
      {
         if(ch_dn) {offset -=1;ch_dn = 0;}
         if(ch_up) {offset +=1;ch_up = 0;}
         IF (offset == -1) {offset = 1; if(dir == 0) dir = 1; else dir = 0;}
         IF(offset)
         { 
            switch(dir)
            {
               default: temp_freq = storage_buffer[state] + offset; break;
               case 1: temp_freq = storage_buffer[state] - offset; break;
            }
            VFD_data(0xFF, 0xFF, offset, 0xFF, (4 - dir));
         }

         ELSE 
         {
            VFD_data(0xFF, 0xFF, 1, 0xFF, 5); beep(); delay_ms(200);
         }
         set_PLL(temp_freq, 0, 0xFF);
         res = 0;
      }
      if(offset_val) {delay_ms(2000); break;}
      if(!res)
      {
         IF (misc_dial (offset,dir)) res = 1;
      }

      if(!res)
      {
         btnres = buttons(0);
         if(btnres)
         {
            res = 1;
            IF(!dir)
            {
               switch(btnres)
               {
                  case 2: ch_dn = 1; break;
                  case 3: ch_up = 1; break;
                  case 6: res = 3; break;
                  case 12: ch_dn = 1; break;
                  case 11: ch_up = 1; break;
                  case 13: res = 2; break;
                  case 31: res = 2; break;
               }
            }
            else
            {
               switch(btnres)
               {
                  case 2: ch_up = 1; break;
                  case 3: ch_dn = 1; break;
                  case 6: res = 3; break;
                  case 12: ch_up = 1; break;
                  case 11: ch_dn = 1; break;
                  case 13: res = 2; break;
                  case 31: res = 2; break;
               }
            }

            IF(btnres == 1)
            {
               beep();
               VFD_data(0xFF, 0xFF, storage_buffer[state], 0xFF, 0);
               delay_ms(1000);
            }
            if(command_received) res = 3;
            if((res == 2) || (res == 3)) break;
            
         }
      }

      




   }
   if(res == 2)
   {
      IF(!dir) save_offset_f(10000 + offset);
      IF(dir) save_offset_f(offset);
      
   }
   if(res == 3)
   {
      save_offset_f(0);
      VFD_data(0xFF, 0xFF, 1, 0xFF, 5); beep();
      errorbeep(3);
      delay_ms(1000);
   }

   setup_offset = 0;
    

#endif

}
void quick_disp_flash(int8 beeps);
void LONG_up()
{
   IF((state == 1) || (state == 2)) {IF(mem_channel  < 14)++mem_channel; write8(mem_ch_n, mem_channel); }
   IF(state  == 3) {IF(band3  == 9)band3 = 0; ELSE ++band3; storage_buffer[3] = band_bank[band3]; }
   quick_disp_flash(2);
}

void LONG_dn()
{
   IF((state == 1) || (state == 2)) {IF(mem_channel  > 0)--mem_channel; write8(mem_ch_n, mem_channel);}
   IF(state  == 3) {IF(band3  == 0)band3 = 9; ELSE --band3; storage_buffer[3] = band_bank[band3];}
   quick_disp_flash(2);
}



void toggle_speed_dial()
{
   #ifdef include_dial_accel
   if(speed_dial) {speed_dial = 0;} else {speed_dial = 1;}
   errorbeep(speed_dial + 1);
   write8(dial_n,speed_dial);
   #endif
}

void toggle_cb_mode()
{
#ifdef include_cb
   if((gen_tx) && (!cb_disabled))
   {
      int8 old_state = read8(old_state_n);
      if((old_state < 1) || (old_state > 3)) old_state = vfo + 1;

      switch (state)
      {
         case 4: state = old_state; cl = 0; write8(state_n, state); break;
         default: old_state = state; write8(old_state_n, old_state); state = 4; cl = 0; write8(state_n, state); load_cb_state(1); break;
      }
      
   } else errorbeep(3);
#endif  
}

void toggle_cb_region()
{
#ifdef include_cb
   IF(state == 4)
   {
      if(cb_region < 7) ++cb_region;
      else cb_region = 0;
      //VFD_data(0xFF, dcs, cb_channel, 0xFF, 0,0,2,0);
      write8(cb_reg_n,cb_region);
      load_cb_state(1);
   }

   RETURN;
#endif
}

void toggle_cat_pause()
{
   if(app_detected) {errorbeep(3); return;}
   if(pause_cat) {pause_cat = 0;} else {pause_cat = 1;}
   cat_tx_request = 0; 
   errorbeep(pause_cat + 1);

}


void toggle_offset_type()
{
   IF(per_band_offset == 1) {clear_offsets(1); per_band_offset = 0;} else {clear_offsets(0); per_band_offset = 1;}
      errorbeep(per_band_offset + 1);
   write8(band_offset_n,per_band_offset);
}

void reset_bands()
{
   errorbeep(3);
   for(INT i  = 30; i <= 40; i++)
   {write32(i, band_bank[i - 30]); }
   FOR(i  = 41; i <= 51; i++)
   {write32(i, band_bank[i - 41]); }
   reset_cpu();
}

#endif




