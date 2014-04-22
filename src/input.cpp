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

Player::Player(TextureData texdata, int x, int y) : Entity(texdata, x, y) {
	this->onGround = 0;
	this->jumpTime = 0;
		
	this->collision = 1;
	this->health = 1;
}
void Player::SetupRenderLayer() {
	this->renderLayer = RL_FOREGROUND;
	Entity::SetupRenderLayer();
}
void Player::Update(double dt) {
	this->HandleKeyboard(dt);
	
	if(abs(this->vel.y) > JUMP_THRESHOLD) {ply->onGround = 0;}
			
	Direction collideDir;
	for (int i=0; i < MAX_COLLISION_ITERATIONS; i++) {
		collideDir = (Direction) 0;
		Entity *hit = this->CollisionMovement(collideDir, dt);
		if(hit != NULL) {
			if(collideDir & DOWN) ply->onGround = 1;
		} else {
			break;
		}
	}
	this->Movement(dt);
	
	// Clamp movement to sides of level
	if((this->pos.x + this->rect.w) > curLevel->w) {
		this->pos.x = curLevel->w - this->rect.w;
	}
	else if(this->pos.x < 0) {
		this->pos.x = 0;
	}
	
	Entity::Update(dt);
}

void Player::HandleKeyboard(double dt) {
	Player* ply = this;
	static int keysPressed[255];
	
	static SDL_Event keyevent;
	while(SDL_PollEvent(&keyevent)) {
		switch(keyevent.type) {
			case SDL_QUIT:
				quit = 1;
				return;
			case SDL_KEYDOWN:
				keysPressed[keyevent.key.keysym.scancode] = 1;
				
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
							generateLevel(curLevel->id);
							return;
						}
						break;
					case SDLK_SPACE:
						if(ply->onGround) {
							ply->jumpTime = 0.000000001;
							ply->vel.y = -275;
						}
						break;				
				}
				break;
			case SDL_KEYUP:
				keysPressed[keyevent.key.keysym.scancode] = 0;
				
				switch(keyevent.key.keysym.sym) {
					case SDLK_SPACE:
						ply->jumpTime = 0;
						break;
					case SDLK_e:
						ply->interact();
						break;	
				}
		}
	}
	
	if(ply->onGround) {
		if(keysPressed[SDL_SCANCODE_A] || keysPressed[SDL_SCANCODE_LEFT]) {
			ply->vel.x = -PLAYER_MAX_SPEED;
			ply->face(LEFT);
		}
		else if(keysPressed[SDL_SCANCODE_D] || keysPressed[SDL_SCANCODE_RIGHT]) {
			ply->vel.x = PLAYER_MAX_SPEED;
			ply->face(RIGHT);
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
	
	if(ply->jumpTime) {
		ply->jumpTime += dt;
		ply->vel.y -= GRAVITY_ACCEL*dt * (PLAYER_JUMP_TIME - ply->jumpTime/2)/PLAYER_JUMP_TIME;
		if(ply->jumpTime > PLAYER_JUMP_TIME) {
			ply->jumpTime = 0;
		}
	}
	ply->vel.y += GRAVITY_ACCEL*dt;


	// Camera
	int plyPosX = ply->pos.x + ply->rect.w - camera.x;
	int plyPosY = ply->pos.y + ply->rect.h - camera.y;
	if(plyPosX > (camera.w/2 + CAM_DEADZONE)) {
		camera.x += (plyPosX - (camera.w/2 + CAM_DEADZONE)) * CAM_SPEED * dt;
	} else if(plyPosX < (camera.w/2 - CAM_DEADZONE)) {
		camera.x += (plyPosX - (camera.w/2 - CAM_DEADZONE)) * CAM_SPEED * dt;
	} if(plyPosY > (camera.h/2 + CAM_DEADZONE)) {
		camera.y += (plyPosY - (camera.h/2 + CAM_DEADZONE)) * CAM_SPEED * dt;
	} else if(plyPosY < (camera.h/2 - CAM_DEADZONE)) {
		camera.y += (plyPosY - (camera.h/2 - CAM_DEADZONE)) * CAM_SPEED * dt;
	}
	// Clamp camera to sides of level
	if(camera.x < 0) camera.x = 0;
	if(camera.x > (curLevel->w - WIDTH)) camera.x = curLevel->w - WIDTH;
	if(camera.y < 0) camera.y = 0;
	if(camera.y > (curLevel->h - HEIGHT)) camera.y = curLevel->h - HEIGHT;
}
