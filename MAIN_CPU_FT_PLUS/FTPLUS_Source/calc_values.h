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

void set_dial_lock(int1 res);
void set_clarifier(int1 res);
void set_split(int1 res);

int8 get_dcs()
{
   INT8 res = 15;
   for(INT8 i  = 0; i < 8; i++)
   {
      IF((dl == dcs_res[(i * 4) + 1])&&(cl == dcs_res[(i * 4) + 2])&&(sl == dcs_res[(i * 4) + 3])){res = dcs_res[i * 4]; break; }
   }

   if(dl) set_dial_lock(1); else set_dial_lock(0);
   if(cl) set_clarifier(1); else set_clarifier(0);
   if(sl) set_split(1); else set_split(0);
   save8(dcs_n,res);
   RETURN res;
}

void set_dcs(INT8 res)
{
   for (INT8 i = 0; i < 8; i++)
   {
      IF(res == dcs_res[(i * 4)]){dl = dcs_res[(i * 4) + 1]; cl = dcs_res[(i * 4) + 2]; sl = dcs_res[(i * 4) + 3]; break;}
   }
   
   if(dl) set_dial_lock(1); else set_dial_lock(0);
   if(cl) set_clarifier(1); else set_clarifier(0);
   if(sl) set_split(1); else set_split(0);
   save8(dcs_n,res);
}

int8 calc_band(INT32 frequency)
{
   for(INT i  = 0; i < 10; i++)
   {
      IF((frequency >= band_bank[i])&&(frequency < band_bank[i + 1]))break;
   }

   RETURN i;
}
