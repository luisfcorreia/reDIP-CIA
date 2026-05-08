#include "cia.h"
#include "timer.h"
#include "tod.h"
#include "serial.h"
#include "hardware.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

static cia_registers_t regs;
static cia_inputs_t inputs;
static cia_outputs_t outputs;
static uint8_t prev_phi2 = 0;
static uint8_t phi2_up = 0;
static uint8_t phi2_dn = 0;
static bool ta_int_pending = false;

static uint8_t icr_flags = 0;
static uint8_t icr_mask = 0;
static uint8_t icr7 = 0;
static bool irq = false;
static bool irq_prev = false;

static bool cra_w_prev = false;
static bool crb_w_prev = false;

void cia_init(void) {
    memset(&regs, 0, sizeof(regs));
    memset(&inputs, 0, sizeof(inputs));
    memset(&outputs, 0, sizeof(outputs));
    timer_init();
    tod_init();
    serial_init();
    CIA_DPRINTF("[CIA] initialized\n");
}

void cia_update_inputs(cia_inputs_t *in) {
    memcpy(&inputs, in, sizeof(cia_inputs_t));
}

void cia_get_outputs(cia_outputs_t *out) {
    memcpy(out, &outputs, sizeof(cia_outputs_t));
}

static uint8_t compute_pra(void) {
    return (regs.pra & regs.ddra) | (inputs.pa_in & ~regs.ddra);
}

static uint8_t compute_prb(void) {
    uint8_t prb = (regs.prb & regs.ddrb) | (inputs.pb_in & ~regs.ddrb);
    uint8_t ta_pb, tb_pb;
    timer_get_pb_outputs(&ta_pb, &tb_pb);
    if (regs.cra & 0x02) prb = (prb & 0xBF) | (ta_pb ? 0x40 : 0);
    if (regs.crb & 0x02) prb = (prb & 0x7F) | (tb_pb ? 0x80 : 0);
    return prb;
}

void cia_tick(uint8_t phi2, uint8_t cs_n, uint8_t rw_n, uint8_t addr, uint8_t data_in, cia_inputs_t *inputs_in, cia_outputs_t *outputs_out) {
    cia_update_inputs(inputs_in);

    phi2_up = phi2 && !prev_phi2;
    phi2_dn = !phi2 && prev_phi2;
    prev_phi2 = phi2;

    if (phi2_up) {
        inputs.pa_in = inputs_in->pa_in;
        inputs.pb_in = inputs_in->pb_in;
    }

    if (phi2_dn && !cs_n && !rw_n) {
        cia_write(addr, data_in);
    }

    if (phi2 && !cs_n && rw_n) {
        uint8_t data_out = cia_read(addr);
        outputs.data = data_out;
    } else {
        outputs.data = 0xFF;
    }

    uint8_t cra = regs.cra;
    uint8_t crb = regs.crb;

    bool cnt_up = inputs_in->cnt_rise;

    bool cra_w = !cs_n && !rw_n && (addr == CIA_REG_CRA);
    bool crb_w = !cs_n && !rw_n && (addr == CIA_REG_CRB);

    timer_tick(phi2_dn, inputs_in->cnt, cnt_up, ta_int_pending, cra, regs.talo, regs.tahi, crb, regs.tblo, regs.tbhi, cra_w_prev, crb_w_prev);

    if (phi2_dn) {
        cra_w_prev = cra_w;
        crb_w_prev = crb_w;
    }

    if (phi2_dn) {
        tod_tick(phi2_up, phi2_dn, false, 0, 0, NULL);
    }

    uint8_t timer_irq;
    bool ta_pending, tb_pending;
    timer_check_irq(&timer_irq, &ta_pending, &tb_pending);

    if (ta_pending) ta_int_pending = true;

    bool we_sdr = !cs_n && !rw_n && (addr == CIA_REG_SDR);
    serial_tick(phi2_up, phi2_dn, we_sdr, data_in, ta_int_pending, cnt_up, inputs_in->sp, 0);

    if (phi2_up) {
        serial_set_ctrl(cra);
    }

    bool r_icr = !cs_n && rw_n && (addr == CIA_REG_ICR);
    bool w_icr = !cs_n && !rw_n && (addr == CIA_REG_ICR);

    if (r_icr) {
        icr7 = 0;
        irq = false;
        icr_flags = 0;
    }

    uint8_t int_src = 0;
    uint8_t serial_irq, flag_irq, tod_irq;
    serial_check_irq(&serial_irq);
    flag_irq = inputs_in->flag_edge;
    tod_irq = tod_alarm_match();

    uint8_t sources = 0;
    sources |= (timer_irq ? 0x01 : 0);
    sources |= (tod_irq ? 0x02 : 0);
    sources |= (flag_irq ? 0x04 : 0);
    sources |= (serial_irq ? 0x08 : 0);

    for (int i = 0; i < 5; i++) {
        if (sources & (1 << i)) {
            icr_flags |= (1 << i);
        } else if (r_icr) {
            icr_flags &= ~(1 << i);
        }
    }

    if (w_icr) {
        icr_mask = data_in & 0x1F;
    }

    bool ir_set = (icr_flags & icr_mask) != 0;

    bool ir_clr = r_icr;

    if (ir_set && !ir_clr) {
        icr7 = 1;
        irq = true;
    } else if (ir_clr && !ir_set) {
        icr7 = 0;
        irq = false;
    }

    outputs.irq_n = irq ? 0 : 1;

    outputs.pa_out = compute_pra();
    outputs.pb_out = compute_prb();
    outputs.ddra = regs.ddra;
    outputs.ddrb = regs.ddrb;
    outputs.pc_n = 1;
    outputs.sp = serial_get_sp_out() ? 1 : 0;
    outputs.cnt = serial_get_cnt_out() ? 1 : 0;

    cia_get_outputs(outputs_out);
}

