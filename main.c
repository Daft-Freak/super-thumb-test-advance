#include <seven/prelude.h>
#include <seven/hw/bios/decompression.h>
#include <seven/hw/video.h>
#include <seven/hw/video/bg_scroll.h>
#include <seven/hw/video/bg_tiled.h>

#include "assets/8x8font.h"

static void load_font() {
    BG_PALETTE[0] = 0;
    BG_PALETTE[1] = 0x7FFF;

    static const struct BitUnPackParam unpack = {
        .src_length = sizeof(font8x8_char_data_1bpp),
        .src_width = 1,
        .dst_width = 4
    };
    biosBitUnPack(font8x8_char_data_1bpp, GFX_BASE_ADDR(0), &unpack);
}

static void clear_text() {
    // clear char block 2
    uint16_t zero = 0;
    biosCpuSet(&zero, MAP_BASE_ADDR(2), CS_SRC_FIXED | (0x800 / 4));
}

static void write_text(int x, int y, const char *text) {
    // char block 2
    uint16_t *ptr = MAP_BASE_ADDR(2) + x + y * 32;

    for(; *text; text++)
        *ptr++ = *text - ' ';
}

int main() {
    REG_DISPCNT = VIDEO_MODE_REGULAR | VIDEO_BG0_ENABLE;

    REG_BG0CNT = BG_MAP_BASE(2) | BG_GFX_BASE(0) | BG_SIZE_256X256;
    REG_BG0HOFS = 0;
    REG_BG0VOFS = 0;

    // enable irq/vblank
    irqInitDefault();
    irqEnable(IRQ_VBLANK);
    REG_DISPSTAT = LCD_VBLANK_IRQ_ENABLE;

    load_font();
    clear_text();

    write_text(0, 0, "Hello!");

    while(true) {
        biosVBlankIntrWait();
    }
}
