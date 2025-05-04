#ifdef include_cat
int1 command_received = 0;
char temp_byte;
char buffer[30]; 
int next_in = 0;
int1 valid;

#INT_RDA
void  RDA_isr(VOID)
{
      if(!command_received)
      {
         if(kbhit())
         {
            temp_byte = getc();
            buffer[next_in] = temp_byte;
         
            if(cat_mode == 0)
            {
            if(temp_byte == 0x3B) SWITCH_CAT = 1;
            if(next_in == 4) command_received = 1; else ++next_in;
            }
            if(cat_mode == 1)
            {
            if(temp_byte == 0x3B) command_received = 1; else ++next_in;
            }
         } 
      }
      if(command_received)
      {
         if(kbhit()) {getc();}
         next_in = 0;
      }

}

void up_down(int1 updown, INT8 state);
INT8 t1 = 0, t2= 0, t3= 0, t4= 0, t5= 0, t6= 0, t7= 0;
INT i;

   VOID calc_if()
   {
      IF((mem_mode == 0)||(mem_mode == 1))
      {
         ifbuf[5] = 48 + d3; //Mhz10
         ifbuf[6] = 48 + d4; //Mhz1
         ifbuf[7] = 48 + d5; //Khz100
         ifbuf[8] = 48 + d6; //Khz10
         ifbuf[9] = 48 + d7; //Khz1
         ifbuf[10] = 48 + d8; //Hz100
         ifbuf[11] = 48 + d9; //Hz10
      }

      
      IF(mem_mode == 2)
      {
         split_value(frequency, t1, t2, t3, t4, t5, t6, t7);
         ifbuf[5] = 48 + t1;
         ifbuf[6] = 48 + t2;
         ifbuf[7] = 48 + t3;
         ifbuf[8] = 48 + t4;
         ifbuf[9] = 48 + t5;
         ifbuf[10] = 48 + t6;
         ifbuf[11] = 48 + t7;
      }

      IF(mem_channel > 9)
      {
         ifbuf[26] = 48 + 1; //Mem CH first digit(eg 0)in ASCII not HEX
         ifbuf[27] = 48 + (mem_channel - 10); //Mem CH second digit(eg 8)in ASCII not HEX
      }

      ELSE
      {
         ifbuf[26] = 48; //Mem CH first digit(eg 0)in ASCII not HEX
         ifbuf[27] = 48 + (mem_channel); //Mem CH second digit(eg 8)in ASCII not HEX
      }

      IF(cat_tx_transmitting)ifbuf[28]  =('1'); else ifbuf[28] = ('0'); //TX / RX(0 RX, 1 TX)in ASCII
      ifbuf[29] = dummy_mode; //Mode 1 = LSB, 2 = USB, 3 = CW, 4 = FM, 5 = AM, 6 = FSK, 7 = CWN. All dummy values. Should reflect mode change in application
      IF(active_vfo == 0){ifbuf[30] = ('0'); ifbuf[32] = ('0'); }//VFO0(VFO A)
      IF(active_vfo == 1){ifbuf[30] = ('1'); ifbuf[32] = ('1'); }//VFO1(VFO B)
      ifbuf[31] = ('0'); //Scan(0 = off, 1 = on)
   }

   VOID send_if()
   {
      //calc_IF();
      FOR(i  = 0; i < 38; i++)
      {
         putc(ifbuf[i]);
      }
   }

   VOID send_cat()
   {
      delay_ms(1);
      for (INT i = 0; i < 24; i++)
      {
         INT element = cat_ans[i];
         putc(element);
         IF(element == ';') break;
      }
   }

   VOID cat_flush()
   {
      for(INT i  = 0; i < 24; i++)
      {
         cat_ans[i] = '0';
      }
   }

   VOID cat_read_freq(int32 temp_value, int8 base)
   {
      int8 i3,i4,i5,i6,i7,i8,i9;
      split_value(temp_value, i3,i4,i5,i6,i7,i8,i9);
      cat_ans[base] = (48 + i3);
      cat_ans[base + 1] = (48 + i4);
      cat_ans[base + 2] = (48 + i5);
      cat_ans[base + 3] = (48 + i6);
      cat_ans[base + 4] = (48 + i7);
      cat_ans[base + 5] = (48 + i8);
      cat_ans[base + 6] = (48 + i9);
   }

   INT32 temp_value;

   int32 cat_set_freq(INT8 base)
   {
      
      cat_ans[base] = buffer[5];
      cat_ans[base + 1] = buffer[6];
      cat_ans[base + 2] = buffer[7];
      cat_ans[base + 3] = buffer[8];
      cat_ans[base + 4] = buffer[9];
      cat_ans[base + 5] = buffer[10];
      cat_ans[base + 6] = buffer[11];
      
      d3 = cat_ans[base] - 48; //10mhz
      d4 = cat_ans[base + 1] - 48; //1mhz
      d5 = cat_ans[base + 2] - 48; //100khz
      d6 = cat_ans[base + 3] - 48; //10khz
      d7 = cat_ans[base + 4] - 48; //1khz
      d8 = cat_ans[base + 5] - 48; //100hz
      d9 = cat_ans[base + 6] - 48;
      join_value(temp_value, d3,d4,d5,d6,d7,d8,d9);
      
      
      RETURN temp_value;
   }

   VOID AI_switch()
   {
      IF(buffer[2] == '0') AI = 0;
      IF(buffer[2] == '1') AI = 1;
   }

   
   VOID FA_read()
   {
      cat_flush();
      cat_read_freq(cat_storage_buffer[0], 5);
      
      cat_ans[0] = 'F';
      cat_ans[1] = 'A';
      cat_ans[13] = ';';
      send_cat();
   }

   VOID FB_read()
   {
      cat_flush();
      cat_read_freq(cat_storage_buffer[1], 5);
      
      cat_ans[0] = 'F';
      cat_ans[1] = 'B';
      cat_ans[13] = ';';
      send_cat();
   }

   VOID FA_set()
   {
      cat_flush();
      temp_value = cat_set_freq(5);
      active_vfo = 0; frequency = temp_value;
      cat_storage_buffer[0] = frequency;
      FA_read();
   }

   VOID FB_set()
   {
      cat_flush();
      temp_value = cat_set_freq(5);
      active_vfo = 1; frequency = temp_value;
      cat_storage_buffer[1] = frequency;
      FB_read();
   }

   VOID FR_ans(int8 res)
   {
      cat_ans[0] = 'F';
      cat_ans[1] = 'R';
      IF(res == 0) cat_ans[2] = '0'; else cat_ans[2] = '1';
      cat_ans[3] = ';';
      send_cat();
   }

   VOID FR_set()
   {
      cat_flush();

      IF(buffer[2] == '0')
      {
         IF(active_vfo == 1)save_band_vfo_f(1, band, cat_storage_buffer[1]);
         active_vfo = 0; save8(vfo_n,0);
         frequency = cat_storage_buffer[0];
         FR_ans(0);
      }

      IF(buffer[2] == '1')
      {
         IF(active_vfo == 0)save_band_vfo_f(0, band, cat_storage_buffer[0]);
         active_vfo = 1; save8(vfo_n,1);
         frequency = cat_storage_buffer[1];
         FR_ans(1);
      }
   }

   VOID FR_read()
   {
      cat_flush();
      IF(active_vfo == 0)FR_ans(0);
      IF(active_vfo == 1)FR_ans(1);
   }

   VOID FT_ans(int8 res)
   {
      cat_ans[0] = 'F';
      cat_ans[1] = 'T';
      IF(res == 0) cat_ans[2] = '0'; else cat_ans[2] = '1';
      cat_ans[3] = ';';
      send_cat();
   }

   VOID FT_set()
   {
      cat_flush();

      IF(buffer[2] == '0')
      {
         IF(active_vfo == 1)save_band_vfo_f(1, band, cat_storage_buffer[1]);
         active_vfo = 0; save8(vfo_n,0);
         frequency = cat_storage_buffer[0];
         FT_ans(0);
      }

      IF(buffer[2] == '1')
      {
         IF(active_vfo == 0)save_band_vfo_f(0, band, cat_storage_buffer[0]);
         active_vfo = 1; save8(vfo_n,1);
         frequency = cat_storage_buffer[1];
         FT_ans(1);
      }
   }

   VOID FT_read()
   {
      cat_flush();
      IF(active_vfo == 0) FT_ans(0);
      IF(active_vfo == 1) FT_ans(1);
   }

   VOID LK_ans(int8 res)
   {
      cat_ans[0] = 'L';
      cat_ans[1] = 'K';
      IF(res == 0) cat_ans[2] = '0'; else cat_ans[2] = '1';
      cat_ans[3] = ';';
      send_cat();
   }

   VOID LK_set()
   {
      cat_flush();

      IF(buffer[2] == '0')
      {
         dl = 0;
         LK_ans(0);
      }

      IF(buffer[2] == '1')
      {
         dl = 1;
         LK_ans(1);
      }

      dcs = get_dcs();
   }

   VOID LK_read()
   {
      cat_flush();

      IF( ! dl)
      {
         LK_ans(0);
      }

      IF(dl)
      {
         LK_ans(1);
      }
   }

   VOID ID_read()
   {
      IF(id_enable)
      {
         cat_flush();
         cat_ans[0] = idbuf[0];
         cat_ans[1] = idbuf[1];
         cat_ans[2] = idbuf[2];
         cat_ans[3] = idbuf[3];
         cat_ans[4] = idbuf[4];
         cat_ans[5] = ';';
         send_cat();
      }
   }

   VOID IE_set()
   {
      IF (buffer[2] == '0') id_enable = 0;
      IF (buffer[2] == '1') id_enable = 1;
      save8(id_enable_n,id_enable);
      beep();
   }

   VOID cat_transmit(int1 tx_request);

   INT8 parse_cat_command_kenwood ()
   {
      INT32 temp_value;
      INT8 report_back = 0;
      INT8 state = read_state();
      INT i;

      //AI DNUP FAFB FN ID IF LK MC MD MR MW RC RDRU RT RXTX SC SP
      INT8 res = 0;
      FOR(i  = 0; i < 31; i++)
      {
         IF((buffer[0]  == cat_comm[(i * 4)])&&(buffer[1] == cat_comm[(i * 4) + 1])&&(buffer[cat_comm[(i * 4) + 2]] == ';'))
         {res = cat_comm[(i * 4) + 3]; BREAK; }
      }

      
      SWITCH(res)
      {
         //all no answer
         CASE 1: ID_read(); break; //ID
         case 2: AI_SWITCH(); break;
         CASE 3: up_down(0, state); report_back = 1; break;
         CASE 4: up_down(1, state); report_back = 1; break;
         CASE 5: FA_read(); break;
         CASE 6: FB_read(); break;
         CASE 7: FA_set(); report_back = 1; break;
         CASE 8: FB_set(); report_back = 1; break;
         case 9: mode_SWITCH_kenwood(buffer[2]); report_back = 1; break; //FN
         CASE 10: FR_set(); report_back = 2;break;
         CASE 11: FR_read(); break;
         CASE 12: FT_set(); report_back = 2;break;
         CASE 13: FT_read(); break;
         case 14: calc_IF(); send_if(); break;
         CASE 15: IE_set(); break;
         CASE 16: LK_read(); break; //LK;
         CASE 17: LK_set(); report_back = 2; break; //LK + 0 or 1;
         CASE 18: break; //MC
         CASE 19: dummy_mode = (buffer[2]); save8(dummy_mode_n,dummy_mode); break;
         CASE 20: temp_value = cat_storage_buffer[active_vfo]; break;
         CASE 21: break; //MW
         CASE 22: break; //Clear clar freq
         CASE 23: break; //Clar freq - 1 or 10
         CASE 24: break; //Clar freq + 1 or 10
         CASE 25: break; //toggle clar on off
         CASE 26: cat_tx_request = 0; cat_transmit(cat_tx_request); break; //set rx mode
         CASE 27: cat_tx_request = 1; cat_transmit(cat_tx_request); break; //set tx mode
         CASE 28: break; //PMS on / off
         CASE 29: break; //split on / off
         case 30: SWITCH_cat = 1; break;
         
         
      }

      command_received = 0;
      if(report_back) return report_back;
      force_update = 1;
      RETURN 0;
   }

   VOID split_button(int8 state);
   VOID dial_lock_button();
   VOID clarifier_button(int8 state);
   VOID mem_op(int8 option);

   




