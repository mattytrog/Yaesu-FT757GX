
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
   
   if(vfo_grid != 0xFF) disp_buf[4] = vfo_grid; else disp_buf[4] = 15;
   if(dcs_grid != 0xFF) disp_buf[5] = dcs_grid; else disp_buf[5] = 15; 
   if(channel_grid != 0xFF) disp_buf[12] = channel_grid; else disp_buf[12] = 15;
   split_value (value, d3, d4, d5, d6, d7, d8, d9);
   
   if(blank_digit)
   {
         if (display_type == 0)
         {
         if((blank_digit >=1) && (blank_digit <= 6))
         IF (value < 1000000) disp_buf[6] = 0; else disp_buf[6] = d3;
         IF (value < 100000) disp_buf[7] = 0; else disp_buf[7] = d4;
         IF (value < 10000) disp_buf[8] = 0; else disp_buf[8] = d5;
         IF (value < 1000) disp_buf[9] = 0; else disp_buf[9] = d6;
         if (value < 100) disp_buf[10] = 0; else disp_buf[10] = d7;
         if (value < 10) disp_buf[11] = 0; else disp_buf[11] = d8;
         
         if(blank_digit == 9) disp_buf[6] = 15;
         if(blank_digit == 10) disp_buf[7] = 15;
         if(blank_digit == 11) disp_buf[8] = 15;
         if(blank_digit == 12) disp_buf[9] = 15;
         if(blank_digit == 13) disp_buf[10] = 15;
         if(blank_digit == 14) disp_buf[11] = 15;
         }
         
         if(display_type == 1)
         {
         if((blank_digit >=1) && (blank_digit <= 7))
         //IF (value < 1000000) disp_buf[6] = 0; else disp_buf[6] = d3;
         IF (value < 100000) disp_buf[6] = 0; else disp_buf[6] = d4;
         IF (value < 10000) disp_buf[7] = 0; else disp_buf[7] = d5;
         IF (value < 1000) disp_buf[8] = 0; else disp_buf[8] = d6;
         if (value < 100) disp_buf[9] = 0; else disp_buf[9] = d7;
         if (value < 10) disp_buf[10] = 0; else disp_buf[10] = d8;
         if (value < 1) disp_buf[11] = 0; else disp_buf[11] = d9;
         
//!         if(blank_digit == 8) disp_buf[6] = 15;
//!         if(blank_digit == 9) disp_buf[7] = 15;
//!         if(blank_digit == 10) disp_buf[8] = 15;
//!         if(blank_digit == 11) disp_buf[9] = 15;
//!         if(blank_digit == 12) disp_buf[10] = 15;
         if(blank_digit == 15) disp_buf[11] = 15;
         }
         
   }
   if(!blank_digit)
   {
   IF (display_type == 0){disp_buf[6] = d3; disp_buf[7] = d4; disp_buf[8] = d5; disp_buf[9] = d6; disp_buf[10] = d7; disp_buf[11] = d8; }
   IF (display_type == 1){disp_buf[6] = d4; disp_buf[7] = d5; disp_buf[8] = d6; disp_buf[9] = d7; disp_buf[10] = d8; disp_buf[11] = d9; }
   IF (display_type == 2){disp_buf[6] = 15; disp_buf[7] = 15; disp_buf[8] = d7; disp_buf[9] = d8; disp_buf[10] = d9; disp_buf[11] = 15; }
   IF (display_type == 3){disp_buf[6] = d7; disp_buf[7] = d8; disp_buf[8] = d9; disp_buf[9] = 15; disp_buf[10] = 15; disp_buf[11] = 15; }
   IF (display_type == 5){disp_buf[6] = 15; disp_buf[7] = 15; disp_buf[8] = 15; disp_buf[9] = 0; disp_buf[10] = 15; disp_buf[11] = 15; }
   
   IF (display_type == 6){disp_buf[6] = 1; disp_buf[7] = 15; disp_buf[8] = d7; disp_buf[9] = d8; disp_buf[10] = d9; disp_buf[11] = 15; }
   IF (display_type == 7){disp_buf[6] = 2; disp_buf[7] = 15; disp_buf[8] = d7; disp_buf[9] = d8; disp_buf[10] = d9; disp_buf[11] = 15; }
   IF (display_type == 8){disp_buf[6] = 3; disp_buf[7] = 15; disp_buf[8] = d7; disp_buf[9] = d8; disp_buf[10] = d9; disp_buf[11] = 15; }
   
   IF(display_type == 0)
   {
   IF (value < 1000000) disp_buf[6] = 15;
   IF (value < 100000) disp_buf[7] = 15;
   IF (value < 10000) disp_buf[8] = 15;
   IF (value < 1000) disp_buf[9] = 15;
   if (value < 100) disp_buf[10] = 15;
   if (value < 10) disp_buf[11] = 15;
   if (value < 1) disp_buf[11] = 15;
   }
   }
   
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

void cls()
{
   clear_disp_buf ();
   WHILE (! pb2){}
   send_disp_buf (0);
}

void flash_freq_data(INT8 current, int32 frequency)
{
   disp_buf[4] = 15;
   disp_buf[5] = 15;
   disp_buf[12] = 15;
   STATIC int16 counter = 0;
   STATIC int1 flash;
   ++counter;
   if(frequency > max_freq) {frequency = max_freq;}
   if(frequency < min_freq) {frequency = min_freq;}
   
IF (counter > 200 )
   {
      IF (flash == 1)flash = 0; else flash = 1;
      counter = 0;
   }

   IF (current == 7)
   {
      switch(flash)
      {
      case 1: VFD_data (0xFF, 0xFF, frequency, 0xFF, 1,current, 1, 0) ; break;
      case 0: VFD_data (0xFF, 0xFF, frequency, 0xFF, 1, current+8, 1, 0) ; break;
      }
   }

   else
   {
      switch(flash)
      {
      case 1: VFD_data (0xFF, 0xFF, frequency, 0xFF, 1,current, 0, 0) ; break;
      case 0: VFD_data (0xFF, 0xFF, frequency, 0xFF, 1, current + 8, 0, 0) ; break;
      }
   }
}

void mode_SWITCH(int mode);

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

