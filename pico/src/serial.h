#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>
#include <stdbool.h>

void serial_init(void);
void serial_set_ctrl(uint8_t cra);
void serial_tick(bool phi2_up, bool phi2_dn, bool we_sdr, uint8_t data, bool ta_int, bool cnt_up, bool sp_in, bool res);
void serial_set_data(uint8_t data);
uint8_t serial_get_data(void);
void serial_check_irq(uint8_t *irq);
bool serial_get_cnt_out(void);
bool serial_get_sp_out(void);

#endif