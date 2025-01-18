#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<inttypes.h>

#include<neslib.h>

#include "uwurandom_core.h"
#include "uwurandom_ops.h"

#define EFAULT 14

void vprint(int x, int y, const char* text, size_t len) {
    vram_adr(NTADR_A(x, y));
    for (size_t i = 0; i < len; ++i) {
        vram_put(text[i]);
    }
}

void vprints(int x, int y, const char* text) {
    vram_adr(NTADR_A(x, y));
    size_t len = strlen(text);
    for (size_t i = 0; i < len; ++i) {
        vram_put(text[i]);
    }
}

// void vprint_wrap(int x, int y, const char* text, size_t len, size_t max_line_len) {
//     int line = y;
//     size_t remaing = len;
//     while (remaing) {
//         size_t num_chars = remaing > max_line_len ? max_line_len : remaing;
//         vprint(x, line, text, num_chars);
//         text += num_chars;
//         remaing -= num_chars;
//         ++line;
//     }
// }

static char dialog_buf[103] = {0};
// static char dialog_buf2[28] = {0};

// uwu_state state;

void clear_dialog() {
    memset(dialog_buf, 0, sizeof(dialog_buf));
}

// void write_uwu_line(int x, int y, int len, uwu_state* state) {
//     write_chars(state, dialog_buf, len);
//     vprint(x, y, dialog_buf, len);
// }

// llvm-mos-sdk doesn't implement strnlen :(
size_t strnlen(const char *str, size_t maxlen) {
    for (size_t i = 0; i < maxlen; i++) {
        if (str[i] == '\0') {
            return i;
        }
    }
    return maxlen;
}

enum GameState{
    GAME_STATE_TEXT_SCROLL,
    GAME_STATE_TEXT_WAIT,
} GameState;

