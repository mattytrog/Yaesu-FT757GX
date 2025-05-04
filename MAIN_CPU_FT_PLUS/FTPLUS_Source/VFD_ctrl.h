void clear_disp_buf()
{
   for (INT i = 4; i < 13; i++){disp_buf[i] = 15; }
}

void send_disp_buf(INT1 fast_update)
{
   STATIC int8 count = 0;
   STATIC int8 oldcheck;
   INT8 newcheck = 0;

   IF (! force_update)
   {
      IF (fast_update)
      {
         ++count;
         if (count < vfdrefresh) RETURN;
      }

      else{for (int i = 0; i < 13; i++){newcheck += disp_buf[i]; } if (newcheck == oldcheck) RETURN; }
      count = 0;
      oldcheck = 0;
   }

   WHILE (pb2){}
   WHILE (! pb2){} //and low again - then we send.
   for (INT i = 0; i < 13; i++)
   {
      k8 = (disp_buf[i])&0xF;
      k4 = (disp_buf[i] >> 1)&0xF;
      k2 = (disp_buf[i] >> 2)&0xF;
      k1 = (disp_buf[i] >> 3)&0xF;
      
      // display interrupt. Period is between 370 - 410uS between each nibble.
      // This needs testing with real TMS!!!
      disp_INT = 1;
      delay_us (30);
      disp_INT = 0;
      delay_us (370);
      oldcheck += disp_buf[i];
   }

   //printf (" % d % d\r\n", newcheck, oldcheck);
   force_update = 0;
   k1 = 0; k2 = 0; k4 = 0; k8 = 0;
}

void VFD_data(INT8 vfo_grid, int8 dcs_grid, int32 value, int8 channel_grid, int1 zeroes, int8 blank_digit, int8 display_type, int1 fast_update)
{
   int8 g3,g4,g5,g6,g7,g8,g9;
   if(vfo_grid != 0xFF) disp_buf[4] = vfo_grid; else disp_buf[4] = 15;
   if(dcs_grid != 0xFF) disp_buf[5] = dcs_grid; else disp_buf[5] = 15; 
   if(channel_grid != 0xFF) disp_buf[12] = channel_grid; else disp_buf[12] = 15;
   split_value (value, d3, d4, d5, d6, d7, d8, d9);
   
   g3 = d3; g4 = d4; g5 = d5; g6 = d6; g7 = d7; g8 = d8; g9 = d9;
   if(!zeroes)
   {
   IF (value < 1000000) g3 = 15;
   IF (value < 100000) g4 = 15;
   IF (value < 10000) g5 = 15;
   IF (value < 1000) g6 = 15;
   if (value < 100) g7 = 15;
   if (value < 10) g8 = 15;
   if (value < 1) g9 = 0;   
   }
   
   switch(blank_digit)
   {
   case 1: g3 = 15; break;
   case 2: g4 = 15; break;
   case 3: g5 = 15; break;
   case 4: g6 = 15; break;
   case 5: g7 = 15; break;
   case 6: g8 = 15; break;
   case 7: g9 = 15; break;
   }
   
   IF (display_type == 0){disp_buf[6] = g3; disp_buf[7] = g4; disp_buf[8] = g5; disp_buf[9] = g6; disp_buf[10] = g7; disp_buf[11] = g8; }
   IF (display_type == 1){disp_buf[6] = g4; disp_buf[7] = g5; disp_buf[8] = g6; disp_buf[9] = g7; disp_buf[10] = g8; disp_buf[11] = g9; }
   IF (display_type == 2){disp_buf[6] = 15; disp_buf[7] = 15; disp_buf[8] = g7; disp_buf[9] = g8; disp_buf[10] = g9; disp_buf[11] = 15; }
   IF (display_type == 3){disp_buf[6] = g7; disp_buf[7] = g8; disp_buf[8] = g9; disp_buf[9] = 15; disp_buf[10] = 15; disp_buf[11] = 15; }
   IF (display_type == 4){disp_buf[6] = 15; disp_buf[7] = 15; disp_buf[8] = 15; disp_buf[9] = g7; disp_buf[10] = g8; disp_buf[11] = g9; }
   IF (display_type == 5){disp_buf[6] = 15; disp_buf[7] = 15; disp_buf[8] = 0; disp_buf[9] = 0; disp_buf[10] = 0; disp_buf[11] = 15; }
   
   IF (display_type == 6){disp_buf[6] = 1; disp_buf[7] = 15; disp_buf[8] = 15; disp_buf[9] = g8; disp_buf[10] = g9; disp_buf[11] = 15; }
   IF (display_type == 7){disp_buf[6] = 2; disp_buf[7] = 15; disp_buf[8] = 15; disp_buf[9] = g8; disp_buf[10] = g9; disp_buf[11] = 15; }
   IF (display_type == 8){disp_buf[6] = 3; disp_buf[7] = 15; disp_buf[8] = 15; disp_buf[9] = g8; disp_buf[10] = g9; disp_buf[11] = 15; }
   send_disp_buf (fast_update);
}

