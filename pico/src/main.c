#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "cia.h"
#include "hardware.h"

#include "bus_capture.pio.h"
#include "bus_output.pio.h"
#include "timer_pio.pio.h"
#include "phi2_edges.pio.h"
#include "flag_edges.pio.h"
#include "cnt_edges.pio.h"
#include "rs_edges.pio.h"

#define BUS_PIO     pio0
#define CAPTURE_SM  0
#define OUTPUT_SM   1
#define PHI2_SM     2
#define FLAG_SM     3

#define TIMER_PIO   pio1
#define CNT_SM      1
#define RS_SM       2

int main() {
    stdio_init_all();
    printf("\n=== reDIP-CIA RP2350 Build ===\n");
    printf("Clock: 250MHz\n");

    hardware_init();

    uint capture_offset = pio_add_program(BUS_PIO, &bus_capture_program);
    bus_capture_program_init(BUS_PIO, CAPTURE_SM, capture_offset);
    printf("BUS CAPTURE PIO loaded\n");

    uint output_offset = pio_add_program(BUS_PIO, &bus_output_program);
    bus_output_program_init(BUS_PIO, OUTPUT_SM, output_offset);
    printf("BUS OUTPUT PIO loaded\n");

    uint phi2_offset = pio_add_program(BUS_PIO, &phi2_edges_program);
    phi2_edges_program_init(BUS_PIO, PHI2_SM, phi2_offset);
    printf("PHI2 EDGES PIO loaded\n");

    uint flag_offset = pio_add_program(BUS_PIO, &flag_edges_program);
    flag_edges_program_init(BUS_PIO, FLAG_SM, flag_offset);
    printf("FLAG EDGES PIO loaded\n");

    uint cnt_offset = pio_add_program(TIMER_PIO, &cnt_edges_program);
    cnt_edges_program_init(TIMER_PIO, CNT_SM, cnt_offset);
    printf("CNT EDGES PIO loaded\n");

    uint rs_offset = pio_add_program(TIMER_PIO, &rs_edges_program);
    rs_edges_program_init(TIMER_PIO, RS_SM, rs_offset);
    printf("RS EDGES PIO loaded\n");

    cia_init();
    printf("CIA initialized\n");

    printf("Entering main loop...\n");

    uint8_t data_pending = 0;
    uint8_t pending_data = 0;

    bool phi2_up = false;
    bool phi2_dn = false;
    bool flag_edge = false;
    bool cnt_rise = false;
    bool cnt_fall = false;
    bool rs_changed = false;
    uint8_t rs = 0;
    uint8_t phi2 = 0;

    uint8_t prb_rw = 0;

    while (true) {
        phi2_up = false;
        phi2_dn = false;
        flag_edge = false;
        cnt_rise = false;
        cnt_fall = false;
        rs_changed = false;

        while (!pio_sm_is_rx_fifo_empty(BUS_PIO, PHI2_SM)) {
            uint32_t edge = pio_sm_get(BUS_PIO, PHI2_SM);
            if (edge & 1) phi2_up = true;
            else phi2_dn = true;
        }

        while (!pio_sm_is_rx_fifo_empty(BUS_PIO, FLAG_SM)) {
            pio_sm_get(BUS_PIO, FLAG_SM);
            flag_edge = true;
        }

        while (!pio_sm_is_rx_fifo_empty(TIMER_PIO, CNT_SM)) {
            uint32_t edge = pio_sm_get(TIMER_PIO, CNT_SM);
            if (edge & 1) cnt_rise = true;
            else cnt_fall = true;
        }

        while (!pio_sm_is_rx_fifo_empty(TIMER_PIO, RS_SM)) {
            rs = pio_sm_get(TIMER_PIO, RS_SM) & 0xF;
            rs_changed = true;
        }

        uint8_t cs_n = gpio_get(CS_PIN);
        uint8_t rw_n = gpio_get(RW_PIN);
        if (!rs_changed) {
            rs = 0;
            for (int i = 0; i < 4; i++) {
                rs |= (gpio_get(RS0_PIN + i) & 1) << i;
            }
        }

        phi2 = gpio_get(PHI2_PIN);
        bool flag_n = gpio_get(FLAG_PIN);

        prb_rw = (prb_rw << 1) | ((!cs_n && rs == CIA_REG_PRB) ? 1 : 0);
        bool pc_n = !(prb_rw & 1) && (prb_rw & 4);
        gpio_put(PC_PIN, pc_n ? 0 : 1);

        if (!pio_sm_is_rx_fifo_empty(BUS_PIO, CAPTURE_SM)) {
            uint32_t data_word = pio_sm_get(BUS_PIO, CAPTURE_SM);
            pending_data = data_word & 0xFF;
            data_pending = 1;
        }

        if (phi2_up && !cs_n) {
            if (!rw_n) {
                if (data_pending) {
                    cia_write(rs, pending_data);
                    data_pending = 0;
                }
            } else {
                uint8_t read_data = cia_read(rs);
                while (pio_sm_is_tx_fifo_full(BUS_PIO, OUTPUT_SM)) {
                    tight_loop_contents();
                }
                pio_sm_put(BUS_PIO, OUTPUT_SM, read_data);
            }
        }

        if (phi2_dn && cs_n) {
            data_pending = 0;
        }

        cia_inputs_t inputs = {
            .pa_in = hardware_get_pa(),
            .pb_in = hardware_get_pb(),
            .flag_n = flag_n ? 1 : 0,
            .flag_edge = flag_edge ? 1 : 0,
            .cnt = gpio_get(CNT_PIN),
            .cnt_rise = cnt_rise ? 1 : 0,
            .cnt_fall = cnt_fall ? 1 : 0,
            .tod = gpio_get(TOD_PIN),
            .sp = gpio_get(SP_PIN)
        };

        cia_outputs_t outputs;
        cia_tick(phi2, cs_n, rw_n, rs, data_pending ? pending_data : 0, &inputs, &outputs);

        hardware_set_ddr(outputs.ddra, outputs.ddrb);
        hardware_set_pa(outputs.pa_out);
        hardware_set_pb(outputs.pb_out);

        gpio_put(IRQ_PIN, outputs.irq_n);

        gpio_put(TIMER_A_PIN, (outputs.pb_out >> 6) & 1);
        gpio_put(TIMER_B_PIN, (outputs.pb_out >> 7) & 1);
    }

    return 0;
}