

#INT_RDA

char temp_byte;
char buffer[40]; 
int8 next_in = 0;

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
            if(temp_byte == 0x3B) {next_in = 0; command_received = 1; catmode_change_request = 1;}
            if(next_in == 4) {next_in = 0; command_received = 1;} else ++next_in;
            
            }
            if(cat_mode == 1)
            {
            if(temp_byte == 0x3B) {next_in = 0; command_received = 1;} else ++next_in;
            if((next_in == 5) && (temp_byte == 0x05)) {next_in = 0; command_received = 1; catmode_change_request = 1;}
            }
         } 
      }
      //if(SWITCH_CAT) check_cat();
      if(command_received)
      {
         if(kbhit()) {getc();}
#ifdef include_app_support
         if((buffer[0] == 0x31) && (buffer[1] == 0x46) && (buffer[2] == 0x3B))
         {
            disable_interrupts(INT_TIMER2);
            puts("1F2;");
            app_detected = 1;
            next_in = 0; command_received = 0;
            counter = 100;
            while(pb2){}
            enable_interrupts(INT_TIMER2);
         }
#endif
      }
clear_interrupt(INT_RDA);
}



int32 join_value(int8 d3, int8 d4, int8 d5, int8 d6, int8 d7, int8 d8, int8 d9)
{
   int32 v = 0;
   v = (int32)d3;
   v = v * 10 + d4;
   v = v * 10 + d5;
   v = v * 10 + d6;
   v = v * 10 + d7;
   v = v * 10 + d8;
   v = v * 10 + d9;
   return v;
}


#ifdef include_cat_kenwood

const char cat_comm[176] =
{

   //1st char, 2nd char, location of terminator, res. eg A is 0, I is 1, 0 or 1 would be 2, terminator at 3, result is function 1.
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
   'Y', 'A', 2, 30, // YA = SET. NO ANSWER. Yaesu mode.
   
   //configurator section
   '1', 'A', 3, 31, // Dial normal - accelerated
   '1', 'B', 3, 32, // Auto save on/off
   '1', 'C', 3, 33, // Fine tune on/off
   '1', 'D', 7, 34, // Alignment mode
   '1', 'E', 2, 35, // Manual tuning mode
   //'1', 'F', 2, 36, // Heartbeat
   '2', 'A', 3, 37, // State
   '2', 'B', 4, 38, // CB channel
   '2', 'C', 3, 39, // CB Band
   '2', 'D', 3, 40, // Baud
   '2', 'E', 3, 41, // CAT mode
   //'2', 'F', 12, 42 // Set button operations
   '3', '0', 2, 43 // Set button operations
   '3', '1', 2, 44 // Set button operations
   
   //34,36,31,37,40,39,35,38,32,33
   //!
};

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

void cat_transmit(INT1 tx_request)
{
   #ifdef debug_cat_tx

   if (tx_request == 1) {Q64(5); dl = 1; dcs = get_dcs();} else {Q64(0); dl = 0; dcs = get_dcs();}
   #ELSE
   static int1 old_dl = dl;
   if (tx_request == 1) 
   {
   
   dl = 0;
   Q64(7);
   }
   else
   {
   Q64(0);
   dl = old_dl;
   old_dl = 0;
   }
   #endif

}

VOID send_cat()
   {
      //display_stop;
      for (INT i = 0; i < 25; i++)
      {
         putchar(cat_ans[i]);
         IF(cat_ans[i] == ';') break;
      }
      for(i  = 0; i < 25; i++)
      { 
         cat_ans[i] = '0';
      }
      //display_start;
      
   }

VOID send_if()
   {
      FOR(INT i  = 0; i < 38; i++)
      {
         putchar(ifbuf[i]);
         IF(ifbuf[i] == ';') break;
      }
//!      for(i  = 0; i < 38; i++)
//!      {
//!         ifbuf[i] = '0';
//!         IF(ifbuf[i] == ';') break;   
//!      }
   }
   
