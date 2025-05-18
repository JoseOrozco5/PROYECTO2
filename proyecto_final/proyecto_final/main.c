/*
 * proyecto_final.c
 *
 * Created: 17/05/2025 01:54:32
 * Author : jose_
 */ 

// Librerias
#define F_CPU 8000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>																		// necesaria para el sscanf
#include <string.h>


// Incluir librerias hechas
#include "PWM1/pwm1.h"
#include "PWM2/pwm2.h"
#include "UART/UART.h"
#include "EEPROM/eeprom.h"

// Function prototypes
void init_ADC();
void setup();
void guardar();
void mostrar();


// variables
uint8_t pot1, pot2, pot3, pot4;															// una variable para cada potenciometro
uint8_t servo1, servo2, servo3, servo4;													// una variable para cada servo
uint8_t alternador = 0;																	// variable para alternar entre potenciometros (multiplexado)
uint8_t adc_value;																		// valor del adc
uint8_t modo = 0;																		// modo en el que estoy
uint8_t antonio_banderas = 0;															// flag de modo
uint8_t bandera_guardar = 0;															// flag de guardar
uint8_t bandera_mostrar = 0;															// flag de mostrar
uint8_t adafruit = 0;																	// flag para mostrar mensaje como de bienvenida
//----------------EEPROM---------------//
uint8_t contador_pos = 0;																// variable para guardar registro de memoria
uint8_t pos_actual = 0;																	// posicion N
uint16_t addresse;
//---------------UART------------------//
uint8_t paquete = 0;																		// flag para comunicacion serial (se relaciona con main)
uint8_t caracteres =0;																	// lo que escribo en la terminal
char signal;																			// señal para serial
char buffer[16];




// Main
int main(void)
{
	// DIVIDIR PRESCALER PARA 8MHz
	CLKPR |= (1 << CLKPCE);
	CLKPR |= (1 << CLKPS0);
	
	setup();
	init_PWM1(19999);
	init_PWM2();
	init_ADC();
	init_USART();
	sei();
	while (1)
	{
		
		switch(modo)
		{
			case 0:
			//------------LED MODO MANUAL------------------//
			PORTB &= ~(1 << PORTB4);
			PORTB &= ~(1 << PORTB0);
			PORTB |= (1 << PORTB5);																// LED que indica modo manual
			//-----------------Iniciar ADC----------------//
			ADCSRA |= (1 << ADSC);																// se hace la lectura del adc (se llama a la interrupcion)
			adafruit = 0;	
			break;
			
			case 1:
			//------------LED MODO EEPROM--------------//
			PORTB &= ~(1 << PORTB5);
			PORTB &= ~(1 << PORTB4);
			PORTB |= (1 << PORTB0);																// LED que indique modo EEPROM
			adafruit = 0;
			break;
			
			case 2:
			//--------------LED MODO ADAFRUIT----------//
			PORTB &= ~(1 << PORTB5);
			PORTB &= ~(1 << PORTB0);
			PORTB |= (1 << PORTB4);																// LED que indica modo Adafruit
			/*
			if (adafruit == 0)
			{
				writeString("\n");
				writeString("Usted esta en modo Adafruit, por favor mueva un slider. \n");
				writeString("Indique que servomotor desea mover e ingrese un valor de 0 a 255. Ejemplo 1,120  \n");
				writeString("\n");
				adafruit = 1;
			}*/
			if (paquete == 1)
			{
				paquete = 0;														// limpio bandera para proxima interaccion
				uint8_t motor = buffer[1] - '0';
				uint8_t ang = 0;
				
				for (char *paketa = strchr(buffer, ':') + 1; *paketa; ++paketa)
				{
					ang	= ang * 10 + (*paketa - '0');
				}
				
				/*if (sscanf(buffer, "%hhu,%hhu", &motor, &ang) == 2)
				{*/
					switch(motor)
					{
						case 1:
						pulso_PWM1(ang);
						break;
						case 2:
						pulso2_PWM1(ang);
						break;
						case 3:
						pulse_PWM2(ang);
						break;
						case 4:
						pulse2_PWM2(ang);
						break;
					}
				/*else{
					writeString("Opción no valida, por favor ingrese el servomotor y angulo como \"1,180\". \n ");
				}*/
			//writeString(signal);
			_delay_ms(20);	
			motor = 0;
			ang = 0;	
			}
			break;
			
		}
		
	}
}

// NON-Interrupt functions
void init_ADC()
{
	ADMUX = 0;																		// LIMPIAR REGISTRO
	ADMUX |= (1 << REFS0);															// utilizar AVcc como referencia (5v)
	ADMUX |= (1 << ADLAR);															// justificado a la izquierda (y leo 8 bits mas significativos)
	ADMUX |= (1 << MUX0);															// utilizar PC1 para potenciometro
	
	ADCSRA = 0;																		// LIMPIAR REGISTRO
	ADCSRA |= (1 << ADEN);															// Activar ADC
	ADCSRA |= (1 << ADIE);															// Activar interrupcion del ADC
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);							// prescaler 128
	//ADCSRA |= (1 << ADSC);															// iniciar primera conversión
}

