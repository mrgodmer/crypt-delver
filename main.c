#include <nds.h>
#include <stdio.h>
#include "game.h"

static PrintConsole topCon, botCon;
static Game game;

/* Buttons on the bottom screen, in console tile rows (8 px each). */
typedef struct { const char *label; int row0, row1; Action act; } Button;
static const Button BUTTONS[] = {
    { "ATTACK",         3,  6, ACT_ATTACK },
    { "POWER STRIKE",   7, 10, ACT_SKILL  },
    { "GUARD",         11, 14, ACT_GUARD  },
    { "POTION",        15, 18, ACT_POTION },
};
#define N_BUTTONS ((int)(sizeof(BUTTONS) / sizeof(BUTTONS[0])))

static void bar(int cur, int max, int width, char *out) {
    int fill = max > 0 ? (cur * width) / max : 0;
    if (cur > 0 && fill == 0) fill = 1;
    for (int i = 0; i < width; i++) out[i] = i < fill ? '#' : '-';
    out[width] = 0;
}

static void draw_top(void) {
    char b[20];
    consoleSelect(&topCon);
    consoleClear();
    iprintf("=== FLOOR %d ===\n\n", game.floor);
    bar(game.e.hp, game.e.maxhp, 16, b);
    iprintf("%s\n[%s]\n%d/%d\n\n", game.e.name, b, game.e.hp, game.e.maxhp);
    bar(game.p.hp, game.p.maxhp, 16, b);
    iprintf("YOU  HP [%s] %d/%d\n", b, game.p.hp, game.p.maxhp);
    iprintf("     MP %d/%d  Potions %d\n", game.p.mp, game.p.maxmp, game.p.potions);
    iprintf("     Gold %d\n\n", game.p.gold);
    for (int i = 0; i < LOG_LINES; i++) iprintf("%s\n", game.log[i]);
}

static void draw_bottom(void) {
    consoleSelect(&botCon);
    consoleClear();
    if (game.state == ST_GAMEOVER) {
        iprintf("\x1b[8;7HGAME OVER\n");
        iprintf("\x1b[10;4HTouch to try again");
        return;
    }
    for (int i = 0; i < N_BUTTONS; i++) {
        const Button *bt = &BUTTONS[i];
        iprintf("\x1b[%d;2H+----------------------------+", bt->row0);
        iprintf("\x1b[%d;4H%s", bt->row0 + 1, bt->label);
        iprintf("\x1b[%d;2H+----------------------------+", bt->row1);
    }
}

static void new_game(void) {
    game_init(&game, (unsigned int)time(NULL));
}

int main(void) {
    videoSetMode(MODE_0_2D);
    videoSetModeSub(MODE_0_2D);
    vramSetBankA(VRAM_A_MAIN_BG);
    vramSetBankC(VRAM_C_SUB_BG);
    consoleInit(&topCon, 0, BgType_Text4bpp, BgSize_T_256x256, 31, 0, true, true);
    consoleInit(&botCon, 0, BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);

    new_game();
    draw_top();
    draw_bottom();

    while (pmMainLoop()) {
        swiWaitForVBlank();
        scanKeys();
        int down = keysDown();
        bool acted = false;

        if (down & KEY_TOUCH) {
            touchPosition t;
            touchRead(&t);
            if (game.state == ST_GAMEOVER) {
                new_game();
                acted = true;
            } else {
                int row = t.py / 8;
                for (int i = 0; i < N_BUTTONS; i++) {
                    if (row >= BUTTONS[i].row0 && row <= BUTTONS[i].row1) {
                        game_act(&game, BUTTONS[i].act);
                        acted = true;
                    }
                }
            }
        }
        /* D-pad / face-button shortcuts */
        if (game.state == ST_BATTLE) {
            if (down & KEY_A) { game_act(&game, ACT_ATTACK); acted = true; }
            if (down & KEY_B) { game_act(&game, ACT_GUARD);  acted = true; }
            if (down & KEY_X) { game_act(&game, ACT_SKILL);  acted = true; }
            if (down & KEY_Y) { game_act(&game, ACT_POTION); acted = true; }
        } else if (down & KEY_START) {
            new_game();
            acted = true;
        }

        if (acted) { draw_top(); draw_bottom(); }
    }
    return 0;
}
