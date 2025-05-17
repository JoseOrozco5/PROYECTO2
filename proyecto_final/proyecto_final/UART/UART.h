/*
 * UART.h
 *
 * Created: 17/05/2025 02:02:59
 *  Author: jose_
 */ 

#include <avr/io.h>
#ifndef UART_H_
#define UART_H_

// Setear funciones
void init_USART();
void WriteChar(char j);
void cadena(char* txt);												// el char* sirve para representar cadenas de texto



#endif /* UART_H_ */