void VFD_special_data(INT8 option)
{
   clear_disp_buf ();
   IF (option == 2){disp_buf[9] = 12; disp_buf[10] = 14; }
   IF (option == 3){disp_buf[10] = 11; }
   IF (option == 4){disp_buf[6] = 10; disp_buf[7] = 1; disp_buf[8] = 1; disp_buf[9] = 15; disp_buf[10] = 12; disp_buf[11] = 11; }//ALL CB
   IF (option == 5){disp_buf[6] = 13; disp_buf[7] = 1; disp_buf[8] = 10 ;disp_buf[10] = 10; }//DIA O
   IF (option == 6){disp_buf[6] = 13; disp_buf[7] = 1; disp_buf[8] = 10 ;disp_buf[10] = 11; }//DIA 1
   send_disp_buf (0);
   delay_ms (500);
}



void cls()
{
   clear_disp_buf ();
   WHILE (! pb2){}
   send_disp_buf (0);
}

//!void flash_freq_data(INT8 current, int32 frequency)
//!{
//!   disp_buf[4] = 15;
//!   disp_buf[5] = 15;
//!   disp_buf[12] = 15;
//!   STATIC int16 counter = 0;
//!   STATIC int1 flash;
//!   if((current > 0) && (current < 10)) ++counter;
//!   
//!   if(current >= 10) {current -=10; flash = 0;}
//!   else
//!   if((current > 0) && (current < 10))
//!   {
//!      IF (counter > vfo_button_tuning_flash_speed )
//!      {
//!         IF (flash == 1)flash = 0; else flash = 1;
//!         counter = 0;
//!      }
//!   }
//!   
//!   IF (current == 7)
//!   {
//!      switch(flash)
//!      {
//!      case 1: VFD_data (0xFF, 0xFF, frequency, 0xFF, 1,current, 1, 0) ; break;
//!      case 0: VFD_data (0xFF, 0xFF, frequency, 0xFF, 1, 0, 1, 0) ; break;
//!      }
//!   }
//!
//!   else
//!   {
//!      switch(flash)
//!      {
//!      case 1: VFD_data (0xFF, 0xFF, frequency, 0xFF, 1,current, 0, 0) ; break;
//!      case 0: VFD_data (0xFF, 0xFF, frequency, 0xFF, 1, 0, 0, 0) ; break;
//!      }
//!   }
//!}
//!
void vfo_disp(INT8 vfo, int8 dcs, int32 freq, int8 ch, int1 fast_update)
{
   INT8 v1, v2, v10;
   
   v2 = dcs;

   IF (mem_mode == 0)
   {
      IF (vfo == 0)v1 = 1;
      IF (vfo == 1)v1 = 12;
      v10 = 15;
   }

   
   IF (mem_mode == 1)
   {
      v1 = 2;
      v10 = ch;
   }

   
   VFD_data (v1, v2, freq, v10, 0,0,fine_tune_display, fast_update);
}

void mode_SWITCH(int8 mode);
void beep();

void blink_vfo_display(INT8 PA1, int8 PA2, int32 PA3, int8 PA4)
{
   for (INT i = 0; i < 2; i++)
   {
      INT8 temp_mode = mem_mode;
      mode_SWITCH (1);
      vfo_disp (PA1, PA2, PA3, PA4, 0) ;
      beep (); delay_ms (50);
      vfo_disp (PA1, PA2, PA3, 15, 0) ; delay_ms (150);
      mode_SWITCH (temp_mode);
   }
}

void refresh_screen(int8 state)
{  
   if(state != 4) vfo_disp (active_vfo, dcs, frequency, mem_channel, 0) ;
   #ifdef include_cb
   else VFD_data (0xFF, 0xFF, cb_channel, 0xFF, 0,0, (cb_region + 6), 0);
   #endif
}
