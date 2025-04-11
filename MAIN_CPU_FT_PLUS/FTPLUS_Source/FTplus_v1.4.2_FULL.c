
//Yaesu CPU firmware v1.4 (c)2025
//Written by Matthew Bostock 
//Testing and research Siegmund Souza
//PLL diagrams and valuable info Daniel Keogh
#include <18F452.h>
#include "user_settings.h"

#include <bootloader.h>
#fuses HS,PUT, NOWDT,NOPROTECT,NOLVP, BORV27
#use delay(clock=20000000)
#use rs232(baud=4800, xmit=PIN_C6, rcv=PIN_C7, parity=N, stop=2, ERRORS)



#ifdef include_cat

int1 command_received = 0;
#endif

//create ram store, follows eeprom-style IF enabled
#ifndef store_to_eeprom
int32 ram_bank[60];
int8 var_bank[40];
#endif
void beep();
void load_10hz(INT8 val);
void load_100hz(INT8 val);
int32 load32(INT8 base);
void save8(INT8 base, int8 value);
int8 load8(INT8 base);

//Mem_map INT32 hex        OFFSET
//!0 VFOa                  00
//!1 VFOb                  04
//!2 vfo cache             08
//!3 mr cache              0C
//!4 mem 0                 10
//!5 mem 1                 14
//!6 mem 2                 18
//!7 mem 3                 1C
//!8 mem 4                 20
//!9 mem 5                 24
//!10  mem 6               28
//!11 mem 7                2C
//!12 mem 8                30
//!13 mem 9                34
//!14 mem A                38
//!15 mem B                3C
//!16 mem C                40
//!17 mem D                44
//!18 mem E                48
//!19 single offset        4C
//!20 multi offset 0.5     50
//!21 multi offset 1.5     54
//!22 multi offset 2.5     58
//!23 multi offset 4.0     5C
//!24 multi offset 7.5     60
//!25 multi offset 10.5    64
//!26 multi offset 14.5    68
//!27 multi offset 18.5    6C
//!28 multi offset 21.5    70
//!29 multi offset 25.0    74
//!30 VFOA band 1          78
//!31 VFOA band 1.8        7C
//!32 VFOA band 3.5        80
//!33 VFOA band 7.0        84
//!34 VFOA band 10.1       88
//!35 VFOA band 14.0       8C
//!36 VFOA band 18.0       90
//!37 VFOA band 21.0       94
//!38 VFOA band 24.0       98
//!39 VFOA band 28.0       9C
//!40 VFOA band 30         A0
//!41 VFOB band 1          A4
//!42 VFOB band 1.8        A8
//!43 VFOB band 3.5        AC
//!44 VFOB band 7.0        B0
//!45 VFOB band 10.1       B4
//!46 VFOB band 14.0       B8
//!47 VFOB band 18.0       BC
//!48 VFOB band 21.0       C0
//!49 VFOB band 24.0       C4
//!50 VFOB band 28.0       C8
//!51 VFOB band 30         CC
//!52 clar RX              D0
//!53 clar TX              D4
//!55 SPARE                D8
//!55 SPARE                DC
//only save IF value different from current contents


void split_value(int32 value, INT8 &d3, int8 &d4, int8 &d5, int8 &d6, int8 &d7, int8 &d8, int8 &d9)
{
   INT32 tmp_value = value;
   d3  = 0; WHILE (tmp_value > 999999){tmp_value -= 1000000; d3 += 1; }
   d4  = 0; WHILE (tmp_value > 99999){tmp_value -= 100000; d4 += 1; }
   d5  = 0; WHILE (tmp_value > 9999){tmp_value -= 10000; d5 += 1; }
   d6  = 0; WHILE (tmp_value > 999){tmp_value -= 1000; d6 += 1; }
   d7  = 0; WHILE (tmp_value > 99){tmp_value -= 100; d7 += 1; }
   d8  = 0; WHILE (tmp_value > 9) {tmp_value -= 10; d8 += 1; }
   d9  = 0; WHILE (tmp_value > 0) {tmp_value -= 1; d9 += 1; }
   //dbuf[0] = d3; dbuf[1] = (d4 * 10) + d5; dbuf[2] = (d6 * 10) + d7;dbuf[2] = (d8 * 10) + d9;
}

