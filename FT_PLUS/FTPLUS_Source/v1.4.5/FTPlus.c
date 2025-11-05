
//Yaesu CPU firmware v1.4.5 (c)2025
//Written by Matthew Bostock 
//Testing and research Siegmund Souza
//PLL diagrams and valuable info Daniel Keogh

//!v1.4.5 Changelog
//!CB system reorganised, removed 80ch mode as now all bands are programmed in blocks of 40
//All CB bands now programmed. Bands now chacked and validated automatically
//Improved CAT auto-mode selection
//Improved CAT stability
//Fixed button operation on original TMS display chips. Sorry about that
//Added ALL CB scanner to PMS. Press 500k switch before pressing PMS to scan through all CB bands
//changed out-of-band detection to a timer0 interrupt
//reduced hex size by more duplication deletion

#include <18F4520.h>
//#include <bootloader.h>
#fuses HS,PUT, NOWDT,NOPROTECT,NOLVP,BORV27
#use delay(clock=20000000)
#use rs232(baud=4800, xmit=PIN_C6, rcv=PIN_C7, parity=N, stop=2, ERRORS)

#define minimum_freq 10000
#define maximum_freq 3200000

//uncomment for bare minimum size. No mic buttons, dial acceleration, CB or CAT. I advise a bigger PIC!
//#define small_rom 
//remove bits and pieces if you wish, or problem is suspected
#ifndef small_rom
#define include_cat                       //uncomment = disable CAT entirely
#define include_dial_accel                //uncomment = no accelerated dial
#define include_cb                        //uncomment = no CB function
#define include_offset_programming        //uncomment = no offset correction for out of tune rigs
#define include_pms                       //uncomment = remove PMS scanning feature
#define include_manual_tuning             //uncomment = remove button tuning (accessed by dial lock long press, when dial locked
#define include_savetimer_switch          //uncomment = remove ability to turn eeprom saving on or off with buttons. Instead we use defines in code
#define include_set_minmax                //uncomment = remove ability to reprogram minimum and maximum
#endif

#define store_to_eeprom
#define jump_500k 50000                   //amt to jump when 500k button pressed. in kc * 10
//PMS Settings
#define scan_pause_count 3000              // Pause after of squelch break
#define VFO_dwell_time 1                  // Delay after tuning to next frequency. Lower numbers = faster scanning speed, though may overshoot if too fast
#define MR_dwell_time 200       
#define CB_dwell_time 10       
#define default_cat_mode 0                //0 = yaesu, 1 = kenwood
#define vfo_button_tuning_flash_speed 10
#define default_save_timer 3              //roughly seconds. Not accurate. About a second or two off over 30 seconds. Close enough for agricultural.

                                          //uncomment each not required CB band. Bands are still flashed, but flashed to be disabled
#define super_super_low_CB
#define super_low_CB
#define low_CB
#define mid_CB
#define high_CB
#define super_high_CB
#define super_super_high_CB
#define UK_CB


#include "def_var.h"
#include "arrays.h"
#include "cb_bands.h"
#include "eeprom.h"
#include "calc_values.h"
#include "SUB_FUNC_ctrl.h"
#include "rotary_encoder_ctrl.h"
#include "VFD_ctrl.h"
#include "load_values.h"
#include "rig_state.h"
#include "PLL_ctrl.h"
#include "button_input.h"
#include "mic_button_input.h"
#include "cat.h"
#include "tx_override.h"
#include "PMS.h"




void basic_setup()
{
   setup_low_volt_detect(LVD_27 | LVD_TRIGGER_BELOW);
   setup_timer_0(T0_INTERNAL | T0_DIV_4);       // Timer0 configuration:
   //set_timer0(t2);
   setup_adc (ADC_OFF) ;
   set_tris_a (0b00001) ;
   set_tris_b (0b00000000) ;
   set_tris_c (0b11111111) ;
   set_tris_d (0b11111111) ;
   set_tris_e (0b000) ;
   
   Q64 (0) ;
   cls();
   delay_ms (200) ;
  
   start:
  
   clear_interrupt(INT_TIMER0);
   enable_interrupts(INT_TIMER0);
   #ifdef include_cat
   clear_interrupt(INT_rda); 
   enable_interrupts (INT_rda); 
   
    //enable interrupts FOR CAT
   enable_interrupts (global);
   #endif
   
   PLL_REF () ;
   
   mic_pressed = 0;
   pms = 0;
   AI = 0;
   fine_tune_display = 0;
   counter1 = 0;
   counter = 0;
   cat_tx_request = 0;
   stx = 0; ctx = 0;
   pause_cat = 0;
#ifdef include_cat
   command_received = 0;
#endif
   SWITCH_CAT = 0;

#ifdef include_cb
   check_cb();
#endif
}