uint8_t cia_read(uint8_t addr) {
    CIA_DPRINTF("[CIA] read reg 0x%02X\n", addr);
    switch (addr) {
        case CIA_REG_PRA: return compute_pra();
        case CIA_REG_PRB: return compute_prb();
        case CIA_REG_DDRA: return regs.ddra;
        case CIA_REG_DDRB: return regs.ddrb;
        case CIA_REG_TALO: return timer_get_lo(TIMER_A);
        case CIA_REG_TAHI: return timer_get_hi(TIMER_A);
        case CIA_REG_TBLO: return timer_get_lo(TIMER_B);
        case CIA_REG_TBHI: return timer_get_hi(TIMER_B);
        case 0x08: case 0x09: case 0x0A: case 0x0B:
            return tod_read(addr);
        case CIA_REG_SDR: return serial_get_data();
        case CIA_REG_ICR: return (icr7 << 7) | icr_flags;
        case CIA_REG_CRA: return regs.cra;
        case CIA_REG_CRB: return regs.crb;
        default: return 0xFF;
    }
}

void cia_write(uint8_t addr, uint8_t data) {
    CIA_DPRINTF("[CIA] write reg 0x%02X = 0x%02X\n", addr, data);
    switch (addr) {
        case CIA_REG_PRA: regs.pra = data; break;
        case CIA_REG_PRB: regs.prb = data; break;
        case CIA_REG_DDRA: regs.ddra = data; break;
        case CIA_REG_DDRB: regs.ddrb = data; break;
        case CIA_REG_TALO: regs.talo = data; timer_set_lo(TIMER_A, data); break;
        case CIA_REG_TAHI: regs.tahi = data; timer_set_hi(TIMER_A, data); break;
        case CIA_REG_TBLO: regs.tblo = data; timer_set_lo(TIMER_B, data); break;
        case CIA_REG_TBHI: regs.tbhi = data; timer_set_hi(TIMER_B, data); break;
        case 0x08: case 0x09: case 0x0A: case 0x0B:
            tod_write(addr, data); break;
        case CIA_REG_SDR: serial_set_data(data); break;
        case CIA_REG_ICR: regs.icr = data & 0x7F; break;
        case CIA_REG_CRA: regs.cra = data; timer_set_ctrl(TIMER_A, data); break;
        case CIA_REG_CRB: regs.crb = data; timer_set_ctrl(TIMER_B, data); break;
    }
}