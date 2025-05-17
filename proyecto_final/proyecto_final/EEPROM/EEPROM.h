/*
 * EEPROM.h
 *
 * Created: 17/05/2025 02:01:30
 *  Author: jose_
 */ 

#include <avr/io.h>
#ifndef EEPROM_H_
#define EEPROM_H_

// Setear funciones
void writeEEPROM(uint8_t dato, uint16_t direccion);							// la direccion es de 10 bits, por eso usamos uint16
uint8_t readEEPROM(uint16_t direccion);



#endif /* EEPROM_H_ */