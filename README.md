# LVGL V8.3 ported to the GD32F470I EVAL

This project ports `LVGL V8.3.11` to the `GD32F470I EVAL` platform for GUI demonstrations.

## Hardware Information

The `GD32F470I Development Kit` is based on:

- `GD32F470IK` microcontroller (`ARM Cortex-M4` core, FPU, DSP instructions)
- `3 MB` on-chip Flash memory and `256 KB` on-chip SRAM (including `192 KB` SRAM0 and `64 KB` TCM SRAM)
- External `32 MB` SDRAM for graphics framebuffers and application data
- `480 x 272` RGB TFT display with resistive/capacitive touch panel
- SD card and SPI Flash (`GD25Qxx`) storage support through the board support package
- On-board high-precision RTC clock and temperature sensor
- Multiple user LEDs, function keys, and universal expansion pin headers

## Project Information

- GUI framework: `LVGL V8.3.11`
- RTOS: `FreeRTOS V10.4.1`
- Toolchain: `Keil MDK-ARM (ARM Compiler 6) / IAR / GD32EmbeddedBuilder`
- Target board: `GD32F470I EVAL`
- Display configuration: `480 x 272 / RGB565 16-bit color / landscape`

## Third-Party Components

| Category   | In use | Component  | Version     | License         |
| ---------- | ------ | ---------- | ----------- | --------------- |
| GUI        | `Yes`  | `LVGL`     | `V 8.3.11`  | `MIT`           |
| Filesystem | `Yes`  | `FatFs`    | `R0.16`     | `FatFs license` |
| RTOS       | `Yes`  | `FreeRTOS` | `V 10.4.1`  | `MIT`           |

> When adding a new third-party library, update this table accordingly and retain its license text and copyright notices.
