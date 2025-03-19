
void program_mem_scan();

#ifdef include_manual_tuning

void manual_adjust_frequency()
   {
   get_state();
   INT32 temp_freq = frequency;
   STATIC int old_temp_freq;
   VFD_data(0xFF,0xFF,frequency,0xFF,1,0,0,1);
   INT8 res = 0;
   INT8 current_digit = 1;
   
   WHILE(true)
   {
      flash_freq_data(current_digit,temp_freq);
      tx_oob_check(temp_freq);
      res = buttons(1);
      IF((res  == 6) ||(res == 13))
      {
         
         IF(current_digit  < 8)++current_digit;else current_digit  = 1;
      }

      
      IF((res  == 3) ||(res == 11))
      {
         //if(temp_freq >= max_freq) {temp_freq = max_freq; current_digit = 0;}
         IF(current_digit  == 1)
         {
            
            IF(d3 > 3)d3 = 0; else d3 += 1;
         }

         IF(current_digit  == 2)
         {
            
            IF(d4 > 9) d4 = 0;else d4 += 1;
         }

         
         IF(current_digit  == 3)
         {
            
            IF(d5 > 9) d5 = 0;else d5 += 1;
         }

         
         IF(current_digit  == 4)
         {
            
            IF(d6 > 9) d6 = 0;else d6 += 1;
         }

         
         IF(current_digit  == 5)
         {
            
            IF(d7 > 9) d7 = 0;else d7 += 1;
         }

         
         IF(current_digit  == 6)
         {
            
            IF(d8 > 9) d8 = 0;else d8 += 1;
         }

         
         IF(current_digit  == 7)
         {
            
            IF(d9 > 9) d9 = 0;else d9 += 1;
         }
         if((d3 <= 0) && (d4 <= 0) && (d5 <= 0) && (d6 <= 0) && (d7 <= 0) && (d8 <= 0) && (d9 <= 0)) {d3 = 0; d4 = 0; d5 = 5; d6 = 0; d7 = 0; d8 = 0; d9 = 0;}
         if((d3 >= 3) && (d4 >= 0) && (d5 >= 0) && (d6 >= 0) && (d7 >= 0) && (d8 >= 0) && (d9 >= 0)) {d3 = 2; d4 = 9; d5 = 9; d6 = 9; d7 = 9; d8 = 9; d9 = 9;}
      }

      ELSE
      IF((res  == 2) ||(res == 12))
      {
         //IF(temp_freq <= min_freq) {temp_freq = min_freq; current_digit = 2;}
         IF(current_digit  == 1)
         {
            IF(d3 < 1) d3 = 9;else d3 -= 1;
         }

         
         IF(current_digit  == 2)
         {
            IF(d4 < 1) d4 = 9;else d4 -= 1;
         }

         
         IF(current_digit  == 3)
         {
            IF(d5 < 1) d5 = 9;else d5 -= 1;
         }

         
         IF(current_digit  == 4)
         {
            IF(d6 < 1) d6 = 9;else d6 -= 1;
         }

         
         IF(current_digit  == 5)
         {
            IF(d7 < 1) d7 = 9;else d7 -= 1;
         }

         
         IF(current_digit  == 6)
         {
            IF(d8 < 1) d8 = 9;else d8 -= 1;
         }

         
         IF(current_digit  == 7)
         {
            IF(d9 < 1) d9 = 9;else d9 -= 1;
         }
         if((d3 <= 0) && (d4 <= 0) && (d5 <= 0) && (d6 <= 0) && (d7 <= 0) && (d8 <= 0) && (d9 <= 0)) {d3 = 0; d4 = 0; d5 = 5; d6 = 0; d7 = 0; d8 = 0; d9 = 0;}
         if((d3 >= 3) && (d4 >= 0) && (d5 >= 0) && (d6 >= 0) && (d7 >= 0) && (d8 >= 0) && (d9 >= 0)) {d3 = 2; d4 = 9; d5 = 9; d6 = 9; d7 = 9; d8 = 9; d9 = 9;}
      }

      
      
      
      IF(temp_freq != old_temp_freq)
      {
         join_value(temp_freq,d3,d4,d5,d6,d7,d8,d9);
         //!         IF(temp_freq > max_freq) temp_freq = max_freq;
         //!         IF(temp_freq < min_freq) temp_freq = min_freq;
         update_PLL(temp_freq);
         old_temp_freq = temp_freq;
         
      }

      
      IF((res  == 5) || (res == 18)) { break;}
      
      IF(current_digit  == 8)
      {
         pms = 1;
         res = program_mem_scan(1,temp_freq);
         temp_freq = frequency;
         ++current_digit;
      }

      
      
   }

   beep();
   frequency = temp_freq;
   save_band_vfo_f(active_vfo,band,frequency);
   refresh_screen(read_all_state());
   while (( mic_fast) || (!mic_up) || (!mic_dn)){}
            mic_down = 0;
   }

#endif
