; This software should be assembled with Microchip MPLAB 8.36
; Programming details Brown out  4,5V    Power up timer on
;
;
; ****************************************************************************
; * FREQUENCY DISPLAY (Sparepart for TMS2370 M30026  FT757 GX and GXII)      *
; *  Version 1.0                                                             *
; *  April, 2006                                                             *
; *  written by Rudi Kauls , DB6RK                                           *
; ****************************************************************************
;  History:
;> Version 1.1 from 18.04.06		all funktions tested with FT757GXII,
;									when changing frequency with the knob very fast 
;									HAM/GEN is engaged sometimes
;> Version 1.2 from 17.05.06		Changed decoding of the display because there is a
;									problem with the pcb.
;									Restart problems with the new pcb :
;									forgotten to initialize the whole display memory.			 		
;									Sometimes restart problems depending on the power
;									rise time
;> Version 1.3 from 23.09.09        Wait_Ready routine added for compensating the timing
;                                   of the FT757GX 
;                         
; Target Controller -      PIC16F737
;                          __________
;          Reset------RE4 |1       28| RB7------Data0
;          SCN0-------RA0 |2       27| RB6------Data1
;          SCN1-------RA1 |3       26| RB5------Data2 
;     	   SCN2-------RA2 |4       25| RB4------Data3
;          SCN3-------RA3 |5       24| RB3------SCN8
;          SCN4-------RA4 |6       23| RB2------Ham/Gen
;          SCN5-------RA5 |7       22| RB1------Handshake
;        Ground-------VSS |8       21| RB0------Strobe
;          SCN6-------RA6 |9       20| VDD------+5V
;          SCN7-------RA7 |10      19| VSS------Ground
;     VFD Seg_dp------RC0 |11      18| RC7------VFD Seg_a
;     VFD Seg_g-------RC1 |12      17| RC6------VFD Seg_b
;     VFD Seg_f-------RC2 |13      16| RC5------VFD Seg_c
;     VFD Seg_e-------RC3 |14      15| RC4------VFD Seg_d
;                          ----------
;
;
; ****************************************************************************
; * Device type and options.                                                 *
; ****************************************************************************
;
        processor PIC16F737
        include P16F737.inc
;
; ****************************************************************************
;		wdt off,reset off, brown out on at 4,5V, internal rc osc.            *
; ****************************************************************************
del_ctr				equ		0x50
i_dat				equ		0x51
Flags				equ		0x52
wpointer 			equ		0x53
HAM					equ		0x33
; ****************************************************************************
; * The 16F737 resets to 0x00.                                               * 
; ****************************************************************************             
;
        ORG     0x0000                
reset_entry

        goto    start             ;
     
Disp_Decode

        addwf   PCL,f             ; 
        retlw   b'00000011'       ; 0			
        retlw   b'00000001'		  ; 1,8			
        retlw   b'10011001'   	  ; 2,4		
        retlw   b'01100011'		  ; 3,c			
       	retlw   b'00100101' 	  ;	4,2
        retlw   b'00010001' 	  ; 5,a
        retlw   b'01000001'       ; 6,6
        retlw   b'01100001' 	  ; 7,e
        retlw   b'10011111'       ;	8,1
        retlw   b'00001001'       ; 9,9
        retlw   b'01001001'       ; a,5 
        retlw   b'10000101'       ; b,d
      	retlw   b'00001101'		  ;	c,3
        retlw   b'11000001'       ; d,b 
        retlw   b'00011111' 	  ; e,7 
        retlw   b'11111111'		  ;	f,f
       
;;
; *****************************************************************************
;
start
;clear display memory (0,1,2,3,4,5,6,7,8)

		movlw	0x00
		movwf	0x32
		movwf	0x33
		movwf	0x34
		movwf	0x35
		movwf	0x36
		movwf	0x37
		movwf	0x38
		movwf	0x39
		movwf	0x3A
		movwf	0x3B
		movwf	0x3C
		movwf	0x3D
		movwf	0x3E
		movwf	0x3F

; Setup clock, setup ports


  	clrf    INTCON           	; no interrupts

	movlw	0xff
	movwf	PORTC				; segments off

    bsf     STATUS,RP0      	; rp1 on

	movlw	0x4E
	movwf	OSCCON				;internal clock 1 mc

	movlw	b'10000011'	 		;clock for TMR0 = t/16
	movwf 	OPTION_REG

    movlw   0x0f				;analog off
    movwf	ADCON1				;

	clrf    TRISA            	;port A all outputs
								;
	movlw	b'11110001'	  		; port b mixed
    movwf   TRISB           	; 

	clrf	TRISC				; port c all outputs
	
	bcf		STATUS,RP0
 

	movlw	0xff
    movwf	PORTA
	movwf	PORTB
	movwf	PORTC



