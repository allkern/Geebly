![workflow badge](https://github.com/Lycoder/Geebly/actions/workflows/windows-workflow.yml/badge.svg)
![workflow badge](https://github.com/Lycoder/Geebly/actions/workflows/ubuntu-workflow.yml/badge.svg)
<p align="center">
  <img src="https://user-images.githubusercontent.com/15825466/97769482-3c496d00-1b0a-11eb-8930-72a60e210d15.png">
</p>

# Geebly
A Gameboy/Color emulator written in C++, currently under development.
<p align="center">
  <img src="https://user-images.githubusercontent.com/15825466/105384568-258a5200-5bf1-11eb-949c-8c274d88229e.gif" alt="Running Super Mario Bros. Deluxe (GBC)")
</p>

## Usage
### Linux
`geebly <settings> <file>`

### Windows (Powershell)
`./geebly <settings> <file>`

### Settings
There are different settings for every part of the emulation engine, Boot ROM/BIOS related settings are prefixed with a `B`, general emulation settings are prefixed with a `E`, etc. Common settings' shorthands usually are just one letter long though.

Here's all the settings currently present in the CLI:
|Setting|Shorthand|Description|Default|
|--|--|--|--|
|`--debug`|`-d`|Enable the Geebly Debugger|Disabled
|`--boot`|`-b`|Specify a boot ROM|`"dmg_boot.bin"`
|`--scale`|`-s`|Set the display scale|1
|`--no-patch-bios-checks`/`--patch-bios-checks`|`-Bchecks`/`-Bno-checks`|Patch Boot ROM checks|No patches
|`--no-skip-bootrom`/`--skip-bootrom`|`-Bno-skip`/`-Bskip`|Skip the scrolling boot logo|No skip
|`--vram-access-emulation`/`--no-vram-access-emulation`|`Evram-access`/`-Eno-vram-access`|Enable/Disable VRAM/OAM mode 2 & 3 inaccessibility emulation|Disabled
|`--cgb-mode`|`-Ecgb`|Switches to CGB (Game Boy Color) mode|Disabled
|`--enable-joyp-irq-delay/--disable-joyp-irq-delay`|`-Ejoyp-irq-delay/-Eno-joyp-irq-delay`|Enable/disable JOYP IRQ delay|Enabled

## Current state
### Game Boy Color Emulation
Out of nowhere, there's now Game Boy Color emulation! This mostly comprises additional PPU registers and logic, and the CPU's double speed mode, that doesn't really affect the emulated CPU itself, but more the way devices do timing-based stuff based off the CPU clock.

#### Implemented (or mostly implemented) CGB-specific registers:
- `KEY1`
- `SVBK`
- `VBK`
- `BCPS/BGPI`
- `BCPD/BGPD`

### CPU
The CPU is fully implemented.

Almost all IRQs are supported, with the exception of `SC` (no Serial support yet), plus `STAT` might be broken, but testing points to it being close to perfectly emulated.

### Mapper/cartridge type support
Most common mappers are fully implemented, that is:
- `ROM only` 
- `MBC1`
- `MBC2`
- `MBC3`
- `MBC5`

All of them with full ROM/SRAM banking support!

RTC and Rumble functions are not supported.

### Graphics (PPU/GPU)
Color is supported! in CGB mode, there's two switchable VRAM banks, but the second one should contain a Background Tile Attribute Map that enables selecting CGB palettes in a per-tile basis, among other things like X/Y flipping tiles, etc.

All three layers are implemented, with full scrolling support for both background and window.

The OAM DMA controller is also implemented, it will log warnings when invalid transfers are attempted. The HDMA/GDMA CGB controllers are not yet supported, but short-term planned.

### Sound (SPU/APU)
Sound emulation is finally supported, but only on Windows, as I don't really have a separate Linux machine to test things on, I'm relying in WSL2 only, which doesn't really have good sound support, I could maybe mess around with `pulseaudio`, but it didn't seem like a straightforward process to me, so that's why I decided to not support sound emulation on Linux for now.

Its not an accurate emulation, nor all features are supported, only channels 1, 2 and 4 are supported, that's the two square channels and a noise channel.

Both square channels support volume envelope, trigger, sound length, but not sweep.

The noise channel's sound is completely off, the documented frequency calculation formula gave out really high frequencies that were inaudible, and so I had to workaround scaling them so something could be heard, this resulted in a high pitched sound being audible alongside the noise, which is undesirable, but at least we get to hear noise stabs or percussion-like sounds.

### Timers
`DIV` and `TIMA` (`TAC` and `TMA` aswell) are accurately emulated.

### Blargg's tests
`cpu_instrs.gb` passes all 11 tests!
