#ifndef CIA_H
#define CIA_H

#include <stdint.h>

#define CIA_DEBUG 1

#if CIA_DEBUG
#define CIA_DPRINTF(...) printf(__VA_ARGS__)
#else
#define CIA_DPRINTF(...) ((void)0)
#endif

typedef enum {
    CIA_REG_PRA = 0x00,
    CIA_REG_PRB = 0x01,
    CIA_REG_DDRA = 0x02,
    CIA_REG_DDRB = 0x03,
    CIA_REG_TALO = 0x04,
    CIA_REG_TAHI = 0x05,
    CIA_REG_TBLO = 0x06,
    CIA_REG_TBHI = 0x07,
    CIA_REG_TODLO = 0x08,
    CIA_REG_TODMI = 0x09,
    CIA_REG_TODHI = 0x0A,
    CIA_REG_SDR = 0x0C,
    CIA_REG_ICR = 0x0D,
    CIA_REG_CRA = 0x0E,
    CIA_REG_CRB = 0x0F
} cia_reg_t;

typedef struct {
    uint8_t pra;
    uint8_t prb;
    uint8_t ddra;
    uint8_t ddrb;
    uint8_t talo;
    uint8_t tahi;
    uint8_t tblo;
    uint8_t tbhi;
    uint8_t todlo;
    uint8_t todmi;
    uint8_t todhi;
    uint8_t sdr;
    uint8_t icr;
    uint8_t cra;
    uint8_t crb;
} cia_registers_t;

typedef struct {
    uint8_t pa_in;
    uint8_t pb_in;
    uint8_t flag_n;
    uint8_t flag_edge;
    uint8_t cnt;
    uint8_t tod;
    uint8_t sp;
} cia_inputs_t;

typedef struct {
    uint8_t pa_out;
    uint8_t pb_out;
    uint8_t ddra;
    uint8_t ddrb;
    uint8_t irq_n;
    uint8_t pc_n;
    uint8_t sp;
    uint8_t cnt;
    uint8_t data;
} cia_outputs_t;

void cia_init(void);
void cia_tick(uint8_t phi2, uint8_t cs_n, uint8_t rw_n, uint8_t addr, uint8_t data_in, cia_inputs_t *inputs, cia_outputs_t *outputs);
uint8_t cia_read(uint8_t addr);
void cia_write(uint8_t addr, uint8_t data);
void cia_update_inputs(cia_inputs_t *inputs);
void cia_get_outputs(cia_outputs_t *outputs);

#endif