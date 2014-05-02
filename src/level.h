#ifndef __LEVEL_H
#define __LEVEL_H

#include "json/json-forwards.h"

struct Level {
	int w, h;
	int id;
	Json::Value *json;
	Level();
	Level(int inLevel, Json::Value *json);
};

extern char menuMode[];
extern SDL_Texture *backgroundRLTexture;

void checkWinLoss();
void generateLevel(int level);
bool loadJSONLevel(int level, Json::Value &root);
void drawBackground(SDL_Renderer *renderer, double dt);
void compileBackground(SDL_Renderer *renderer);
void drawHud(double dt);
void drawHealth();
void handleAction(Entity* ent, Action action);

#endif
