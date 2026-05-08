#ifndef HARDWARE_H
#define HARDWARE_H

#include <stdint.h>
#include <stdbool.h>

void hardware_init(void);
void hardware_set_ddr(uint8_t a, uint8_t b);
void hardware_set_pa(uint8_t val);
void hardware_set_pb(uint8_t val);
uint8_t hardware_get_pa(void);
uint8_t hardware_get_pb(void);

#define PHI2_PIN  24
#define CS_PIN    25
#define RW_PIN    26
#define FLAG_PIN  27
#define IRQ_PIN   28
#define TOD_PIN   29
#define CNT_PIN   30
#define SP_PIN    31
#define RES_PIN   32
#define PC_PIN    33

#define RS0_PIN   16
#define RS1_PIN   17
#define RS2_PIN   18
#define RS3_PIN   19

#define DATA0_PIN 0
#define DATA1_PIN 1
#define DATA2_PIN 2
#define DATA3_PIN 3
#define DATA4_PIN 4
#define DATA5_PIN 5
#define DATA6_PIN 6
#define DATA7_PIN 7

#define PA0_PIN  0
#define PA1_PIN  1
#define PA2_PIN  2
#define PA3_PIN  3
#define PA4_PIN  4
#define PA5_PIN  5
#define PA6_PIN  6
#define PA7_PIN  7

#define PB0_PIN  8
#define PB1_PIN  9
#define PB2_PIN  10
#define PB3_PIN  11
#define PB4_PIN  12
#define PB5_PIN  13
#define PB6_PIN  14
#define PB7_PIN  15

#define TIMER_A_PIN 14
#define TIMER_B_PIN 15

#endif