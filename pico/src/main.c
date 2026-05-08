#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "cia.h"
#include "hardware.h"

#include "bus_capture.pio.h"
#include "bus_output.pio.h"
#include "timer_pio.pio.h"

#define BUS_PIO    pio0
#define CAPTURE_SM 0
#define OUTPUT_SM  1

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

    cia_init();
    printf("CIA initialized\n");

    printf("Entering main loop...\n");

    uint8_t prev_phi2 = 0;
    uint8_t data_pending = 0;
    uint8_t pending_data = 0;

    bool flag_prev = true;
    uint8_t prb_rw = 0;

    while (true) {
        uint8_t phi2 = gpio_get(PHI2_PIN);
        uint8_t phi2_up = !prev_phi2 && phi2;
        uint8_t phi2_dn = prev_phi2 && !phi2;
        prev_phi2 = phi2;

        uint8_t cs_n = gpio_get(CS_PIN);
        uint8_t rw_n = gpio_get(RW_PIN);
        uint8_t rs = 0;
        for (int i = 0; i < 4; i++) {
            rs |= (gpio_get(RS0_PIN + i) & 1) << i;
        }

        bool flag_n = gpio_get(FLAG_PIN);
        bool flag_edge = flag_prev && !flag_n;
        flag_prev = flag_n;

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