
//Yaesu CPU firmware v1.4.3 (c)2025
//Written by Matthew Bostock 
//Testing and research Siegmund Souza
//PLL diagrams and valuable info Daniel Keogh

//!v1.4.3a Changelog
//!Fixed VFO B being duplicated into VFO A (Flrig Kenwood mode)
//!Fixed VFO B not updated from cold start Kenwood mode
//!Fixed button tuning receiving changes over CAT
//!Removed code duplication, saving 2% on hex size
//!Accelerated dial adjusted for more instant speed and control
//!Changed beep hold-high period to ensure compatibility
//!   with PIC-based VFO dial boards
//!Added and tested support for PIC18F4520


#include <18F452.h>
#include <bootloader.h>
#fuses HS,PUT, NOWDT,NOPROTECT,NOLVP, BORV27
#use delay(clock=20000000)
#use rs232(baud=4800, xmit=PIN_C6, rcv=PIN_C7, parity=N, stop=2, ERRORS)

#include "user_settings.h"
#include "arrays.h"
#include "eeprom.h"
#include "calc_values.h"
#include "SUB_FUNC_ctrl.h"
#include "rotary_encoder_ctrl.h"
#include "VFD_ctrl.h"
#include "load_values.h"
#include "rig_state.h"
#include "PLL_ctrl.h"
#include "button_input.h"
#include "cat.h"
#include "tx_override.h"
#include "PMS.h"