void calc_freq(int1 ifbuffer, int8 state)
   {
      int32 tmp_val = 0;
      INT8 t1 = 0, t2= 0, t3= 0, t4= 0, t5= 0, t6= 0, t7= 0;
      switch(state)
      {
         case 1: tmp_val = storage_buffer[1]; break;
         case 2: if(ifbuffer) tmp_val = storage_buffer[1]; else tmp_val = storage_buffer[2]; break;
         case 3: tmp_val = storage_buffer[3]; break;
         case 4: tmp_val = storage_buffer[4]; break;
      }
      
      
      if(tmp_val) split_value(tmp_val, t1, t2, t3, t4, t5, t6, t7);
      
      if(ifbuffer)
      {
      ifbuf[5] = 48 + t1;
      ifbuf[6] = 48 + t2;
      ifbuf[7] = 48 + t3;
      ifbuf[8] = 48 + t4;
      ifbuf[9] = 48 + t5;
      ifbuf[10] = 48 + t6;
      ifbuf[11] = 48 + t7;
      }
      else
      {
      cat_ans[5] = 48 + t1;
      cat_ans[6] = 48 + t2;
      cat_ans[7] = 48 + t3;
      cat_ans[8] = 48 + t4;
      cat_ans[9] = 48 + t5;
      cat_ans[10] = 48 + t6;
      cat_ans[11] = 48 + t7;
      }
   }
   
int32 cat_set_freq(INT8 base)
{
   int32 temp_value;
   cat_ans[base] = buffer[5];
   cat_ans[base + 1] = buffer[6];
   cat_ans[base + 2] = buffer[7];
   cat_ans[base + 3] = buffer[8];
   cat_ans[base + 4] = buffer[9];
   cat_ans[base + 5] = buffer[10];
   cat_ans[base + 6] = buffer[11];
   
   temp_value = join_value(cat_ans[base] - 48,cat_ans[base + 1] - 48,cat_ans[base + 2] - 48,
   cat_ans[base + 3] - 48,cat_ans[base + 4] - 48,cat_ans[base + 5] - 48,cat_ans[base + 6] - 48);
   
   if((temp_value < min_freq) || (temp_value > max_freq)) {temp_value = storage_buffer[get_state()];}
   RETURN temp_value;
}

int32 cat_get_val()
   {      
      int32 temp_value;
      buffer[0] = 48; buffer[1] = 48;
      temp_value = join_value(buffer[0] - 48,buffer[1] - 48,buffer[2] - 48,
      buffer[3] - 48,buffer[4] - 48,buffer[5] - 48,buffer[6] - 48);
      
      RETURN temp_value;
   }
   
VOID calc_if()
{
   int8 state = get_state();
   ifbuf[0] = 'I';
   ifbuf[1] = 'F';
   
   calc_freq(1, state);
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
   IF(state == 1){ifbuf[30] = ('0'); ifbuf[32] = ('0'); }//VFO0(VFO A)
   IF(state == 2){ifbuf[30] = ('1'); ifbuf[32] = ('1'); }//VFO1(VFO B)
   ifbuf[31] = ('0'); //Scan(0 = off, 1 = on)
}

void calc_ans(char a, char b, int8 terminator, int8 val)
   {

      cat_ans[0] = a;
      cat_ans[1] = b;
      if(val != 0xFF) cat_ans[2] = val + 48;
      cat_ans[terminator] = ';';
      send_cat();
   
   }

void mode_SWITCH_kenwood(int8 mode)
{
   int8 state = 0;
   IF(mode == (48 + 0))state = 1;
   IF(mode == (48 + 1))state = 2;
   IF(mode == (48 + 2))state = 3;
   write8(state_n, state);
}

void FA_read()
{

   calc_freq(0, get_state());
   calc_ans('F','A', 13, 0xFF);
}


void FB_read()
{

   calc_freq(0, 2);
   calc_ans('F','B', 13, 0xFF);
}

void FA_set()
{
 
   storage_buffer[1] = cat_set_freq(5);
   FA_read();

}
void FB_set()
{

   storage_buffer[2] = cat_set_freq(5); 
   FB_read();
}

void FR_read()
{

   switch(get_state())
   {
      case 1: cat_ans[2] = '0'; break;
      case 2: cat_ans[2] = '1'; break;
   }
   calc_ans('F','R', 3, cat_ans[2]);
}


void FT_read()
{
   switch(get_state())
   {
      case 1: cat_ans[2] = '0'; break;
      case 2: cat_ans[2] = '1'; break;
   }
   calc_ans('F','T', 3, cat_ans[2]);
}

void FR_set()
{

   switch(buffer[2])
   {
      case '0': state = 1; break;
      case '1': state = 2; break;
   }
   write8(state_n, state);
   FR_read();
}

