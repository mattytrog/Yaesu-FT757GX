int8 tx_oob_check(INT32 freq)
{
   if(freq > max_freq) freq = max_freq;
   if(freq < min_freq) freq = min_freq;
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
   frequency = freq;
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