void setup()
{
	DDRB |= (1 << PORTB5) | (1 << PORTB4) | (1 << PORTB0);							// SALIDA LED´S modos
	
	DDRD &= ~((1 << PORTD4) | (1 << PORTD5) | (1 << PORTD6));						// entradas botones
	PORTD |= (1 << PORTD4) | (1 << PORTD5) |(1 << PORTD6);							// pull up internos
	//-------------LED´S EEPROM---------------//
	DDRC |= (1 << PORTC0) | (1 << PORTC5);
	DDRD |= (1 << PORTD7) | (1 << PORTD2);
	PORTC &= ~((1 << PORTC0) | (1 << PORTC5));
	PORTD &= ~((1 << PORTD7) | (1 << PORTD2));
	//-----------Interrupciones--------------//
	PCICR  |= (1<<PCIE2);															// pin-change interrupt puertoD
	PCMSK2 |= (1<<PCINT22) | (1 << PCINT21) | (1 << PCINT20);						// BOTON MODO EN PD6, PD5 MOSTRAR Y PD4 GUARDAR
}

void guardar()
{
	
	switch (contador_pos)
	{
		case 0:
		PORTC &= ~(1 << PORTC0);
		PORTC |= (1 << PORTC5);
		PORTD &= ~(1 << PORTD7);
		PORTD &= ~(1 << PORTD2);
		writeEEPROM(servo1, 0);													// registro de memoria (0, 4, 8 y 12)
		writeEEPROM(servo2, 1);													// registro de memoria (1, 5, 9 y 13)
		writeEEPROM(servo3, 2);													// registro de memoria (2, 6, 10 y 14)
		writeEEPROM(servo4, 3);
		break;
		case 1:
		PORTC |= (1 << PORTC0);
		PORTC &= ~(1 << PORTC5);
		PORTD &= ~(1 << PORTD7);
		PORTD &= ~(1 << PORTD2);
		writeEEPROM(servo1, 4);													// registro de memoria (0, 4, 8 y 12)
		writeEEPROM(servo2, 5);													// registro de memoria (1, 5, 9 y 13)
		writeEEPROM(servo3, 6);													// registro de memoria (2, 6, 10 y 14)
		writeEEPROM(servo4, 7);
		break;
		case 2:
		PORTC &= ~(1 << PORTC0);
		PORTC &= ~(1 << PORTC5);
		PORTD |= (1 << PORTD7);
		PORTD &= ~(1 << PORTD2);
		writeEEPROM(servo1, 8);													// registro de memoria (0, 4, 8 y 12)
		writeEEPROM(servo2, 9);													// registro de memoria (1, 5, 9 y 13)
		writeEEPROM(servo3, 10);													// registro de memoria (2, 6, 10 y 14)
		writeEEPROM(servo4, 11);
		break;
		case 3:
		PORTC &= ~(1 << PORTC0);
		PORTC &= ~(1 << PORTC5);
		PORTD &= ~(1 << PORTD7);
		PORTD |= (1 << PORTD2);
		writeEEPROM(servo1, 12);													// registro de memoria (0, 4, 8 y 12)
		writeEEPROM(servo2, 13);													// registro de memoria (1, 5, 9 y 13)
		writeEEPROM(servo3, 14);													// registro de memoria (2, 6, 10 y 14)
		writeEEPROM(servo4, 15);
		break;
	}
	contador_pos = (contador_pos + 1) % 4;
}

void mostrar()
{
	
	switch(pos_actual)
	{
		case 0:
		PORTC &= ~(1 << PORTC0);
		PORTC &= ~(1 << PORTC5);
		PORTD &= ~(1 << PORTD7);
		PORTD |= (1 << PORTD2);
		pulso_PWM1(readEEPROM(0));												// Posiciones 0, 4, 8 y 12 para servo1
		pulso2_PWM1(readEEPROM(1));
		pulse_PWM2(readEEPROM(2));
		pulse2_PWM2(readEEPROM(3));
		break;
		case 1:
		PORTC &= ~(1 << PORTC0);
		PORTC &= ~(1 << PORTC5);
		PORTD |= (1 << PORTD7);
		PORTD &= ~(1 << PORTD2);
		pulso_PWM1(readEEPROM(4));
		pulso2_PWM1(readEEPROM(5));												// Posiciones 1, 5, 9 y 13 para servo2
		pulse_PWM2(readEEPROM(6));
		pulse2_PWM2(readEEPROM(7));
		break;
		case 2:
		PORTC |= (1 << PORTC0);
		PORTC &= ~(1 << PORTC5);
		PORTD &= ~(1 << PORTD7);
		PORTD &= ~(1 << PORTD2);
		pulso_PWM1(readEEPROM(8));
		pulso2_PWM1(readEEPROM(9));
		pulse_PWM2(readEEPROM(10));												// Posiciones 2, 6, 10 y 14 para servo3
		pulse2_PWM2(readEEPROM(11));
		break;
		case 3:
		PORTC &= ~(1 << PORTC0);
		PORTC |= (1 << PORTC5);
		PORTD &= ~(1 << PORTD7);
		PORTD &= ~(1 << PORTD2);
		pulso_PWM1(readEEPROM(12));
		pulso2_PWM1(readEEPROM(13));
		pulse_PWM2(readEEPROM(14));
		pulse2_PWM2(readEEPROM(15));												// Posiciones 3, 7, 11 y 15 para servo4
		break;
	}
	
	pos_actual = (pos_actual + 1) % 4;
}

