<p align="center">
  <img width="85%" height="85%" src="https://user-images.githubusercontent.com/15825466/117024784-b1bc1300-acd0-11eb-9656-c4f6e1cc391c.png" alt="Geebly">
</p>

<div align="center">
  
### A Game Boy/Color Emulator and Emulation API written in C++

---

![Geebly running Dangan GB](https://user-images.githubusercontent.com/15825466/117328287-dac3db80-ae69-11eb-9ef2-1e9ebf8e199e.gif)![Geebly running the Gejmbåj demo](https://user-images.githubusercontent.com/15825466/117328294-dd263580-ae69-11eb-87a2-942136e2e204.gif)![Geebly running Super Mario Bros. Deluxe](https://user-images.githubusercontent.com/15825466/117328291-dbf50880-ae69-11eb-9e9f-6388101d57ce.gif)![Geebly running Donkey Kong Country](https://user-images.githubusercontent.com/15825466/117328298-ddbecc00-ae69-11eb-8976-ec459c9ea3bb.gif)

</div>

## Usage
### Linux
`geebly <settings> <file>`

### Windows
`./geebly <settings> <file>` or `geebly <settings> <file>`

### Settings
There are different settings for every part of the emulation engine, Boot ROM/BIOS related settings are prefixed with a `B`, general emulation settings are prefixed with a `E`, etc. Common settings' shorthands usually are just one letter long though.

Here's all the settings currently present in the CLI:
|Setting|Shorthand|Description|Default|
|--|--|--|--|
|`--master-volume`|`-v`|Set the master volume (`float` or `int`) <ul><li>`0.0` = Silence</li><li>`1.0` = Max</li><li>`> 1.0` = Clipping</li></ul>|`1.0`
|`--mono`|`-m`|Disable stereo sound|No
|`--debug`|`-d`|Launch with debugger|Disabled
|`--boot`|`-b`|Specify a boot ROM|`"dmg_boot.bin"`
|`--scale`|`-s`|Set the display scale|1
|`--no-patch-bios-checks`/`--patch-bios-checks`|`-checks`/`-no-checks`|Patch Boot ROM checks|No patches
|`--skip-bootrom`|`-no-boot`|Skip the Boot ROM|Don't skip
|`--vram-access-emulation`/`--no-vram-access-emulation`|`vram-access`/`-no-vram-access`|Enable/Disable VRAM/OAM mode 2 & 3 inaccessibility emulation|Enabled
|`--cgb-mode`|`-cgb`|Change model to Game Boy Color|Disabled
|`--enable-joyp-irq-delay/--disable-joyp-irq-delay`|`-joyp-irq-delay/-no-joyp-irq-delay`|Enable/disable JOYP IRQ delay|Enabled
|`--gui`|`-g`|Launch GUI|Enabled
|`--ntsc-codec`|`-ntsc`|Enable NTSC codec|Disabled
|`--sgb-mode`|`-sgb`|Change model to Super Game Boy|Disabled
|`--no-logs`|`-n`|Disable logging|Disabled
|`--blend-frames`|`-i`|Enable frame blending/interpolation|Enabled

## Tests
Geebly passes 77 tests on Daid's shootout

## Building
So many things have changed in this last version that the build process is just not the same anymore, I'll fix that in upcoming commits
