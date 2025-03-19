#ifdef user_settings
#define min_freq 50000
#define max_freq 3000000
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
#define store_to_eeprom
#endif
#define out_speed1 2       //Slowest. Required FOR all dials...
#define out_speed2 10      //Slightly faster
#define out_speed3 123     //Faster still
#define out_speed4 255     //Super-fast

//PMS Settings
#define scan_pause_count 500  // Pause after of squelch BREAK
#define VFO_dwell_time 2       // Delay after tuning to next frequency. Lower numbers = faster scanning speed, though may overshoot IF too fast
#define MR_dwell_time 50       
#define CB_dwell_time 10       
#define DEFAULT_cat_mode 0       //0 = yaesu, 1 = kenwood
#define vfo_button_tuning_flash_speed 100
//#define debug
//#define serial_mode
//#define ncodes
//#define debug_cat_tx
//#define cat_debug
//#define eeprom_save_debug32
//#define eeprom_save_debug8
#endif
