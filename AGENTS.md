# reDIP-CIA Agent Guide

## Project Type
Dual-platform MOS 6526/8521 CIA emulator:
- **FPGA**: Lattice iCE5LP1K (SystemVerilog)
- **Microcontroller**: Raspberry Pi RP2350 (PICO + C)

## Build Commands

### FPGA (Gateware)
```bash
# Build default MOS 8521 emulation
make

# Build MOS 6526 variant instead
make MOS6526=1

# Lint with verilator
make lint

# Lint with slang
make lint-slang

# Run simulation (requires verilator)
make sim

# Program FPGA hardware (requires FTDI cable)
make prog

# Clean build artifacts
make clean
```

### RP2350 (Pico)
```bash
# Build the firmware
cd pico/build && make

# Flash to device (pico2 or RP2350)
cp pico/build/redip_cia.uf2 /media/$USER/RPI-RP2/
```

## Key Details
- **FPGA Toolchain**: yosys + nextpnr-ice40 + icepack + iceprog
- **FPGA Simulation**: verilator (requires verilator with C++20)
- **Pico Toolchain**: arm-none-eabi-gcc + pico-sdk
- **FPGA Output**: `redip_cia.bin` for programming, `redip_cia.asc` for timing analysis
- **Pico Output**: `redip_cia.uf2` for drag-and-drop flash
- **PCF files**: `redip_cia.pcf` (default), `redip_via.pcf`, `redip_pia.pcf` for different chip variants

## Directory Structure
- `gateware/` - All SystemVerilog source and build system
- `pico/` - RP2350 firmware (PICO + C)
- `hardware/` - PCB design files

## Lint Order
Run `make lint` before committing. Use `make lint-slang` as alternative linter.