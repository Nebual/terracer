#include <stdio.h>
#include <sys/time.h>
#include <math.h>
#include <stdlib.h>

#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#include "entity.h"
#include "main.h"
#include "util.h"

TTF_Font *prototype;

SDL_Color WHITE;

void initFonts(){
	prototype = TTF_OpenFont("res/fonts/Prototype.ttf", 24);
	
	WHITE = {255,255,255};
}

void fpsCounter() {	
	static int curSec;
	static int frames = 0;
	if(curtime() != curSec) {
		curSec = curtime();
		printf("FPS: %d\n", frames);
		frames = 0;
	}
	frames++;
}

short int sign(double i) {
	return i > 0 ? 1 : -1;
}

int min(int a, int b) {
	return a < b ? a : b;
}
int max(int a, int b) {
	return a > b ? a : b;
}

double clamp(double val, double min, double max) {
	if(val < min) return min;
	if(val > max) return max;
	return val;
}

double angnorm(double ang) {
	return fmod(ang + 180, 360) - 180;
}

int curtime() {
	static struct timeval curtime;
	gettimeofday(&curtime, NULL);
	return curtime.tv_sec;
}
int curtime_u() {
	static struct timeval curtime;
	gettimeofday(&curtime, NULL);
	return curtime.tv_usec;
}

int random_range(int min, int max){
	return rand() % (max-min) + min;
}

void playSound(Mix_Chunk *snd) {
	#ifndef __arm__
	if(snd == NULL || Mix_PlayChannel(-1, snd, 0 ) == -1) {printf("Snd loading error: %s\n", Mix_GetError());}
	#endif
}

void displayText(int x, int y, const char text[], SDL_Color color){
	if(SDL_Texture* texture = readyText(x, y, text, color)){
		SDL_Rect rect = {x, y, 0, 0};
		SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
		
		int ret = SDL_RenderCopy(renderer, texture, NULL, &rect);
		if(ret != 0) {printf("Render failed: %s\n", SDL_GetError());}
		
	}

}

SDL_Texture* readyText(int x, int y, const char text[], SDL_Color color){
	SDL_Surface *text_surface;
	if(!(text_surface=TTF_RenderText_Solid(prototype, text, color))) {
		printf("TTF_Init: %s\n", TTF_GetError());
		return 0;
	} else {
		SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, text_surface);
		SDL_FreeSurface(text_surface);
		
		return texture;
	}	
}

void displayTextCentered(int x, int y, const char text[], SDL_Color color){
	if(SDL_Texture* texture = readyText(x, y, text, color)){
		SDL_Rect rect = {x, y, 0, 0};
		SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
		
		rect.x -= rect.w/2;
		
		int ret = SDL_RenderCopy(renderer, texture, NULL, &rect);
		if(ret != 0) {printf("Render failed: %s\n", SDL_GetError());}
		
	}
}

Timer* timers[256]; int timersC = 0;

void TimerRun() {
	Uint32 curtimeMS = SDL_GetTicks();
	for(int i=0; i<timersC; i++) {
		if(timers[i] == NULL) continue;
		if(timers[i]->endTime <= curtimeMS) {
			timers[i]->callback();
			timers[i]->endTime = curtimeMS + timers[i]->delay;
			if(timers[i]->reps != 0) { // reps == 0 means "do forever"
				timers[i]->reps--;
				if(timers[i]->reps <= 0) {timers[i] = NULL;}
			}
		}
	}
}

void TimerCreate(int id, Uint32 delay, short int reps, void (*callback)()) {
	//Timer timer = {id, SDL_GetTicks() + delay, delay, reps, callback};
	Timer *timer = (Timer*) malloc(sizeof(Timer));
	timer->id = id;
	timer->endTime = SDL_GetTicks() + delay;
	timer->delay = delay;
	timer->reps = reps;
	timer->callback = callback;
	timers[timersC] = timer; timersC++;
}
void TimerDestroy(int id) {
	for(int i=0; i<timersC; i++) {
		if(timers[i] == NULL) continue;
		if(timers[i]->id == id) {
			timers[i] = NULL;
			break;
		}
	}
}
