#ifndef TOD_H
#define TOD_H

#include <stdint.h>
#include <stdbool.h>

void tod_init(void);
void tod_tick(bool phi2_edge, bool write_strobe, uint8_t addr, uint8_t data, uint8_t *data_out);
uint8_t tod_read(uint8_t addr);
void tod_write(uint8_t addr, uint8_t val);
bool tod_alarm_match(void);
void tod_alarm_clear(void);

#endif