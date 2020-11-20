I'm really sorry for not committing in a while, I just got a sudden streak of enlightment and got really excited about all the games that started working, I couldn't stop implementing new things as soon as I discovered a game used them!

Take a look at the sections below for a more detailed description of what exactly transpired:

## Overall
 - Some games correctly boot and get to start screens, the only arguably playable game right now is Super Mario Land, some other games freeze on a blank screen, while others don't quite yet manage to "initialize" the screen. Most of them at least do something now!
 - Games now run at a locked 60 FPS!
 - Joypad support is partially implemented, only the `START`, `SELECT`, `A` and `B` buttons work for now. These are mapped to `Enter`, `Q`, `A` and `S`, respectively, check the readme for more playability info
 - The boot ROM works almost perfectly, all of its checks need to be disabled for now, but that lovely falling Nintendo logo works as expected!
 - I've also added a simple CLI, check out the readme for more info about that

## CPU
### Major
 - Implemented all remaining instructions (including CB-prefixed), except `halt` and `stop`
 - Implemented basic interrupt handling. Vblank is the only interrupt supported for now
### Minor
 - Improved `daa`'s implementation

## PPU
### Major
 - Implemented vertical scrolling (and thus `SCY`)
 - Implemented basic sprite support (along with DMA transfers)
### Minor
 - Improved VRAM/OAM access emulation

## Cartridge support
 - Partially implemented MBC3 and in-cart RAM support

## Memory/Bus
 - Fixed a bug that prevented access to `WRA1`. Many games put their stacks on that WRAM bank 1, now they boot (or even partially work!)

## Etc
### Major
 - Implemented the DMA controller, I really have no way of programatically testing correct behaviour on all situations, but sprites seem to be working correctly
 - Implemented a CLI

That's it, those are the most important changes for now. Thanks for reading!
