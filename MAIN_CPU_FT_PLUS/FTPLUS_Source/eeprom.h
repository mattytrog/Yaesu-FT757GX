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
//!54 SPARE                D8
//!55 SPARE                DC

//create ram store, follows eeprom-style IF enabled
#ifndef store_to_eeprom
int32 ram_bank[60];
int8 var_bank[40];
#endif

void beep();
void write32(int8 address, unsigned int32 data)
{
   int32 temp_data;
   int8 i;
   for (i = 0; i < 4; i++)
      * ( (int8 * ) (&temp_data) + i) = read_eeprom (address +  i);

   
   if (data == temp_data) return;
   for (i = 0; i < 4; i++)
      write_eeprom (address + i, * ( (int8 *) (&data) + i));
#ifdef eeprom_save_debug32
   beep () ;
#endif
   //load_10hz (0) ;
}

unsigned int32 read32(int8 address)
{
   int8 i;
   int32 data;
   for (i = 0; i < 4; i++)
      * ( (int8 * ) (&data) + i) = read_eeprom (address +  i);

   return (data) ;
}

void save32(int8 base, int32 value)
{
   //
#ifdef store_to_eeprom
   write32 (base * 4, value);
#ELSE
   ram_bank[base] = value;
#endif
}

int32 load32(int8 base) 
{
#ifdef store_to_eeprom
      return read32(base * 4);
#ELSE
      return ram_bank[base];
#endif
}

void save8(int8 base, int8 value)
{
   //
#ifdef store_to_eeprom
      int8 temp_data = read_eeprom(base + 0xE0);
      if(temp_data == value) return;
      write_eeprom(base + 0xE0, value);
      #ifdef eeprom_save_debug8
      beep();
      #endif
#ELSE
      var_bank[base] = value;
#endif
}

int8 load8(int8 base) 
{
#ifdef store_to_eeprom
      return read_eeprom(base + 0xE0);
#ELSE
      return var_bank[base];
      #endif
}


void save_band_vfo_f(int8 vfo, int8 band, int32 value)
{
   IF (vfo == 0)                                {save32 ((30 + band), value); cat_storage_buffer[0] = value; }
   IF (vfo == 1)                                {save32 ((41 + band), value); cat_storage_buffer[1] = value; }
}

int32 load_band_vfo_f(int8 vfo, int8 band) 
{
   if (vfo == 0)                                return (load32 (30 + band));
   if (vfo == 1)                                return (load32 (41 + band));
}

void save_offset_f(int32 value)
{
   IF(per_band_offset)                          save32 ((PLLband + 20), value); 
   else                                         save32 (19, value);
}

int32 load_offset_f()
{
   if(per_band_offset)                          return (load32 (PLLband + 20)); 
   else                                         return (load32 (19));
}


void save_mem_ch_f(int8 channel, int32 value)   {save32 (channel+4, value);}
int32 load_mem_ch_f(int8 channel)               {return (load32(channel+4));}
void save_cache_f(int32 value)                  {save32 (2, value);}
int32 load_cache_f()                            {return (load32 (2));}
void save_cache_mem_mode_f(int32 value)         {save32 (3, value);}
int32 load_cache_mem_mode_f()                   {return (load32 (3));}


void save_clar_RX_f(int32 value)    {save32(41, value);}
int32 load_clar_RX_f()              {return (load32 (41));}
void save_clar_TX_f(int32 value)    {save32(42, value);}
int32 load_clar_TX_f()              {return (load32 (42));}

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



