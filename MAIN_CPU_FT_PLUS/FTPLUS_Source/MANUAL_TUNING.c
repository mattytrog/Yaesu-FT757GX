

void program_mem_scan();

#ifdef include_manual_tuning

void manual_adjust_frequency()
   {
   get_state();
   INT32 temp_freq = frequency;
   STATIC int old_temp_freq;
   VFD_data(0xFF,0xFF,frequency,0xFF,1,0,0,0);
   INT8 res = 0;
   INT8 catres = 0;
   
   beep();
   INT8 current_digit = 1;
   while (( mic_fast) || (!mic_up) || (!mic_dn)){current_digit = 0;}
   
   WHILE(true)
   {
      res = buttons(3);
      if(res == 0xFF) flash_freq_data(current_digit + 10,temp_freq);
      IF((res  == 6) ||(res == 13))
      {
         
         IF(current_digit  < 7)++current_digit;else current_digit  = 1;
      }
  
      flash_freq_data(current_digit,temp_freq);
      start:
      tx_oob_check(temp_freq);

      

      
      IF((res  == 3) ||(res == 11) || (res == 33) || (res == 16))
      {
         
         //if(temp_freq >= max_freq) {temp_freq = max_freq; current_digit = 0;}
         IF(current_digit  == 1)
         {
            
            IF(d3 > 3)
            {
            d3 = 0;
            temp_freq += 1000000;
            }
            else 
            {
            d3 += 1;
            temp_freq += 1000000;
            }
         }

         IF(current_digit  == 2)
         {
            
            IF(d4 > 9) 
            {
            d4 = 0;
            temp_freq += 100000;
            }
            else 
            {
            d4 += 1;
            temp_freq += 100000;
            }
         }

         
         IF(current_digit  == 3)
         {
            
            IF(d5 > 9) 
            {
            d5 = 0;
            temp_freq += 10000;
            }
            else 
            {
            d5 += 1;
            temp_freq += 10000;
            }
         }

         
         IF(current_digit  == 4)
         {
            
            IF(d6 > 9) 
            {
            d6 = 0;
            temp_freq += 1000;
            }
            else 
            {
            d6 += 1;
            temp_freq += 1000;
            }
         }

         
         IF(current_digit  == 5)
         {
            
            IF(d7 > 9)
            {
            d7 = 0;
            temp_freq += 100;
            }
            else 
            {
            d7 += 1;
            temp_freq += 100;
            }
         }

         
         IF(current_digit  == 6)
         {
            
            IF(d8 > 9) 
            {
            d8 = 0;
            temp_freq += 10;
            }
            else 
            {
            d8 += 1;
            temp_freq += 10;
            }
         }

         
         IF(current_digit  == 7)
         {
            
            IF(d9 > 9) 
            {
            d9 = 0;
            temp_freq += 1;
            }
            else 
            {
            d9 += 1;
            temp_freq += 1;
            }
         }
         
         if((res == 33) || (res == 16)) 
         {
         res = buttons(3);
         if ((res == 33) || (res == 16)) 
         {
         flash_freq_data(current_digit + 10,temp_freq);
         temp_freq = update_PLL(temp_freq);
         goto start;
         }
         }
      }

      ELSE
      IF((res  == 2) ||(res == 12) || (res == 32) || (res == 17))
      {
         IF(current_digit  == 1)
         {
            IF(d3 < 1)
            {
            d3 = 9;
            temp_freq -= 1000000;
            }
            else
            {
            d3 -= 1;
            temp_freq -= 1000000;
            }
            
         }

         
         IF(current_digit  == 2)
         {
            IF(d4 < 1) 
            {
            d4 = 9;
            temp_freq -= 100000;
            }
            else
            {
            d4 -= 1;
            temp_freq -= 100000;
            }
         }

         
         IF(current_digit  == 3)
         {
            IF(d5 < 1) 
            {
            d5 = 9;
            temp_freq -= 10000;
            }
            else
            {
            d5 -= 1;
            temp_freq -= 10000;
            }
         }

         
         IF(current_digit  == 4)
         {
            IF(d6 < 1) 
            {
            d6 = 9;
            temp_freq -= 1000;
            }
            else 
            {
            d6 -= 1;
            temp_freq -= 1000;
            }
         }

         
         IF(current_digit  == 5)
         {
            IF(d7 < 1)
            {
            d7 = 9;
            temp_freq -= 100;
            }
            else 
            {
            d7 -= 1;
            temp_freq -= 100;
            }
         }

         
         IF(current_digit  == 6)
         {
            IF(d8 < 1) 
            {
            d8 = 9;
            temp_freq -= 10;
            }
            else
            {
            d8 -= 1;
            temp_freq -= 10;
            }
         }

         
         IF(current_digit  == 7)
         {
            IF(d9 < 1) 
            {
            d9 = 9;
            temp_freq -= 1;
            }
            else 
            {
            d9 -= 1;
            temp_freq -= 1;
            }
         }
         if((res == 32) || (res == 17)) 
         {
         res = buttons(3);
         if ((res == 32) || (res == 17)) 
         {
         flash_freq_data(current_digit + 10,temp_freq);
         temp_freq = update_PLL(temp_freq);
         goto start;
         }
         }
      }

      
      if(!res) catres = check_cat();
      if(catres == 1) temp_freq = frequency;
      
      
      IF(temp_freq != old_temp_freq)
      {
         temp_freq = update_PLL(temp_freq);
         old_temp_freq = temp_freq;
         
      }
      
      
      IF((res  == 5) || (res == 18)) { break;}
      
//!      IF(current_digit  == 8)
//!      {
//!         pms = 1;
//!         res = program_mem_scan(1,temp_freq);
//!         temp_freq = frequency;
//!         ++current_digit;
//!      }

      
      
   }

   beep();
   frequency = temp_freq;
   save_band_vfo_f(active_vfo,band,frequency);
   refresh_screen(read_all_state());
   while (( mic_fast) || (!mic_up) || (!mic_dn)){}
            mic_down = 0;
   }

#endif

