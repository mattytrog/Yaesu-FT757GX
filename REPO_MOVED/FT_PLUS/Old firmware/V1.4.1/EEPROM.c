
void write32(int8 address, UNSIGNED int32 data)
{
   INT32 temp_data;
   INT8 i;
   FOR (i = 0; i < 4; i++)
      * ( (INT8 * ) (&temp_data) + i) = read_eeprom (address +  i);

   
   if (data == temp_data) RETURN;
   FOR (i = 0; i < 4; i++)
      write_eeprom (address + i, * ( (INT8 *) (&data) + i));
#ifdef eeprom_save_debug32
   beep () ;
#endif
   //load_10hz (0) ;
}

unsigned int32 read32(INT8 address)
{
   INT8 i;
   INT32 data;
   FOR (i = 0; i < 4; i++)
      * ( (INT8 * ) (&data) + i) = read_eeprom (address +  i);

   RETURN (data) ;
}

void save32(INT8 base, int32 value)
{
   //
   IF(eeprom_enabled)
   {
      write32 (base * 4, value);
   }

   ELSE
   {
      ram_bank[base] = value;
   }
}

int32 load32(INT8 base) 
{
   IF(eeprom_enabled)
   {
      RETURN read32(base * 4);
   }

   ELSE
   {
      RETURN ram_bank[base];
   }
}

void save8(INT8 base, int8 value)
{
   //
   IF(eeprom_enabled)
   {
      INT8 temp_data = read_eeprom(base + 0xE0);
      if(temp_data == value) RETURN;
      write_eeprom(base + 0xE0, value);
      #ifdef eeprom_save_debug8
      beep();
      #endif
   }

   ELSE
   {
      var_bank[base] = value;
   }
}

int8 load8(INT8 base) 
{
   IF(eeprom_enabled)
   {
      RETURN read_eeprom(base + 0xE0);
   }

   ELSE
   {
      RETURN var_bank[base];
   }
}

