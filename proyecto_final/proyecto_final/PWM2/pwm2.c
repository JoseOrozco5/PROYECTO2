/*
 * pwm2.c
 *
 * Created: 17/05/2025 02:04:30
 *  Author: jose_
 */ 
// Librerias
#include "pwm2.h"

// Variables
uint8_t servo3;
uint8_t servo4;

// NON-Interrupt subroutines
void init_PWM2(void){
	DDRB |= (1 << PORTB3);																// Salida servo3
	DDRD |= (1 << PORTD3);																// salida servo4
	TCCR2A = 0;																			// LIMPIAR TCCR2A
	TCCR2A |= (1 << COM2A1) | (1 << COM2B1);											// non inverting fast pwm (COM2A1 para servo3 y COM2B1 para servo4)
	TCCR2A |= (1 << WGM20) | (1 << WGM21);												// fast pwm
	TCCR2B = 0;																			// LIMPIAR TCCR2B
	TCCR2B |= (1 << CS22) | (1 << CS21)	|(1 << CS20);									// PRESCALER 1024
	//OCR2A = SUPERIOR;																			// top
	
}

void pulse_PWM2(uint8_t angle3)
{
	OCR2A = 0.19 * (float)angle3 + 5.0;												// 0.19 - 180
}

void pulse2_PWM2(uint8_t angle4)
{
	OCR2B = 0.19 * (float)angle4 + 5.0;
}
// Interrupt routines