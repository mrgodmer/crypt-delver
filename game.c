#include "game.h"
#include <stdio.h>
#include <string.h>

#define SKILL_COST 4
#define POTION_HEAL 15

static const EnemyDef ENEMIES[] = {
    { "Bone Rat",     14, 4, 0 },
    { "Moss Ghoul",   20, 5, 1 },
    { "Ash Wraith",   18, 7, 0 },
    { "Stone Sentry", 30, 6, 3 },
    { "Hollow Knight",40, 9, 2 },
};
#define N_ENEMIES ((int)(sizeof(ENEMIES) / sizeof(ENEMIES[0])))

static int rnd(Game *g, int n) {
    g->rng = g->rng * 1664525u + 1013904223u;
    return (int)((g->rng >> 16) % (unsigned)n);
}

static void say(Game *g, const char *fmt, int a, int b) {
    for (int i = 0; i < LOG_LINES - 1; i++)
        strcpy(g->log[i], g->log[i + 1]);
    snprintf(g->log[LOG_LINES - 1], LOG_WIDTH, fmt, a, b);
}

static void say_s(Game *g, const char *fmt, const char *s) {
    for (int i = 0; i < LOG_LINES - 1; i++)
        strcpy(g->log[i], g->log[i + 1]);
    snprintf(g->log[LOG_LINES - 1], LOG_WIDTH, fmt, s);
}

static void spawn(Game *g) {
    const EnemyDef *d = &ENEMIES[rnd(g, N_ENEMIES)];
    int scale = g->floor - 1;
    g->e.name = d->name;
    g->e.maxhp = d->hp + scale * 4;
    g->e.hp = g->e.maxhp;
    g->e.atk = d->atk + scale / 2;
    g->e.def = d->def + scale / 4;
    say_s(g, "A %s appears!", g->e.name);
}

void game_init(Game *g, unsigned int seed) {
    memset(g, 0, sizeof(*g));
    g->rng = seed ? seed : 12345u;
    g->state = ST_BATTLE;
    g->floor = 1;
    g->p.maxhp = g->p.hp = 40;
    g->p.maxmp = g->p.mp = 10;
    g->p.atk = 8;
    g->p.def = 2;
    g->p.potions = 3;
    spawn(g);
}

static int calc_damage(Game *g, int atk, int def) {
    int d = atk - def + rnd(g, 3);
    return d < 1 ? 1 : d;
}

static void enemy_turn(Game *g) {
    int d = calc_damage(g, g->e.atk, g->p.def);
    if (g->guarding) d /= 2;
    if (d < 1) d = 1;
    g->p.hp -= d;
    say(g, "Enemy hits you for %d.%.0d", d, 0);
    g->guarding = 0;
    if (g->p.hp <= 0) {
        g->p.hp = 0;
        g->state = ST_GAMEOVER;
        say(g, "You fell on floor %d.%.0d", g->floor, 0);
    }
}

static void enemy_defeated(Game *g) {
    int gold = 3 + g->floor * 2;
    g->p.gold += gold;
    say(g, "Victory! +%d gold.%.0d", gold, 0);
    g->floor++;
    g->p.hp += g->p.maxhp / 4;
    if (g->p.hp > g->p.maxhp) g->p.hp = g->p.maxhp;
    g->p.mp += 3;
    if (g->p.mp > g->p.maxmp) g->p.mp = g->p.maxmp;
    if (rnd(g, 3) == 0) {
        g->p.potions++;
        say(g, "Found a potion!%.0d%.0d", 0, 0);
    }
    if (g->floor % 3 == 0) { g->p.atk++; g->p.maxhp += 5; }
    spawn(g);
}

void game_act(Game *g, Action a) {
    if (g->state != ST_BATTLE) return;
    int d;
    switch (a) {
    case ACT_ATTACK:
        d = calc_damage(g, g->p.atk, g->e.def);
        g->e.hp -= d;
        say(g, "You strike for %d.%.0d", d, 0);
        break;
    case ACT_SKILL:
        if (g->p.mp < SKILL_COST) { say(g, "Not enough MP!%.0d%.0d", 0, 0); return; }
        g->p.mp -= SKILL_COST;
        d = calc_damage(g, g->p.atk * 2, g->e.def);
        g->e.hp -= d;
        say(g, "Power Strike! %d dmg.%.0d", d, 0);
        break;
    case ACT_GUARD:
        g->guarding = 1;
        g->p.mp += 2;
        if (g->p.mp > g->p.maxmp) g->p.mp = g->p.maxmp;
        say(g, "You brace yourself.%.0d%.0d", 0, 0);
        break;
    case ACT_POTION:
        if (g->p.potions <= 0) { say(g, "No potions left!%.0d%.0d", 0, 0); return; }
        g->p.potions--;
        g->p.hp += POTION_HEAL;
        if (g->p.hp > g->p.maxhp) g->p.hp = g->p.maxhp;
        say(g, "Healed %d HP.%.0d", POTION_HEAL, 0);
        break;
    }
    if (g->e.hp <= 0) { g->e.hp = 0; enemy_defeated(g); return; }
    enemy_turn(g);
}