void FT_set()
{
   

   switch(buffer[2])
   {
      case '0': state = 1; break;
      case '1': state = 2; break;
   }
   write8(state_n, state);
   FT_read();
}

   void dial1A() 
   {
      switch(buffer[2])
      {
         case '0': speed_dial = 0; break;
         case '1': speed_dial = 1;break;
      }
      calc_ans('1','A', 3, speed_dial);
      write8(dial_n, speed_dial);
   }
   
   void autosave1B() 
   {
      switch(buffer[2])
      {
         case '0': savetimerON = 0;break;
         case '1': savetimerON = 1;break;
      }
      calc_ans('1','B', 3, savetimerON);
      write8(savetimer_n, savetimerON);
   }
   
   void finetune1C()
   {
      switch(buffer[2])
      {
         case '0': fine_tune_active = 0;break;
         case '1': fine_tune_active = 1;break;
      }
      calc_ans('1','C', 3, fine_tune_active);
      write8(fine_tune_n, fine_tune_active);
   }
   
   void offsettype1D()
   {
      switch(buffer[2])
      {
         case '0': per_band_offset = 0;break;
         case '1': per_band_offset = 1;break;
      }
      calc_ans('1','D', 3, per_band_offset);
      write8(band_offset_n, per_band_offset);
   }
#ifdef include_enhanced_btn   
   void alignment1E() 
   {
      
      program_offset((int16)cat_get_val());
      calc_ans('1','E', 3, 2);
   
   }
#endif
   
#ifdef include_cb   
   void cbmode2A()
   {
      int8 state = 0;
      switch(buffer[2])
      {
         default: state = vfo + 1; break;
         case '1': state = 1; break;
         case '2': state = 2; break;
         case '3': state = 3; break;
         case '4': state = 4; break;
      }
      
      calc_ans('2','A', 3, state);
   }
   
   void cbch2B() 
   {
      int8 tmp_ch = (buffer[2] - 48) * 10;
      tmp_ch += (buffer[3] - 48);
      
      if((tmp_ch > 0) && (tmp_ch < 41))
      {
      cb_channel = tmp_ch;
      write8(cb_ch_n,cb_channel);
      }
      calc_ans('2', 'B', 3, cb_channel);
      load_cb_state(0);
   }
   void cbbnd2C() 
   {
      if(((buffer[2] - 48) > 0) && ((buffer[2] - 48) < 9))
      {
      cb_region = buffer[2] - 49;
      write8(cb_reg_n,cb_region);
      }
      calc_ans('2', 'C', 3, cb_region+1);
      load_cb_state(0);
   }
#endif
   void baud2D()
   {
      if(((buffer[2] - 48) > 0) && ((buffer[2] - 48) < 9))
      {
      baud_rate_n = buffer[2] - 48;
      write8(baud_n, baud_rate_n);
      }
      calc_ans('2', 'D', 3, baud_rate_n);
   }
   void catmode2E() 
   {
      switch(buffer[2])
      {
         case '0': cat_mode = 0; break;
         case '1': cat_mode = 1; break;
      }
      calc_ans('2', 'E', 3, cat_mode);
      write8(cat_mode_n,cat_mode) ;
   }
#ifdef include_app_support
   void dumpset30()
   {
      for (int i = 0; i < 255; i++)
      {
         putchar(read_ram_eeprom(i));
      }

      putchar (';');
   }

void load_values();
   void receiveset31()
   {
      disable_interrupts(INT_RDA);
      int i = 0;
      while(true)
      {
         if(kbhit()) 
         {
         write_ram_eeprom(i, getc());
         if(read_ram_eeprom(i) == 0x3B) {break;} 
         ++i;
         }
        
          
         //
      }
      while(kbhit()) {getc(); delay_us(20);}  
      
      
      int j = 0;
      while(j<i)
      {
        write8(j, read_ram_eeprom(j));
        delay_us(20);
        if(read8(j) == read_ram_eeprom(j)) ++j; else {errorbeep(3); break;}
      }
      enable_interrupts(INT_RDA);
      beep();
      load_values();
      load_all_buffers();
      PLL_REF();
   }
#else
void dumpset30(){}
void receiveset31(){}
#endif

