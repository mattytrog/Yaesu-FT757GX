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
