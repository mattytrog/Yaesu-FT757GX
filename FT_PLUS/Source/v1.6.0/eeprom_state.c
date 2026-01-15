//Mem_map INT32 hex        OFFSET                     NEW
//!0 SPARE                 0x00                       
//!1 SPARE                 0x04
//!2 SPARE                 0x08
//!3 SPARE                 0x0C
//!4  memory channel 0     0x10     default 0700000   0  0x00
//!5  memory channel 1     0x14     default 0700000   1  0x03
//!6  memory channel 2     0x18     default 0700000   2  0x06
//!7  memory channel 3     0x1C     default 0700000   3  0x09
//!8  memory channel 4     0x20     default 0700000   4  0x0C
//!9  memory channel 5     0x24     default 0700000   5  0x0F
//!10 memory channel 6     0x28     default 0700000   6  0x12
//!11 memory channel 7     0x2C     default 0700000   7  0x15
//!12 memory channel 8     0x30     default 0700000   8  0x18
//!13 memory channel 9     0x34     default 0700000   9  0x1B
//!14 memory channel 10(A) 0x38     default 0700000   10 0x1E
//!15 memory channel 11(B) 0x3C     default 0700000   11 0x21
//!16 memory channel 12(C) 0x40     default 0700000   12 0x24
//!17 memory channel 13(D) 0x44     default 0700000   13 0x27
//!18 memory channel 14(E) 0x48     default 0700000   14 0x2A
//!19 single offset        0x4C     default 0000000   15 0x2D
//!20 per-band offset 0    0x50     default 0000000   16 0x30
//!21 per-band offset 1    0x54     default 0000000   17 0x33
//!22 per-band offset 2    0x58     default 0000000   18 0x36
//!23 per-band offset 3    0x5C     default 0000000   19 0x39
//!24 per-band offset 4    0x60     default 0000000   20 0x3C
//!25 per-band offset 5    0x64     default 0000000   21 0x3F
//!26 per-band offset 6    0x68     default 0000000   22 0x42
//!27 per-band offset 7    0x6C     default 0000000   23 0x45
//!28 per-band offset 8    0x70     default 0000000   24 0x48
//!29 per-band offset 9    0x74     default 0000000   25 0x4B  
//!30 VFOA band 0          0x78     default 0050000   26 0x4E
//!31 VFOA band 1          0x7C     default 0180000   27 0x51
//!32 VFOA band 2          0x80     default 0350000   28 0x54
//!33 VFOA band 3          0x84     default 0700000   29 0x57
//!34 VFOA band 4          0x88     default 1010000   30 0x5A
//!35 VFOA band 5          0x8C     default 1400000   31 0x5D
//!36 VFOA band 6          0x90     default 1800000   32 0x60
//!37 VFOA band 7          0x94     default 2100000   33 0x63
//!38 VFOA band 8          0x98     default 2400000   34 0x66
//!39 VFOA band 9          0x9C     default 2800000   35 0x69
//!40 VFOA band 10         0xA0     default 3000000   36 0x6C
//!41 VFOB band 0          0xA4     default 0050000   37 0x6F
//!42 VFOB band 1          0xA8     default 0180000   38 0x72
//!43 VFOB band 2          0xAC     default 0350000   39 0x75
//!44 VFOB band 3          0xB0     default 0700000   40 0x78
//!45 VFOB band 4          0xB4     default 1010000   41 0x7B
//!46 VFOB band 5          0xB8     default 1400000   42 0x7E
//!47 VFOB band 6          0xBC     default 1800000   43 0x81
//!48 VFOB band 7          0xC0     default 2100000   44 0x84
//!49 VFOB band 8          0xC4     default 2400000   45 0x87
//!50 VFOB band 9          0xC8     default 2800000   46 0x8A
//!51 VFOB band 10         0xCC     default 3000000   47 0x8D
//!52 PLL1REF              0xD0
//!53 PLL2REF              0xD4
//!54 SPARE                0xD8
//!55 SPARE                0xDC