#ifdef include_cat_kenwood
int8 action_kenwood_cat()
{
   int8 res = 0;
   int8 report_back = 0;
   FOR(int i  = 0; i < 45; i++)
      {
         IF((buffer[0]  == cat_comm[(i * 4)])&&(buffer[1] == cat_comm[(i * 4) + 1])&&(buffer[cat_comm[(i * 4) + 2]] == ';'))
         {res = cat_comm[(i * 4) + 3]; BREAK; }
      }
   
   SWITCH(res)
      {
         //all no answer
         //CASE 1: ID_read(); break; //ID
         //case 2: AI_SWITCH(); break;
         CASE 3: btn_dn_handler(0); report_back = 1; break;
         CASE 4: btn_up_handler(0); report_back = 1; break;
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
         //CASE 15: IE_set(); break;
         //CASE 16: LK_read(); break; //LK;
         //CASE 17: LK_set(); report_back = 2; break; //LK + 0 or 1;
         CASE 18: break; //MC
         CASE 19: dummy_mode = (buffer[2]); break; //save8(dummy_mode_n,dummy_mode); break;
         //CASE 20: temp_value = storage_buffer[active_vfo]; break;
         //CASE 21: break; //MW
         //CASE 22: break; //Clear clar freq
         //CASE 23: break; //Clar freq - 1 or 10
         //CASE 24: break; //Clar freq + 1 or 10
         //CASE 25: break; //toggle clar on off
         CASE 26: cat_tx_request = 0; cat_transmit(cat_tx_request); break; //set rx mode
         CASE 27: cat_tx_request = 1; cat_transmit(cat_tx_request); break; //set tx mode
         //CASE 28: break; //PMS on / off
         //CASE 29: break; //split on / off
         case 30: catmode_change_request = 1; break;
         case 31: dial1A(); break; //dial
         case 32: autosave1B(); break; //autosave
         case 33: finetune1C(); break; //finetune
         case 34: offsettype1D(); break; //finetune
#ifdef include_enhanced_btn
         case 35: alignment1E(); report_back = 1; break; //alignment       
         //case 36: mantune1F(); report_back = 3; break; //man_tune
#endif

#ifdef include_cb
         case 37: cbmode2A(); report_back = 1;break; //cb ch
         case 38: cbch2B(); report_back = 1;break; //cb ch
         case 39: cbbnd2C(); report_back = 1;break; //cb band
#endif
         case 40: baud2D(); break; //baud
         case 41: catmode2E(); break; //cat mode
         //case 41: break;
         //case 42: break;

         case 43: dumpset30(); report_back = 3; break;
         case 44: receiveset31(); report_back = 1; break;

      }
   
   return report_back;
}

void cat_tx_set()
{
   #ifdef debug_cat_tx
   cat_tx_transmitting = dl;

   #ELSE
   cat_tx_transmitting = tx_mode;

   #endif
   IF ( (cat_tx_request) || (cat_tx_transmitting))
   {
      IF ( (cat_tx_request) && ( ! cat_tx_transmitting)) 
      {
      cat_transmit (1);
      
      }
      
      IF ( ( ! cat_tx_request) && (cat_tx_transmitting))
      {
      cat_transmit (0);
      }
   }

   #ifdef cat_debug

   printf ("CAT TX REQUEST % d", cat_tx_request);
   printf ("CAT TRANSMITTING % d", cat_tx_transmitting);

   #endif
}
#endif
#endif

int8 action_yaesu_cat()
{
#ifdef include_cat_yaesu
   INT8 byte5 = buffer[4];
   INT8 byte4_upper = ((buffer[3] >> 4) & 0xF);
   INT8 byte4_lower = buffer[3] & 0xF;
   INT8 byte3_upper = ((buffer[2] >> 4) & 0xF);
   INT8 byte3_lower = buffer[2] & 0xF;
   INT8 byte2_upper = ((buffer[1] >> 4) & 0xF);
   INT8 byte2_lower = buffer[1] & 0xF;
   INT8 byte1_upper = ((buffer[0] >> 4) & 0xF);
   INT8 byte1_lower = buffer[0] & 0xF;
   
   SWITCH(byte5)
   {
      CASE 0x01: split_button_handler(); beep(); break;
      CASE 0x02: mrvfo_handler(); break;
      CASE 0x03: vfom_handler(); break;
      CASE 0x04: dial_lock_button_handler(); beep(); break;
      CASE 0x05: vfoab_handler(); break;
      CASE 0x06: mvfo_handler(); break;
      CASE 0x07: btn_up_handler(0); beep(); break;
      CASE 0x08: btn_dn_handler(0); beep(); break;
      CASE 0x09: clarifier_button_handler(); beep(); break;
      CASE 0x0A: temp_freq = join_value(byte4_lower,byte3_upper,byte3_lower,byte2_upper,byte2_lower,byte1_upper,byte1_lower); storage_buffer[get_state()] = temp_freq; temp_freq = 0; break;
      CASE 0x0B: vfom_swap_handler(); break;
      CASE 0x0F: temp_freq = join_value(byte1_upper,byte1_lower,byte2_upper,byte2_lower,byte3_upper,byte3_lower,byte4_upper); storage_buffer[get_state()] = temp_freq; temp_freq = 0; break;
       
      case 0xFC: beep(); IF( ! gen_tx)gen_tx = 1; else gen_tx = 0; break;
      CASE 0xFE: write8 (checkbyte_n, 0xFF); reset_cpu(); break;
      CASE 0xFF: reset_cpu(); break;
      #ifdef include_cat
      case 0xFD: IF(gen_tx)toggle_cb_mode(); break;
      #endif
         
      }
#endif
   return 1;
}