void join_value(int32 &value, INT8 d3, int8 d4, int8 d5, int8 d6, int8 d7, int8 d8, int8 d9)
{
int32 temp_value = 0;
temp_value = (((int32)d3 * 1000000) + ((int32)d4 * 100000) + ((int32)d5 * 10000) + ((int32)d6 * 1000) + ((int32)d7 * 100)+((int32)d8 * 10) + ((int32)d9));
value = temp_value;
//!
}


int8 get_state()
{
   INT8 state = 0;

   IF (mem_mode == 0)
   {
      IF (active_vfo == 0) {save_band_vfo_f (0, band, frequency); state = 1; }
      IF (active_vfo == 1) {save_band_vfo_f (1, band, frequency); state = 2; }
   }

   IF (mem_mode == 1) {save_cache_mem_mode_f (frequency); save8(mem_ch_n,mem_channel); state = 3; }
#ifdef include_cb
   IF (mem_mode == 2) {save8(cb_ch_n, cb_channel); state = 4;}
#endif
   RETURN state;
}

int8 read_state()
{
   INT8 state = 0;

   IF (mem_mode == 0)
   {
      IF (active_vfo == 0) {state = 1; }
      IF (active_vfo == 1) {state = 2; }
   }

   IF (mem_mode == 1) {state = 3; }
   IF (mem_mode == 2) {state = 4; }
   RETURN state;
}

int8 read_all_state()
{
   INT8 state = 0;

   IF (mem_mode == 0)
   {
      IF (active_vfo == 0) {frequency = load_band_vfo_f (0, band); state = 1; }
      IF (active_vfo == 1) {frequency = load_band_vfo_f (1, band); state = 2; }
   }

   IF (mem_mode == 1) {frequency = load_cache_mem_mode_f (); mem_channel = load8(mem_ch_n); state = 3; }
   #ifdef include_cb
   IF (mem_mode == 2) {cb_channel = load8(cb_ch_n); state = 4; }
   #endif
   RETURN state;
}

int8 refresh_all_state()
{
   INT8 state = 0;

   IF (mem_mode == 0)
   {
      IF (active_vfo == 0) {frequency = load_band_vfo_f (0, band); state = 1; }
      IF (active_vfo == 1) {frequency = load_band_vfo_f (1, band); state = 2; }
   }

   IF (mem_mode == 1) { mem_channel = load8(mem_ch_n); frequency = load_mem_ch_f (mem_channel);state = 3; }
   #ifdef include_cb
   IF (mem_mode == 2) {cb_channel = load8(cb_ch_n); state = 4; }
   #endif
   RETURN state;
}
#ifdef include_cb
int32 load_cb(int8 &cb_channel, int8 cb_region)
{
      IF (cb_region == 0){channel_start = 1; channel_amount = 40;} 
      IF (cb_region == 1){channel_start = 1; channel_amount = 40;} 
      IF (cb_region == 2){channel_start = 1; channel_amount = 80;}
      if(cb_channel < channel_start) cb_channel = channel_start;
      if(cb_channel > (channel_start + (channel_amount - 1))) cb_channel = (channel_start + (channel_amount - 1));
      if(cb_region == 0) cb_frequency = cb_channel_bank[cb_channel - 1];
      if(cb_region == 1) cb_frequency = cb_channel_bank[cb_channel + 39];
      if(cb_region == 2) cb_frequency = cb_channel_bank[cb_channel - 1];
      
      return cb_frequency;
}
#endif

//frequency loader FOR each mode. For mode 0 (VFO mode), probably not used
int32 load_frequency(INT8 mode)
{
   IF (mode == 0) {
   cat_storage_buffer[0] = load_band_vfo_f (0, band);
   cat_storage_buffer[1] = load_band_vfo_f (1, band);
   if(active_vfo == 0) frequency = cat_storage_buffer[0];
   if(active_vfo == 1) frequency = cat_storage_buffer[1];
   }
   IF (mode == 1) frequency = load_cache_mem_mode_f ();

   #ifdef include_cb
   IF (mode == 2) frequency = load_cb(cb_channel, cb_region);
   #endif
   RETURN frequency;
}

void save_frequency(INT8 mode)
{
   IF (mem_mode == 0) save_band_vfo_f (active_vfo, band, frequency);
   IF (mem_mode == 1) save_cache_mem_mode_f (frequency);

   #ifdef include_cb
   IF (mem_mode == 2) save8(cb_ch_n,cb_channel);

   #endif
}
void errorbeep(INT8 beeps);

