#include "tod.h"
#include <stdio.h>
#include <stdbool.h>

static uint8_t tod_10ths = 0;
static uint8_t tod_sec = 0;
static uint8_t tod_min = 0;
static uint8_t tod_hour = 0;

static uint8_t alarm_10ths = 0;
static uint8_t alarm_sec = 0;
static uint8_t alarm_min = 0;
static uint8_t alarm_hour = 0;

static uint8_t latch_10ths = 0;
static uint8_t latch_sec = 0;
static uint8_t latch_min = 0;
static uint8_t latch_hour = 0;
static bool latched = false;

static bool alarm_enabled = false;
static bool alarm_flag = false;
static bool alarm_eq = false;
static bool alarm_eq_next = false;
static bool alarm_eq_prev = false;

static bool phi20 = false;
static bool phi20_prev = false;
static bool phi20_up = false;
static bool phi20_dn = false;
static bool tod_sample = true;
static bool tod_start = false;

static uint8_t jc2 = 0;

static uint8_t bcd_inc(uint8_t val, uint8_t max) {
    uint8_t hi = (val >> 4) & 0x0F;
    uint8_t lo = val & 0x0F;
    lo++;
    if (lo > 9) {
        lo = 0;
        hi++;
        if (hi > (max / 10)) hi = 0;
    }
    return (hi << 4) | lo;
}

void tod_init(void) {
    tod_10ths = 0;
    tod_sec = 0;
    tod_min = 0;
    tod_hour = 0x12;  // CIA bug: Powerup to 01:00:00.0 (BCD: 0x12 = 1 hour, 2 BCD digit)
    alarm_10ths = alarm_sec = alarm_min = alarm_hour = 0;
    latch_10ths = latch_sec = latch_min = latch_hour = 0;
    latched = false;
    alarm_enabled = false;
    alarm_flag = false;
    alarm_eq = alarm_eq_next = alarm_eq_prev = false;
    phi20 = phi20_prev = false;
}

uint8_t tod_read(uint8_t addr) {
    switch (addr & 0x0F) {
        case 0x08:
            return tod_sample ? tod_10ths : latch_10ths;
        case 0x09:
            return tod_sample ? tod_sec : latch_sec;
        case 0x0A:
            return tod_sample ? tod_min : latch_min;
        case 0x0B:
            return tod_sample ? tod_hour : latch_hour;
        case 0x0C: return alarm_10ths;
        case 0x0D: return alarm_sec;
        case 0x0E: return alarm_min;
        case 0x0F: return (alarm_enabled ? 0x80 : 0x00) | (alarm_flag ? 0x40 : 0x00);
        default: return 0xFF;
    }
}

void tod_write(uint8_t addr, uint8_t val) {
    switch (addr & 0x0F) {
        case 0x08:
            tod_10ths = val & 0x0F;
            latched = false;
            break;
        case 0x09: tod_sec = val; break;
        case 0x0A: tod_min = val; break;
        case 0x0B:
            tod_hour = val & 0x9F;
            alarm_flag = false;
            break;
        case 0x0C: alarm_10ths = val & 0x0F; break;
        case 0x0D: alarm_sec = val; break;
        case 0x0E: alarm_min = val; break;
        case 0x0F:
            alarm_enabled = (val & 0x80) != 0;
            alarm_hour = val & 0x9F;
            break;
    }
}

void tod_tick(bool phi2, bool phi2_up, bool phi2_dn, bool write_strobe, uint8_t addr, uint8_t data, uint8_t *data_out) {
    static bool we_10ths_prev = false;
    static bool rd_10ths_prev = false;
    static bool tod_start_state = false;
    static bool tod_sample_state = true;
    static bool res = false;

    if (phi2_dn) {
        jc2 = ((jc2 & 1) << 1) | ((~jc2 >> 1) & 1);
    }

    phi20_prev = phi20;
    phi20 = (jc2 == 0) && phi2;
    phi20_up = phi20 && phi2_up;
    phi20_dn = phi20_prev && phi2_dn;

    if (phi20_up) {
        alarm_eq_next = (tod_10ths == alarm_10ths && tod_sec == alarm_sec &&
                        tod_min == alarm_min && tod_hour == alarm_hour);
    }

    if (phi20_dn) {
        alarm_eq = alarm_eq_next;
    }

    if (phi2_dn) {
        bool alarm_int = !alarm_eq_prev && alarm_eq;
        if (alarm_int && alarm_enabled) {
            alarm_flag = true;
        }
        alarm_eq_prev = alarm_eq;

        we_10ths_prev = write_strobe && ((addr & 0x0F) == 0x08);
        rd_10ths_prev = false;
        tod_start_state = tod_start;
        tod_sample_state = tod_sample;
        res = false;
    }

    bool rd_hr = false;
    bool we_hr = false;

    if (write_strobe && (addr & 0x0F) >= 0x08 && (addr & 0x0F) <= 0x0F) {
        tod_write(addr, data);
        if ((addr & 0x0F) == 0x0B) {
            we_hr = true;
            tod_start = false;
        }
    }

    if (rd_hr) {
        tod_sample = false;
    } else if (rd_10ths_prev || res) {
        tod_sample = true;
    }

    if (we_hr || res) {
        tod_start = false;
    } else if (we_10ths_prev) {
        tod_start = true;
    }

    if (phi20_up && tod_sample) {
        latch_10ths = tod_10ths;
        latch_sec = tod_sec;
        latch_min = tod_min;
        latch_hour = tod_hour;
    }

    if (phi20 && tod_start) {
        tod_10ths++;
        if (tod_10ths > 9) {
            tod_10ths = 0;
            tod_sec = bcd_inc(tod_sec, 59);
            if (tod_sec == 0) {
                tod_min = bcd_inc(tod_min, 59);
                if (tod_min == 0) {
                    tod_hour = bcd_inc(tod_hour, 23);
                }
            }
        }
    }

    if (data_out && (addr & 0x0F) >= 0x08 && (addr & 0x0F) <= 0x0F) {
        *data_out = tod_read(addr);
    }
}

bool tod_alarm_match(void) {
    return alarm_flag;
}

void tod_alarm_clear(void) {
    alarm_flag = false;
}