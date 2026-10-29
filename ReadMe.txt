

A custom-designed, compact desktop alarm clock powered by the Seeed Studio XIAO RP2040 microcontroller. Features a custom PCB, mechanical switch matrix, integrated display, piezo buzzer, and a 3D-printable enclosure.

---

## Features

- **Microcontroller:** Seeed Studio XIAO RP2040
- **Display:** High-contrast display module driven via SPI/I2C
- **Keypad Matrix:** 4 tactile mechanical switches arranged in a 2x2 scanning matrix with diode protection
- **Audio:** Integrated piezo buzzer for alarm signals
- **Enclosure:** 2-part custom 3D-printable shell (top bezel & base plate)

---

## Hardware Specifications & Pinout

### Microcontroller Connections (XIAO RP2040)
| Function | Pin Name | Description |
| :--- | :--- | :--- |
| **Row 1** | `PA02_A0_D0` | Keypad Matrix Row 1 |
| **Row 2** | `PA4_A1_D1` | Keypad Matrix Row 2 |
| **Column 1** | `PA10_A2_D2` | Keypad Matrix Column 1 |
| **Column 2** | `PA11_A3_D3` | Keypad Matrix Column 2 |
| **SDA** | `PA8_A4_D4_SDA` | Serial Data |
| **SCL** | `PA9_A5_D5_SCL` | Serial Clock |
| **Reset (RES)** | `PB08_A6_D6_TX` | Display Reset Line |
| **Data/Command (DC)** | `PB09_A7_D7_RX` | Display D/C Line |
| **Chip Select (CS)** | `PA7_A8_D8_SCK` | Display CS Line |
| **Backlight (BLK)** | `PA5_A9_D9_MISO` | Display Backlight Control |
| **Buzzer** | `PA6_A10_D10_MOSI` | Piezo Buzzer Output |

---

## Repository Structure

- `PCB/` — Schematic and board layout files
- `production/` — Gerber files and manufacturing exports
- `Screenshots/` — PCB layouts, CAD schematics, and 3D renders
- `LICENSE` — MIT License

---

## License

Distributed under the MIT License. See `LICENSE` for more information.
