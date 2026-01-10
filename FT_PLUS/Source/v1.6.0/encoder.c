#if defined (include_cb) || defined(adjust_baud_rate)

int8 misc_dial8(int8 &value, int8 direction, int8 gearing)
{
   static int8 temp_count = 0;
   res1 = read_counter();
   if(res1 == res2) return 0;
   else
   {
      temp_count +=1;
      res2 = res1;
      if(temp_count > gearing)
      {
         
         if(!direction)
         {
            if(dial_dir) value+=1; else value-=1;
         }  
         else
         {
            if(dial_dir) value-=1; else value+=1;
         }
         temp_count = 0;
         return 1;
      }
      return 0;
   }
}
#endif


int8 misc_dial(int16 &value, int8 direction)
{
   res1 = read_counter();
   if(res1 == res2) return 0;
   else
   {
      if(!direction)
      {
         if(dial_dir) value+=1; else value-=1;
      }  
      else
      {
         if(dial_dir) value-=1; else value+=1;
      }
      return 1;
   } 
   return 0;
}





#ifdef include_dial_support

int8 freq_dial_basic(int32 &value, int8 dial_increment)
{
   if(!dl)
   {
      res1 = read_counter();
      if(res1 == res2) return 0;
      else
      {
            if(dial_dir) value+=dial_increment;
            else value-=dial_increment;
            return 1;
      }
   }
}

#endif

#ifdef include_dial_accel

#define dial_timer_max 255 //max spin down-timer. All percentages are based of this. Default 40000
#define dial_timer_increment 6
#define dial_timer_pullback 1
#define dialspeed1 10
#define dialspeed2 60

int16 freq_dial_accel(int32 &value, int16 start_increment)
{
res1 = read_counter();
   
   int1 res = 0;
   static int8 count = 0;
   static int8 stopcount = 0;  
   
   static int1 startcount = 0;
   //if(reset) { dial_timer = 400; dial_increment = default_increment;}
   
   
   static int16 dial_increment = start_increment;
   
   if(res1 != res2)
   {
      if(dial_dir) 
      {
      value +=dial_increment;
      }
      else 
      {
      value -=dial_increment;
      }
      res = 1;
      startcount = 1;
      stopcount = 0;
   }

   if(startcount)
   {
      if(count < 255) ++count;
   }

   if(count >= 30)
   {

      switch(res)
      {
         default: if(dial_timer > dial_timer_pullback) dial_timer -= dial_timer_pullback; else dial_timer = 0; break;
         case 1: if(dial_timer < dial_timer_max)dial_timer += dial_timer_increment; else dial_timer = dial_timer_max; break;
      }
   count = 0;

   }
   int8 dialtimer_check = (dial_timer >>2);
   if((dialtimer_check > 0) && (dialtimer_check < dialspeed1)) dial_increment = start_increment;
   if((dialtimer_check >= dialspeed1) && (dialtimer_check < dialspeed2)) {fine_tune_inh = 1; dial_increment = 15;}
   if(dialtimer_check >= dialspeed2) {fine_tune_inh = 1; dial_increment = 30;}
   if(res == 0) ++stopcount;

   if(stopcount>=128) {dial_timer = 0; startcount = 0; count = 0; dial_increment = start_increment;}
   
   //VFD_data (0xFF, 0xFF, (dial_timer >> 2), 0xFF, 1);
   
   //if (dial_timer < 300) dial_timer +=1;
   
return res;



//!   res1 = read_counter();
//!   
//!   int8 res = 0;
//!   static int16 count = 0;
//!   static int16 stop_count = 0;  
//!   static int16 dial_timer = 0;
//!   //if(reset) { dial_timer = 400; dial_increment = default_increment;}
//!   
//!   
//!   static int16 dial_increment = 0;
//!   if(!dial_increment) dial_increment = start_increment;
//!   
//!   if(res1 != res2)
//!   {
//!      count = 0; stop_count = 0;
//!      
//!      if(dial_dir) 
//!      {
//!      value +=dial_increment;
//!      }
//!      else 
//!      {
//!      value -=dial_increment;
//!      }
//!      
//!      if(dial_timer < dial_timer_max)dial_timer += dial_timer_decrement; else dial_timer = dial_timer_max;
//!      if(dial_timer > slow_speed_threshold) dial_increment = (dial_timer - slow_speed_threshold); else dial_increment = start_increment;
//!      if(dial_increment > 1) res = 2;
//!      if(dial_increment == 1) res = 1;//VFD_data(0xFF, 0xFF, dial_increment, 0xFF, 1,0);
//!      
//!         //VFD_data (0xFF, 0xFF, (dial_increment), 0xFF, 0,0, 1, 0); delay_ms(1000);
//!
//!   }
//!   else
//!   {
//!   ++count;
//!   if(count>negative_pulse_sample_rate) 
//!      {
//!      count = 0; //checktime = 0;
//!      if((dial_timer) > dial_timer_pullback )
//!      {
//!      dial_timer -= dial_timer_pullback;
//!      ++stop_count;
//!      }
//!      
//!      }
//!     
//!   if(stop_count > stop_reset_count) {stop_count = 0; dial_timer = 0; count = 0; dial_increment = start_increment; }
//!   res = 0;   
//!   }
//!   
//!   //if (dial_timer < 300) dial_timer +=1;
//!   
//!return res;
}     

#endif


