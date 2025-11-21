void tx_oob_check(INT32 freq)
{
         for (INT i = 0; i < 10; i++)
            {
               IF ( (freq >= blacklist[i * 2]) && (freq <= blacklist[ (i * 2) + 1])) {valid = 1; break; }
             ELSE valid = 0;
            }
   checked = 1;
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
int8 state = get_state();
static int8 old_state;
if(state != 4) 
{
   IF(sl)
   {
      IF (tx_mode)
      {
         
         IF ( ! stx)
         {
            old_state = state;
            if(state == 2) set_state(5);
            else if (state == 1) set_state(6);
            stx = 1;
            RETURN 2;
         }
      }

      ELSE
      {
         IF (stx)
         {
            set_state(old_state);
            stx = 0;
            RETURN 2;
         }
      }
   }

   
   IF (cl)
   {
      IF (tx_mode)
      {
         IF ( ! ctx)
         {
            clar_rx = storage_buffer[get_state()];
            storage_buffer[get_state()] = clar_tx;
           
            ctx = 1;
            RETURN 3;
         }
      }

      ELSE
      {
         IF (ctx)
         {
            
            clar_tx = storage_buffer[get_state()];
            storage_buffer[get_state()] = clar_rx;
            ctx = 0;
            RETURN 3;
         }
      }
   }

} 

   
   RETURN 0;
}
