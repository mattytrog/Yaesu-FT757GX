#ifdef include_dial_accel

#define dial_timer_max 5000 //max spin down-timer. All percentages are based of this. Default 40000
#define slow_speed_threshold 50
#define stop_reset_count 5 //stop spin check sampling count. How fast dial reverts to lowest increment. Higher - longer, Lower, shorter. Default 25
#define negative_pulse_sample_rate 8 //how often to check for negative pulse to fight against the decrement. Higher number means easier acceleration. Lower is harder (eg faster and harder). Default 50
#define dial_timer_decrement 1 //overall timer decrement when spinning. How fast the main timker decreases. Default 5. Lower = longer spin before it speeds up. Not harder, just longer
#define dial_timer_pullback 2 //dial increment when negative (eg when slowing down spinning). How hard we fight against the decrement. Needs to be more than decrement. Default 20. Lower = acceleration increments kick in earlier



int16 freq_dial_accel_type1(int32 &value, int16 start_increment)
{
   //if(res1 == res2) return 0;
   int8 res = 0;
   static int16 count = 0;
   static int16 stop_count = 0;  
   static int16 dial_timer = 0;
   //if(reset) { dial_timer = 400; dial_increment = default_increment;}
   
   
   static int16 dial_increment = 0;
   if(!dial_increment) dial_increment = start_increment;
   //if(dial_timer <= 1) dial_timer = 0;
   //if(dial_timer >= dial_timer_max) dial_timer = dial_timer_max;
   if(res1 != res2)
      {
      count = 0; stop_count = 0;
      tmp = dial_dir;
      if(tmp) 
      {
      value +=dial_increment;
      }
      else 
      {
      value -=dial_increment;
      }
      
      if(dial_timer < dial_timer_max)dial_timer += dial_timer_decrement; else dial_timer = dial_timer_max;
      res2 = res1;
      if(dial_timer > slow_speed_threshold) dial_increment = (dial_timer - slow_speed_threshold); else dial_increment = start_increment;
      if(dial_increment > 1) res = 2;
      if(dial_increment == 1) res = 1;;//VFD_data(0xFF, 0xFF, dial_increment, 0xFF, 1,0);
      
         //VFD_data (0xFF, 0xFF, (dial_increment), 0xFF, 0,0, 1, 0); delay_ms(1000);

   }
   else
   {
   ++count;
   if(count>negative_pulse_sample_rate) 
      {
      count = 0; //checktime = 0;
      if((dial_timer) > dial_timer_pullback )
      {
      dial_timer -= dial_timer_pullback;
      ++stop_count;
      }
      
      }
     
   if(stop_count > stop_reset_count) {stop_count = 0; dial_timer = 0; count = 0; dial_increment = start_increment; }
   res = 0;   
   }
   
   //if (dial_timer < 300) dial_timer +=1;
   
return res;
}
//#endif


#endif

int16 freq_dial_basic(int32 &value, int16 dial_increment)
{
   static int8 temp_count = 0;
   if(res1 == res2) return 0;
   if(res1 != res2) temp_count +=1;
      
   if (temp_count > 9)
      {
         temp_count = 0;
         tmp = dial_dir;
         if(tmp) value+=dial_increment;
         else value-=dial_increment;
         res2 = res1;
         return 1;
      }
      
   return 0;
}


int8 misc_dial(int32 &value, int8 direction)
{
   if(res1 == res2) return 0;
   if(res1 != res2)
      {
      tmp = dial_dir;
      if(!direction)
      {
         if(tmp) value+=1; else value-=1;
      }  
      else
      {
         if(tmp) value-=1; else value+=1;
      }
      res2 = res1;
      return 1;
   } 
   return 0;
}

int8 misc_dial8(int8 &value, int8 direction)
{
   static int8 temp_count = 0;
   if(res1 == res2) return 0;
   if(res1 != res2)
      {
      temp_count +=1;
      if(temp_count > 9)
      {
         tmp = dial_dir;
         if(!direction)
      {
         if(tmp) value+=1; else value-=1;
      }  
      else
      {
         if(tmp) value-=1; else value+=1;
      }
      temp_count = 0;
      return 1;
      }
      res2 = res1;
      return 0;
      }
      return 0;
}
