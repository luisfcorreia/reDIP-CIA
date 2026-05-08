#include "timer.h"
#include <stdio.h>
#include <stdbool.h>

typedef struct {
    uint16_t prescaler;
    uint16_t counter;
    uint8_t ctrl;
    uint8_t hi_w_prev;
    uint8_t start_prev;
    uint8_t reload_prev;
    uint8_t count_prev;
    uint8_t intr;
    uint8_t toggle;
    uint8_t toggle_prev;
} timer_state_t;

static timer_state_t timer_a, timer_b;

void timer_init(void) {
    timer_a.prescaler = 0xFFFF;
    timer_a.counter = 0xFFFF;
    timer_a.ctrl = 0;
    timer_b.prescaler = 0xFFFF;
    timer_b.counter = 0xFFFF;
    timer_b.ctrl = 0;
}

void timer_set_lo(timer_id_t id, uint8_t val) {
    timer_state_t *t = (id == TIMER_A) ? &timer_a : &timer_b;
    t->prescaler = (t->prescaler & 0xFF00) | val;
}

void timer_set_hi(timer_id_t id, uint8_t val) {
    timer_state_t *t = (id == TIMER_A) ? &timer_a : &timer_b;
    t->prescaler = (t->prescaler & 0x00FF) | (val << 8);
}

void timer_set_ctrl(timer_id_t id, uint8_t ctrl) {
    timer_state_t *t = (id == TIMER_A) ? &timer_a : &timer_b;
    t->ctrl = ctrl;
}

uint8_t timer_get_lo(timer_id_t id) {
    timer_state_t *t = (id == TIMER_A) ? &timer_a : &timer_b;
    return t->counter & 0xFF;
}

uint8_t timer_get_hi(timer_id_t id) {
    timer_state_t *t = (id == TIMER_A) ? &timer_a : &timer_b;
    return (t->counter >> 8) & 0xFF;
}

void timer_tick(bool phi2_dn, bool cnt, bool cnt_up, bool ta_int,
                uint8_t cra, uint8_t talo, uint8_t tahi,
                uint8_t crb, uint8_t tblo, uint8_t tbhi) {
    if (!phi2_dn) return;

    timer_state_t *ta = &timer_a;
    timer_state_t *tb = &timer_b;

    uint8_t force_load_a = (cra >> 2) & 1;
    uint8_t start_a = (cra >> 0) & 1;
    uint8_t count_mode_a = (cra >> 4) & 1;
    uint8_t toggle_a = (cra >> 1) & 1;

    uint8_t force_load_b = (crb >> 2) & 1;
    uint8_t start_b = (crb >> 0) & 1;
    uint8_t count_mode_b = (crb >> 4) & 1;
    uint8_t toggle_b = (crb >> 1) & 1;
    uint8_t inmode_b = (crb >> 4) & 3;

    bool count_a = false;
    if (start_a) {
        if (count_mode_a) {
            count_a = cnt_up;
        } else {
            count_a = phi2_dn;
        }
    }

    bool count_b = false;
    if (start_b) {
        switch (inmode_b) {
            case 0:
                count_b = phi2_dn;
                break;
            case 1:
                count_b = cnt_up;
                break;
            case 2:
                count_b = ta_int;
                break;
            case 3:
                count_b = ta_int && cnt;
                break;
        }
    }

    uint16_t prescaler_a = ta->prescaler;
    if (count_a && ta->counter == 0) {
        uint8_t ufl = (ta->reload_prev ? (prescaler_a != 0) : (ta->counter != 0));
        if (ufl) {
            uint8_t reload = ufl || force_load_a || (ta->hi_w_prev && !start_a);
            if (reload) ta->counter = prescaler_a;
            if (count_a) ta->intr = 1;
        }
    } else if (count_a) {
        ta->counter--;
    }

    uint16_t prescaler_b = tb->prescaler;
    if (count_b && tb->counter == 0) {
        uint8_t ufl = (tb->reload_prev ? (prescaler_b != 0) : (tb->counter != 0));
        if (ufl) {
            uint8_t reload = ufl || force_load_b || (tb->hi_w_prev && !start_b);
            if (reload) tb->counter = prescaler_b;
            if (count_b) tb->intr = 1;
        }
    } else if (count_b) {
        tb->counter--;
    }

    if (toggle_a && ta->intr) {
        ta->toggle = !ta->toggle;
    }
    if (toggle_b && tb->intr) {
        tb->toggle = !tb->toggle;
    }

    ta->hi_w_prev = 1;
    tb->hi_w_prev = 1;
    ta->start_prev = start_a;
    tb->start_prev = start_b;
    ta->reload_prev = force_load_a;
    tb->reload_prev = force_load_b;
}

void timer_check_irq(uint8_t *irq, bool *ta_pending, bool *tb_pending) {
    *ta_pending = timer_a.intr;
    *tb_pending = timer_b.intr;
    *irq = timer_a.intr || timer_b.intr;
}

void timer_get_pb_outputs(uint8_t *ta_pb, uint8_t *tb_pb) {
    *ta_pb = (timer_a.ctrl & 0x02) ? timer_a.toggle : timer_a.intr;
    *tb_pb = (timer_b.ctrl & 0x02) ? timer_b.toggle : timer_b.intr;
}