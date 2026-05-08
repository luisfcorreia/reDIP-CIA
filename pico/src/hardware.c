#include "hardware.h"
#include "hardware/gpio.h"
#include <stdio.h>

static uint8_t ddra = 0x00;
static uint8_t ddrb = 0x00;

void hardware_init(void) {
    printf("[HW] initializing...\n");

    gpio_init(PHI2_PIN);
    gpio_set_dir(PHI2_PIN, GPIO_IN);

    gpio_init(CS_PIN);
    gpio_set_dir(CS_PIN, GPIO_IN);

    gpio_init(RW_PIN);
    gpio_set_dir(RW_PIN, GPIO_IN);

    for (int i = RS0_PIN; i <= RS3_PIN; i++) {
        gpio_init(i);
        gpio_set_dir(i, GPIO_IN);
    }

    gpio_init(FLAG_PIN);
    gpio_set_dir(FLAG_PIN, GPIO_IN);

    gpio_init(IRQ_PIN);
    gpio_set_dir(IRQ_PIN, GPIO_OUT);
    gpio_put(IRQ_PIN, 1);

    gpio_init(TOD_PIN);
    gpio_set_dir(TOD_PIN, GPIO_IN);

    gpio_init(CNT_PIN);
    gpio_set_dir(CNT_PIN, GPIO_IN);

    gpio_init(SP_PIN);
    gpio_set_dir(SP_PIN, GPIO_IN);

    gpio_init(PC_PIN);
    gpio_set_dir(PC_PIN, GPIO_OUT);
    gpio_put(PC_PIN, 1);

    for (int i = PA0_PIN; i <= PA7_PIN; i++) {
        gpio_init(i);
        gpio_set_dir(i, GPIO_OUT);
        gpio_put(i, 1);
    }

    for (int i = PB0_PIN; i <= PB7_PIN; i++) {
        gpio_init(i);
        gpio_set_dir(i, GPIO_OUT);
        gpio_put(i, 1);
    }

    printf("[HW] initialized - DIP40 RP2350B\n");
    printf("[HW] PA: GPIO0-7, PB: GPIO8-15\n");
    printf("[HW] PHI2:24 CS:25 R/W:26 RS:16-19\n");
    printf("[HW] FLAG:27 IRQ:28 TOD:29 CNT:30 SP:31 PC:33\n");
}

void hardware_set_ddr(uint8_t a, uint8_t b) {
    ddra = a;
    ddrb = b;
}

void hardware_set_pa(uint8_t val) {
    for (int i = 0; i < 8; i++) {
        uint8_t bit = (val >> i) & 1;
        uint8_t dir = (ddra >> i) & 1;
        if (dir) {
            gpio_put(PA0_PIN + i, bit ? 1 : 0);
        } else {
            gpio_set_dir(PA0_PIN + i, GPIO_IN);
        }
    }
}

void hardware_set_pb(uint8_t val) {
    for (int i = 0; i < 8; i++) {
        uint8_t bit = (val >> i) & 1;
        uint8_t dir = (ddrb >> i) & 1;
        if (dir) {
            gpio_put(PB0_PIN + i, bit ? 1 : 0);
        } else {
            gpio_set_dir(PB0_PIN + i, GPIO_IN);
        }
    }
}

uint8_t hardware_get_pa(void) {
    uint8_t val = 0;
    for (int i = 0; i < 8; i++) {
        val |= (gpio_get(PA0_PIN + i) & 1) << i;
    }
    return val;
}

uint8_t hardware_get_pb(void) {
    uint8_t val = 0;
    for (int i = 0; i < 8; i++) {
        val |= (gpio_get(PB0_PIN + i) & 1) << i;
    }
    return val;
}