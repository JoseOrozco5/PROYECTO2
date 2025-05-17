/*
 * EEPROM.c
 *
 * Created: 17/05/2025 02:01:16
 *  Author: jose_
 */ 
// Librerias
#include "eeprom.h"


// NON-Interrupt subroutines

void writeEEPROM(uint8_t dato, uint16_t direccion)
{
	// 0b00000010 sigue escribiendo, si 0b00000000 ya termino de escribir
	while (EECR &(1 << EEPE));															// ESPERAMOS A QUE BIT 0b000000X0 sea 0 (osea ya termino de escribir)
	// Ya termino entonces ya puede empezar a escribir otra vez (le digo a donde "direccion")
	EEAR = direccion;																	// ar (address)
	// Establecer dato que quiero guardar
	EEDR = dato;																		// dr (data)
	EECR |= (1 << EEMPE);																// Master Write enable
	EECR |= (1 << EEPE);																// habilito escritura
}

uint8_t	 readEEPROM(uint16_t direccion)
{
	while (EECR &(1 << EEPE));															// ESPERAMOS A QUE BIT 0b000000X0 sea 0 (osea ya termino de escribir)
	EEAR = direccion;																	// Le digo donde leer
	EECR |= (1 << EERE);																// empezar lectura
	return EEDR;																		// regresa lectura
}

