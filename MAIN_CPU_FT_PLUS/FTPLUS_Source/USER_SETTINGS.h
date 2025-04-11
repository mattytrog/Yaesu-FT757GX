
#define min_freq 10000
#define max_freq 3200000
#define vfdrefresh 5
#define btnrefresh 8

//uncomment FOR bare minimum size. No mic buttons, dial acceleration, CB or CAT. I advise a bigger PIC!
//#define small_rom 
//remove bits and pieces IF you wish, or problem is suspected
#ifndef small_rom
#define include_cat
#define include_dial_accel
#define include_cb
#define include_offset_programming
#define include_pms
#define include_manual_tuning
#endif
#define store_to_eeprom

#define speed1 2       //Slowest. Required FOR all dials...

//PMS Settings
#define scan_pause_count 500  // Pause after of squelch BREAK
#define VFO_dwell_time 2       // Delay after tuning to next frequency. Lower numbers = faster scanning speed, though may overshoot IF too fast
#define MR_dwell_time 50       
#define CB_dwell_time 10       
#define DEFAULT_cat_mode 0       //0 = yaesu, 1 = kenwood
#define vfo_button_tuning_flash_speed 1000
#define default_save_timer 3 //roughly seconds. Not accurate. About a second or two off over 30 seconds. Close enough for agricultural.
//#define debug
//#define serial_mode
//#define ncodes
//#define debug_cat_tx
//#define cat_debug
//#define eeprom_save_debug32
//#define eeprom_save_debug8

# byte PORTA = 0x0f80
# byte PORTB = 0x0f81
# byte PORTC = 0x0f82
# byte PORTD = 0x0f83
# byte PORTE = 0x0f84
# bit dial_clk=PORTA.0  //input  // from q66 counter input (normally high, when counter = 0, goes low FOR uS). unused. Using up/down counter = 0 which is same result. Use for TX
# bit disp_INT=PORTA.4  //output // display interupt
# bit k1=PORTA.5  //output // display bit 0
# bit pina6=PORTA.6
# bit pina7=PORTA.7
# bit CPU_BIT1=PORTB.0  //output // bus data bit 0 pll d0
# bit CPU_BIT2=PORTB.1  //output //          bit 1     d1
# bit CPU_BIT4=PORTB.2  //output //          bit 2     d2
# bit CPU_BIT8=PORTB.3  //output //          bit 3     d3
# bit CPU_33_BIT16=PORTB.4  //output // pll a0
# bit CPU_34_BIT32=PORTB.5  //output //     a1
# bit CPU_35_BIT64=PORTB.6  //output //     a2
# bit CPU_36_BIT128=PORTB.7  //output // strobe FOR pll2  q42
# bit pb0=PORTC.0  //input  // keypad pb0
# bit pb1=PORTC.1  //input  // keypad pb1
# bit pb2=PORTC.2  //input  // keypad pb2   // also tells us when not to write to the display
# bit sw_500k=PORTC.3  //input  // 500khz step SWITCH
# bit dial_dir=PORTC.4  //input  // Dial counting direction
# bit mic_up=PORTC.5  //input  // mic up button
# bit mic_dn=PORTC.6  //input  // mic down button
# bit pinc7=PORTC.7  //output // remote (CAT) wire (may use this FOR some sort of debugging output)
# bit sw_pms=PORTD.0  //input 
# bit mic_fast=PORTD.1  //input  // microphone fst (fast) button
# bit squelch_open=PORTD.2  //input  // Squelch open when high (FOR scanning)
# bit tx_mode=PORTD.3  //input  // PTT/On The Air (even high when txi set)
# bit pind4=PORTD.4  //input  // bcd counter sensing  bit 3
# bit pind5=PORTD.5  //input  // used FOR saveing vfo bit 2
# bit pind6=PORTD.6  //input  // even the undisplayed bit 1
# bit pind7=PORTD.7  //input  // 10hz component       bit 0
# bit k2=PORTE.0  //output // display bit 1   also these bits are FOR scanning the keypad
# bit k4=PORTE.1  //output // display bit 2
# bit k8=PORTE.2  //output // display bit 3

int8 BITSA,BITSB,BITSC,BITSD;

# bit   tmp_pin4=BITSA.0
# bit   tmp_pin5=BITSA.1 
# bit   tmp_pin6=BITSA.2 
# bit   tmp=BITSA.3
# bit   gen_tx=BITSA.4 
# bit   dl=BITSA.5  
# bit   active_vfo=BITSA.6
# bit   tick=BITSA.7
# bit   fine_tune_display=BITSB.0 
# bit   sl=BITSB.1 
# bit   cl=BITSB.2 
# bit   btn_down=BITSB.3 
# bit   force_update = BITSB.4
# bit   dir=BITSB.5
# bit   pms = BITSB.6
# bit   per_band_offset = BITSB.7
# bit   timerstart = BITSC.0
# bit   LONG_press = BITSC.1
# bit   setup_offset = BITSC.2
# bit   update = BITSC.3
# bit   gtx = BITSC.4
# bit   stx = BITSC.5
# bit   ctx = BITSC.6
# bit   fine_tune = BITSC.7
# bit   AI = BITSD.0
# bit   cat_mode = BITSD.1
# bit   save_AI = BITSD.2
# bit   cat_tx_request = BITSD.3
# bit   cat_tx_transmitting = BITSD.4
# bit   id_enable = BITSD.5
# bit   mic_down = BITSD.6
# bit   savetimerON = BITSD.7

#ifdef include_cb

