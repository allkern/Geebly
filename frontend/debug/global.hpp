#pragma once

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define TITLEBAR_SIZE 22
#define MENUBAR_SIZE 22

#define CPU_PANEL_X 0
#define CPU_PANEL_Y MENUBAR_SIZE
#define CPU_PANEL_WIDTH 300
#define CPU_PANEL_HEIGHT (WINDOW_HEIGHT - TITLEBAR_SIZE)
#define CPU_PANEL_XEND (CPU_PANEL_X + CPU_PANEL_WIDTH)
#define CPU_PANEL_YEND (CPU_PANEL_Y + CPU_PANEL_HEIGHT)

#define PPU_PANEL_X CPU_PANEL_XEND
#define PPU_PANEL_Y MENUBAR_SIZE
#define PPU_PANEL_WIDTH 640
#define PPU_PANEL_HEIGHT (WINDOW_HEIGHT - TITLEBAR_SIZE)
#define PPU_PANEL_XEND (PPU_PANEL_X + PPU_PANEL_WIDTH)
#define PPU_PANEL_YEND (PPU_PANEL_Y + PPU_PANEL_HEIGHT)

namespace frontend {
    namespace debug {
        bool show_cpu_panel = true,
             show_ppu_panel = true,
             show_screen_panel = true;
    }
}