#ifndef __LEVEL_H
#define __LEVEL_H

#include <vector>
#include "json/json-forwards.h"

struct Level {
	int w, h;
	std::string id;
	Json::Value *json;
	Level();
	Level(std::string inLevel, Json::Value *json);
};

struct WorldTip : Drawable {
	Vector origin;
	WorldTip(const Entity *parent, const char *text, Direction dir);
	~WorldTip();
	void Draw(double dt);
};

extern std::vector<WorldTip*> worldtips; 
extern char menuMode[];
extern SDL_Texture *backgroundRLTexture;
extern Entity* posLookup[100][100];

void checkWinLoss();
void cleanLevel();
void generateLevel(std::string &level);
bool loadJSONLevel(std::string &level, Json::Value &root);
void drawBackground(SDL_Renderer *renderer, double dt);
void compileBackground(SDL_Renderer *renderer);
void drawHud(double dt);
void drawHealth();
void handleAction(Entity* ent, Action action);

#endif
