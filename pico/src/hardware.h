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

#define PHI2_PIN  26
#define CS_PIN    27
#define RW_PIN    23
#define FLAG_PIN  24
#define IRQ_PIN   21
#define TOD_PIN   19
#define CNT_PIN   41
#define SP_PIN    40
#define RES_PIN   35
#define PC_PIN    18

#define RS0_PIN   36
#define RS1_PIN   37
#define RS2_PIN   38
#define RS3_PIN   39

#define DATA0_PIN 27
#define DATA1_PIN 28
#define DATA2_PIN 29
#define DATA3_PIN 30
#define DATA4_PIN 31
#define DATA5_PIN 32
#define DATA6_PIN 33
#define DATA7_PIN 34

#define PA0_PIN  2
#define PA1_PIN  3
#define PA2_PIN  4
#define PA3_PIN  5
#define PA4_PIN  6
#define PA5_PIN  7
#define PA6_PIN  8
#define PA7_PIN  9

#define PB0_PIN  10
#define PB1_PIN  11
#define PB2_PIN  12
#define PB3_PIN  13
#define PB4_PIN  14
#define PB5_PIN  15
#define PB6_PIN  16
#define PB7_PIN  17

#define TIMER_A_PIN 16
#define TIMER_B_PIN 17

#endif