#define clarifier_button 1
#define down_button 2
#define up_button 3
#define mvfo_button 4
#define vfoab_button 5
#define dial_lock_button 6
#define vfom_button 7
#define mrvfo_button 8
#define split_button 9
#define vfom_swap_button 10

int8 dcs_res[32] = 
{
   //dcs dl cl sl
   15, 0, 0, 0,
   14, 0, 1, 1,
   12, 0, 0, 1,
   4, 1, 1, 1,
   3, 1, 1, 0,
   2, 0, 1, 0,
   1, 1, 0, 0,
   0, 1, 0, 1
};

int8 get_dcs()
{
   INT8 res = 15;
   for(INT8 i  = 0; i < 8; i++)
   {
      IF((dl == dcs_res[(i * 4) + 1])&&(cl == dcs_res[(i * 4) + 2])&&(sl == dcs_res[(i * 4) + 3])){res = dcs_res[i * 4]; break; }
   }
   RETURN res;
}

void set_dcs(INT8 res)
{
   for (INT8 i = 0; i < 8; i++)
   {
      IF(res == dcs_res[(i * 4)]){dl = dcs_res[(i * 4) + 1]; cl = dcs_res[(i * 4) + 2]; sl = dcs_res[(i * 4) + 3]; break;}
   }
}



int8 buttonaction (INT8 opt)
{
   if((dl) && (opt == 36)) opt = 30;
   SWITCH(opt)
   {
#if defined (include_standard_btn) || defined(include_cat_basic)
      CASE 1: beep();         clarifier_button_handler();         break;
      CASE 2: beep();         btn_dn_handler(0);                  break;
      CASE 3: beep();         btn_up_handler(0);                  break;
      CASE 4:                 mvfo_handler();                     break; //MVFO
      CASE 5: beep();         vfoab_handler();                    break; //VFOAB
      CASE 6: beep();         dial_lock_button_handler();         break;
      CASE 7:                 vfom_handler();                     break; //VFOM
      CASE 8: beep();         mrvfo_handler();                    break; //MRVFO
      CASE 9: beep();         split_button_handler();             break;
      CASE 10:                vfom_swap_handler();                break; //VFOM SWAP
      CASE 11:                micup();                            break;
      CASE 12:                micdn();                            break;
      CASE 13: beep();        micfst();                           break;
      CASE 14:                micup_fst();                        break;
      CASE 15:                micdn_fst();                        break;
      case 16:                micup_hold();                       break;
      case 17:                micdn_hold();                       break;
      case 18: beep();        micfst_hold();                      break;
      case 19:                micup_fst_hold();                   break;
      case 20:                micdn_fst_hold();                   break;
      case 35:                long_press_vfoab();                 break;
      case 36:                long_press_dial_lock_ndl();         break;
#endif
#ifdef include_enhanced_btn
      case 30:                long_press_dial_lock_dl();          break;
      case 31:                long_press_clarifier();             break;
      case 32:                long_press_btn_dn();                break;
      case 33:                long_press_btn_up();                break;
      case 34:                long_press_mvfo();                  break;
      
      case 37:                long_press_vfom();                  break;
      case 38:                long_press_mrvfo();                 break;
      case 39:                long_press_split();                 break;
      case 40:                long_press_swap();                  break;
#endif
   }
   
   dcs = get_dcs();
   RETURN opt;
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

#define countdelay 500
#define holdcount 100
#define micholdcount 30
int8 micdelay = 20;
int8 buttons(INT8 option)
{
   
   STATIC INT8 btnres = 0;
   STATIC INT8 micres = 0;
   STATIC INT8 count = 0;
   STATIC INT8 mic_count = 0;
   INT8 rtnres = 0;
   int8 res = 0;
   int8 debounce;
   
   if(option == 2) debounce = 0;
   else debounce = 4;
   IF(pb2)
   {
      while(pb2){}
      if(!btn_down)
      {
         btnres = scan_buttons(); if(btnres) btn_down = 1;
         count= 0;
      }
      
      if(btn_down)
      {
         if(scan_buttons())
         {
            if(count < 255) ++count;
            if(option != 2) delay_us(countdelay);
         }

         if(count > debounce)
         {
            if(!option)
            {
               if(count < holdcount) res = 1;
               if(count >= holdcount) res = 2;
            }
            
            if(option == 1)
            {
               if(count < holdcount) res = 1;
               if(count >= holdcount) res = 3;
            }
            
            if(option == 2)
            {
               rtnres = btnres;
               btnres = 0;
               return rtnres;
            }
         }
         
         if(res == 1)
         {
            if(!scan_buttons())
            {
               //add some protection for staticy buttons
               for(int i = 0; i < 10; ++i)
               {
                  if(scan_buttons()) break;
               }
               if(i >= 10)
               {
                  if(long_press) {rtnres = 0; long_press = 0;}
                  else rtnres = btnres;
                  btnres = 0;
                  btn_down = 0;
                  return rtnres;
               }
            }
         }
         
         if(res == 2)
         {
               if(btnres) rtnres = btnres + 30;
               btnres = 0;
               count = 1;
               long_press = 1;
               btn_down = 0;
               return rtnres; 
         }
         
         if(res == 3)
         {
               if(btnres) rtnres = btnres + 30;
               if(!scan_buttons())
               {
                  count = 1;
                  rtnres = 0;
               }
               long_press = 1;
               return rtnres; 
         }
      
      
      }
      
      if(!mic_down)
      {
         micres = scan_mic_buttons();
         if(micres) mic_down = 1;
         mic_count= 0;
      
      }
      
      if(mic_down)
      {
         if(mic_count < 255) ++mic_count;
            delay_us(micdelay);
      
         if(mic_count)
         {

               if(mic_count < micholdcount) res = 1;
               if(mic_count >= micholdcount) res = 2;

         }
         
         if(res == 1)
         {
            if(!scan_mic_buttons())
            {
               if(long_press) {long_press = 0; return 0;}
               else rtnres = micres;
               micres = 0;
               mic_down = 0;
               return rtnres;
            }
         }
         
         if(res == 2)
         {
               micres = scan_mic_buttons();
               rtnres = micres + 5;
               if((!scan_mic_buttons()) || (rtnres == 18))
               {
                  mic_down = 0;
                  if(rtnres != 18) rtnres = 0;
                  
               }
               long_press = 1;
               return rtnres; 
         }
         
         
      
      
      }
   }
     
      

      IF(sw_pms)
      {
         if(!app_detected)
         {
            beep(); WHILE(sw_pms){}
            delay_ms(200);
            IF(pms)pms  = 0; else pms = 1;
         } else pms = 0;
      }
   RETURN 0;
}
