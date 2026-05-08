#include "serial.h"
#include "hardware.h"
#include <stdint.h>
#include <stdbool.h>

static uint8_t sdr;
static uint8_t sr;
static bool sdr_to_sr;
static bool sr_to_sdr;
static bool txmode;
static bool txmode_prev;
static bool tx_init;
static bool tx_init_prev;
static bool tx_active;
static bool tx_active_prev;
static bool tx_osc_out;
static bool tx_osc_out_prev;
static bool tx_osc_in;
static bool tx_osc_in_prev;
static bool sp_res;
static bool sp_int;
static uint8_t sr_cnt;
static uint8_t sr_cnt_out;
static bool sr_clk;
static bool sr_clk_prev;
static bool sr_cnt_shift;
static bool sr_done;
static bool sr_done_prev;
static bool phi2_up_prev;
static bool we_sdr_prev;
static bool tx_sp;
static bool tx_sp_next;
static bool tx_cnt;
static bool cnt_out_local;

void serial_init(void) {
    sdr = 0;
    sr = 0;
    sdr_to_sr = false;
    sr_to_sdr = false;
    txmode = false;
    txmode_prev = false;
    tx_init = false;
    tx_init_prev = false;
    tx_active = false;
    tx_active_prev = false;
    tx_osc_out = false;
    tx_osc_out_prev = false;
    tx_osc_in = false;
    tx_osc_in_prev = false;
    sp_res = false;
    sp_int = false;
    sr_cnt = 0;
    sr_cnt_out = 0;
    sr_clk = false;
    sr_clk_prev = false;
    sr_cnt_shift = false;
    sr_done = true;
    sr_done_prev = true;
    phi2_up_prev = false;
}

void serial_set_ctrl(uint8_t cra) {
    txmode = (cra & 0x40) != 0;
}

void serial_set_data(uint8_t data) {
    sdr = data;
}

uint8_t serial_get_data(void) {
    return sdr;
}

void serial_tick(bool phi2_up, bool phi2_dn, bool we_sdr, uint8_t data, bool ta_int, bool cnt_up, bool sp_in, bool res) {
    bool rx_clk, tx_clk;
    bool tx_sp_next;
    bool tx_cnt;

    rx_clk = !txmode && cnt_up;
    tx_clk = txmode && (!tx_osc_out_prev && tx_osc_out);

    sp_res = res || (txmode_prev ^ txmode);

    sp_int = !sr_done_prev && sr_done;

    cnt_out_local = (tx_cnt || !txmode) ? 1 : 0;

    if (we_sdr_prev && txmode) {
        tx_init = 1;
    } else if (sp_res || sdr_to_sr) {
        tx_init = 0;
    } else {
        tx_init = tx_init_prev;
    }

    if (sp_res || (!tx_init && sp_int)) {
        tx_active = 0;
    } else if (sdr_to_sr) {
        tx_active = 1;
    } else {
        tx_active = tx_active_prev;
    }

    cnt_out_local = (tx_cnt || !txmode) ? 1 : 0;

    we_sdr_prev = we_sdr;
    txmode_prev = txmode;

    if (phi2_dn) {
        tx_osc_in_prev = tx_osc_in;
        tx_osc_in = ta_int && (tx_active || tx_osc_out);

        tx_cnt = !tx_osc_out_prev;

        tx_sp = tx_sp_next;

        sr_clk_prev = sr_clk;
        sr_clk = rx_clk || tx_clk;
        sdr_to_sr = tx_init && (!tx_active || sp_int);
    }

    if (phi2_up) {
        tx_init_prev = tx_init;
        tx_active_prev = tx_active;

        if (sdr_to_sr && !sr_clk) {
            sr = sdr;
        } else if (sr_clk && !sdr_to_sr) {
            sr = (sr << 1) | (!(sp_in || txmode) ? 1 : 0);
        }

        tx_osc_out_prev = tx_osc_out;
        if (sp_res) {
            tx_osc_out = 0;
        } else if (!tx_osc_in_prev && tx_osc_in) {
            tx_osc_out = !tx_osc_out;
        }

        sr_to_sdr = !txmode && sp_int;

        if (!txmode) {
            tx_sp_next = 0;
        } else if (sr_clk) {
            tx_sp_next = (sr >> 7) & 1;
        }
    }

    if (phi2_up) {
        sr_cnt_shift = sr_clk;
    }

    if (sr_cnt_shift) {
        if (sp_res) {
            sr_cnt = 1;
            sr_cnt_out = 0;
        } else {
            uint8_t new_bit = !(sr_cnt_out >> 3) & 1;
            sr_cnt = (sr_cnt_out << 1) | new_bit;
            sr_cnt_out = (sr_cnt_out << 1) | new_bit;
        }
    } else {
        if (!sr_clk && sp_res) {
            sr_cnt = 0;
            sr_cnt_out = 0;
        } else {
            sr_cnt_out = sr_cnt;
        }
    }

    phi2_up_prev = phi2_up;

    if (phi2_up_prev) {
        sr_done_prev = sr_done;
        sr_done = (sr_cnt_out == 0) || sp_res;
    }

    if (phi2_dn && we_sdr) {
        sdr = data;
    } else if (sr_to_sdr) {
        sdr = ~sr;
    }
}

void serial_check_irq(uint8_t *irq) {
    *irq = sp_int ? 1 : 0;
}

bool serial_get_cnt_out(void) {
    return cnt_out_local;
}

bool serial_get_sp_out(void) {
    return tx_sp | !txmode;
}