void main()
{
  //!   setup_adc_ports(AN0);
   //!   setup_adc(ADC_CLOCK_INTERNAL);
   //!   set_adc_channel(0);
   setup_adc (ADC_OFF) ;
   set_tris_a (0b00001) ;
   set_tris_b (0b00000000) ;
   set_tris_c (0b11111111) ;
   set_tris_d (0b11111111) ;
   set_tris_e (0b000) ;
   
   Q64 (0) ;
   cat_tx_request = 0;
   stx = 0; ctx = 0;
   delay_ms (200) ;

   start:
   //enable_interrupts (INT_EXT); // turn on interrupts
   #ifdef include_cat
   disable_interrupts (INT_rda) ;
   enable_interrupts (INT_rda); //toggle interrupts to ensure serial is ready
    //enable interrupts FOR CAT
   #endif
   
   enable_interrupts (global);
   PLL_REF () ;
   
   mic_pressed = 0;
   pms = 0;
   AI = 0;
#ifdef store_to_eeprom 
 k4 = 1; delay_us (1);  
 if(pb1) 
 {
 while(pb1) {errorbeep(2);}
 save8(checkbyte_n, 0xFF);
 reset_cpu();
 }
 k4 = 0;
  

      if (load8(checkbyte_n) != 1) {set_defaults (); load_values (); }
      else load_values () ;
      refresh_all_state();
      //load_frequency (mem_mode) ;
#ELSE 
load_values () ;
#endif

   
   #ifdef include_cat
   counter1 = 0;
   counter = 0;
   k8 = 1; delay_us (1);
   if (pb0)
   {
      counter1 = 0; counter = 0; swap_cat_mode ();
      counter1 = 0; counter = 0; change_baud_rate ();
   }
   if(pb1) while(pb1){ cycle_mode_speed();}
   
   k8 = 0;
   
   
   
   baud_rate_n = load8(baud_n);
   counter1 = 0; counter = 0;

   #endif
   
   k4 = 1; delay_us(1);
   if(pb0) 
   {
   while(pb0){}
   set_min_max();
   }
   k4 = 0;
   
   fine_tune_display = 0;
   k1 = 1; delay_us (1);
   if (pb0) gen_tx = 0; // //widebanded?
   else gen_tx = 1;
   
   if(pb1) toggle_savetimer();
   k1 = 0;
   res1 = read_counter ();
   res2 = res1;
   //
   if (mem_mode != 2)
   {
   vfo_disp (active_vfo, dcs, frequency, mem_channel, 0);
   send_disp_buf(0);
   }
   #ifdef include_cb
   else 
   {
   VFD_data (0xFF, 0xFF, cb_channel, 0xFF, 0,0,2, 0);
   frequency = load_cb(cb_channel, cb_region);
   }
   #endif
   frequency = update_PLL(frequency);

   #ifdef cat_debug
   printf ("BAUD: % d\r\n", baud_rate_n);
   printf ("CAT mode: % d\r\n", cat_mode);

   #endif
   
   update = 1;
   beep () ;
   countermax = 6000;
      WHILE (true)
   {
      res1 = read_counter ();

      #ifdef include_pms
      IF (pms)
      {
         IF (program_mem_scan (frequency)) update =  1;
      }
      #endif

      #ifdef include_cat
      IF (! update)
      {
         if(((baud_rate_n < 5) && (counter > 1000)) || (baud_rate_n >= 5))
         {
         if(check_cat()) update = 1;
         }
      }
      
      IF (counter1 > 15000)
      {
         IF (AI)
         {
            calc_IF () ;
            send_IF () ;
         }

         counter1 = 0;
      }

      #endif
      IF (! update)
      {
         btnres = buttons (1);

         IF (btnres)
         {
            IF (! fine_tune){btnres = buttonaction (btnres, 10); }
            ELSE{btnres = buttonaction (btnres, 1); }
            update = 1;
         }
      }

      
      IF (mem_mode != 2)
      {
         #ifdef include_dial_accel
         IF (! update)
         {
            IF (speed_dial == 1)
            {
               IF (! fine_tune) vfodialres = freq_dial_accel_type1 (frequency, speed1);
               ELSE vfodialres = freq_dial_accel_type1 (frequency, 1);
            }

            IF (speed_dial == 0)
            {
               IF (! fine_tune) vfodialres = freq_dial_basic (frequency, speed1);
               ELSE vfodialres = freq_dial_basic (frequency, 1);
            }

            IF (vfodialres) {update = 1;}
         }

         #ELSE
         IF (! update)
         {
            IF (! fine_tune) vfodialres = freq_dial_basic (frequency, speed1);
            ELSE vfodialres = freq_dial_basic (frequency, 1);
            IF (vfodialres) {update = 1;}
         }

         #endif
      }

      #ifdef include_cb
      ELSE
      IF (! update)
      {
         cbdialres = misc_dial8 (cb_channel, 0);
         IF (cbdialres) update = 1;
      }

      #endif
      IF (counter > countermax)
      {
         IF (fine_tune) fine_tune_display = 0;
         IF (mem_mode != 2) vfo_disp (active_vfo, dcs, frequency, mem_channel, 0);

         #ifdef include_cb
         ELSE
         VFD_data (0xFF, dcs, cb_channel, 0xFF, 0,0, (cb_region + 6), 0);

         #endif
         counterstart = 0;
      }

      
      
      
      
      IF (! update)
      {
         txres = transmit_check ();

         IF (txres)
         {
            update = 1;
            force_update = 1;
         }
      }
      
      
      savetimermax = (10000);
      IF (savetimer >= savetimermax)
      {
         if(savetimerON) get_state();
         savetimer = 0;
      }
      
      IF (update)
      {
         //printf ("\r\n % ld", cat_storage_buffer[active_vfo]);
         update = 0;
         counterstart = 1; savetimer = 0;counter = 0;
         IF ( (fine_tune)&& (! catres))
         {
            IF ( (btnres == 21)|| (vfodialres == 1)) fine_tune_display = 1; else fine_tune_display = 0;
         }

         
         IF (mem_mode != 2)
         {
            IF ( ! fine_tune_display) vfo_disp (active_vfo, dcs, frequency, mem_channel,  0);
            ELSE vfo_disp (active_vfo, dcs, frequency, mem_channel, 0) ;
         }

         #ifdef include_cb
         ELSE
         {
         frequency = load_cb(cb_channel, cb_region);
         VFD_data (0xFF, dcs, cb_channel, 0xFF, 0,0, (cb_region + 6), 0);
         }
         
         #endif
         frequency = update_PLL (frequency) ;
         cat_storage_buffer[active_vfo] = frequency;
         micres = 0; vfodialres = 0; cbdialres = 0; btnres = 0; txres = 0; catres = 0;
      }

      
      if(savetimer < savetimermax){
      ++savetimer;
      }
      if(AI) ++counter1;
      IF (counterstart) ++counter;
   }
}
