
#ifdef include_cat

const CHAR cat_comm[200] = 
{

   //1st CHAR, 2nd char, location of terminator, res. eg A is 0, I is 1, 0 or 1 would be 2, terminator at 3, result is function 1.
   'I', 'D', 2, 1, // ID = READ. ANSWER.  Answer (ID006;)
   'A', 'I', 3, 2,  //AI0 = SET. NO ANSWER. IF off. AI1 = SET. ANSWER. IF on. Answer IF
   'D', 'N', 2, 3,  // DN = SET. NO ANSWER. DOWN. No reply. Action up button
   'U', 'P', 2, 4,  // UP = SET. NO ANSWER. UP. No reply. Action down button
   'F', 'A', 2, 5,  // FA = READ. ANSWER. VFOA Answer (FA000) (Freq) (0) (;)
   'F', 'B', 2, 6,  // FB = READ. ANSWER. VFOB Answer (FB000) (Freq) (0) (;)
   'F', 'A', 13, 7, // FA = SET. ANSWER. VFOA Set & Answer (FA000) (freq) (0) (;)
   'F', 'B', 13, 8, // FB = SET. ANSWER. VFOB Set & Answer (FB000) (freq) (0) (;)
   'F', 'N', 3, 9,  // FN = SET. NO ANSWER. (0;) = VFOA, (1;) = VFOB, (2;) = MR,
   'F', 'R', 3, 10,  // FR = SET. ANSWER. (FR) (0;) = VFOA, (1;) = VFOB, Answer FR (VFO)(;)
   'F', 'R', 2, 11,  // FR = READ. ANSWER. (FR) (0;) = VFOA, (1;) = VFOB
   'F', 'T', 3, 12,  // FT = SET. ANSWER. (FT) (0;) = VFOA, (1;) = VFOB, Answer FT (VFO)(;)
   'F', 'T', 2, 13,  // FT = READ. ANSWER. (FT) (0;) = VFOA, (1;) = VFOB
   'I', 'F', 2, 14, // IF = READ. ANSWER. Answer IF
   'I', 'E', 3, 15, // IW = *CUSTOM SET Rig ID
   'L', 'K', 2, 16, // LK = READ. ANSWER. Answer LK0; or LK1; UNLOCK OR LOCK
   'L', 'K', 3, 17, // LK = SET. ANSWER. LK0; Lock off. LK1; Lock on....Answer LK0; / LK1; UNLOCK/LOCK
   'M', 'C', 5, 18, // MC = SET. NO ANSWER. MC Memory channel. (MC)(0)(CH). eg MC002; = mem 2
   'M', 'D', 3, 19, // MD = SET. NO ANSWER. MD; MODE - Fake mode 1 = LSB, 2 = USB, 3 = CW, 4 = FM, 5 = AM, 7 = CWN
   'M', 'R', 6, 20, // MR = READ. ANSWER. (MR) (0) (0) (memch) (;). ANSWER (MR) (0) (0) (mem ch. 2 digits) (000) (Frequency. + 0) (dummy mode) (0) (0) (00) (0) ;
   'M', 'W', 23, 21,// MW = SET. NO ANSWER (MW) (0) (0) (mem ch. 2 digits) (000) (Frequency. + 0) (dummy mode) (0) (0) (00) (0) ;

   'R', 'C', 2, 22, // RC = SET. NO ANSWER. Clarifier offset = 0.
   'R', 'D', 2, 23, // RD = SET. NO ANSWER. Clarifier freq decrease.
   'R', 'U', 2, 24, // RU = SET. NO ANSWER.  Clarifier freq increase.
   'R', 'T', 3, 25, // RT = SET. NO ANSWER. RT0; = Clar off. RT1 = Clar on.
   'R', 'X', 2, 26, // RX = SET. NO ANSWER. RX; mode.
   'T', 'X', 2, 27, // TX = SET. NO ANSWER. TX; mode.
   'S', 'C', 3, 28, // SC = SET. NO ANSWER. SC0; PMS off...SC1; PMS on.
   'S', 'P', 3, 29, // SP = SET. NO ANSWER. SP0; split off...SP1 split on.
   'Y', 'A', 2, 30, // SP = SET. NO ANSWER. SP0; split off...SP1 split on.
   //!
};

//create large buffer, based on longest answer. Everything after the terminator is nulled. Buffer is always the same length
char cat_ans[24] = 
{
   '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0'
};
//char* cat_ptr = &cat_ans[];

//38 char IF buffer. For Kenwood information request. Elements are swapped as needed
char ifbuf[38] = 
{
   'I', 'F', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
   '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', ';'
};

char idbuf[5] = 
{
   'I', 'D', '0', '0', '6'
};

void up_down(int1 updown, INT8 state);

#ifdef include_cat
char temp_byte;

char buffer[30]; 
int next_in = 0;
long serial_timer = 0;
int1 valid;

