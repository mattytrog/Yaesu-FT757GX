void Q64(INT8 val)
{
   //Q64 is Octal (000 - 111) (0 - 7) (Bit 8, port A is actually dial clock)
   //Bits are reversed on the bus.
   //A0 is dial clock (or spare). A1 = LSB. Only 8 values. Hand code FOR speed.
   //eg Beep is binary 4... could just DO beep() = 2, but would not actually be
   //correct on schematic, though would work fine
   //Correction by Matthew 19/2/25
   
   switch(val)
   {
      case 0: PORTA = 0; break;
      case 1: PORTA = 8; break;
      case 2: PORTA = 4; break;
      case 3: PORTA = 12; break;
      case 4: PORTA = 2; break;
      case 5: PORTA = 10; break;
      case 6: PORTA = 6; break;
      case 7: PORTA = 14; break;
   
   }
   Q64_val = val;
}

#define cycles_delay 2

void PLL1()
{
   Q64_tmp = Q64_val;
   Q64(1);
   delay_cycles(cycles_delay);
   Q64(Q64_tmp);
}

void PLL2()
{
   CPU_36_BIT128 = 1;
   delay_cycles(cycles_delay);
   CPU_36_BIT128 = 0;
}

void counter_preset_enable()
{
   Q64_tmp = Q64_val;
   Q64(2);
   delay_cycles(cycles_delay);
   Q64(Q64_tmp);
}

void banddata()
{
   Q64_tmp = Q64_val;
   Q64(3);
   delay_cycles(cycles_delay);
   Q64(Q64_tmp);
}

void beep()
{
   Q64_tmp = Q64_val;
   Q64(4);
   delay_cycles(cycles_delay);
   Q64(Q64_tmp);
}


void set_dial_lock(int1 res)
{
   if(res  == 1)Q64(5);
   if(res  == 0)Q64(0);
}

void set_clarifier(int1 res)
{
   
   if(res  == 1){if(load_clar_TX_f() == 0) save_clar_TX_f(frequency);}
   if(res  == 0) save_clar_TX_f(0);
}

void set_split(int1 res)
{
   if(res  == 1) {if(!split_transmit) split_transmit = 1;}
   if(res  == 0) split_transmit = 0;
}


void set_tx_inhibit(INT1 res)
{
   if(res == 1)Q64(6);
   if(res == 0)Q64(0);
}

void cat_transmit(INT1 tx_request)
{
   #ifdef debug_cat_tx

   //if (tx_request == 1) {Q64(5); dl = 1; dcs = get_dcs();} else {Q64(0); dl = 0; dcs = get_dcs();}
   #ELSE

   if (tx_request == 1) 
   {
   set_dial_lock(0);
   Q64(7);
   }
   else
   {
   Q64(0);
   if(dl) set_dial_lock(1);
   }
   #endif

}

void errorbeep(INT8 beeps)
{
   for (INT8 i = 0; i < beeps; ++i)
   {beep(); delay_ms(200);}
}

int8 tmp_port_b = 0;

void save_port_b()
{
   tmp_port_b = PORTB;
}

void restore_port_b()
{
   PORTB = tmp_port_b;
   tmp_port_b = 0;
}

int8 read_counter()
{
   INT8 res = 0;
   if(pind4) res +=8;
   if(pind5) res +=4;
   if(pind6) res +=2;
   if(pind7) res +=1;
   RETURN res;
}

void load_10hz(INT8 val)
{
   save_port_b();
   INT8 loc100 = 112;
   PORTB = loc100 + val;
   counter_preset_enable();
   restore_port_b();
}

void load_100hz(INT8 val)
{
   //save_port_b();
   INT8 loc100 = 112;
   PORTB = loc100 + val;
   //restore_port_b();
}


