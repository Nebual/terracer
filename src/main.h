#ifndef __MAIN_H
#define __MAIN_H

#include "common.h"

extern int quit;
extern int nextlevel;

int initWindow(SDL_Window **window,SDL_Renderer **renderer, int argc, char *argv[]);
int main(int argc,char *argv[]);

#endif
