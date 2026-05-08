#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    TIMER_A,
    TIMER_B
} timer_id_t;

void timer_init(void);
void timer_tick(bool phi2_dn, bool cnt, bool cnt_up, bool ta_int,
                uint8_t cra, uint8_t talo, uint8_t tahi,
                uint8_t crb, uint8_t tblo, uint8_t tbhi,
                bool cra_w_prev, bool crb_w_prev);
void timer_set_lo(timer_id_t id, uint8_t val);
void timer_set_hi(timer_id_t id, uint8_t val);
void timer_set_ctrl(timer_id_t id, uint8_t ctrl);
void timer_check_irq(uint8_t *irq, bool *ta_pending, bool *tb_pending);
void timer_get_pb_outputs(uint8_t *ta_pb, uint8_t *tb_pb);
uint8_t timer_get_lo(timer_id_t id);
uint8_t timer_get_hi(timer_id_t id);

#endif