const INT32 cb_channel_bank[80] = 
{
   2696500, 2697500, 2698500, 2700500, 2701500, 2702500, 2703500, 2705500, 2706500, 2707500,
   2708500, 2710500, 2711500, 2712500, 2713500, 2715500, 2716500, 2717500, 2718500, 2720500,
   2721500, 2722500, 2725500, 2723500, 2724500, 2726500, 2727500, 2728500, 2729500, 2730500,
   2731500, 2732500, 2733500, 2734500, 2735500, 2736500, 2737500, 2738500, 2739500, 2740500,
   2760125, 2761125, 2762125, 2763125, 2764125, 2765125, 2766125, 2767125, 2768125, 2769125,
   2770125, 2771125, 2772125, 2773125, 2774125, 2775125, 2776125, 2777125, 2778125, 2779125,
   2780125, 2781125, 2782125, 2783125, 2784125, 2785125, 2786125, 2787125, 2788125, 2789125,
   2790125, 2791125, 2792125, 2793125, 2794125, 2795125, 2796125, 2797125, 2798125, 2799125
};
int32 cb_frequency;
int8 cb_channel;
int8 cb_region;
int8 channel_start;
int8 channel_amount;

#endif

const INT32 band_bank[11] = 
{
   100000, 180000, 350000, 700000, 1010000,
   1400000, 1800000, 2100000, 2400000, 2800000, 3000000
};

const INT32 band_bank_edge[11] = 
{
   50000, 200000, 380000, 720000, 1020000,
   1435000, 1820000, 2145000, 2500000, 3000000, 3000000
};

const INT32 PLL_band_bank[30] = 
{
   //lower limit, upper limit, result
   50000, 149999, 0,
   150000, 249999, 1,
   250000, 399999, 2,
   400000, 749999, 3,
   750000, 1049999, 4,
   1050000, 1449999, 5,
   1450000, 1849999, 6,
   1850000, 2149999, 7,
   2150000, 2499999, 8,
   2500000, 3199999, 9
};

//VAL1  VAL2
//>VAL1 && <VAL2 = OK
//<VAL1 && >VAL2 = Out of band
const INT32 blacklist[20]= 
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

char disp_buf[13] = {10,0,15,15,15,15,15,15,15,15,15,15,15};
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

#ifdef include_cat
int32 baud_rate;
int8 dummy_mode;
int1 SWITCH_cat = 0;
#endif

int32 frequency;
int8 mem_channel, PLLband, band, mem_mode, dcs, speed_dial, mic_pressed, res1, res2, Q64_val, Q64_tmp, baud_rate_n;
int8 d3= 0,d4= 0,d5= 0,d6= 0,d7= 0,d8= 0,d9= 0,d10= 0; 
int1 LONG_press_down, long_press_up;
int8 micres = 0;
   INT8 vfodialres = 0;
   INT8 cbdialres = 0;
   INT8 btnres = 0;
   INT8 txres = 0;
   INT8 catres = 0;
   INT8 counterstart = 0;
   INT16 counter = 0;
   INT16 counter1 = 0;
   INT8 counter2 = 0;
   //INT8 lvcount = 0;
   INT16 countermax = 6000;
   int32 savetimer = 0;
   int32 savetimermax = 0;
   
   int32 cat_storage_buffer[2];

void save32(INT8 base, int32 value);
int32 load32(INT8 base);
void save8(INT8 base, int8 value);
int8 load8(INT8 base);
// 0 1

void save_band_vfo_f(INT8 vfo, int8 band, int32 value)
{
   IF (vfo == 0)                                {save32 ((30 + band), value); cat_storage_buffer[0] = value; }
   IF (vfo == 1)                                {save32 ((41 + band), value); cat_storage_buffer[1] = value; }
}

int32 load_band_vfo_f(INT8 vfo, int8 band) 
{
   if (vfo == 0)                                RETURN (load32 (30 + band));
   if (vfo == 1)                                RETURN (load32 (41 + band));
}

void save_offset_f(INT32 value)
{
   IF(per_band_offset)                          save32 ((PLLband + 20), value); 
   else                                         save32 (19, value);
}

int32 load_offset_f()
{
   if(per_band_offset)                          RETURN (load32 (PLLband + 20)); 
   else                                         return (load32 (19));
}


void save_mem_ch_f(INT8 channel, int32 value)   {save32 (channel+4, value);}
int32 load_mem_ch_f(int8 channel)               {RETURN (load32(channel+4));}
void save_cache_f(INT32 value)                  {save32 (2, value);}
int32 load_cache_f()                            {RETURN (load32 (2));}
void save_cache_mem_mode_f(INT32 value)         {save32 (3, value);}
int32 load_cache_mem_mode_f()                   {RETURN (load32 (3));}


void save_clar_RX_f(INT32 value)    {save32(41, value);}
int32 load_clar_RX_f()              {RETURN (load32 (41));}
void save_clar_TX_f(INT32 value)    {save32(42, value);}
int32 load_clar_TX_f()              {RETURN (load32 (42));}

//single bytes. Start at 0xE0

#define vfo_n 0
#define band_n 1
#define band_offset_n 2
#define mode_n 3
#define mem_ch_n 4
#define dcs_n 5
#define cb_ch_n 6
#define cb_reg_n 7
#define fine_tune_n 8
#define dial_n 9
#define savetimer_n 10
#define cache_n 11
#define cat_mode_n 12
#define baud_n 13
#define dummy_mode_n 14
#define id_enable_n 15
#define checkbyte_n 31

int8 calc_band(INT32 frequency);


