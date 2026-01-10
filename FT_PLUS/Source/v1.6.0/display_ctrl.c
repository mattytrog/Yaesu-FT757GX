

#define flicker_free

#define disp_int_tmr 119 //115 timer2 PR2 reg
#define postscale 1 //1:1 postscale

#ifdef flicker_free
   #define disp_latches 1 // frame-skip more frame skip = faster calculations, slower screen updates
   #define speed1 2       //Slowest. Required for all dials...
#else
   #define disp_latches 6
   #define speed1 2
#endif

#define display_interrupt_hold 16 // * 50 us

void split_value(int32 value, INT8 &d3, int8 &d4, int8 &d5, int8 &d6, int8 &d7, int8 &d8, int8 &d9)
{
   INT32 tmp_value = value;
   d3  = 0; WHILE (tmp_value >= 1000000){tmp_value -= 1000000; d3 += 1; }
   d4  = 0; WHILE (tmp_value >= 100000){tmp_value -= 100000; d4 += 1; }
   d5  = 0; WHILE (tmp_value >= 10000){tmp_value -= 10000; d5 += 1; }
   d6  = 0; WHILE (tmp_value >= 1000){tmp_value -= 1000; d6 += 1; }
   d7  = 0; WHILE (tmp_value >= 100){tmp_value -= 100; d7 += 1; }
   d8  = 0; WHILE (tmp_value >= 10) {tmp_value -= 10; d8 += 1; }
   d9  = 0; WHILE (tmp_value >= 1) {tmp_value -= 1; d9 += 1; }
}

#ifdef include_display
char disp_buf[13] = {10,0,15,15,15,15,15,15,15,15,15,15,15};

void send_disp()
{
   static int8 cycles_to_go = disp_latches;
   static int1 not_added = 0;
   static int8 i = 0;
   if(!pb2) not_added = 1;
   if((pb2) && (cycles_to_go > 0) && (not_added))
   {
      not_added = 0;
      --cycles_to_go;
   }
   if(!cycles_to_go)
      {
         
         k8 = (disp_buf[i])&0xF;
         k4 = (disp_buf[i] >> 1)&0xF; 
         k2 = (disp_buf[i] >> 2)&0xF; 
         k1 = (disp_buf[i] >> 3)&0xF;
         if(pb2)
         {
         disp_INT = 1;
         int8 j = 0;
         while(j<display_interrupt_hold)
         {
            ++j;
            delay_us(5);
         }
         j = 0;
         disp_INT = 0;
         if(i < 13) i+=1;
         }
         if(!pb2) {i = 0; cycles_to_go = disp_latches;}
         
      }
      else {k8 = 0; k4 = 0; k2 = 0; k1 = 0;}

}

void VFD_data(INT8 state, int8 dcs_grid, int32 value, int8 channel_grid, int8 ft)
{
   int8 d3,d4,d5,d6,d7,d8,d9;
   
   switch(state)
   {
      case 1: disp_buf[4] = 1; disp_buf[12] = 15; break;
      case 2: disp_buf[4] = 12; disp_buf[12] = 15; break;
      case 3: disp_buf[4] = 2; disp_buf[12] = channel_grid; break;
      case 5: disp_buf[12] = channel_grid; break;
      default: disp_buf[4] = 15; disp_buf[12] = 15; break;
   }
   
   if(dcs_grid > 15) disp_buf[5] = 15; 
   else disp_buf[5] = dcs_grid;
   
   
   split_value (value, d3, d4, d5, d6, d7, d8, d9);

   if(ft < 3)
   {
   IF (value < 1000000) d3 = 15;
   IF (value < 100000) d4 = 15;
   IF (value < 10000) d5 = 15;
   IF (value < 1000) d6 = 15;
   if (value < 100) d7 = 15;
   if (value < 10) d8 = 15;
   if (value < 1) d9 = 0;
   }

   switch(ft)
   {
   default: disp_buf[6] = d3; disp_buf[7] = d4; disp_buf[8] = d5; disp_buf[9] = d6; disp_buf[10] = d7; disp_buf[11] = d8; break;
   case 1: disp_buf[6] = d4; disp_buf[7] = d5; disp_buf[8] = d6; disp_buf[9] = d7; disp_buf[10] = d8; disp_buf[11] = d9; break;


#ifdef include_cb
   case 2: disp_buf[6] = 15; disp_buf[7] = (cb_region + 1); disp_buf[8] = 15; disp_buf[9] = d8; disp_buf[10] = d9; disp_buf[11] = 15; break;
#endif

 #ifdef include_offset_programming   
   case 3: disp_buf[6] = d6; disp_buf[7] = d7; disp_buf[8] = d8; disp_buf[9] = d9; disp_buf[10] = 15; disp_buf[11] = 15; break; //neg
   case 4: disp_buf[6] = 15; disp_buf[7] = 15; disp_buf[8] = d6; disp_buf[9] = d7; disp_buf[10] = d8; disp_buf[11] = d9; break; //pos
   case 5: disp_buf[6] = 15; disp_buf[7] = 0; disp_buf[8] = 0; disp_buf[9] = 0; disp_buf[10] = 0; disp_buf[11] = 15; break; //zero
#endif  



#ifdef include_manual_tuning
//manual tuning  
   case 9: disp_buf[6] = d3; disp_buf[7] = d4; disp_buf[8] = d5; disp_buf[9] = d6; disp_buf[10] = d7; disp_buf[11] = d8; break;
   case 10: disp_buf[6] = d4; disp_buf[7] = d5; disp_buf[8] = d6; disp_buf[9] = d7; disp_buf[10] = d8; disp_buf[11] = d9; break;
   case 11: disp_buf[6] = 15; disp_buf[7] = d4; disp_buf[8] = d5; disp_buf[9] = d6; disp_buf[10] = d7; disp_buf[11] = d8; break;
   case 12: disp_buf[6] = d3; disp_buf[7] = 15; disp_buf[8] = d5; disp_buf[9] = d6; disp_buf[10] = d7; disp_buf[11] = d8; break;
   case 13: disp_buf[6] = d3; disp_buf[7] = d4; disp_buf[8] = 15; disp_buf[9] = d6; disp_buf[10] = d7; disp_buf[11] = d8; break;
   case 14: disp_buf[6] = d3; disp_buf[7] = d4; disp_buf[8] = d5; disp_buf[9] = 15; disp_buf[10] = d7; disp_buf[11] = d8; break;
   case 15: disp_buf[6] = d3; disp_buf[7] = d4; disp_buf[8] = d5; disp_buf[9] = d6; disp_buf[10] = 15; disp_buf[11] = d8; break;
   case 16: disp_buf[6] = d3; disp_buf[7] = d4; disp_buf[8] = d5; disp_buf[9] = d6; disp_buf[10] = d7; disp_buf[11] = 15; break;
   case 17: disp_buf[6] = d4; disp_buf[7] = d5; disp_buf[8] = d6; disp_buf[9] = d7; disp_buf[10] = d8; disp_buf[11] = 15; break;
#endif


   }
   

}



#INT_TIMER2
void t2_isr(void)
{
send_disp();
clear_interrupt(INT_TIMER2);
}

#endif
