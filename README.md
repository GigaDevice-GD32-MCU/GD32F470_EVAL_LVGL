# LVGL Ported to GD32F470I EVAL

## Overview

This project ports `LVGL` to the `GD32F470I EVAL` evaluation board for GUI demonstrations.

The board features a `GD32F470IK` microcontroller based on the `ARM Cortex-M4` core,
along with a `480 x 272` RGB TFT display and a resistive touch panel.
The project uses landscape orientation with the `RGB565` pixel format and 16-bit color depth.

Project toolchains include `Keil MDK-ARM (ARM Compiler 6)`, `IAR`, and `GD32EmbeddedBuilder`.

## Benchmark

The following benchmark results were obtained using LVGL 9.6.0 with IPA acceleration and double buffering.

| Name                      | Avg. CPU | Avg. FPS | Avg. time | render time | flush time |
| :------------------------ | -------: | -------: | --------: | ----------: | ---------: |
| Empty screen              | 25%      | 28       | 8         | 8           | <1         |
| Moving wallpaper          | 86%      | 25       | 30        | 29          | 1          |
| Single rectangle          | 16%      | 28       | 1         | 1           | <1         |
| Multiple rectangles       | 21%      | 27       | 6         | 6           | <1         |
| Multiple RGB images       | 19%      | 28       | 5         | 5           | <1         |
| Multiple ARGB images      | 29%      | 29       | 8         | 8           | <1         |
| Rotated ARGB images       | 84%      | 15       | 54        | 52          | 2          |
| Multiple labels           | 41%      | 29       | 9         | 9           | <1         |
| Screen sized text         | 93%      | 12       | 72        | 70          | 2          |
| Multiple arcs             | 15%      | 27       | 3         | 3           | <1         |
| Containers                | 27%      | 27       | 8         | 8           | <1         |
| Containers with overlay   | 86%      | 22       | 37        | 32          | 5          |
| Containers with opa       | 52%      | 28       | 16        | 16          | <1         |
| Containers with opa_layer | 37%      | 28       | 13        | 13          | <1         |
| Containers with scrolling | 75%      | 27       | 25        | 25          | <1         |
| Widgets demo              | 81%      | 24       | 26        | 26          | <1         |
| All scenes avg.           | 49%      | 25       | 19        | 19          | <1         |

The following benchmark results were obtained using LVGL 9.6.0 without IPA acceleration and with double buffering.

| Name                      | Avg. CPU | Avg. FPS | Avg. time | render time | flush time |
| :------------------------ | -------: | -------: | --------: | ----------: | ---------: |
| Empty screen              | 55%      | 27       | 20        | 20          | <1         |
| Moving wallpaper          | 89%      | 19       | 44        | 44          | <1         |
| Single rectangle          | 15%      | 27       | 1         | 1           | <1         |
| Multiple rectangles       | 34%      | 27       | 12        | 12          | <1         |
| Multiple RGB images       | 24%      | 27       | 7         | 7           | <1         |
| Multiple ARGB images      | 43%      | 29       | 14        | 14          | <1         |
| Rotated ARGB images       | 85%      | 16       | 52        | 52          | <1         |
| Multiple labels           | 37%      | 27       | 11        | 11          | <1         |
| Screen sized text         | 95%      | 6        | 145       | 145         | <1         |
| Multiple arcs             | 20%      | 27       | 3         | 3           | <1         |
| Containers                | 25%      | 27       | 12        | 12          | <1         |
| Containers with overlay   | 84%      | 15       | 57        | 57          | <1         |
| Containers with opa       | 53%      | 28       | 20        | 20          | <1         |
| Containers with opa_layer | 70%      | 24       | 32        | 32          | <1         |
| Containers with scrolling | 89%      | 19       | 43        | 43          | <1         |
| Widgets demo              | 89%      | 19       | 39        | 39          | <1         |
| All scenes avg.           | 56%      | 22       | 32        | 32          | <1         |

## Specification

- **MCU:** GD32F470IK (Cortex-M4, 240MHz)
- **RAM:** 192KB internal SRAM, 64KB TCM SRAM, 32768KB external SDRAM
- **Flash:** 3072KB internal
- **GPU:** IPA
- **Resolution:** 480*272
- **Display Size:** 4.3 inch
- **Interface:** TLI
- **Color Depth:** 16-bit RGB565
- **Technology:** a-Si TFT
- **DPI:** 130 px/inch
- **Touch Pad:** Resistive touch (SPI)

## Getting Started

The following steps describe a general workflow. Confirm the project paths, software versions,
power supply requirements, and jumper settings against the actual project and the board manual.
This project currently supports three integrated development environments (IDEs): Keil MDK, IAR, and GD32EmbeddedBuilder. This document uses GD32EmbeddedBuilder to demonstrate how to build, download, and debug the project.

### Hardware Setup

1. Confirm that the board model is `GD32F470I EVAL`.
2. Check the RGB display and resistive touch panel connections according to the board manual.
3. Connect the on-board JLINK debug interface to your PC, and configure the power supply and jumpers as required.

### Software Setup

1. Install `GD32EmbeddedBuilder` for your operating system. Refer to the GD32EmbeddedBuilder software requirements for details.
2. Install the appropriate J-Link driver software and confirm that your PC recognizes the debugger.

### Build the Project

1. Clone this repository.
```shell
  git clone "Repository URL"
  ```
2. Open `GD32EmbeddedBuilder` and click **Browse...** to select a local directory as your workspace.
3. Import the project by selecting **File > Import...** and choosing the project directory (`./Template/GD32EBuilder_project`).
4. Build the project by right-clicking the project in the project explorer and selecting **Build Project**.
5. To download and debug the project, right-click the project in the project explorer and select
	**Debug As > Debug Configurations... > GDB General Debugging**. Configure the settings for your setup,
	then click **Debug** to download the firmware and start debugging.

### Debugging

Start a debug session using your IDE and JLINK to set breakpoints, step through code, and inspect variables.
If display or touch issues occur, first check peripheral initialization, display refresh, and touch input handling.
The available debug options depend on your IDE and project configuration.

## Third-Party Components

| Category | In Use | Component | Version | License |
| --- | --- | --- | --- | --- |
| GUI | Yes | `LVGL` | `V9.6.0` | `MIT` |
| RTOS | Yes | `FreeRTOS` | `V10.4.1` | `MIT` |
| Filesystem | Yes | `FatFs` | `R0.16` | `FatFs license` |


When adding a third-party library, update this table and retain its license text and copyright notices.

## Contribution and Support

If you find any issues with the development board feel free to open an Issue in this repository. For LVGL related issues (features, bugs, etc) please use the main [lvgl repository](https://github.com/lvgl/lvgl).

If you found a bug and found a solution too please send a Pull request. If you are new to Pull requests refer to [Our Guide](https://docs.lvgl.io/master/CONTRIBUTING.html#pull-request) to learn the basics.
