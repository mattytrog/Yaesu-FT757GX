
#define minimum_freq 10000
#define maximum_freq 3200000
#define vfdrefresh 5
#define btnrefresh 8

//uncomment for bare minimum size. No mic buttons, dial acceleration, CB or CAT. I advise a bigger PIC!
//#define small_rom 
//remove bits and pieces if you wish, or problem is suspected
#ifndef small_rom
#define include_cat
#define include_dial_accel
#define include_cb
#define include_offset_programming
#define include_pms
#define include_manual_tuning
#endif

#define store_to_eeprom
#define speed1 2       //Slowest. Required for all dials...
#define jump_500k 50000 //amt to jump when 500k button pressed. in kc * 10
//PMS Settings
#define scan_pause_count 255  // Pause after of squelch break
#define VFO_dwell_time 2       // Delay after tuning to next frequency. Lower numbers = faster scanning speed, though may overshoot if too fast
#define MR_dwell_time 50       
#define CB_dwell_time 10       
#define default_cat_mode 0       //0 = yaesu, 1 = kenwood
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
# bit dial_clk=PORTA.0  //input  // from q66 counter input (normally high, when counter = 0, goes low for uS). unused. Using up/down counter = 0 which is same result. Use for TX
# bit disp_int=PORTA.4  //output // display interupt
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
# bit CPU_36_BIT128=PORTB.7  //output // strobe for pll2  q42
# bit pb0=PORTC.0  //input  // keypad pb0
# bit pb1=PORTC.1  //input  // keypad pb1
# bit pb2=PORTC.2  //input  // keypad pb2   // also tells us when not to write to the display
# bit sw_500k=PORTC.3  //input  // 500khz step switch
# bit dial_dir=PORTC.4  //input  // Dial counting direction
# bit mic_up=PORTC.5  //input  // mic up button
# bit mic_dn=PORTC.6  //input  // mic down button
# bit pinc7=PORTC.7  //output // remote (CAT) wire (may use this for some sort of debugging output)
# bit sw_pms=PORTD.0  //input 
# bit mic_fast=PORTD.1  //input  // microphone fst (fast) button
# bit squelch_open=PORTD.2  //input  // Squelch open when high (for scanning)
# bit tx_mode=PORTD.3  //input  // PTT/On The Air (even high when txi set)
# bit pind4=PORTD.4  //input  // bcd counter sensing  bit 3
# bit pind5=PORTD.5  //input  // used for saveing vfo bit 2
# bit pind6=PORTD.6  //input  // even the undisplayed bit 1
# bit pind7=PORTD.7  //input  // 10hz component       bit 0
# bit k2=PORTE.0  //output // display bit 1   also these bits are for scanning the keypad
# bit k4=PORTE.1  //output // display bit 2
# bit k8=PORTE.2  //output // display bit 3

int8 BITSA,BITSB,BITSC,BITSD,BITSE;

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
# bit   long_press = BITSC.1
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
# bit   split_transmit = BITSE.0
# bit   flash = BITSE.1
#ifdef include_cat

int32 baud_rate;
int8 dummy_mode;
int1 switch_cat = 0;

#endif

int32 frequency;
int32 dmhz, d100k, d10k, d1k, d100h, d10h;

int8 mem_channel, PLLband, band, mem_mode, dcs, speed_dial, mic_pressed, res1, res2, Q64_val, Q64_tmp, baud_rate_n;
int8 d3= 0,d4= 0,d5= 0,d6= 0,d7= 0,d8= 0,d9= 0; 
int1 long_press_down, long_press_up;
int8 micres = 0;
int8 vfodialres = 0;
int8 cbdialres = 0;
int8 btnres = 0;
int8 txres = 0;
int8 catres = 0;
int8 counterstart = 0;
int16 counter = 0;
int16 counter1 = 0;
int8 counter2 = 0;
//int8 lvcount = 0;
int16 countermax = 6000;
int32 savetimer = 0;
int32 savetimermax = 0;
int32 cat_storage_buffer[2];
int32 min_freq = 0;
int32 max_freq = 0;
