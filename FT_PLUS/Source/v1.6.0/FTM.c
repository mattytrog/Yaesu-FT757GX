#define 18f4520
//#define 18f452
//#define 16f877a //FTMinus is automatically selected
//#define 16f877 //FTMinus is automatically selected

#define bootloader_upgrade //FTPlus only. If you do not own a Pickit, this MUST BE UNCOMMENTED!!!
//define ft_minus // Very basic mode, echoing standard CPU. Meant for use with smaller PICs

#include "chip_set.c"

#ifdef ft_minus
      
      //#define use_ram_eeprom //Not enough space to use this on small PICs
      #define include_display //VFD control
      #define include_dial_support
      #define include_standard_btn //Button controls / standard memory management features
      //#define adjust_baud_rate
      #define include_cat_yaesu // Yaesu basic CAT mode
      //#define include_cat_kenwood //on small chips, this works on its own, with VFO wheel, maybe with display enabled too. Thats ALL! Baud needs changing in source. Headless CAT server?
      #define include_pms_basic //A basic frequency scanner
      #define include_oob_checking // Out of band checking, based on front panel switch position
      //#define include_fine_tuning // Fine tuning - Hidden digit scrolls into view. Toggle on/off with D-LOCK. Needs display and standard buttons enabled

#else

      //#define use_ram_eeprom
      #define include_display
      #define include_dial_support
      #define include_standard_btn
      #define include_enhanced_btn
      #define adjust_baud_rate
      //#define include_cat_basic // Yaesu basic CAT mode
      //#define include_pms_basic //A basic frequency scanner
      #define include_cat_kenwood
      #define include_cat_yaesu
      #define include_pms_advanced
      #define include_oob_checking
      #define include_offset_programming
      #define include_manual_tuning
      #define include_dial_accel
      #define include_cb
      #define include_fine_tuning
      #define include_app_support
      #define include_custom_PLL_REF
      #define include_custom_filter_selection
      #define include_app_selectable_minmax

#endif

unsigned int32 storage_buffer[5];
unsigned int32 aux_buffer[3];
#define temp_freq storage_buffer[0]
int16 counter;
int16 dial_timer;
int8 res1, res2, dcs, state, band1, band2, band3, mem_channel, PLLband, fl_cnt;
int8 lp_dial_lock_ndl, lp_vfoab;
int8 baud_rate_n;

#ifdef include_cat_kenwood
int8 dummy_mode = 49;
#endif

#ifdef include_enhanced_btn
int16 offset;
int8  lp_dial_lock_dl,
      lp_clarifier,
      lp_btn_dn,
      lp_btn_up,
      lp_mvfo,
      lp_vfom,
      lp_mrvfo,
      lp_split,
      lp_swap;
#ifdef include_cb
int8 cb_channel;
int8 cb_region;
#endif

#endif

#ifdef include_custom_PLL_REF
unsigned int16 PLL1REF;
unsigned int16 PLL2REF;
#endif
#ifdef include_app_selectable_minmax
unsigned int32 minimum_freq = 0;
unsigned int32 maximum_freq = 0;
#endif
#define min_freq 10000
#define max_freq 3200000
//#define eeprom_save_debug8
#define offset_max 9999
//#define eeprom_save_debug32
#define default_PLL1REF 1500
#define default_PLL2REF 30

#define mem_ch_block 0
#define offset_block 15
#define vfoa_block 26
#define vfob_block 37

#include "eeprom_state.c"
#include "display_ctrl.c"
#include "PLL_ctrl.c"
#include "encoder.c"
#include "enh_button.c"
#include "std_button.c"
#include "buttons.c"
#include "cat.c"
#include "pms.c"
#include "cb.c"



