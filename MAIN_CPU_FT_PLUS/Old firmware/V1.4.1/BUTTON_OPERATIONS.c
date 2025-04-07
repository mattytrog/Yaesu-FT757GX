
void clarifier_button(INT8 state)
{
   IF(state  != 4)
   {
      IF(cl)cl  = 0;
      ELSE{sl = 0; cl = 1; save_clar_TX_f(frequency); }
      dcs = get_dcs();
   }

   ELSE
   {
      VFD_data(0xFF, 0xFF, frequency, 0xFF, 0, 0, 0, 0);
      delay_ms(1000);
   }
}

void up_down(int1 updown, INT8 state)
{
   IF(state  < 3)
   {
      IF( ! sw_500k)
      {
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

         save_band_n(band);
         frequency = load_band_vfo_f(active_vfo, band);
         cat_storage_buffer[0] = load_band_vfo_f(0, band);
         cat_storage_buffer[1] = load_band_vfo_f(1, band);
      }

      ELSE
      {
         IF( ! updown)frequency  -= 50000;
         ELSE frequency += 50000;
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

void dial_lock_button()
{
   IF(dl)dl  = 0; else dl = 1;
   set_dl();
   dcs = get_dcs();
}

void split_button(INT8 state)
{
   IF(state < 3)
   {
      beep(); 
      IF(sl)sl  = 0; else{sl = 1; cl = 0; }
      dcs = get_dcs();
   }

   #ifdef include_cb
   IF(state == 4)toggle_cb_region();

   #endif
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

void micfst_hold(int8 state)
{
            IF(state  < 3)mem_op(1);
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

void LONG_press_clarifier() {program_offset();}
void LONG_press_mvfo() {toggle_speed_dial();}
void LONG_press_vfoab() {reset_checkbyte_n(); reset_cpu();}
void manual_adjust_frequency();

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
   
   save_cb_reg_n(cb_region);

   #endif
}

void LONG_press_swap(){toggle_per_band_offset();}
int8 buttonaction (INT8 res, int8 increment)
{
   INT8 state = read_state();
   
   SWITCH(res)
   {
      CASE 1: beep(); clarifier_button(state); break;
      CASE 2: up_down(0, state); break;
      CASE 3: up_down(1, state); break;
      CASE 4: mem_op(3); break; //MVFO
      CASE 5: beep(); mem_op(1); break; //VFOAB
      CASE 6: beep(); dial_lock_button(); break;
      CASE 7: mem_op(4); break; //VFOM
      CASE 8: beep(); mem_op(5); break; //MRVFO
      CASE 9: split_button(state); break;
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
      case 35: LONG_press_vfoab(); break;
      case 36: LONG_press_dl(); break;
      case 37: LONG_press_vfom(); break;
      case 38: LONG_press_mrvfo(); break;
      case 39: LONG_press_split(); break;
      case 40: LONG_press_swap(); break;
      default: RETURN 0; break;
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
                  
                  k2 = 0; k4 = 0; k8 = 0;
                  
                  count = 0;
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
            if(mic_count < holdcountmic) ++mic_count;
            if(mic_count >= holdcountmic) 
            {
            if(micres) rtnres = micres + 5;
            else rtnres = 0;
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





int8 mic_input()
{
   if(mic_fast)RETURN 3;
   if( ! mic_up)RETURN 1;
   if( ! mic_dn)RETURN 2;
   RETURN 0;
}

