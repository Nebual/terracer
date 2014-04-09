#ifndef __LEVEL_H
#define __LEVEL_H

extern int curLevel;
extern int balls;
extern Entity *ballInPlay;
extern char menuMode[];

void checkWinLoss();
int blocksRemain();
void generateLevel(int level);
void drawBackground(SDL_Renderer *renderer, double dt);
void drawHud(double dt);

#endif