void set_defaults()
{
   for(int i = 0; i < 255; ++i)                          {write8(i, 0);}
   for (i = mem_ch_block; i <= (offset_block - 1); i++)  {write32 (i, 700000); }
   for (i = offset_block; i <= (vfoa_block - 1); i++)    {write32 (i, 0); }
   for (i = vfoa_block; i <= (vfob_block - 1); i++)      {write32 (i, band_bank[i - vfoa_block]); }
   for (i = vfob_block; i <= (vfob_block + 10); i++)     {write32 (i, band_bank[i - vfob_block]); }
#ifdef include_custom_PLL_REF
                                                   write32((vfob_block + 11), default_PLL1REF);
                                                   write32((vfob_block + 12), default_PLL2REF);
#endif   
#ifdef include_app_selectable_minmax
                                                   write32((vfob_block + 13), min_freq);
                                                   write32((vfob_block + 14), max_freq);

#endif
#ifdef include_custom_filter_selection
                                                   write8(fil0, 1);
                                                   write8(fil1, 1);
                                                   write8(fil2, 2);
                                                   write8(fil3, 3);
                                                   write8(fil4, 4);
                                                   write8(fil5, 5);
                                                   write8(fil6, 6);
                                                   write8(fil7, 7);
                                                   write8(fil8, 8);
                                                   write8(fil9, 9);
#endif
                                                   write8(band1_n, 3);
                                                   write8(band2_n, 3);
                                                   write8(band3_n, 3);
                                                   write8(state_n, 1); //Active VFO A / B 
                                                   write8(fine_tune_n, 0);
                                                   write8(dial_n, 0);
                                                   write8(baud_n, 3);
                                                   write8(savetimer_n, 1);

                                                   write8(lp_dial_lock_ndl_n, 1);
                                                   write8(lp_vfoab_n, 2);
#ifdef include_enhanced_btn
                                                   write8(lp_dial_lock_dl_n, 3);
                                                   write8(lp_clarifier_n, 4);
                                                   write8(lp_btn_dn_n, 5);
                                                   write8(lp_btn_up_n, 6);
                                                   write8(lp_mvfo_n, 7);
                                                   write8(lp_vfom_n, 8);
                                                   write8(lp_mrvfo_n, 9);
                                                   write8(lp_split_n, 10);
                                                   write8(lp_swap_n, 11);

#endif
#ifdef include_cb
                                                   write8(cb_ch_n, 19);
                                                   write8(cb_reg_n, 3);
#endif
                                                   write8(checkbyte_n, 1); //Check byte
}


void load_values()
{
   if(read8(checkbyte_n) != 1) set_defaults();
   
   dcs = 15;
   state = read8(state_n);
   if(state == 3) vfo = 0;
   band1 = read8(band1_n);
   band2 = read8(band2_n);
   band3 = read8(band3_n);
   mem_channel = read8(mem_ch_n);
   lp_dial_lock_ndl = read8(lp_dial_lock_ndl_n);
   lp_vfoab = read8(lp_vfoab_n);
   autosave = read8(savetimer_n);

#ifdef include_custom_PLL_REF
   PLL1REF = read32(vfob_block + 11);
   PLL2REF = read32(vfob_block + 12);
#endif
#ifdef include_app_selectable_minmax
   minimum_freq = read32(vfob_block + 13);
   maximum_freq = read32(vfob_block + 14);
#endif
#ifdef include_fine_tuning
   fine_tune_active = read8(fine_tune_n);
#endif

#ifdef include_dial_accel
   speed_dial = read8(dial_n);
#endif

#ifdef include_offset_programming
   per_band_offset = read8(band_offset_n);
#endif

#ifdef include_cb
   check_cb();
   cb_channel = read8(cb_ch_n);
   cb_region = read8(cb_reg_n);
#endif

   
#ifdef include_enhanced_btn
   lp_dial_lock_dl = read8(lp_dial_lock_dl_n);
   lp_clarifier = read8(lp_clarifier_n);
   lp_btn_dn = read8(lp_btn_dn_n);
   lp_btn_up = read8(lp_btn_up_n);
   lp_mvfo = read8(lp_mvfo_n);
   
   lp_vfom = read8(lp_vfom_n);
   lp_mrvfo = read8(lp_mrvfo_n);
   lp_split = read8(lp_split_n);
   lp_swap = read8(lp_swap_n);
   
#endif
   load_all_buffers();
   
   dl = 0; cl = 0; sl = 0;
  
}

void setup()
{
   setup_adc(ADC_OFF);
   set_tris_a(0b00001);
   set_tris_b(0b00000000);
   set_tris_c(0b11111111);
   set_tris_d(0b11111111);
   set_tris_e(0b000);
   BITSA = 0; BITSB = 0; BITSC = 0, BITSD = 0;
   //setup_timer_0(T0_INTERNAL|T0_DIV_128);  
   setup_timer_1(T1_INTERNAL | T1_DIV_BY_8);
#ifdef include_display
   setup_timer_2(T2_DIV_BY_16, disp_int_tmr, postscale);
#endif
   
   //clear_interrupt(INT_TIMER0);              
   clear_interrupt(INT_TIMER1);                 
   clear_interrupt(INT_TIMER2);
   //enable_interrupts(INT_TIMER0);            
   enable_interrupts(INT_TIMER1);               
   enable_interrupts(INT_TIMER2);
   enable_interrupts(INT_rda); //toggle interrupts to ensure serial is ready
   enable_interrupts (global);
   PORTA = 0;
   Q64(0);
   
   
}

