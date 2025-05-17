/*
 * pwm2.h
 *
 * Created: 17/05/2025 02:04:43
 *  Author: jose_
 */ 

#include <avr/io.h>
#ifndef PWM2_H_
#define PWM2_H_

// Setear funciones
void init_PWM2(void);
void pulse_PWM2(uint8_t angle3);
void pulse2_PWM2(uint8_t angle4);



#endif /* PWM2_H_ */