void cold_boot_overrides()
{


 k4 = 1;
   delay_us (1);
   if(pb1) 
   {
      errorbeep(2);
      while(pb1) {}
      save8(checkbyte_n, 0xFF);
      reset_cpu();
   }
 k4 = 0;
      
   if (load8(checkbyte_n) != 1) 
   {
      set_defaults (); 
      load_values (); 
   }
   else load_values () ;

#ifdef include_cat
   k8 = 1; 
      delay_us (1);
      if (pb0) //split
      {
         counter1 = 0; counter = 0; swap_cat_mode ();
         counter1 = 0; counter = 0; change_baud_rate ();
      }
      if(pb1) while(pb1){ cycle_mode_speed();} //mr/vfo
#ifdef include_savetimer_switch   
      if(pb2) {toggle_savetimer(); while(pb2){} }//vfo>m}
#endif
   k8 = 0;
#endif
   
//!   k4 = 1;
//!      delay_us(1);
//!      if(pb0) 
//!      {
//!         while(pb0){}
//!         set_min_max();
//!      }
//!   k4 = 0;
   
   
   k1 = 1;
      delay_us (1);
      if (pb0) gen_tx = 0; // //widebanded?
      else gen_tx = 1;
      
      if((gen_tx == 0) && (get_state(0) == 4)) {set_state(1);}

   k1 = 0;
   read_all_state();  
}


void main()
{
   
   basic_setup();
   cold_boot_overrides();
   
   res1 = read_counter ();
   res2 = res1;
   baud_rate_n =  load8(baud_n);
   int8 state = get_state(0); int8 update = 1;
  
   while(true)
   {
   res1 = read_counter ();
   
   #ifdef include_pms
   IF (pms){pmsres = program_mem_scan (frequency); update =  1;}
   #endif
   
   #ifdef include_cat
   if(!pause_cat)
   {
      if(!update)
      {
        update = check_cat(); 
      }
   }
      IF (counter1 > 15000)
   {
      calc_IF () ; send_IF () ; counter1 = 0;
   }
   #endif
   
   if(!update)
   {
      btnres = buttons (1);
      IF (btnres)
      {
         IF (! fine_tune){update = buttonaction (btnres, 10); }
         ELSE{update = buttonaction (btnres, 1); counter = 0;}
         state = get_state(0);
      }
      
   }
   
   if(!update)
   {
   
      if(state != 4)
      {
         IF (speed_dial == 0)
         {
            switch(fine_tune)
            {
            case 0: update = (freq_dial_basic (frequency, 3)); break;
            case 1: update = (freq_dial_basic (frequency, 1));break;
            }
         }
         IF (speed_dial == 1)
         {
            switch(fine_tune)
            {
            case 0: update = (freq_dial_accel_type1 (frequency, speed1)); break;
            case 1: update = (freq_dial_accel_type1 (frequency, 1));break;
            }
         }
         
         if(update) counter = 0;
      }
      else
      {
         if(!cl) update = misc_dial8 (cb_channel, 0);
         else update = freq_dial_basic (frequency, speed1);
         
         if(update) {counter = 0; if(!cl) read_all_state();}
      }
      
   }
    
   if(!update) update = (transmit_check ()) ; 
     
    IF (update)
    {
      if(update == 1)
      {
         if(fine_tune) fine_tune_display = 1;
         
      } else fine_tune_display = 0; 
      counterstart = 1;
      update = 0; 
      
      update_screen(state, 0);
      update_PLL (frequency) ;
      update_buffers(frequency);
    
   }
   
   
    if(counter == 6000)
    {
       fine_tune_display = 0; update = 2;
    }
    
    if(counter > 24000)
    {
       counterstart = 0; counter = 0;
       if((!cl) && (!sl))
       {
         if(savetimerON == 1) get_state(1);
       }
    }
    
    
      
     
   
   
   
   
   send_disp_buf (150);
   IF (counterstart) ++counter;
   if(AI) ++counter1;
   }
}