int8 perform_cat_action()
{
   int8 res = 0;
   if(!command_received) return res;
   else
   {
      if(catmode_change_request)
      {
         if(cat_mode == 0) cat_mode = 1;
         else
         if(cat_mode == 1) cat_mode = 0;
         catmode_change_request = 0;
         write8(cat_mode_n,cat_mode);
      }
#ifdef include_cat_yaesu
      if(cat_mode == 0) res = action_yaesu_cat();
#endif
#ifdef include_cat_kenwood
      if(cat_mode == 1) res = action_kenwood_cat();
#endif
      command_received = 0;
   
      return res;
   
   }
}




void set_baud_rate(int1 skip)
{
#ifdef adjust_baud_rate
enable_interrupts(INT_TIMER2);
   INT1 upd = 1;
   INT8 res = 0;
   int8 opt = read8(baud_n);
   
//display_start;
   if(!skip)
   {
      temp_freq = 0;
      res1 = read_counter();
      while (true)
      {
         
         //delay_ms (60) ;
         
         if(!upd) upd = misc_dial8(opt, 0, 9);
         if(!upd) res = buttons(1);
         if(res)
         {
            if(res == 3) ++opt;
            if(res == 2) --opt;
            upd = 1;
         }
         
         if(upd)
         {
            switch (opt)
            {
               case 1: temp_freq = 1200; upd = 1; break;
               case 2: temp_freq = 2400; upd = 1; break;
               case 3: temp_freq = 4800; upd = 1; break;
               case 4: temp_freq = 9600; upd = 1; break;
               case 5: temp_freq = 19200; upd = 1; break;
               case 6: temp_freq = 38400; upd = 1; break;
               case 7: temp_freq = 57600; upd = 1; break;
               case 8: temp_freq = 115200; upd = 1; break;
            }
         }
         if(opt < 1) opt = 1;
         else
         if(opt > 8) opt = 8;
         if (upd)
         {
            VFD_data (0xFF, 0xFF, temp_freq, 1,1);
            upd = 0;
         }
         if(res == mvfo_button) break;
   
         
      }
   }
   

      switch (opt)
      {
         
         case 1: set_uart_speed (1200); break;
         case 2: set_uart_speed (2400); break;
         case 3: set_uart_speed (4800); break;
         case 4: set_uart_speed (9600); break;
         case 5: set_uart_speed (19200); break;
         case 6: set_uart_speed (38400); break;
         case 7: set_uart_speed (57600); break;
         case 8: set_uart_speed (115200); break;
         default: set_uart_speed (4800); break;
      }
      baud_rate_n = opt;
      
      if(!skip)
      {
      write8(baud_n, opt) ;
      errorbeep (2) ;
      delay_ms(200);
      }
#endif
}

#ifdef include_app_support
int1 send_magic_packet()
{
   int1 res = 0;
   set_uart_speed (4800);
   disable_interrupts(INT_RDA);
   delay_ms(100);
      
      
   cat_ans[0] = 'F';
   cat_ans[1] = 'T';
   cat_ans[2] = 'C';
   cat_ans[3] = 'F';
   cat_ans[4] = 'G';
   cat_ans[5] = ';';
   send_cat();
   enable_interrupts(INT_RDA);
   int8 count = 0;
   while (count < 200)
   {
      if(perform_cat_action() == 3) {res = 1; break;}
      ++count; delay_ms(1);
      
   }
   
   return res;
}


#endif



