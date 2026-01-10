void program_offset()
{
   #ifdef include_offset_programming

   setup_offset = 1;
   INT32 offset_val = load_offset_f();
   set_pll(storage_buffer[get_state()],1);
   INT32 testfreq = storage_buffer[get_state()];
   
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
               testfreq = storage_buffer[get_state()] - offset_val;
               VFD_data(0xFF, 0xFF, offset_val, 0xFF, 1,0,3,0);
            }

            ELSE
            {
               testfreq = storage_buffer[get_state()] + offset_val;
               VFD_data(0xFF, 0xFF, offset_val, 0xFF, 1,0,4,0);
            }
         }

         ELSE VFD_data(0xFF, 0xFF, 1, 0xFF, 0,0,5,0);
         update_PLL(testfreq);
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
         VFD_data(0xFF, 0xFF, storage_buffer[get_state()], 0xFF, 0,0,0,0);
         delay_ms(1000);
         VFD_data(0xFF, 0xFF, storage_buffer[get_state()], 0xFF, 0,0,1,0);
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

   }

   beep();

   #endif

}


#ifdef include_manual_tuning

#separate
int8 check_cat();
int1 manual_adjust_frequency()
   {
      INT32 temp_freq = storage_buffer[get_state()];
      int32 orig_freq = temp_freq;
      STATIC int32 old_temp_freq;
      VFD_data(0xFF,0xFF,temp_freq,0xFF,1,0,0,0);
      beep();
      int8 d3,d4,d5,d6,d7,d8,d9;
      split_value(temp_freq, d3,d4,d5,d6,d7,d8,d9);
      int16 counter = 0;
      int8 current_digit = 1;
      int8 res = 0;
      while (( mic_fast) || (!mic_up) || (!mic_dn)){current_digit = 1;}
      
      while(true)
      {
         
         
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
         res = buttons(0);
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
         
#ifdef include_cat      
         if(!res)
         {
         if(check_cat() == 1) {flash = 0; counter = 0; temp_freq = storage_buffer[get_state()];}
         }
#endif     

         IF(temp_freq != old_temp_freq)
         {
         storage_buffer[get_state()] = temp_freq;
         update_PLL(temp_freq);
         old_temp_freq = temp_freq;
         }
         
         IF((res  == 5) || (res == 18)) { break;}
               

      
      }
      
   beep();
   
   while (( mic_fast) || (!mic_up) || (!mic_dn)){}
            mic_down = 0;
   storage_buffer[get_state()] = temp_freq;
   if(temp_freq != orig_freq) return 1;
   return 0;
   }
#endif

#ifdef include_set_minmax
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
            break;
            }
         
      }
   
   }
   k4 = 0;
   save32 (54, tmp_limit_low);
   save32 (55, tmp_limit_hi);
   reset_cpu();
}
#endif