// Interrupt routines
ISR(ADC_vect)
{

	adc_value = ADCH;																	// le cargo el valor del ADCH a adc_value
	
	switch (alternador)
	{
		
		case 0:

		alternador = 1;																	// Servo1 en PB1
		ADMUX = 0;																		// LIMPIAR REGISTRO
		ADMUX |= (1 << REFS0);															// utilizar AVcc como referencia (5v)
		ADMUX |= (1 << ADLAR);															// justificado a la izquierda (y leo 8 bits mas significativos)
		ADMUX |= (1 << MUX1);															// utilizar PC2 para potenciometro 2 (MUX1)
		
		pot1 = adc_value;
		servo1 = (pot1 * 180 / 255);													// convertir adc en angulo
		pulso_PWM1(servo1);																// ajustar ancho de pulso
		
		break;
		
		case 1:
		alternador = 2;																	// Servo2 en PB2
		
		ADMUX = 0;																		// LIMPIAR REGISTRO
		ADMUX |= (1 << REFS0);															// utilizar AVcc como referencia (5v)
		ADMUX |= (1 << ADLAR);
		ADMUX |= (1 << MUX0);															// PC3 como entrada (MUX0 y MUX1)
		ADMUX |= (1 << MUX1);
		
		pot2 = adc_value;
		servo2 = (pot2 * 180 / 255);
		pulso2_PWM1(servo2);
		
		break;
		
		case 2:
		alternador = 3;																	// Servo3 en PB3
		
		ADMUX = 0;																		// LIMPIAR REGISTRO
		ADMUX |= (1 << REFS0);															// utilizar AVcc como referencia (5v)
		ADMUX |= (1 << ADLAR);
		ADMUX |= (1 << MUX2);															// PC4 como entrada (pot4)
		
		pot3 = adc_value;
		servo3 = (pot3 * 180 / 255);
		pulse_PWM2(servo3);
		
		break;
		
		case 3:
		alternador = 0;																	// Servo4 en PD3
		
		ADMUX = 0;																		// LIMPIAR REGISTRO
		ADMUX |= (1 << REFS0);															// utilizar AVcc como referencia (5v)
		ADMUX |= (1 << ADLAR);
		ADMUX |= (1 << MUX0);															// Regresamos a pot1 (PC1)
		
		pot4 =  adc_value;
		servo4 = ((pot4 * 180 + 128) / 255);
		pulse2_PWM2(servo4);
		break;
		
	}
	
}

ISR(USART_RX_vect)
{
	signal = UDR0;																		// leer caracter enviado a la terminal
	
	if (signal != '\n' && signal != '\r' && caracteres < 15)												// si no presiono enter ni escribo mas de 15 caracteres pasa lo de abajo
	{
		buffer[caracteres++] = signal;													// guardo los datos que meti a la terminal en el buffer/string
		//WriteChar(signal);
	}
	else
	{
		paquete = 1;																		// enciendo flag para el main
		writeString(buffer);
		writeString("\n");
		buffer[caracteres] = '\0';														// cierro cadena (es como poner . al final de la oracion)
		caracteres = 0;																	// limpio variable para proxima vez
	}
}

ISR(PCINT2_vect)
{
	//---------Guardar posicion EEPROM----------//
	if (!(PIND & (1 << PORTD4)) && bandera_guardar == 0)									// Si se presiona el boton y todavia no se ha terminado de procesar la pulsación
	{
		bandera_guardar = 1;
		guardar();
		} else {
		bandera_guardar = 0;
	}
	
	//---------Mostrar posicion EEPROM----------//
	if (!(PIND & (1 << PORTD5)) && bandera_mostrar == 0)
	{
		bandera_mostrar = 1;
		mostrar();
		} else{
		bandera_mostrar = 0;
	}
	
	//------------Cambio de modo--------------//
	if (!(PIND &(1 << PORTD6)) && antonio_banderas == 0)														// Si se apacha PD4 se le suma 1 a modo (es decir cambia de modo)
	{
		antonio_banderas = 1;
		modo = (modo + 1) %3;
		} else{
		antonio_banderas = 0;
	}
}