void VFD_data(INT8 vfo_grid, int8 dcs_grid, int32 value, int8 channel_grid, int1 zeroes, int8 blank_digit, int8 display_type, int1 fast_update);

void toggle_savetimer()
{
if(savetimerON == 1) savetimerON = 0; else savetimerON = 1;
VFD_data (0xFF, 0xFF, savetimerON + 1, 0xFF, 0,0, 1, 0);
errorbeep(savetimerON + 2);
save8(savetimer_n, savetimerON);
delay_ms(1000);
}


int32 update_PLL(INT32 calc_frequency);

#ifdef include_mic_buttons

int8 mic_button(INT16 increment);

#endif

void calc_IF();

void send_IF();
int8 parse_cat_command_kenwood ();
int8 parse_cat_command_yaesu ();
int16 freq_dial_accel_type1(INT32 &value, int16 start_increment);
//int16 freq_dial_accel_type2(INT32 &value, int16 start_increment);
int16 freq_dial_basic(INT32 &value, int16 dial_increment);
int8 misc_dial8(INT8 &value, int8 direction);
void Q64(INT8 val);
int8 program_mem_scan(int1 man_tune, INT32 curr_freq);
int8 buttonaction (INT8 res, int8 increment);
void PLL_REF();
void set_defaults();
void load_values();
int8 transmit_check();
void check_limits();
int8 read_counter();
void cls();
void swap_cat_mode();
void change_baud_rate();
int8 buttons(int1 LONG_press_enabled);

void VFD_data(INT8 vfo_grid, int8 dcs_grid, int32 value, int8 channel_grid, int1 zeroes, int8 blank_digit, int8 display_type, int1 fast_update);
void vfo_disp(INT8 vfo, int8 dcs, int32 freq, int8 ch, int1 fast_update);
int8 calc_band(INT32 frequency);
int8 read8(INT8 base);
void cycle_mode_speed();
int8 check_cat();
void refresh_screen(int8 state)
{  
   if(state != 4) vfo_disp (active_vfo, dcs, frequency, mem_channel, 0) ;
   #ifdef include_cb
   else VFD_data (0xFF, 0xFF, cb_channel, 0xFF, 0,0, (cb_region + 6), 0);
   #endif
}

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
   cls () ;
#ifdef store_to_eeprom 
 k4 = 1; delay_us (1);  
 if(pb1) 
 {
 while(pb1) {errorbeep(2);}
 save8(checkbyte_n, 0xFF);
 reset_cpu();
 }
 k4 = 0;
  

      IF (load8(checkbyte_n) != 1) {set_defaults (); load_values (); }
      ELSE load_values () ;
      load_frequency (mem_mode) ;
#ELSE 
load_values () ;
#endif

   
   #ifdef include_cat
   counter1 = 0;
   counter = 0;
   k8 = 1; delay_us (1);
   IF (pb0)
   {
      counter1 = 0; counter = 0; swap_cat_mode ();
      counter1 = 0; counter = 0; change_baud_rate ();
   }
   if(pb1) while(pb1){ cycle_mode_speed();}
   k8 = 0;
   
   baud_rate_n = load8(baud_n);
   counter1 = 0; counter = 0;
   k8 = 0;

   #endif
   
   fine_tune_display = 0;
   k1 = 1; delay_us (1);
   IF (pb0) gen_tx = 0; // //widebanded?
   ELSE gen_tx = 1;
   if(pb1) toggle_savetimer();
   k1 = 0;
   res1 = read_counter ();
   res2 = res1;
   //
   IF (mem_mode != 2)vfo_disp (active_vfo, dcs, frequency, mem_channel, 0);

   #ifdef include_cb
   ELSE VFD_data (0xFF, 0xFF, cb_channel, 0xFF, 0,0,2, 0);

   #endif
   

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

      IF (pms)
      {
         IF (program_mem_scan (0, frequency)) update =  1;
      }

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
         if(savetimerON) save_frequency (mem_mode);
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


#include "VFD.c"
#include "buttons.c"
#include "cat.c"
#include "eeprom.c"
#include "dial.c"
#include "rig_functions.c"
#include "pms.c"
#include "setup_checks.c"
#include "memory_operations.c"
#include "button_operations.c"
#include "manual_tuning.c"
#include "user_settings.c"
