int8 tx_oob_check(INT32 freq)
{
   INT8 valid = 0;

   IF ( ! gen_tx)
   {
      IF (tx_mode)
      {
         IF ( ! gtx)
         {
            for (INT i = 0; i < 10; i++)
            {
               IF ( (freq >= blacklist[i * 2]) && (freq <= blacklist[ (i * 2) + 1])) {valid = 1; break; }
            }

            IF ( ! valid)
            {
               beep () ;
               WHILE (tx_mode) set_tx_inhibit (1);
            }

            gtx = 1;
         }
      }

      ELSE
      IF (gtx)
      {
         set_tx_inhibit (0) ;
         gtx = 0;
      }
   } ELSE valid = 1;

   RETURN valid;
}

int8 transmit_check()
{
   #ifdef debug_cat_tx
   cat_tx_transmitting = dl;

   #ELSE
   cat_tx_transmitting = tx_mode;

   #endif
   IF ( (cat_tx_request) || (cat_tx_transmitting))
   {
      IF ( (cat_tx_request) && ( ! cat_tx_transmitting)) cat_transmit (1);
      IF ( ( ! cat_tx_request) && (cat_tx_transmitting)) cat_transmit (0);
   }

   #ifdef cat_debug

   printf ("CAT TX REQUEST % d", cat_tx_request);
   printf ("CAT TRANSMITTING % d", cat_tx_transmitting);

   #endif

   IF(sl)
   {
      IF (tx_mode)
      {
         IF ( ! stx)
         {
            save_band_vfo_f (active_vfo, band, frequency);
            IF (active_vfo == 0){active_vfo = 1;}
            else IF (active_vfo == 1){active_vfo = 0;}
            frequency = load_band_vfo_f (active_vfo, band);
            stx = 1;
            RETURN 1;
         }
      }

      ELSE
      {
         IF (stx)
         {
            save_band_vfo_f (active_vfo, band, frequency);
            IF (active_vfo == 0){active_vfo = 1;}
            else IF (active_vfo == 1){active_vfo = 0;}
            frequency = load_band_vfo_f (active_vfo, band);
            stx = 0;
            RETURN 1;
         }
      }
   }

   
   IF (cl)
   {
      IF (tx_mode)
      {
         IF ( ! ctx)
         {
            save_clar_RX_f (frequency) ;
            frequency = load_clar_TX_f ();
            ctx = 1;
            RETURN 1;
         }
      }

      ELSE
      {
         IF (ctx)
         {
            save_clar_TX_f (frequency) ;
            frequency = load_clar_RX_f ();
            ctx = 0;
            RETURN 1;
         }
      }
   }

   
   tx_oob_check (frequency) ;
   
   RETURN 0;
}

void set_defaults()
{
   for (INT i = 0; i <= 18; i++)
   {save32 (i, 700000); }
   FOR (i = 19; i <= 29; i++)
   {save32 (i, 0); }
   FOR (i = 30; i <= 40; i++)
   {save32 (i, band_bank[i - 30]); }
   FOR (i = 41; i <= 51; i++)
   {save32 (i, band_bank[i - 41]); }
   FOR (i = 52; i <= 53; i++)
   {save32 (i, 0); }
   //save32 (3, 1010000);
   save8(vfo_n, 0); //Active VFO A / B
   save8(mode_n, 0); //Mem mode
   save8(mem_ch_n, 0); //DEFAULT mem channel
   save8(fine_tune_n, 0); //Fine - tuning display (disabled by DEFAULT)
   save8(dial_n,0); //dial_accel / type 0 = disabled, 1 = type1
   save8(dcs_n, 15) ;
   save8(band_offset_n,0) ;
   save8(cat_mode_n, DEFAULT_cat_mode) ;
   IF (load8(baud_n) == 0xFF) save8(baud_n,0);
   save8(dummy_mode_n, 49) ;
   save8(id_enable_n,0) ;
   save8(savetimer_n, 1);
   //save_cache_vfo_f (700000) ;
   save_cache_mem_mode_f (700000) ;
   save8(band_n,(calc_band (700000))) ;
   #ifdef include_cb

   save8(cb_ch_n, 19); //CB ch 1 - 80 on DEFAULT channel
   save8(cb_reg_n, 0); //CB region 0 - CEPT, 1 UK, 2 80CH (1 - 80) - CEPT 1 - 40, UK 41 - 80
   #endif

   save8(checkbyte_n, 1); //Check byte
#ifdef store_to_eeprom
      reset_cpu () ;
#endif
}

