#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <seven/prelude.h>
#include <seven/hw/bios/decompression.h>
#include <seven/hw/video.h>
#include <seven/hw/video/bg_scroll.h>
#include <seven/hw/video/bg_tiled.h>
#include <seven/util/log.h>

#include "assets/8x8font.h"

static int cursor_x = 0, cursor_y = 0;

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

static void write_text_pos(int x, int y, const char *text) {
    // char block 2
    uint16_t *ptr = MAP_BASE_ADDR(2) + x * 2 + y * 64;

    for(; *text; text++)
        *ptr++ = *text - ' ';
}

static void write_text(const char *text) {
    for(; *text; text++) {
        if(*text == '\n') {
            cursor_y++;
            cursor_x = 0;

            if(cursor_y == 20) {
                // scroll
                // using actual scrolling would be more efficient, but it really doesn't matter here
                for(int y = 0; y < 20; y++)
                    biosCpuSet(MAP_BASE_ADDR(2) + (y + 1) * 64, MAP_BASE_ADDR(2) + y * 64, 64 / 4);
                cursor_y--;
            }
        } else if(*text == '\r')
            cursor_x = 0;
        else {
            uint16_t *ptr = MAP_BASE_ADDR(2) + cursor_x * 2 + cursor_y * 64;
            *ptr = *text - ' ';
            cursor_x++;
        }
    }
}

static int log_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);

    // get length
    va_list tmp_args;
    va_copy(tmp_args, args);
    int len = vsnprintf(NULL, 0, format, tmp_args) + 1;
    va_end(tmp_args);

    char *buf = malloc(len);
    int ret = vsnprintf(buf, len, format, args);
    va_end(args);

    write_text(buf);
    logOutput(LOG_WARN, buf);

    free(buf);
    return ret;
  }

int main() {
    REG_DISPCNT = VIDEO_MODE_REGULAR | VIDEO_BG0_ENABLE;

    REG_BG0CNT = BG_MAP_BASE(2) | BG_GFX_BASE(0) | BG_SIZE_256X256;
    REG_BG0HOFS = 0;
    REG_BG0VOFS = 0;

    logInit();
    logSetMaxLevel(LOG_INFO);

    // enable irq/vblank
    irqInitDefault();
    irqEnable(IRQ_VBLANK);
    REG_DISPSTAT = LCD_VBLANK_IRQ_ENABLE;

    load_font();
    clear_text();

    write_text("Hello!\n\n");

    while(true) {
        biosVBlankIntrWait();
    }
}
