#include <stdio.h>
#include <sys/time.h>
#include <math.h>
#include <getopt.h>

#include <SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>


#include "util.h"
#include "entity.h"
#include "main.h"
#include "input.h"
#include "level.h"

double jumping = 0;
int playerOnGround = 0;

void handleKeyboard(double dt, Entity *ply) {
	static int keysPressed[255];
	
	static SDL_Event keyevent;
	while(SDL_PollEvent(&keyevent)) {
		switch(keyevent.type) {
			case SDL_QUIT:
				quit = 1;
				return;
			case SDL_KEYDOWN:
				keysPressed[keyevent.key.keysym.scancode] = 1;
				if(DEBUG) printf("Key pressed: %d\n", keyevent.key.keysym.scancode);
				
				switch(keyevent.key.keysym.sym) {
					case SDLK_c:
					case SDLK_q:
						if(keyevent.key.keysym.mod & KMOD_CTRL) {quit = 1;}
						break;
					case SDLK_h:
						if(DEBUG) printf("Starting GC, old entsC: %d", entsC);
						Entity::GC();
						if(DEBUG) printf(", new entsC: %d\n", entsC);
						break;
					case SDLK_n:
						if(menuMode[0] != '\0'){
							quit = 1;
							return;
						}
						break;
					case SDLK_y:
						if(menuMode[0] != '\0'){
							curLevel = 1;
							generateLevel(curLevel);
							return;
						}
						break;
					case SDLK_SPACE:
						if(playerOnGround) {
							jumping = 0.000000001;
							ply->vel.y = -275;
						}
						break;
				
				}
				break;
			case SDL_KEYUP:
				keysPressed[keyevent.key.keysym.scancode] = 0;
				
				switch(keyevent.key.keysym.sym) {
					case SDLK_SPACE:
						jumping = 0;
						break;
				}
		}
	}
	
	if(playerOnGround) {
		if(keysPressed[SDL_SCANCODE_A] || keysPressed[SDL_SCANCODE_LEFT]) {
			ply->vel.x = -PLAYER_MAX_SPEED;
		}
		else if(keysPressed[SDL_SCANCODE_D] || keysPressed[SDL_SCANCODE_RIGHT]) {
			ply->vel.x = PLAYER_MAX_SPEED;
		}
		else {
			ply->vel.x = 0;
		}
	} else {
		if(keysPressed[SDL_SCANCODE_A] || keysPressed[SDL_SCANCODE_LEFT]) {
			ply->vel.x = max(ply->vel.x - PLAYER_AIR_ACCEL*dt, -PLAYER_MAX_SPEED);
		}
		else if(keysPressed[SDL_SCANCODE_D] || keysPressed[SDL_SCANCODE_RIGHT]) {
			ply->vel.x = min(ply->vel.x + PLAYER_AIR_ACCEL*dt, PLAYER_MAX_SPEED);
		}
		else {
			ply->vel.x -= sign(ply->vel.x) * PLAYER_AIR_ACCEL/2*dt;
		}
	}
	
	if(jumping) {
		jumping += dt;
		ply->vel.y -= GRAVITY_ACCEL*dt * (PLAYER_JUMP_TIME - jumping/2)/PLAYER_JUMP_TIME;
		if(jumping > PLAYER_JUMP_TIME) {
			jumping = 0;
		}
	}
	ply->vel.y += GRAVITY_ACCEL*dt;
}
