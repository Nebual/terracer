#include "common.h"

Entity *ents[MAX_ENTITIES];
int entsC = 0;
Drawable *renderLayers[RL_MAX][MAX_ENTITIES];
int renderLayersC[RL_MAX];

SDL_Renderer *renderer;
Player *ply;
Hud *hud;
SDL_Rect camera = {0,0,0,0};

int WIDTH, HEIGHT, WINDOW_WIDTH, WINDOW_HEIGHT, WIDTH_OFFSET, HEIGHT_OFFSET;
