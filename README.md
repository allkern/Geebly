<p align="center">
  <img src="https://user-images.githubusercontent.com/15825466/97769482-3c496d00-1b0a-11eb-8930-72a60e210d15.png">
</p>

# Geebly
A Gameboy emulator written in C++, currently under development.

![alt text](https://user-images.githubusercontent.com/15825466/99772991-88287a00-2aea-11eb-9d80-630cd04bd06b.gif "Running Tetris")

## Usage
`geebly <settings> rom_name.gb`

### Settings
There are different settings for every part of the emulation engine, Boot ROM/BIOS related settings are prefixed with a `B`, general emulation settings are prefixed with a `E`, etc. Common settings' shorthands usually are just one letter long though.

Here's all the settings currently present in the CLI:
|Setting|Shorthand|Description|Default|
|--|--|--|--|
|`--debug`|`-d`|Enable the Geebly Debugger|Disable
|`--bios`|`-b`|Specify a Boot ROM|`"bios.bin"`
|`--scale`|`-s`|Set the display scale|1
|`--no-patch-bios-checks`/`--patch-bios-checks`|`-Bchecks`/`-Bno-checks`|Patch Boot ROM checks|Patch checks
|`--no-skip-bootrom`/`--skip-bootrom`|`-Bno-skip`/`-Bskip`|Skip the scrolling boot logo|No skip
|`--vram-access-emulation`/`--no-vram-access-emulation`|`Evram-access`/`-Eno-vram-access`|Enable/Disable VRAM/OAM mode 2 & 3 inaccessibility emulation|Enable

## Current state
### CPU
The CPU is fully implemented, with the exception of `stop` and `halt`. Interrupts are partially supported, `Vblank` is the only one that's currently implemented

### Mapper/cartridge type support
Mappers `0x0` (ROM Only), `MBC1` and `MBC3` are currently supported, those last two being partially implemented

### PPU/GPU
Cycle-accurate. Only the background layer is supported as of now.

Vertical scrolling (`SCY`) is implemented.

`LCDC`, `STAT` (partially), `LY` and `SCY` are currently implemented, and mapped to their corresponding HRAM addresses

### Blargg's tests
`cpu_instrs.gb` reports 12 out of 12 tests failed. Though I think this is a misdiagnosis, the register values before the tests might be wrong, and that's whats causing checksum errors when actually testing instructions/behaviour.
