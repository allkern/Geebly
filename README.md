<p align="center">
  <img src="https://user-images.githubusercontent.com/15825466/97769482-3c496d00-1b0a-11eb-8930-72a60e210d15.png">
</p>

# Geebly
A Gameboy emulator written in C++, currently under development.

## Current state
### CPU
The CPU is almost fully implemented. About 7 (not counting cb-prefixed) out of 214 opcodes have not yet been implemented:
- Rotate opcodes (`rla`, `rra`, etc)
- Carry flag opcodes (`ccf`, `scf`)
- `cpl`
- `stop`
- `halt`
- cb-prefixed opcodes

### Mapper/cartridge type support
Mappers `0x0` (ROM Only) and `0x1` (MBC1, partially) are supported, many more games are now loadable due to the implementation of MBC1! 

### PPU/GPU
Only the background layer is supported as of now. The PPU is cycle-accurate though (interrupts wouldn't be possible otherwise), timing is based on the cycles elapsed since the last CPU instruction.

`LCDC`, `STAT` (partially) and `LY` are currently implemented, and mapped to their corresponding HRAM addresses.

### Etc
The interrupt system is not yet implemented, however, just like everything else on this list, its easily implementable, as I really put a lot of effort on devising a cohesive structure so that new features can be easily added or changed.
