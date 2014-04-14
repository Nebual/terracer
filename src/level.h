#ifndef __LEVEL_H
#define __LEVEL_H

extern int curLevel;
extern char menuMode[];

void checkWinLoss();
void generateLevel(int level);
void loadJSONLevel(int level);
void drawBackground(SDL_Renderer *renderer, double dt);
void drawHud(double dt);

#endif
