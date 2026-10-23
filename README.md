# CHIP-8 Emulator

[![C++20](https://img.shields.io/badge/Language-C%2B%2B20-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B20)
[![CMake](https://img.shields.io/badge/Build-CMake%203.28%2B-064F8C.svg?logo=cmake)](https://cmake.org/)
[![SFML](https://img.shields.io/badge/Library-SFML%203.1.0-8CC445.svg?logo=cplusplus)](https://www.sfml-dev.org/)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](LICENSE.txt)

A cross-platform, cycle-accurate **CHIP-8 Virtual Machine and Interpreter** written in modern C++20 using CMake and SFML 3.

https://github.com/user-attachments/assets/4a80efb1-89b0-4650-8c3a-6e5b073cc0f3

---

## Features

- **Full Instruction Set:** Implements all 35 standard CHIP-8 opcodes, including math, logic, jumps, subroutines, sprite drawing, and keypad skip operations.
- **Decoupled Dual-Clock Architecture:** CPU instruction execution rate (configurable in Hz, default 700 Hz) runs independently from the 60 Hz hardware delay and sound timers.
- **Procedural Audio Synthesis:** Integrated square wave tone buzzer synthesized dynamically using `SFML::Audio` when the sound timer is active (no external audio assets needed).
- **Fast Opcode Dispatch:** Employs member function pointer lookup tables for rapid, clean instruction decoding.
- **Efficient GPU Rendering:** Renders $64 \times 32$ monochrome graphics using SFML textures with preallocated memory buffers and conditional redraws (`draw_flag`).
- **Ergonomic CLI:** Flexible command-line interface with sensible defaults, comprehensive error diagnostics, and `--help` documentation.
- **Cross-Platform:** Native support and pre-configured CMake presets for Linux, Windows, and macOS.

---

## Virtual Machine Architecture

CHIP-8 is an interpreted architecture originally created in the mid-1970s for microcomputers like the COSMAC VIP.

```
                      +-----------------------------+
                      |       4096-Byte RAM         |
                      |  0x000 - 0x1FF: Interpreter |
                      |  0x050 - 0x09F: Built-in Font
                      |  0x200 - 0xFFF: Program/ROM |
                      +--------------+--------------+
                                     |
                             [Fetch 2 Bytes]
                                     v
                       +-------------+-------------+
                       |     Opcode Dispatcher     |
                       |  (Function Pointer Table) |
                       +------+--------------+-----+
                              |              |
           +------------------+              +-------------------+
           v                                                     v
+-----------------------+                             +--------------------+
|   Registers & Stack   |                             |    I/O & Timers    |
| - 16x 8-bit Regs (V0) |                             | - 60 Hz Delay Tim  |
| - 16-bit Index (I)    |                             | - 60 Hz Sound Tim  |
| - 16-bit PC           |                             | - 64x32 Framebuf   |
| - 16-level Call Stack |                             | - 16-Key Keypad    |
+-----------------------+                             +--------------------+
```

### Specifications

| Component | Specification |
| :--- | :--- |
| **Memory** | 4,096 bytes (4 KB). Programs load at `0x200`. Built-in fontset loaded at `0x050`. |
| **Registers** | 16 general-purpose 8-bit registers (`V0` to `VF`). `VF` doubles as the carry/borrow/collision flag. |
| **Index Register (I)** | 16-bit register pointing to memory addresses. |
| **Program Counter (PC)** | 16-bit register tracking the current instruction address. |
| **Call Stack** | 16 levels of 16-bit return addresses. |
| **Timers** | 8-bit Delay Timer (`DT`) and Sound Timer (`ST`), decremented at 60 Hz. |
| **Display** | $64 \times 32$ monochrome display buffer. Sprites are drawn via bitwise XOR with collision detection. |
| **Keypad** | 16-key hexadecimal keypad (`0x0` through `0xF`). |

---

## Keypad Mapping

CHIP-8 uses a $4 \times 4$ hexadecimal keypad, mapped to modern QWERTY keyboards as follows:

```
 CHIP-8 Keypad:                QWERTY Keyboard:
+---+---+---+---+             +---+---+---+---+
| 1 | 2 | 3 | C |             | 1 | 2 | 3 | 4 |
+---+---+---+---+             +---+---+---+---+
| 4 | 5 | 6 | D |     ==>     | Q | W | E | R |
+---+---+---+---+             +---+---+---+---+
| 7 | 8 | 9 | E |             | A | S | D | F |
+---+---+---+---+             +---+---+---+---+
| A | 0 | B | F |             | Z | X | C | V |
+---+---+---+---+             +---+---+---+---+
```

- **Escape:** Close the emulator window and exit.

---

## Building from Source

### Prerequisites

- **C++20 Compliant Compiler:** GCC 12+, Clang 15+, or MSVC 19.30+
- **CMake:** Version 3.28 or later
- **Ninja** (recommended) or Make
- **Graphics/Audio Libraries (Linux only):**
  ```bash
  sudo apt install libx11-dev libxrandr-dev libxcursor-dev libudev-dev libgl1-mesa-dev libopenal-dev
  ```
  *(SFML 3.1.0 is fetched and compiled automatically via CMake FetchContent)*

### Build Instructions

#### Using CMake Presets (Recommended)

```bash
# Configure preset (Linux)
cmake --preset linux-debug

# Build executable
cmake --build out/build/linux-debug
```

#### Manual Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

---

## Usage

```bash
chip8emu <ROM> [Scale] [ClockHz]
```

### Options & Arguments

- `<ROM>`: Path to the `.ch8` ROM file (*required*).
- `[Scale]`: Window scale multiplier (*optional*, default: `15` $\rightarrow 960 \times 480$).
- `[ClockHz]`: CPU clock frequency in instructions per second (*optional*, default: `700` Hz).
- `-h`, `--help`: Show usage instructions and exit.

### Examples

```bash
# Run with default settings (15x scale, 700 Hz)
./chip8emu share/octojam1title.ch8

# Run with custom 20x scale and 600 Hz clock speed
./chip8emu share/flightrunner.ch8 20 600

# View built-in help
./chip8emu --help
```

---

## Included Sample ROMs

A selection of public domain and homebrew CHIP-8 ROMs are provided in the [`share/`](share/) directory:

- [`share/flightrunner.ch8`](share/flightrunner.ch8) - High-speed obstacle dodger.
- [`share/octojam1title.ch8`](share/octojam1title.ch8) - Octojam title demo.
- [`share/petdog.ch8`](share/petdog.ch8) - Interactive virtual pet simulation.
- [`share/pumpkindressup.ch8`](share/pumpkindressup.ch8) - Pumpkin customization game.

---

## License

This project is licensed under the terms of the [GNU General Public License v3.0](LICENSE.txt).
