//============================================================================
// Name        : StopWatch.c
// Author      : Mohammed Ayman
// Version     :
// Copyright   : Your copyright notice
// Description : StopWatch implementation using ATmega32
//============================================================================


#include<avr/io.h>
#include<util/delay.h>
#include<avr/interrupt.h>

#define Comp_Val 15625
typedef unsigned char uint8;

/*
 * Description:
 * Timer1 will be used with pre-scaler F_CPU/64
 * F_Timer = 15.625 KHz, timer Period = 0.064 ms
 * so to get 1sec we need 15625 counts
 */

typedef struct Time
{
	uint8 Sec_Unit;
	uint8 Sec_Tenth;
	uint8 Min_Unit;
	uint8 Min_Tenth;
	uint8 Hr_Unit;
	uint8 Hr_Tenth;
}Time;

Time time={0,0,0,0,0,0};

void Timer1_init()
{
	/* Configure timer control register TCCR1A (non-PWM operation)
	 * 1. Disconnect OC1A and OC1B  COM1A1=0 COM1A0=0 COM1B0=0 COM1B1=0
	 * 2. FOC1A=1 FOC1B=0
	 * 3. CTC Mode WGM10=0 WGM11=0 (Mode Number 4)
	 */

	TCCR1A=(1<<FOC1A);
	TCNT1=0;
	OCR1A=Comp_Val;			// Set Compare value
	TIMSK |= (1<<OCIE1A);	// Enable Timer1 Compare A Interrupt

	/* Configure timer control register TCCR1B
	 * 1. CTC Mode WGM12=1 (Mode Number 4)
	 * 2. pre-scaler 64 CS12=0 , CS11=1 , CS10=1
	 */
	TCCR1B=(0x0B);			//	TCCR1B = (1<<WGM12) | (1<<CS11)|(1<<CS10)

}

ISR(TIMER1_COMPA_vect)
{
	time.Sec_Unit++;
	if(time.Sec_Unit >9)
	{
		time.Sec_Unit=0;
		time.Sec_Tenth++;
	}
	if(time.Sec_Tenth>5)
	{
		time.Sec_Tenth=0;
		time.Min_Unit++;
	}
	if(time.Min_Unit >9)
	{
		time.Min_Unit=0;
		time.Min_Tenth++;
	}
	if(time.Min_Tenth>5)
	{
		time.Min_Tenth=0;
		time.Hr_Unit++;
	}
	if(time.Hr_Unit >9)
	{
		time.Hr_Unit=0;
		time.Hr_Tenth++;
	}
}

void Display_7seg(uint8 watch,uint8 enable)
{
	PORTC = (PORTC&0xF0) | (watch&0x0F); //Put the number in portC
	PORTA|=(1<<enable);
	_delay_ms(3);
	PORTA &=~(1<<enable);
}

ISR(INT0_vect) //reset
{
	time.Hr_Tenth=0;
	time.Hr_Unit=0;
	time.Min_Tenth=0;
	time.Min_Unit=0;
	time.Sec_Tenth=0;
	time.Sec_Unit=0;

	TCNT1=0;
}

ISR(INT1_vect) //Pause by disable the clock
{
	TCCR1B &= ~(1<<CS12) & ~(1<<CS11) & ~(1<<CS10);
}

ISR(INT2_vect) //resume
{
	TCCR1B |= (1<<CS11)|(1<<CS10);
}


int main(void)
{
	/******************Interrupts*****************/



	DDRD&=~ (1<<PD2); // INT0 Switch (Reset)
	PORTD|= (1<<PD2); //Internal PULL-UP


	DDRD&=~ (1<<PD3); // INT1 Switch (Pause)


	DDRB&=~ (1<<PB2); // INT2 Switch (Resume)
	PORTB|= (1<<PB2); //Internal PULL-UP


	GICR|= (1<<INT2) | (1<<INT1)| (1<<INT0); //Enable INT0,1,2


	MCUCR&= ~(1<<ISC01); //Falling edge INT0


	MCUCR|= (1<<ISC11)|(1<<ISC10); //Rising Edge INT1


	MCUCSR&= ~(1<<ISC2); //Falling edge INT2


	SREG|= (1<<7); //Enable Interrupts



	/******************Enable 7-seg*****************/
	DDRA|=(0x3F); //o/p Pins for enable of 7-seg

	DDRC|=(0x0F); //o/p Pins for decoder

	Timer1_init();
	while(1)
	{
		Display_7seg(time.Sec_Unit,PA0);
		Display_7seg(time.Sec_Tenth,PA1);
		Display_7seg(time.Min_Unit,PA2);
		Display_7seg(time.Min_Tenth,PA3);
		Display_7seg(time.Hr_Unit,PA4);
		Display_7seg(time.Hr_Tenth,PA5);

	}
}