int main() {
    uwu_state state = {};
    int init_err = uwu_init_state(&state, uwu_op_table_default, ARRAY_SIZE(uwu_op_table_default));

    if (init_err) {
        return init_err;
    }

    static const char palette[16] = {0x0c, 0x05, 0x15, 0x35};

    // splash
    ppu_off();
    oam_clear();
    pal_bg(palette);

    vprints(7, 5, "~ UWURANDOM  NES ~");
    vprints(10, 7, "by Dunkyl \x81\x81");
    vprints(15, 11, "\x08\x09");
    vprints(15, 12, "\x18\x19");

    vprints(1, 15, "uwurandom \x9F 2024 valadaptive");

    ppu_on_all(); 
    oam_clear();

    for (int i = 0; i < 480; ++i) {
        ppu_wait_nmi();
    }
    

    ppu_off(); // screen off
    vram_fill(0, 0x2000);
    pal_bg(palette);

    // cat
    vram_adr(NTADR_A(15, 10));
    vram_put(0x86);
    vram_put(0x87);
    vram_adr(NTADR_A(15, 11));
    vram_put(0x96);
    vram_put(0x97);
    vram_adr(NTADR_A(15, 12));
    vram_put(0xA6);
    vram_put(0xA7);

    // text box
    vram_adr(NTADR_A(2, 22));
    vram_put(0x82);
    for (int i = 0; i < 26; ++i) {
        vram_put(0x83);
    }
    vram_put(0x85);
    for (int i = 0; i < 4; ++i) {
        vram_adr(NTADR_A(2, 23+i));
        vram_put(0x84);
        vram_adr(NTADR_A(29, 23+i));
        vram_put(0x94);
    }
    vram_adr(NTADR_A(2, 27));
    vram_put(0x92);
    for (int i = 0; i < 26; ++i) {
        vram_put(0x93);
    }
    vram_put(0x95);

    ppu_on_all();
    oam_clear();
    pal_spr(palette);

    char time = 0;
    int big_time = 0;

    char game_state = GAME_STATE_TEXT_SCROLL;
    size_t text_scroll = 0;
    #define UWUN 103
    clear_dialog();
    uwu_write_chars(&state, dialog_buf, UWUN);

    char dialog_len = UWUN; //strlen(dialog_buf);

    #define MAX_FAST_TEXT 24
    char fast_text_bufc[MAX_FAST_TEXT] = {0};
    uint8_t fast_text_bufx[MAX_FAST_TEXT] = {0};
    uint8_t fast_text_bufy[MAX_FAST_TEXT] = {0};
    size_t fast_text_index = 0;

    while (true) {
        
        ppu_wait_nmi();
        oam_clear();
        char pad1 = pad_poll(0);

        

        if (game_state == GAME_STATE_TEXT_WAIT) {
            if (time < 30) {
                oam_spr(226, 208,0x80, 0); 
            } else {
                oam_spr(226, 208,0x90, 0); 
            }
            if (pad1 & PAD_A) {
                game_state = GAME_STATE_TEXT_SCROLL;
                text_scroll = 0;
                ppu_off();
                clear_dialog();
                fast_text_index = 0;
                vram_adr(NTADR_A(3, 23));
                vram_write(dialog_buf, 26);
                vram_adr(NTADR_A(3, 24));
                vram_write(dialog_buf, 26);
                vram_adr(NTADR_A(3, 25));
                vram_write(dialog_buf, 26);
                vram_adr(NTADR_A(3, 26));
                vram_write(dialog_buf, 26);
                uwu_write_chars(&state, dialog_buf, UWUN);
                dialog_len = strnlen(dialog_buf, 103);
                ppu_on_all();
            }
        } else {
            if (text_scroll == dialog_len) {
                game_state = GAME_STATE_TEXT_WAIT;
                text_scroll = 0;
            } else {
                // ppu_off();
                char cursorx = 3 + text_scroll;
                char cursory = 23;
                while (cursorx > 28) {
                    cursorx -= 26;
                    cursory += 1;
                }
                fast_text_bufc[fast_text_index] = dialog_buf[text_scroll];
                fast_text_bufx[fast_text_index] = cursorx;
                fast_text_bufy[fast_text_index] = cursory;
                ++fast_text_index;
                ++text_scroll;
                // reset fast text and place all characters as bg tiles
                if (fast_text_index == MAX_FAST_TEXT) {
                    fast_text_index = 0;
                    ppu_off();
                    for (size_t i = 0; i < MAX_FAST_TEXT; ++i) {
                        vram_adr(NTADR_A(fast_text_bufx[i], fast_text_bufy[i]));
                        vram_put(fast_text_bufc[i]);
                    }
                    ppu_on_all();
                }
            }
        }
        
        for (size_t i = 0; i < fast_text_index; ++i) {
            oam_spr(fast_text_bufx[i]<<3, (fast_text_bufy[i]<<3)-1 + (fast_text_index%3), fast_text_bufc[i], 0); 
        }

        ++time;
        if (time == 60) {
            time = 0;
            ++big_time;
        }

        
        if (game_state == GAME_STATE_TEXT_SCROLL) {
            oam_spr(17<<3, (10<<3)-1, 0x8E, 0);
            oam_spr(18<<3, (10<<3)-1, 0x8F, 0);
            oam_spr(17<<3, (11<<3)-1, 0x9E, 0);
            // oam_spr(16<<3, (11<<3)-1, 0x9F, 0);
            if (time%10 < 5) { // mouth open
                oam_spr(15<<3, (11<<3)-1, 0x98, 0);
                oam_spr(16<<3, (11<<3)-1, 0x99, 0);
                // oam_spr((15<<3)+2, (13<<3)-1, 0xB8, OAM_FLIP_V); //kbd
                // oam_spr((16<<3)+2, (13<<3)-1, 0xB9, OAM_FLIP_V);
            } else {
                // oam_spr((15<<3)+2, (13<<3)+1, 0xB8, OAM_FLIP_V);//kbd
                // oam_spr((16<<3)+2, (13<<3)+1, 0xB9, OAM_FLIP_V);
            }
        } else {
            // oam_spr((15<<3)+2, (13<<3)+4, 0xB8, OAM_FLIP_V);//kbd
            // oam_spr((16<<3)+2, (13<<3)+4, 0xB9, OAM_FLIP_V);
            if (big_time % 6 == 2) { // blink hold
                oam_spr(15<<3, (10<<3)-1, 0x8A, 0);
                oam_spr(16<<3, (10<<3)-1, 0x8B, 0);
                oam_spr(15<<3, (11<<3)-1, 0x9A, 0);
                oam_spr(16<<3, (11<<3)-1, 0x9B, 0);
            } else if (big_time % 6 == 1) {
                if (time > 50) { // blink start
                    oam_spr(15<<3, (10<<3)-1, 0x8C, 0);
                    oam_spr(16<<3, (10<<3)-1, 0x8D, 0);
                    oam_spr(15<<3, (11<<3)-1, 0x9C, 0);
                    oam_spr(16<<3, (11<<3)-1, 0x9D, 0);
                }
            }
        }
    }

    // silence the warning about uwu_destroy_state being unused by running it
    // right after the infinite loop finishes
    uwu_destroy_state(&state);
}
