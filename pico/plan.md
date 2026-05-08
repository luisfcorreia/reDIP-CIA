# reDIP-CIA RP2350 Implementation Plan

## Current State
- Build compiles successfully (`redip_cia.uf2`)
- CIA core logic is implemented (cia.c)
- Timer logic is implemented (timer.c)
- PIO programs exist but are stubs
- hardware.c is placeholder
- TOD and serial are minimal stubs

## Implementation Phases

### Phase 1: PIO-CPU Integration (main.c) - DONE
- Poll FIFO buffers in main loop
- Extract bus cycles: PHI2, CS, R/W, ADDR, DATA
- Pass to CIA tick function
- Write CIA output data back to ports PIO

### Phase 2: PIO Program Enhancement - DONE

**bus.pio:**
- Full address capture: R/W + ADDR[0-5] on rising edge
- Data capture: DATA[0-7] on falling edge
- Two-word FIFO push per PHI2 cycle

**ports.pio:**
- Input sampling: PA/PB on PHI2 rising edge
- Output via TX FIFO support
- Open-drain mode via CPU GPIO control

**timer_pio.pio:**
- Timer A/B output on GPIO29/30
- Toggle mode support
- TX FIFO for timer state updates

### Phase 3: Hardware Pin Mapping (hardware.c) - DONE
- Full pin mapping for RP2350B (48 GPIO):
  - PA0-7 / D0-7: GPIO0-7 (shared, PIO-controlled)
  - PB0-7: GPIO8-15
  - RS0-3: GPIO16-19
  - PHI2: GPIO24
  - CS: GPIO25
  - R/W: GPIO26
  - FLAG: GPIO27
  - IRQ: GPIO28
  - TOD: GPIO29
  - CNT: GPIO30
  - SP: GPIO31
- Open-drain logic in hardware_set_pa/pb
- Direction register support (ddra/ddrb)

### Phase 3b: PIO Bus Controller - DONE
- SM0 (bus_capture): Captures DATA on PHI2 falling edge → RX FIFO
- SM1 (bus_output): Drives data bus during read cycles ← TX FIFO
- CPU polls CS, R/W, RS via GPIO
- Full bus cycle handling with PIO assist

### Phase 4: TOD Implementation (tod.c) - DONE
- 24-hour BCD counter (10ths, sec, min, hour)
- Alarm matching with IRQ support
- Latch on read (reading 0x08 latches all)
- PHI2 edge tick for 50/60Hz clock input
- Integrated with CIA read/write

### Phase 5: Serial Implementation (serial.c) - DONE
- Full Verilog-compatible serial module (cia_serial.sv)
- 8-bit shift register with Johnson counter
- Two modes: txmode=0 (CNT external clock), txmode=1 (Timer A underflow)
- SDR write triggers transmission (timer mode)
- SDR read returns ~sr (inverted!)
- SP pin: input (external mode) / output (timer mode)
- CNT pin: input clock / output toggle (timer mode)
- sp_int interrupt on 8-bit transfer complete
- Integrated with CIA tick, CRA control, timer A IRQ

### Phase 6: Timer CNT Mode - DONE
- Timer A: CRA bit 4 (inmode) selects PHI2 or CNT counting
- Timer B: CRB bits 5-4 (inmode) select PHI2/CNT/TA underflow modes
- Proper cnt_up edge detection passed to timer_tick

### Phase 7: /PC Signal - DONE
- Shift register tracks PRB read/write (addr 0x01)
- Active low pulse on PRB access
- Output on GPIO33

### Phase 8: FLAG Edge Detection - DONE
- Detects falling edge on FLAG pin
- Uses edge (not level) for ICR bit 2 interrupt

### Phase 9: Timer IRQ Persistence - DONE
- Timer interrupts persist until ICR read (SR latch behavior)
- ta_int_pending tracks Timer A interrupt for serial clock

---

## Files to Modify

| File | Current Status | Work Required |
|------|----------------|----------------|
| main.c | Stub loop | FIFO polling + data routing |
| bus.pio | 4-bit addr | 8-bit addr + full bus capture |
| ports.pio | Basic in | Open-drain + output support |
| timer_pio.pio | Basic out | Toggle/one-shot handling |
| hardware.c | Placeholder | Pin definitions + I/O |
| tod.c | Stub | 24h clock + alarm |
| serial.c | Stub | Shift register |