INT8 parse_cat_command_yaesu ()
   {
      INT32 byte5 = buffer[4];
      INT32 byte4_upper = ((buffer[3] >> 4) & 0xF);
      INT32 byte4_lower = buffer[3] & 0xF;
      INT32 byte3_upper = ((buffer[2] >> 4) & 0xF);
      INT32 byte3_lower = buffer[2] & 0xF;
      INT32 byte2_upper = ((buffer[1] >> 4) & 0xF);
      INT32 byte2_lower = buffer[1] & 0xF;
      INT32 byte1_upper = ((buffer[0] >> 4) & 0xF);
      INT32 byte1_lower = buffer[0] & 0xF;
      INT8 state = read_state();
      
      SWITCH(byte5)
      {
         CASE 0x01: split_button(); beep(); break;
         CASE 0x02: mem_op(5); beep(); break;
         CASE 0x03: mem_op(4); beep(); break;
         CASE 0x04: dial_lock_button(); beep(); break;
         CASE 0x05: mem_op(1); beep(); break;
         CASE 0x06: mem_op(3);; beep(); break;
         CASE 0x07: up_down(1, state); beep(); break;
         CASE 0x08: up_down(0, state); beep(); break;
         CASE 0x09: clarifier_button(); beep(); break;
         CASE 0x0A: frequency = ((byte4_lower * 1000000) + (byte3_upper * 100000) + (byte3_lower * 10000) + (byte2_upper * 1000) + (byte2_lower * 100) + (byte1_upper * 10) + byte1_lower); break;
         CASE 0x0B: mem_op(2); beep(); break;
         CASE 0x0F: frequency = ((byte1_upper * 1000000) + (byte1_lower * 100000) + (byte2_upper * 10000) + (byte2_lower * 1000) + (byte3_upper * 100) + (byte3_lower * 10) + byte4_upper); break;
          
         case 0xFC: beep(); IF( ! gen_tx)gen_tx = 1; else gen_tx = 0; break;
         CASE 0xFE: save8 (checkbyte_n, 0xFF); reset_cpu(); break;
         CASE 0xFF: reset_cpu(); break;
         #ifdef include_cb
         case 0xFD: IF(gen_tx)toggle_cb_mode(); break;
         #endif
         
      }

      IF ((byte5 >= 0xE0) && (byte5 <= 0xEE))
      {
         save_mem_ch_f
         ((byte5 - 0xE0),
         (byte4_lower * 1000000) +
         (byte3_upper * 100000) +
         (byte3_lower * 10000) +
         (byte2_upper * 1000) +
         (byte2_lower * 100) +
         (byte1_upper * 10) +
         (byte1_lower));
         errorbeep(3);
      }

      force_update = 1;
      RETURN 1;
   }

int8 check_cat()
   {
         int8 catres = 0;
         IF (cat_mode == 0)
         {
            IF (command_received)
            {
               command_received = 0;
               catres = parse_cat_command_yaesu ();
            }
         }

         
         IF (cat_mode == 1)
         {
            IF (command_received)
            {
               command_received = 0;
               catres = parse_cat_command_kenwood ();
            }
         }

         if (SWITCH_cat == 1)
         {
            IF (cat_mode == 0)cat_mode = 1; else cat_mode = 0;
            save8(cat_mode_n,cat_mode) ;
            beep () ;
            SWITCH_cat = 0;
         }
         return catres;
   
   }
   
      #endif
