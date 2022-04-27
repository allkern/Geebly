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
I've improved both Windows and Linux build systems a lot, and tested them in various systems. I made sure to cover all possible scenarios.

### Linux
Building in Linux hosts with `apt` is really simple:
```sh
git clone https://github.com/Lycoder/Geebly
cd Geebly
make environment
make
```

`make environment` will set up the build environment, this includes cloning the various repositories the emulator depends on, these are `ocornut/imgui`, `skaslev/gl3w`, `Lycoder/lgw` and `Lycoder/sdl_shader`. It also installs `sdl2-dev` and `sdl2-ttf-dev`

### Windows
Building in Windows hosts is a little bit more involved, and usually requires actions after the build is completed, its also really simple nonetheless.

Keep in mind the scripts provided require Powershell and script execution enabled (See [this](https://docs.microsoft.com/en-us/powershell/module/microsoft.powershell.core/about/about_execution_policies?view=powershell-7.2))
```
git clone https://github.com/Lycoder/Geebly
cd Geebly
./prepare-build
./build-win
```

Assuming you have a C++20-compatible compiler, it should build no problems. It might complain about `-m64`, in that case,  your compiler doesn't support 64-bit builds, just remove the flag.

The emulator should compile fine, but you might run into some problems after its built.

#### Troubleshooting
Common issues:
- The UI system requires the font Ubuntu Mono to be in the current directory to work, just download the font [here](https://fonts.google.com/specimen/Ubuntu+Mono), then copy the file `UbuntuMono-Regular.ttf` to Geebly's folder and rename it to "ubuntu-mono.ttf"
- The program might not start, in that case, you're probably lacking DLLs inside the folder you're running the emulator from, just copy `SDL2.dll` and `SDL2_ttf.dll` to your folder. Keep in mind the `prepare-build` script already copies those two DLLs to the root Geebly folder, if its still complaining, you might be lacking some compiler DLLs,  such as: `libstdc++-7.dll`, and similar.

Please open an issue or just PM me on Discord (Lycoder#8480) if you find any more issues
