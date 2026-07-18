int8 buttonaction (INT8 opt, int8 increment)
{
   int8 res = 0;
   SWITCH(opt)
   {
      CASE 1: beep(); clarifier_button_handler(); break;
      CASE 2: beep(); btn_dn_handler(0); break;
      CASE 3: beep(); btn_up_handler(0); break;
      CASE 4: mvfo_handler(); break; //MVFO
      CASE 5: beep(); vfoab_handler(); break; //VFOAB
      CASE 6: beep(); dial_lock_button_handler(); break;
      CASE 7: vfom_handler(); break; //VFOM
      CASE 8: beep(); mrvfo_handler(); break; //MRVFO
      CASE 9: beep(); split_button_handler(); break;
      CASE 10: vfom_swap_handler(); break; //VFOM SWAP
      CASE 11: micup(increment); res = 1; break;
      CASE 12: micdn(increment); res = 1; break;
      CASE 13: beep(); micfst(); break;
      CASE 14: micup_fst(); break;
      CASE 15: micdn_fst(); break;
      case 16: micup_hold(); break;
      case 17: micdn_hold(); break;
      case 18: beep(); micfst_hold(); break;
      case 19: micup_fst_hold(); break;
      case 20: micdn_fst_hold(); break;
      case 31: beep(); LONG_press_clarifier(); break;
      case 32: LONG_dn(); break;
      case 33: LONG_up(); break;
      case 34: LONG_press_mvfo(); break;
      case 35: LONG_press_vfoab(); break;
      case 36: LONG_press_dl(); break;
      case 37: LONG_press_vfom(); break;
      case 38: beep(); LONG_press_mrvfo(); break;
      case 39: LONG_press_split(); break;
      case 40: LONG_press_swap(); break;
      //default: RETURN 0; break;
   }

   if((!res) && (opt)) return 2;
   RETURN res;
}

int8 scan_mic_buttons()
{
   int8 res = 0;
   if (!mic_fast&& ! mic_up) res = 11;
   if (!mic_fast&& ! mic_dn) res = 12;
   if (mic_fast&&mic_up&&mic_dn)  res = 13;
   if (mic_fast&& ! mic_up) res = 14;
   if (mic_fast&& ! mic_dn) res = 15;
   return res;
}


#define ondelay 1
int8 scan_buttons()
{
int8 res = 0;
         k4 = 0; k8 = 0; k1 = 0; k2 = 1; delay_us(ondelay);
         IF(pb2) res = clarifier_button;//(RESULT: 1)Clarifier
         IF(pb1) res = down_button;//(RESULT: 2)Down
         IF(pb0) res = up_button;//(RESULT: 3)Up
         
         k2 = 0; k4 = 1; delay_us(ondelay);
         IF(pb2) res = mvfo_button;//(RESULT: 4)M > VFO
         IF(pb1) res = vfoab_button;//(RESULT: 5)VFO A / B
         IF(pb0) res = dial_lock_button;//(RESULT: 6)Dial lock
         
         k4 = 0; k8 = 1; delay_us(ondelay);
         IF(pb2) res = vfom_button;//(RESULT: 7)VFO > M
         IF(pb1) res = mrvfo_button;//(RESULT: 8)MR / VFO
         IF(pb0) res = split_button;//(RESULT: 9)SPLIT
         
         k8 = 0; k1 = 1; delay_us(ondelay);
         IF(pb1) res = vfom_swap_button;//(RESULT: 11)VFO < > M
         k8 = 0; k4 = 0; k2 = 0; k1 = 0;
return res;
}

#define countdelay 1
#define holdcount 50
int8 micdelay = 20;
int8 buttons(INT1 option)
{
   
   STATIC INT8 btnres = 0;
   STATIC INT8 micres = 0;
   STATIC INT8 count = 0;
   STATIC INT8 mic_count = 0;
   INT8 rtnres = 0;
   IF(pb2)
   {
      while(pb2){}
      if(btn_down)
      {
         if(!scan_buttons())  
         {
            
            btn_down = 0;
            IF(count > holdcount) btnres = 0;
            rtnres = btnres;
            btnres = 0;
            RETURN rtnres;
         }
      
         if(count < 255) 
         {
            ++count; 
            delay_ms(countdelay);
         }
           
         IF(count > holdcount)
         {
            rtnres = btnres + 30;
            btn_down = 1;
            IF(option == 1) btnres = 0;
            RETURN rtnres;
         }
             
      }    
      else
      {
         
         count = 0;
         btnres = scan_buttons();
         if(option == 2) return btnres;
         if(btnres) btn_down = 1;
         
      
      }
      
      if(mic_down)
      {
         if(!scan_mic_buttons())
         {
            
            mic_down = 0;
            if(mic_count > micdelay) micres = 0;
            rtnres = micres;
            micres = 0;
            RETURN rtnres;
         }
         micres = scan_mic_buttons();
         if(mic_count < 255) 
         {
         ++mic_count;
         delay_ms(countdelay);
         }
            if(micres == 13) micdelay = 100; else micdelay = 20;
            if(mic_count > micdelay) 
               {
               rtnres = micres + 5;
               mic_down = 1;
               //IF(option == 1) micres = 0;
               RETURN rtnres;
               }
         
      
      } else 
      {
         mic_count = 0;
         micres = scan_mic_buttons();
         if(micres) mic_down = 1;
      }
      
      
      //printf("%d\r\n", btn_down);
      
      
   }
      
         
     
   //if ( ( ! mic_fast) && (mic_up) && (mic_dn)) {mic_down = 0; miccnt =  0;}
      

      IF(sw_pms)
      {
         beep(); WHILE(sw_pms){}
         IF(pms)pms  = 0; else pms = 1;
      } 
   RETURN 0;
}