//!int8
//!0 Rig state             0xE0     default 1
//!1 Current band state 1  0xE1     default 3
//!2 Current band state 2  0xE2     default 3
//!3 Current band state 3  0xE3     default 3
//!4 Current band offset   0xE4     default 0
//!5 Curr. memory channel  0xE5     default 0
//!6 Current CB Channel    0xE6     default 19
//!7 Current CB Region     0xE7     default 3
//!8 Current Fine Tune     0xE8     default 0
//!9 Current Dial Mode     0xE9     default 0
//!10 Auto-saving on/off   0xEA     default 1
//!11 Cat-mode             0xEB     default 0
//!12 Baud rate            0xEC     default 3
//!13 Dial type switch     0xED     default 34
//!14 Fine tune switch     0xEE     default 36
//!15 Offset set switch    0xEF     default 31
//!16 Offset type switch   0xF0     default 40
//!17 Band reset switch    0xF1     default 37
//!18 Cat pause switch     0xF2     default 39
//!19 eeprom reset switch  0xF3     default 35
//!20 cb mode switch       0xF4     default 38
//!21 down(holding) switch 0xF5     default 32
//!22 up(holding) switch   0xF6     default 33
//!23 CAT ID enable        0xF7     default 0
//!24 Dummy mode           0xF8     default 49
//!25 SPARE                0xF9
//!26 SPARE                0xFA
//!27 SPARE                0xFB
//!28 SPARE                0xFC
//!29 SPARE                0xFD
//!30 SPARE                0xFE
//!31 Check byte           0xFF     default 1

//24 BIT EEPROM STORAGE. Uses less EEPROM as the unused extra byte in int32 is nulled and all values moved over 1 byte
//each read int32 must be to a zeroed buffer in 24 bit mode or the "extra" byte will be already in the buffer, giving stupid values
//32 bit leaves the unused zero intact. More human-readable as we can see where each block starts/ends
//#define _24bit 3 
#define _32bit 4

#ifdef _24bit
   #define bytecount _24bit
#endif

#ifdef _32bit
   #define bytecount _32bit
#endif

#define fil0                     0xD0
#define fil1                     0xD1
#define fil2                     0xD2
#define fil3                     0xD3
#define fil4                     0xD4
#define fil5                     0xD5
#define fil6                     0xD6
#define fil7                     0xD7
#define fil8                     0xD8
#define fil9                     0xD9

#define state_n                  0xE0
#define band1_n                  0xE1
#define band2_n                  0xE2
#define band3_n                  0xE3
#define band_offset_n            0xE4
#define mem_ch_n                 0xE5
#define cb_ch_n                  0xE6
#define cb_reg_n                 0xE7
#define fine_tune_n              0xE8
#define dial_n                   0xE9
#define savetimer_n              0xEA
#define cat_mode_n               0xEB
#define baud_n                   0xEC
#define lp_dial_lock_ndl_n       0xED
#define lp_vfoab_n               0xEE
#define lp_dial_lock_dl_n        0xEF
#define lp_clarifier_n           0xF0
#define lp_btn_dn_n              0xF1
#define lp_btn_up_n              0xF2
#define lp_mvfo_n                0xF3
#define lp_vfom_n                0xF4
#define lp_mrvfo_n               0xF5
#define lp_split_n               0xF6
#define lp_swap_n                0xF7
#define lp_fast_n                0xF8
#define id_enable_n              0xF9
#define dummy_mode_n             0xFA
#define old_state_n              0xFB
#define checkbyte_n              0xFF

//!void long_press_dial_lock_ndl()  {enhanced_option(lp_dial_lock_ndl);}
//!void long_press_vfoab()          {enhanced_option(lp_vfoab);}
//!void long_press_dial_lock_dl()   {enhanced_option(lp_dial_lock_dl);}
//!void long_press_clarifier()      {enhanced_option(lp_clarifier);}
//!void long_press_btn_dn()         {enhanced_option(lp_btn_dn);}
//!void long_press_btn_up()         {enhanced_option(lp_btn_up);}
//!void long_press_mvfo()           {enhanced_option(lp_mvfo);}
//!void long_press_vfom()           {enhanced_option(lp_vfom);}
//!void long_press_mrvfo()          {enhanced_option(lp_mrvfo);}
//!void long_press_split()          {enhanced_option(lp_split);}
//!void long_press_swap()           {enhanced_option(lp_swap);}


#if defined (18f452) || defined(18f4520)
   unsigned int8 eeprom_mirror[256];
   unsigned int8 read_ram_eeprom(unsigned int8 base){return eeprom_mirror[base];}
   void write_ram_eeprom(unsigned int8 base, unsigned int8 data){eeprom_mirror[base] = data;}
#endif

