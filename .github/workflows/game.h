#ifndef GAME_H
#define GAME_H

#define LOG_LINES 8
#define LOG_WIDTH 32

typedef enum { ACT_ATTACK, ACT_SKILL, ACT_GUARD, ACT_POTION } Action;
typedef enum { ST_BATTLE, ST_GAMEOVER } State;

typedef struct {
    const char *name;
    int hp, atk, def;
} EnemyDef;

typedef struct {
    int hp, maxhp, mp, maxmp, atk, def, potions, gold;
} Player;

typedef struct {
    const char *name;
    int hp, maxhp, atk, def;
} Enemy;

typedef struct {
    State state;
    int floor;
    int guarding;
    unsigned int rng;
    Player p;
    Enemy e;
    char log[LOG_LINES][LOG_WIDTH];
} Game;

void game_init(Game *g, unsigned int seed);
void game_act(Game *g, Action a);

#endif
