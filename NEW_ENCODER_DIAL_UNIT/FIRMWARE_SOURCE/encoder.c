//FTPlus Dial Unit firmware
//(c)2025 Matthew Bostock M0WCA


#include <16F737.h>
#fuses INTRC_IO, NOWDT, BROWNOUT, BORV42, PUT
#use delay(internal=8M)
# byte PORTA = 0x05
# byte PORTB = 0x06
# byte PORTC = 0x07
# byte PORTE = 0x09

//outputs
# bit dir_out=PORTC.0
# bit clk_out=PORTC.1
# bit beep=PORTC.2
# bit abeep=PORTC.3
# bit led1=PORTC.4
# bit led2=PORTC.5
# bit led3=PORTC.6

//inputs
# bit CHA=PORTA.0
# bit CHB=PORTA.1
# bit dl=PORTA.2
# bit buzz=PORTB.0
# bit jp1=PORTB.1
# bit jp2=PORTB.2
# bit jp3=PORTB.3

int8 BITSA;

# bit currentStateCHA=BITSA.0
# bit previousStateCHA=BITSA.1
# bit currentStateCHB=BITSA.2
# bit previousStateCHB=BITSA.3
# bit currentStateBEEP=BITSA.4
# bit previoustStateBEEP=BITSA.5

int8 pulsechain;

unsigned INT32 ontimer = 0;
unsigned INT32 offtimer = 0;

//beep interrupt. Output is either through a speaker (uses PWM to generate beep) or active DC buzzer (line goes high). Beep duration is 100mS
//Delaying cycle while beep occurs isn't being lazy. Original chips can go crazy. This is for compatibility with original Yaesu chips.
#INT_EXT
void ext_isr(VOID)
{
   IF ( ! buzz)
   {
      set_pwm1_duty (20);
      abeep = 1;
      delay_ms (100); //beep length. Further interrupts are ignored until after 100mS
      set_pwm1_duty (0);
      abeep = 0;
      CLEAR_INTERRUPT (INT_EXT); //clear and exit back to main loop
   }
}

void setup()
{
   setup_ccp1 (CCP_PWM); // Configure CCP1 as a PWM
   setup_timer_2 (T2_DIV_BY_16, 31, 1) ;
   setup_adc (ADC_OFF);
   set_tris_a (0b00111);
   set_tris_b (0b00001111);
   set_tris_c (0b00000000);
   set_tris_e (0b000);
   ext_INT_edge (L_TO_H);
   port_b_pullups (true);
   disable_interrupts (INT_EXT);
   enable_interrupts (INT_EXT);
   enable_interrupts (GLOBAL);
   
   //welcome beep
   set_pwm1_duty (20);
   abeep = 1;
   delay_ms (100);
   set_pwm1_duty (0);
   abeep = 0;

   //get base state of channels
   previousStateCHB = CHB;
   previousStateCHA = CHA;
}

#define onpulse 5    //how long each pulse will remain high in uS 
#define offpulse 5   //and how log will be low before checking encoder again

void main()
{
   setup ();

   WHILE (true)
   {
      //pulsechain is the amount of pulses transmitted in relation to pulses received from encoder. Normally 1(stock Yaesu).
      //adding more leads to faster counting, at the expense of accuracy (only noticeable on 5+ pulses)
      pulsechain = 1;
      
      //optional jumpers to speed up counting if required. Not needed in normal use
      IF ( ! jp1) pulsechain +=   1;
      IF ( ! jp2) pulsechain +=   4;
      IF ( ! jp3) pulsechain +=   9;
      
      //work out direction
      //CHA is 180 deg out of phase with CHB acc to datasheet
      
      //IF CHA is high and CHB is low, we wait for CHA pulse to finish. then check if CHB is under it.
      //IF so, then B is at least 180 degrees out of phase with A, thus we are spinning clockwise.
      //Send signal to radio high, look FOR pulses
      IF ( (CHA)&& (! CHB))
      {
         WHILE (CHA)
         {
            
            IF (CHB){ dir_out = 1; break; }
         }
      }

      ELSE
      //IF CHA is low and CHB is high, we wait for CHB pulse to finish. then check if CHA is under it.
      //IF so, then A is at least 180 degrees out of phase with B, thus we are spinning anti - clockwise.
      //Send signal to radio low, look FOR pulses
      IF ( (! CHA)&& (CHB))
      {
         WHILE (CHB)
         {
            
            IF (CHA){ dir_out = 0; break; }
         }
      }

      
      //keep checking A
      currentStateCHA = CHA;
      currentStateCHB = CHB;

      //IF A has changed, send a pulse, update previous reading with now current reading
      IF ((previousStateCHA != currentStateCHA) || (previousStateCHB != currentStateCHB))
      {
         
         led1 = 0; led2 = 0; led3 = 0;
         IF ( ! dl)
         {
            for (INT i = 0; i < pulsechain; ++i)
            {
               FOR (ontimer = 0; ontimer < onpulse; ontimer++) clk_out = 1;
               FOR (offtimer = 0; offtimer < offpulse; offtimer++) clk_out = 0;
            }

         }

         previousStateCHA = currentStateCHA;
         previousStateCHB = currentStateCHB;
      }

      delay_us (1) ;
   }
}

