<p align="center">
  <img src="https://user-images.githubusercontent.com/15825466/97769482-3c496d00-1b0a-11eb-8930-72a60e210d15.png">
</p>

# Geebly
A Gameboy emulator written in C++, currently under development.

![alt text](https://user-images.githubusercontent.com/15825466/99772991-88287a00-2aea-11eb-9d80-630cd04bd06b.gif "Running Tetris")

## Usage
`geebly <settings> <file>`

### Settings
There are different settings for every part of the emulation engine, Boot ROM/BIOS related settings are prefixed with a `B`, general emulation settings are prefixed with a `E`, etc. Common settings' shorthands usually are just one letter long though.

Here's all the settings currently present in the CLI:
|Setting|Shorthand|Description|Default|
|--|--|--|--|
|`--debug`|`-d`|Enable the Geebly Debugger|Disabled
|`--bios`|`-b`|Specify a Boot ROM|`"bios.bin"`
|`--scale`|`-s`|Set the display scale|1
|`--no-patch-bios-checks`/`--patch-bios-checks`|`-Bchecks`/`-Bno-checks`|Patch Boot ROM checks|No patches
|`--no-skip-bootrom`/`--skip-bootrom`|`-Bno-skip`/`-Bskip`|Skip the scrolling boot logo|No skip
|`--vram-access-emulation`/`--no-vram-access-emulation`|`Evram-access`/`-Eno-vram-access`|Enable/Disable VRAM/OAM mode 2 & 3 inaccessibility emulation|Disabled
|`--cgb-mode`|`-Ecgb`|Hardware identifies itself as CGB (GBC) when enabled|Disabled

## Current state
### CPU
The CPU is fully implemented, with the exception of `stop` and `halt`, both are emulated as NOPs.

Interrupts are partially supported, `Vblank` is the only one that's currently implemented

### Mapper/cartridge type support
Mappers `0x0` (ROM Only), `MBC1` and `MBC3` are currently supported, those last two being partially implemented

There's no `SRAM` banking support yet

### PPU/GPU
All three layers, background, window and sprites are implemented, vertical scrolling (`SCY` and `WY`) is supported in both background and window, but horizontal scrolling (`SCX` and `WX`) is only supported in BG, although, really naively; its not a perfect emulation.

The OAM DMA controller is also implemented, it will log warnings when invalid transfers are attempted.

### Timers
Short answer: They're not implemented, but `DIV` can be used as a source of PRNs.

Long answer: After I got Tetris working and in-game, I realized that its PRNG wasn't working, as the only tetromino being generated was the O one. I recalled reading that Tetris in the GameBoy used the `DIV` register as a source for random tetromino generation, and so, I just implemented it as incrementing by CPU cycles elapsed since the last instruction divided by 4 (it could be anything, even a rnd() C call), and that made Tetris work! Other than that, its not doing what it is supposed to do.

`TIMA`, `TAC`, etc. are not implemented.

### Blargg's tests
`cpu_instrs.gb` reports 11 out of 11 tests failed.
Explanation:
In many tests (all of them?), the mechanism through which correctness is checked, is CRC checksums. These are pretty strict, and any emulation error will most probably make calculations deviate from expected and such, the resulting checksum might be wildly wrong, even if what's being tested is actually correct.

I think this because the `06` test reports that every `ld r, r` instruction is wrong, if that was the case, most probably nothing would even work!
