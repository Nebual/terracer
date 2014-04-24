#ifndef __MAIN_H
#define __MAIN_H 1

// Forward declarations
class Entity;
class Player;

extern SDL_Renderer *renderer;

extern Entity *ents[MAX_ENTITIES];
extern int entsC;

extern Drawable *renderLayers[RL_MAX][MAX_ENTITIES];
extern int renderLayersC[RL_MAX];

extern int quit;
extern Player *ply;
extern Hud *hud;
extern SDL_Rect camera;

extern int WIDTH, HEIGHT, WINDOW_WIDTH, WINDOW_HEIGHT, WIDTH_OFFSET, HEIGHT_OFFSET;


int initWindow(SDL_Window **window,SDL_Renderer **renderer, int argc, char *argv[]);
int main(int argc,char *argv[]);

#ifndef DEBUG
#define DEBUG 0
#endif

#endif
