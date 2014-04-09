#ifndef __MAIN_H
#define __MAIN_H 1

extern SDL_Renderer *renderer;

extern Entity *ents[512];
extern int entsC;

extern int quit;
extern Entity *ply;

extern int WIDTH;
extern int HEIGHT;


int initWindow(SDL_Window **window,SDL_Renderer **renderer, int argc, char *argv[]);
int main(int argc,char *argv[]);

#ifndef DEBUG
#define DEBUG 0
#endif

#endif