#INT_RDA
void  RDA_isr(VOID)
{
   timerstart = 1;
   
   IF(command_processed)
   {
      for(INT i  = 0; i < COMMAND_LENGTH; ++i)
      {
         buffer[i] = '\0';
      }

      command_processed = 0;
   }

   IF(kbhit())
   {
      temp_byte = getc();
      buffer[next_in] = temp_byte;
      next_in++;
      serial_timer = 0;
      
      IF( ! cat_mode)
      {
         if(temp_byte == 0x3B){SWITCH_cat = 1;}
         IF(next_in > (COMMAND_LENGTH - 1))command_received = 1;
      }

      ELSE
      {
         IF( ! command_received)
         {
            IF(temp_byte == 0x3B)
            {
               
               IF(next_in < (COMMAND_LENGTH - 1))command_received = 1;

               ELSE
               {
                  next_in = 0;
                  command_received = 0;
                  command_processed = 1;
                  WHILE(kbhit()){ getc(); }
               }
            }

            } else WHILE(kbhit()){ getc(); }
         }

         
         IF(command_received)
         {
            buffer[COMMAND_LENGTH] = '\0';   // terminate string with null
            next_in = 0;
            timerstart = 0;
            WHILE(kbhit()){ getc(); }
         }
      }

   }

   #endif

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
      cat_ans[base] = (48 + d3);
      cat_ans[base + 1] = (48 + d4);
      cat_ans[base + 2] = (48 + d5);
      cat_ans[base + 3] = (48 + d6);
      cat_ans[base + 4] = (48 + d7);
      cat_ans[base + 5] = (48 + d8);
      cat_ans[base + 6] = (48 + d9);
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
         active_vfo = 0; save_vfo_n(0);
         frequency = cat_storage_buffer[0];
         FR_ans(0);
      }

      IF(buffer[2] == '1')
      {
         IF(active_vfo == 0)save_band_vfo_f(0, band, cat_storage_buffer[0]);
         active_vfo = 1; save_vfo_n(1);
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
         active_vfo = 0; save_vfo_n(0);
         frequency = cat_storage_buffer[0];
         FT_ans(0);
      }

      IF(buffer[2] == '1')
      {
         IF(active_vfo == 0)save_band_vfo_f(0, band, cat_storage_buffer[0]);
         active_vfo = 1; save_vfo_n(1);
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
      save_id_enable_n(id_enable);
      beep();
   }

   VOID cat_transmit(int1 tx_request);

   INT8 parse_cat_command_kenwood ()
   {
      INT32 temp_value;
      INT1 report_back = 0;
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
         CASE 10: FR_set(); report_back = 1;break;
         CASE 11: FR_read(); break;
         CASE 12: FT_set(); report_back = 1;break;
         CASE 13: FT_read(); break;
         case 14: calc_IF(); send_if(); break;
         CASE 15: IE_set(); break;
         CASE 16: LK_read(); break; //LK;
         CASE 17: LK_set(); report_back = 1; break; //LK + 0 or 1;
         CASE 18: break; //MC
         CASE 19: dummy_mode = (buffer[2]); save_dummy_mode_n(dummy_mode); break;
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

      command_received = 0; command_processed = 1;
      force_update = 1;
      RETURN report_back;
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
         CASE 0x01: split_button(state); beep(); break;
         CASE 0x02: mem_op(5); beep(); break;
         CASE 0x03: mem_op(4); beep(); break;
         CASE 0x04: dial_lock_button(); beep(); break;
         CASE 0x05: mem_op(1); beep(); break;
         CASE 0x06: mem_op(3);; beep(); break;
         CASE 0x07: up_down(1, state); beep(); break;
         CASE 0x08: up_down(0, state); beep(); break;
         CASE 0x09: clarifier_button(state); beep(); break;
         CASE 0x0A: frequency = ((byte4_lower * 1000000) + (byte3_upper * 100000) + (byte3_lower * 10000) + (byte2_upper * 1000) + (byte2_lower * 100) + (byte1_upper * 10) + byte1_lower); break;
         CASE 0x0B: mem_op(2); beep(); break;
         CASE 0x0F: frequency = ((byte1_upper * 1000000) + (byte1_lower * 100000) + (byte2_upper * 10000) + (byte2_lower * 1000) + (byte3_upper * 100) + (byte3_lower * 10) + byte4_upper); break;
          
         case 0xFC: beep(); IF( ! gen_tx)gen_tx = 1; else gen_tx = 0; break;
         CASE 0xFE: reset_checkbyte_n(); reset_cpu(); break;
         CASE 0xFF: reset_cpu(); break;
         #ifdef include_cb
         case 0xFD: IF(gen_tx)toggle_cb_mode(); break;
         #endif
         
         case 0xFA: IF((byte1_upper  * 16) +(byte1_lower) != 0){save_savetimer_n((byte1_upper * 16) + byte1_lower); savetimerEEPROM = ((byte1_upper * 16) + (byte1_lower)); beep(); savetimermax = 0;} BREAK;
         
         CASE 0xFB:
         INT8 count = 0;
         IF((byte1_upper  * 16) +(byte1_lower) != 0){save_speed1_n((byte1_upper * 16) + byte1_lower); ++count; }
         IF((byte2_upper  * 16) +(byte2_lower) != 0){save_speed2_n((byte2_upper * 16) + byte2_lower); ++count; }
         IF((byte3_upper  * 16) +(byte3_lower) != 0){save_speed3_n((byte3_upper * 16) + byte3_lower); ++count; }
         IF((byte4_upper  * 16) +(byte4_lower) != 0){save_speed4_n((byte4_upper * 16) + byte4_lower); ++count; }
         errorbeep(count);
         reset_cpu();
         BREAK;
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

      IF(frequency >= max_freq) frequency = max_freq;
      IF(frequency < min_freq) frequency = min_freq;
      force_update = 1;
      RETURN 1;
   }

   #endif
