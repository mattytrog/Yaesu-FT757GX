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


//inputs
# bit CHA=PORTA.0
# bit CHB=PORTA.1
# bit dl=PORTA.2
# bit buzz=PORTB.0
# bit jp1=PORTB.1
# bit jp2=PORTB.2
# bit jp3=PORTB.3

int8 BITSA, BITSB;

# bit currentStateCHA=BITSA.0
# bit previousStateCHA=BITSA.1
# bit currentStateCHB=BITSA.2
# bit previousStateCHB=BITSA.3
# bit currentStateBEEP=BITSA.4
# bit previoustStateBEEP=BITSA.5
# bit detected=BITSA.6
# bit beeptimerstart=BITSA.7
# bit buzzstate=BITSB.0

int8 pulsechain;

unsigned INT32 ontimer = 0;
unsigned INT32 offtimer = 0;
unsigned INT32 beeptimer = 0;
//beep interrupt. Output is either through a speaker (uses PWM to generate beep) or active DC buzzer (line goes high). Beep duration is beeplen * 20 approx
//Delaying cycle while beep occurs isn't being lazy. Original chips can go crazy. This is for compatibility with original Yaesu chips.
#INT_EXT
void ext_isr(VOID)
{
   IF (buzz)
   {
   if(!beeptimerstart) beeptimerstart = 1;
   CLEAR_INTERRUPT (INT_EXT); //clear and exit back to main loop
   }
}

#define onpulse 5    //how long each pulse will remain high in uS 
#define offpulse 5   //and how long will be low before checking encoder again
#define beeplen 3000 //60000 uS approx

//only applies to speaker. Doesn't apply to DC buzzer
#define beepvol 2
#define beeppitch 31

void setup()
{
   setup_ccp1 (CCP_PWM); // Configure CCP1 as a PWM
   setup_timer_2 (T2_DIV_BY_16, beeppitch, 1) ;
   setup_adc (ADC_OFF);
   set_tris_a (0b00111);
   set_tris_b (0b00001111);
   set_tris_c (0b00000000);
   set_tris_e (0b000);
   ext_INT_edge (L_TO_H);
   
   //activate pullups briefly while we read jumper state
   port_b_pullups (true);
   //pulsechain is the amount of pulses transmitted in relation to pulses received from encoder. Normally 2(stock Yaesu - 1 per channel).
   //adding more leads to faster counting, at the expense of accuracy (only noticeable on 5+ pulses)
   //pulsechain of 1 = 5khz per revolution
   //pulsechain of 2 = 10khz per revolution
   if((jp1) && (jp2) && (jp3)) pulsechain = 1;
   
   //optional jumpers to speed up counting if required.
   IF ( ! jp1) pulsechain =   2;
   IF ( ! jp2) pulsechain =   5;
   IF ( ! jp3) pulsechain =   10;
   port_b_pullups (false);
   
   disable_interrupts (INT_EXT);
   enable_interrupts (INT_EXT);
   enable_interrupts (GLOBAL);

   //welcome beep
   beeptimerstart = 1;

   //get base state of channels
   previousStateCHB = CHB;
   previousStateCHA = CHA;
}


void main()
{
   setup ();

   WHILE (true)
   {
   
      if(!beeptimer)
      {
      IF ( dl) while(dl){}
         
         
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
               if(beeptimerstart) break;
            }
         }
   
         //IF CHA is low and CHB is high, we wait for CHB pulse to finish. then check if CHA is under it.
         //IF so, then A is at least 180 degrees out of phase with B, thus we are spinning anti - clockwise.
         //Send signal to radio low, look FOR pulses
         IF ( (! CHA)&& (CHB))
         {
            WHILE (CHB)
            {
               
               IF (CHA){ dir_out = 0; break; }
               if(beeptimerstart) break;
            }
         }
   
         
         //keep checking A
         currentStateCHA = CHA;
         currentStateCHB = CHB;
   
         //IF A has changed, send a pulse, update previous reading with now current reading
   
            IF (previousStateCHA != currentStateCHA)
            {
               //detected = 1;
               previousStateCHA = currentStateCHA;
            } 
   
            IF (previousStateCHB != currentStateCHB)
            {
            detected = 1;
             previousStateCHB = currentStateCHB;
            }
            
         if(detected)
         {
         for (INT i = 0; i < pulsechain; ++i)
               {
                  FOR (ontimer = 0; ontimer < onpulse; ontimer++) clk_out = 0;
                  FOR (offtimer = 0; offtimer < offpulse; offtimer++) clk_out = 1;
               }
               detected = 0;
         }
         delay_us (1) ;
      }
      
      if(beeptimerstart)
      {
         ++beeptimer;
         if(beeptimer < beeplen)
         {
         set_pwm1_duty (beepvol);
         abeep = 1;
         }
         else
         {
         set_pwm1_duty (0);
         abeep = 0;
         beeptimer = 0; beeptimerstart = 0;
         }
         delay_us (1) ;
      }
   }
}