#ifdef include_oob_checking
const int32 blacklist[20]= 
{
   150000, 200000,
   350000, 400000,
   700000, 750000,
   1000000, 1050000,
   1400000, 1450000,
   1800000, 1850000,
   2100000, 2150000,
   2400000, 2550000,
   2800000, 3000000
};
#endif
void main()
{
   setup();
   load_values();
   PLL_REF();

    k1 = 1; delay_us (1);
   if (pb0) gen_tx = 0; else gen_tx = 1;
   

  

   #ifdef include_app_support
   app_detected = send_magic_packet();
   int1 oldflash = flash;
   #endif

#ifdef adjust_baud_rate
   #ifdef include_app_support
   if(!app_detected)
  {
   
   #endif
  disable_interrupts(INT_TIMER2);
  k4 = 0; delay_us(1);
  while(pb2){}
   k4 = 1; delay_us(1);
   if(pb2) 
   {
      while(pb2){}
      k4 = 0;
      enable_interrupts(INT_TIMER2);
      set_baud_rate(0);
   } else set_baud_rate(1);
   k4 = 0;
   enable_interrupts(INT_TIMER2);
   #ifdef include_app_support
  }
   else errorbeep(3);
   #endif
  #endif

   delay_ms(200);

   int8 update = 1;
   int8 cat_update = 0;
   dcs = 15;
   res1 = read_counter();
   res2 = res1;
   temp_freq = storage_buffer[state];
   counter = 0;
   dial_timer = 0;
   while(true)
   {
#ifdef include_pms_basic
      if(pms) {
               pms_scan_basic(); 
               update = 1;
               state = get_state();
               temp_freq = storage_buffer[state];
               counter = 100;
               }
#endif

#ifdef include_pms_advanced
      if(pms) {
               pms_scan_advanced(); 
               update = 1;
               state = get_state();
               temp_freq = storage_buffer[state];
               counter = 100;
               }
#endif


#if defined (include_cat_kenwood) || defined (include_cat_yaesu)

      if(!pause_cat)
      {
         cat_update = perform_cat_action();
         if(cat_update) 
         {
            counter = 100;
            cat_update = 0;
            state = get_state();
            temp_freq = storage_buffer[state];
            update = 1;
         }
   
         
      }
#ifdef include_cat_kenwood
      cat_tx_set();
#endif

#endif

      
#ifdef include_standard_btn
      if(!update)
      {
         if(counter >= 100)
         {
            update = buttons(0);
            if(update) 
            {
            
               update = buttonaction(update);
               state = get_state();
               temp_freq = storage_buffer[state];
               counter = 100;
               
            }
         }
      }
#endif

#ifdef include_dial_accel 
if(!update)
      {

         if((state == 4) && (!cl))
         {
            update = misc_dial8 (cb_channel, 0, 9);
            if(update) 
            {
            load_cb_state(0);
            temp_freq = storage_buffer[state];
            }
         }
         else
         if((state != 4) || ((state == 4) && (cl)))
         {
           if(!speed_dial)
           {
            switch(fine_tune_active)
            {
               default: update = freq_dial_basic (temp_freq, 2); break;
               case 1: update = freq_dial_basic (temp_freq, 1); break;
            }
            
            
           }
           else
           {
            switch(fine_tune_active)
            {
               default: update = freq_dial_accel (temp_freq, 2); break;
               case 1: update = freq_dial_accel (temp_freq, 1); break;
            }
           
           }
         }
        if(update) counter = 0;
         
      } 
#else
if(!update)
      {
#ifdef include_cb
         if(state == 4)
         {
            update = misc_dial8 (cb_channel, 0);
            if(update) 
            {
            load_cb_state(0);
            temp_freq = storage_buffer[state];
            }
         }
         else
#endif
         if(state != 4)
         {
#ifdef include_fine_tuning
           switch(fine_tune_active)
           {
              default: update = freq_dial_basic (temp_freq, 2); break;
              case 1: update = freq_dial_basic (temp_freq, 1); break;
           }
#else
   #ifdef include_dial_support
            update = freq_dial_basic (temp_freq, 2);
   #endif
#endif
         }
         if(update) counter = 0;
      }
#endif

if(update)
   {
      if(state == 4) {temp_cl = cl;}
#ifdef include_fine_tuning
      if((!fine_tune_inh) && (!app_detected))
      {
         if(fine_tune_active) fine_tune_display = 1;
      } else fine_tune_display = 0;
#endif
   }
 
      
jump:   
      if((update) || (jump))
      {
#ifdef include_display
         
#ifndef include_app_support
         switch(state)
         {
            default: VFD_data (state,dcs, temp_freq, mem_channel, fine_tune_display); break;
   #ifdef include_cb
            case 4: if(!temp_cl) VFD_data (state,dcs, cb_channel, mem_channel, 2); else VFD_data (state,dcs, temp_freq, mem_channel, fine_tune_display); break;
   #endif
         }
#else
         switch(state)
         {
            default:
            if(!app_detected) VFD_data (state,dcs, temp_freq, mem_channel, fine_tune_display); 
            else
            switch(flash)
            {
               case 0: VFD_data (state,15, temp_freq, mem_channel, fine_tune_display); break;
               case 1: VFD_data (state,4, temp_freq, mem_channel, fine_tune_display); break;
            }
            break;
   #ifdef include_cb
            case 4: 
            if(!app_detected) 
            {
               if(!temp_cl) VFD_data (state,dcs, cb_channel, mem_channel, 2); else VFD_data (state,dcs, temp_freq, mem_channel, fine_tune_display);
            }
            else
            {
               switch(flash)
               {
                  case 0: if(!temp_cl) VFD_data (state,15, cb_channel, mem_channel, 2); else VFD_data (state,15, temp_freq, mem_channel, fine_tune_display); break;
                  case 1: if(!temp_cl) VFD_data (state,4, cb_channel, mem_channel, 2); else VFD_data (state,4, temp_freq, mem_channel, fine_tune_display); break;

               }
            }
            break;
   #endif
         }

#endif







         
#endif         
         update_PLL(temp_freq, 0xFF);
         storage_buffer[state] = temp_freq;
         update = 0; jump = 0; 
#ifdef include_fine_tuning
         fine_tune_inh = 0;
#endif
         counter_start = 1;
         if(dl) counter = 100;
      }

#ifdef include_app_support
      if(app_detected)
      {
         if(flash != oldflash)
         {
            oldflash = flash;
            jump = 1;
            goto jump;
         }
      }
#endif      
#ifdef include_oob_checking      
      IF(!gen_tx)
      {
         if((tx_mode) && (PORTA != 6))
         {
            for (INT i = 0; i < 10; i++)
               {
                  IF ( (storage_buffer[state] >= blacklist[i * 2]) && (storage_buffer[state] <= blacklist[ (i * 2) + 1])) {PORTA = 0; break;}
               }
               IF (i == 10) PORTA = 6;
         }
      }
#endif

      if(state != 4)
      {
         if(cl)
         {
            if((tx_mode) && (!ftx)){ftx = 1; aux_buffer[1] = temp_freq; temp_freq = aux_buffer[2]; jump = 1; goto jump;}
            if((!tx_mode) && (ftx)){ftx = 0; temp_freq = aux_buffer[1]; update = 1; goto jump;}
         }
      }
      if(sl)
      {
         int8 state2;
         if(state == 1) state2 = 2;
         if(state == 2) state2 = 1;
         if((tx_mode) && (!ftx)){ftx = 1; state = state2; temp_freq = storage_buffer[state2]; jump = 1; goto jump;}
         if((!tx_mode) && (ftx)){ftx = 0; state = get_state(); temp_freq = storage_buffer[state]; jump = 1; goto jump;}
      }
      
      if(counter == 10000)
      {
#ifdef include_fine_tuning
         if(fine_tune_active) fine_tune_display = 0;
#endif
         if(state == 4) temp_cl = 0;
         jump = 1;
      }
      
      if(counter == 24000) 
      {
         if(autosave) save_all();
      }
      
      if(counter_start) 
      {
         if(counter < 65535) ++counter;
         else 
         {
            counter_start = 0;
#ifdef include_app_support
            if(app_detected)
            {
               reset_cpu();
            }
#endif
         }
      }
   }

}