void load_values()
{
#ifdef store_to_eeprom
      active_vfo =  load8(vfo_n);
      mem_mode =   load8(mode_n);
      mem_channel =  load8(mem_ch_n);
      fine_tune =   load8(fine_tune_n);
      per_band_offset = load8(band_offset_n);
#ifdef include_cat
      cat_mode =   load8(cat_mode_n);
      baud_rate_n =  load8(baud_n);
      dummy_mode =  load8(dummy_mode_n);
      id_enable =   load8(id_enable_n);
#endif
      speed_dial =  load8(dial_n);
      dcs =    load8(dcs_n);
      band =    load8(band_n);
      savetimerON = load8(savetimer_n);
      #ifdef include_cb

      cb_region =   load8(cb_reg_n);
      cb_channel =  load8(cb_ch_n);

      #endif

#ELSE
      active_vfo =  0;
      mem_mode =   0;
      mem_channel =  0;
      fine_tune =   0;
      per_band_offset = 0;
      cat_mode =   DEFAULT_cat_mode;
      baud_rate_n =  0;
      dummy_mode =  49;
      id_enable =   0;
      speed_dial =  0;
      dcs =    15;
      band =    calc_band (700000);
      savetimerON = 0;

      #ifdef include_cb

      cb_region =   0;
      cb_channel =  19;

      #endif

#endif

   set_dcs (dcs) ;

   SWITCH (baud_rate_n)
   {
      CASE 0: set_uart_speed (4800); break;
      CASE 1: set_uart_speed (1200); break;
      CASE 2: set_uart_speed (2400); break;
      CASE 3: set_uart_speed (4800); break;
      CASE 4: set_uart_speed (9600); break;
      CASE 5: set_uart_speed (19200); break;
      CASE 6: set_uart_speed (38400); break;
      CASE 7: set_uart_speed (57600); break;
      CASE 8: set_uart_speed (115200); break;
   }
}

#ifdef include_cat

void swap_cat_mode()
{
   WHILE(pb0){}
   WHILE (true)
   {
      delay_ms (60) ;
      IF (counter1 == 0) VFD_data (0xFF, 0xFF, (757) , 0xFF, 0, 0, 2, 0);
      IF (counter1 == 20) VFD_data (0xFF, 0xFF, (140), 0xFF, 0, 0, 2, 0);
      k8 = 1; delay_us (1);
      ++counter1;
      IF (counter1 > 40) {counter1 = 0; ++counter; }
      IF (counter == 5) {save8(cat_mode_n,0); break;}
      if (pb0){WHILE (pb0){} break; }
   }

   beep () ;

   IF (counter <=  5)
   {
      IF ( (counter1 > 0)&& (counter1 <= 20)) {save8(cat_mode_n,0); cat_mode = 0; }
      IF ( (counter1 > 20) && (counter1 <= 40) ) {save8(cat_mode_n, 1); cat_mode = 1; }
   }

   counter1 = 0; counter = 0;
   k8 = 0;
}

void change_baud_rate()
{
   WHILE(pb1){}
   INT1 upd = 0;
   INT8 res = 0;

   WHILE (true)
   {
      delay_ms (60) ;

      SWITCH (counter1)
      {
         CASE 0: baud_rate = 1200; upd = 1; break;
         CASE 20: baud_rate = 2400; upd = 1; break;
         CASE 40: baud_rate = 4800; upd = 1; break;
         CASE 60: baud_rate = 9600; upd = 1; break;
         CASE 80: baud_rate = 19200; upd = 1; break;
         CASE 100: baud_rate = 38400; upd = 1; break;
         CASE 120: baud_rate = 57600; upd = 1; break;
         CASE 140: baud_rate = 115200; upd = 1; break;
      }

      IF (upd)
      {
         res += 1;
         VFD_data (0xFF, 0xFF, baud_rate, 0xFF,0,0, 1, 0);
         upd = 0;
      }

      k8 = 1; delay_us (1);
      ++counter1;
      IF (counter1 > 160) {counter1 = 0; res = 0; ++counter; }
      IF (counter == 5) {res = 0; break;}
      if (pb0){WHILE (pb0){} break; }
   }

   beep () ;

   IF (counter <=  5)
   {
      SWITCH (res)
      {
         
         CASE 1: set_uart_speed (1200); break;
         CASE 2: set_uart_speed (2400); break;
         CASE 3: set_uart_speed (4800); break;
         CASE 4: set_uart_speed (9600); break;
         CASE 5: set_uart_speed (19200); break;
         CASE 6: set_uart_speed (38400); break;
         CASE 7: set_uart_speed (57600); break;
         CASE 8: set_uart_speed (115200); break;
         DEFAULT: set_uart_speed (4800); break;
      }

      save8(baud_n, res) ;
      
      errorbeep (3) ;
   } ELSE save8(baud_n,0) ;

   k8 = 0;
}

void cycle_mode_speed()
{
   while(true)
   {
   if(cat_mode == 0) VFD_data (0xFF, 0xFF, (757) , 0xFF, 0, 0, 2, 0);
   if(cat_mode == 1) VFD_data (0xFF, 0xFF, (140) , 0xFF, 0, 0, 2, 0);
   k8 = 1;
   delay_ms(1000);
   SWITCH (load8(baud_n))
         {
            CASE 1: baud_rate = 1200; break;
            CASE 2: baud_rate = 2400; break;
            CASE 3: baud_rate = 4800; break;
            CASE 4: baud_rate = 9600; break;
            CASE 5: baud_rate = 19200; break;
            CASE 6: baud_rate = 38400; break;
            CASE 7: baud_rate = 57600; break;
            CASE 8: baud_rate = 115200; break;
            DEFAULT: baud_rate = 4800; break;
         }
   VFD_data (0xFF, 0xFF, baud_rate, 0xFF,0,0, 1, 0);
   k8 = 1;
   delay_ms(1000);
   if(!pb1) break;
   }
}

#endif

