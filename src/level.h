#ifndef __LEVEL_H
#define __LEVEL_H

#include "json/json-forwards.h"

extern int curLevel;
extern char menuMode[];

void checkWinLoss();
void generateLevel(int level);
bool loadJSONLevel(int level, Json::Value &root);
void drawBackground(SDL_Renderer *renderer, double dt);
void drawHud(double dt);
void drawHealth();

#endif