#ifdef use_ram_eeprom
void eeprom_write(unsigned int8 base, unsigned int8 data){write_ram_eeprom(base, data);}
unsigned int8 eeprom_read(unsigned int8 base){return read_ram_eeprom(base);}
#else

   #if defined (18f452) || defined(18f4520)
      void eeprom_write(unsigned int8 base, unsigned int8 data){write_eeprom(base, data); write_ram_eeprom(base, data);}
   #else
      void eeprom_write(unsigned int8 base, unsigned int8 data){write_eeprom(base, data);}
   #endif
unsigned int8 eeprom_read(unsigned int8 base){return read_eeprom(base);}
#endif


void beep();

void write32(int8 base, unsigned int32 data)
{
   int32 temp_data = 0;
   int8 address = (base * bytecount);
   int8 i;
   for (i = 0; i < bytecount; i++)
      * ( (int8 * ) (&temp_data) + i) = eeprom_read (address +  i);

   
   if (data == temp_data) {return;}
   
   for (i = 0; i < bytecount; i++)
   {
      
      eeprom_write (address + i, * ( (int8 *) (&data) + i));
   }
#ifdef eeprom_save_debug32
   beep () ;
#endif
   //load_10hz (0) ;
}

unsigned int32 read32(int8 base)
{
   int8 address = (base * bytecount);
   int8 i;
   int32 data = 0;
   for (i = 0; i < bytecount; i++)
      * ( (int8 * ) (&data) + i) = eeprom_read (address +  i);

   return (data) ;
}

void write8(int8 base, int8 value)
{

int8 temp_data = read_eeprom(base);
if(temp_data == value) {return;}

eeprom_write(base, value);

#ifdef eeprom_save_debug8
      beep();
#endif
}

int8 read8(int8 base)
{
   return eeprom_read(base);
}


void save_band_state_f(int8 state, int8 band, int32 data)
{
   if (state == 1)                                {write32 ((vfoa_block + band), data);}
   if (state == 2)                                {write32 ((vfob_block + band), data);}
}


int32 load_band_state_f(int8 state, int8 band) 
{
   if (state == 1)                                return (read32 (vfoa_block + band));
   if (state == 2)                                return (read32 (vfob_block + band));
}

void save_mem_ch_f(int8 channel, int32 data)   {write32 (channel+mem_ch_block, data);}
int32 load_mem_ch_f(int8 channel)               {return (read32(channel+mem_ch_block));}

void save_offset_f(int16 data)
{
   IF(per_band_offset)                          write32 ((PLLband + (offset_block + 1)), data); 
   else                                         write32 (offset_block, data);
}

int16 load_offset_f()
{
   if(per_band_offset)                          return (read32 (PLLband + (offset_block + 1))); 
   else                                         return (read32 (offset_block));
}

void clear_offsets(int1 type)
{
   if(!type) write32 (offset_block, 0);
   else
   {
      for(int i = 0; i < 10; i++)
      {
         write32 ((PLLband + (offset_block + 1) + i), 0);
      }

   }

}


#ifdef include_cb
int32 load_cb(int8 &cb_channel, int8 &cb_band_active);

void save_cb_state()
{
   write8(cb_ch_n, cb_channel); 
   write8(cb_reg_n, cb_region);
}

void load_cb_state(int1 opt)
{
   if(opt)
   {
   cb_channel = read8(cb_ch_n);
   cb_region = read8(cb_reg_n);
   }
   storage_buffer[4] = (load_cb(cb_channel, cb_region));
}


#endif


void load_all_buffers()
{
   storage_buffer[1] = load_band_state_f(1, band1);
   storage_buffer[2] = load_band_state_f(2, band2);
   storage_buffer[3] = load_mem_ch_f(mem_channel);
#ifdef include_cb
   storage_buffer[4] = (load_cb(cb_channel, cb_region));
#endif
}

int8 get_state()
{
   int8 state = read8(state_n);
#ifdef include_cb
   if ((state < 1) || (state > 4)) {state = 1; write8(state_n, state);} 
#else
   if ((state < 1) || (state > 3)) {state = 1; write8(state_n, state);}
#endif
   if(state == 1) vfo = 0;
   if(state == 2) vfo = 1;
   return state;
}

void save_all()
{
   write8(band1_n, band1);
   write8(band2_n, band2);
   write8(mem_ch_n, mem_channel);
#ifdef include_cb
   write8(cb_ch_n, cb_channel);
#endif
   save_band_state_f(1, band1, storage_buffer[1]);
   save_band_state_f(2, band2, storage_buffer[2]);
}
   