;******************************************************************************
;	Data from TRX to Display, all 13 nibbles in one cycle
;******************************************************************************

Loop

	movlw			0x32		;setup writepointer
	movwf			wpointer

	movlw			0x28		;setup timer for handshake
	movwf			TMR0
	bsf				PORTB,1		;handshake on

Wait_Data
	btfsc			TMR0,7		;					2
	goto			End_read	;time expired
	btfss			PORTB,0		;					2
	goto			Wait_Data

	movfw			PORTB		;fetching a nibble  1
	andlw			0xf0		;					1
	movwf			i_dat		;					1
	swapf			i_dat		;					1

	movfw			wpointer	;indirect adressing 1
	movwf			FSR			;					1			
	movfw			i_dat		;					1
	movwf			INDF		;					1
	incf			wpointer	;max pointer?		1
	btfsc			wpointer,6	;					2
	clrf			Flags		;					1
Wait_Ready
	btfsc			TMR0,7		;					2
	goto			End_read	;time expired
	btfss			PORTB,0		;					2
	goto			Wait_Data
	goto			Wait_Ready	;					2

End_read						
	bcf				PORTB,1		;handshake off


;******************************************************************************
;	HAM/GEN indicator
;******************************************************************************

	btfsc			HAM,3		;HAM/GEN
	bsf				PORTB,2			
	btfss			HAM,3
	bcf				PORTB,2



	;
	BCF				PORTA,0		;Digit 1
	movlw			0x37		;
	movwf			FSR
	movfw			INDF
	call			Disp_Decode
	movwf			PORTC
	call			Delay

	movlw			0xff		; Digit 2
	movwf			PORTC		
	BSF				PORTA,0		;
	BCF				PORTA,1
	movlw			0x38
	movwf			FSR
	movfw			INDF
	call			Disp_Decode
	movwf			PORTC
	call			Delay

	movlw			0xff		; 10mc
	movwf			PORTC		
	BSF				PORTA,1		;
	BCF				PORTA,2
	movlw			0x39
	movwf			FSR
	movfw			INDF
	call			Disp_Decode
	andlw			0xfe		;D.P.
	movwf			PORTC
	call			Delay

	movlw			0xff		; 1mc
	movwf			PORTC		
	BSF				PORTA,2	
	BCF				PORTA,3		;
	movlw			0x3A		;
	movwf			FSR
	movfw			INDF
	call			Disp_Decode
	movwf			PORTC
	call			Delay

	movlw			0xff		; 100kc
	movwf			PORTC		
	BSF				PORTA,3		;
	BCF				PORTA,4
	movlw			0x3B
	movwf			FSR
	movfw			INDF
	call			Disp_Decode
	movwf			PORTC
	call			Delay

	movlw			0xff		; 10kc
	movwf			PORTC		
	BSF				PORTA,4	;
	BCF				PORTA,5
	movlw			0x3C
	movwf			FSR
	movfw			INDF
	call			Disp_Decode
	andlw			0xfe		;d.P.
	movwf			PORTC
	call			Delay


	movlw			0xff		; 1 kc
	movwf			PORTC		
	BSF				PORTA,5	
	BCF				PORTA,7		;
	movlw			0x3D		;RAM
	movwf			FSR
	movfw			INDF
	call			Disp_Decode
    movwf			PORTC
	call			Delay

	movlw			0xff		; 100 c
	movwf			PORTC		
	BSF				PORTA,7		;
	BCF				PORTA,6
	movlw			0x3E
	movwf			FSR
	movfw			INDF
	call			Disp_Decode
	movwf			PORTC
	call			Delay

	movlw			0xff		;digit 9
	movwf			PORTC		
	BSF				PORTA,6		;
	BCF				PORTB,3
	movlw			0x36		;
	movwf			FSR
	movfw			INDF
	call			Disp_Decode
	movwf			PORTC
	call			Delay

	movlw			0xff		; display off
	movwf			PORTC		
	BSF				PORTB,3	

	goto			Loop			





;*******************************************************************************
;subroutine
;*******************************************************************************
Delay
	movlw			0x5e
	movwf			del_ctr
d1
	decfsz			del_ctr
	goto			d1
	retlw			0	

	fill  (goto start), 100




	END
