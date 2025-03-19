int1 dial_moved()
{
static int8 counter2;
int8 counter1 = read_counter();
if(counter2 != counter1)
{
counter2 = counter1;
return 1;
}
return 0;
}

   //dial output counts(the value returned - the MAIN variable for eg frequency
   //COSMETICS:-
   //note the perculiar dial increments... They could just be nice round numbers like 5, 10, 100, 1000 etc...
   //However... Shifting the next-to-last digit lets all digits in the VFD change. If these were nice round numbers, only the digit being changed would appear to change.
   //This is only a cosmetic thing. Feel free to round them up or down as you like
   
   //outspeed1 = 2 = 10khz approx per revolution. 1 = 5khz approx per revolution



#ifdef include_dial_accel

//here we set our dial timer ( or triggers). As the timer decreases, it will pass these threasholds
//these dial timers can be anything lower than main dial timer. 
//set these to whatever you like. eg if you have a main timer of 2000, you could set timer 1 to 300, timer 2 to 250 etc.
//They need to descend because the main timer (dial_timer) descends

//basic tug-o-war between positive and negative pulses. Dial timers are in percentages / 10
//screen MUST be refreshed immediately. Set update_display flag ( as below)

#define dial_timer_max 1000 //max spin down-timer. All percentages are based of this. Default 40000
//these are the trigger percentages. As dial_timer decreases, we will approach eg 80% of dialtimer, which is percent_of_d_timer_speed2.
//We can change these percentages so different speeds happen earlier or later. Defaults 80, 60, 40, 20, 10.

#define percent_of_d_timer_speed1 80 //default 80%
#define percent_of_d_timer_speed2 60 //default 80%
#define percent_of_d_timer_speed3 40 //default 60%
#define percent_of_d_timer_speed4 20 //default 40%
#define percent_of_d_timer_speed5 0 //default 20%


#define stop_reset_count 5 //stop spin check sampling count. How fast dial reverts to lowest increment. Higher - longer, Lower, shorter. Default 25
#define negative_pulse_sample_rate 50 //how often to check for negative pulse to fight against the decrement. Higher number means easier acceleration. Lower is harder (eg faster and harder). Default 50
#define dial_timer_decrement 5 //overall timer decrement when spinning. How fast the main timker decreases. Default 5. Lower = longer spin before it speeds up. Not harder, just longer
#define dial_timer_pullback 30 //dial increment when negative (eg when slowing down spinning). How hard we fight against the decrement. Needs to be more than decrement. Default 20. Lower = acceleration increments kick in earlier



int16 freq_dial_accel_type1(int32 &value, int16 start_increment)
{
   //if(res1 == res2) return 0;
   int8 res = 0;
   static int16 count = 0;
   static int16 stop_count = 0;  
   static int16 dial_timer = dial_timer_max;
   //if(reset) { dial_timer = 400; dial_increment = default_increment;}
   static int16 dial_timer1, dial_timer2, dial_timer3, dial_timer4, dial_timer5;
   
   
   static int16 dial_increment = start_increment;
   dial_timer1 = (dial_timer_max / 100) * percent_of_d_timer_speed1; //eg 80%
   dial_timer2 = (dial_timer_max / 100) * percent_of_d_timer_speed2;
   dial_timer3 = (dial_timer_max / 100) * percent_of_d_timer_speed3;
   dial_timer4 = (dial_timer_max / 100) * percent_of_d_timer_speed4;
   dial_timer5 = (dial_timer_max / 100) * percent_of_d_timer_speed5;
  
   if(res1 != res2)
      {
      tmp = dial_dir;
      if(tmp) 
      {
      value +=dial_increment;
      }
      else 
      {
      value -=dial_increment;
      }
      
      stop_count = 0;

      if(dial_timer <dial_timer_pullback) dial_timer = dial_timer_pullback;
      if(dial_timer >= dial_timer_max) dial_timer = dial_timer_max;
      dial_timer -= dial_timer_decrement;
      res2 = res1;
      res = dial_increment;
      //VFD_data(0xFF, 0xFF, dial_increment, 0xFF, 1,0);
      
   }
   else
   {
      if(count == negative_pulse_sample_rate)
      {
         if(dial_timer <dial_timer_pullback) dial_timer = dial_timer_pullback;
         if(dial_timer >= dial_timer_max) dial_timer = (dial_timer_max - dial_timer_pullback);
         dial_timer +=dial_timer_pullback;
         //display_driver(0,0,0,0,dial_timer,0);
         if(!dial_moved())
         {
            ++stop_count;
            if(stop_count >= stop_reset_count) 
            {
               dial_timer = dial_timer_max;
               res = 0;
               
            }
         }
      }
      
   }
   ++count;
   if(count>negative_pulse_sample_rate) count = 0;
   //if (dial_timer < 300) dial_timer +=1;
   
   if(dial_timer > dial_timer1)                                   dial_increment = start_increment; 
   if((dial_timer <= dial_timer1) && (dial_timer > dial_timer2))  dial_increment = speed1;
   if((dial_timer <= dial_timer2) && (dial_timer > dial_timer3))  dial_increment = speed2;
   if((dial_timer <= dial_timer3) && (dial_timer > dial_timer4))  dial_increment = speed3;
   if((dial_timer <= dial_timer4) && (dial_timer >= dial_timer5))  dial_increment = speed4;
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




