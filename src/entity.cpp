#include <stdio.h>
#include <sys/time.h>
#include <math.h>
#ifdef _WIN32
	#include <windows.h>
#endif

#include <SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#include "util.h"
#include "entity.h"
#include "main.h"
#include "level.h"
#include "input.h"

const int MAX_COLLISION_ITERATIONS = 3;
const int INTERACT_DIST = 50;

TextureData ballTD;
TextureData blockTDs[127];
TextureData explosionTD;
static Mix_Chunk *bounceSound;
static Mix_Chunk *hitSounds[3];
static Mix_Chunk *failSound;
void initTextures() {
	ballTD = TextureDataCreate("res/ball.png");
	blockTDs[BLOCK_NORMAL] = TextureDataCreate("res/block.png");
	blockTDs[BLOCK_TOUGH] = TextureDataCreate("res/block_tough.png");

	explosionTD.texture = IMG_LoadTexture(renderer, "res/explosion_50.png");
	explosionTD.animMaxFrames = 36;
	explosionTD.w = 50; explosionTD.h = 50;
	explosionTD.animWidth = 8;
	explosionTD.animDuration = 2;

	bounceSound = Mix_LoadWAV("res/sounds/bounce.ogg");
	hitSounds[0] = Mix_LoadWAV("res/sounds/hit1.wav");
	hitSounds[1] = Mix_LoadWAV("res/sounds/hit2.wav");
	hitSounds[2] = Mix_LoadWAV("res/sounds/hit3.wav");
	failSound = Mix_LoadWAV("res/sounds/fail.ogg");
}

TextureData TextureDataCreate(const char texturePath[]) {
	TextureData data = {NULL, 0, 0, 0, 0, 0};
	data.texture = IMG_LoadTexture(renderer, texturePath);
	if (!data.texture) {fprintf(stderr, "Couldn't load %s: %s\n", texturePath, SDL_GetError());}
	SDL_SetTextureBlendMode(data.texture, SDL_BLENDMODE_BLEND);
	SDL_QueryTexture(data.texture, NULL, NULL, &data.w, &data.h);
	
	return data;
}

Entity::Entity(TextureData texdata, Type type, int x, int y) {
	this->texture = texdata.texture;
	this->rect = (SDL_Rect) {x,y,texdata.w,texdata.h};
	this->pos = (Vector) {(double) x,(double) y};
	this->vel = (Vector) {0,0};
	this->type = type;
	this->blockType = BLOCK_NONE;
	this->collision = 0;
	this->collisionSize = (this->rect.w + this->rect.h) / 4; // Average of widthheight / 2
	this->damage = 0;
	this->health = 0;
	this->deathTime = 0;
	this->animTime = 0;
	this->animDuration = 0;
	this->animMaxFrames = 0;
	this->action = NO_ACTION;
	switch(type) {
		case TYPE_PLAYER:
			this->collision = 1;
			break;
		case TYPE_BLOCK:
			this->collision = 1;
			this->health = 100;
			break;
	}
	
	ents[entsC] = this; entsC++;
}
Entity::~Entity() {
	// TODO: These should probably utilize an internal entID and rlID, but then GC would be harder
	for(int i=0; i<entsC; i++) {
		if(ents[i] == this) {ents[i] = NULL; break;}
	}
	//SDL_DestroyTexture(this->texture); // We're caching now
}

void Entity::GC() {
	// HO BOY
	//Garbage collector
	int newC = 0;
	for(int i=0; i<entsC; i++) {
		if(ents[i] != NULL) {
			ents[newC] = ents[i];
			if(i != newC) {ents[i] = NULL;}
			newC++;
		}
	}
	entsC = newC;
}
SDL_Rect* EntityGetFrame(Entity *ent, double dt) {
	if(ent->animDuration != 0) {
		static SDL_Rect srcRect;
		static int curFrame;
		
		ent->animTime += dt;
		curFrame = ent->animMaxFrames * (ent->animTime / ent->animDuration);
		//printf("Test (%.4f, %.4f, %.4f, %d, %d)\n", ent->animTime, ent->animDuration, ent->animTime / ent->animDuration, ent->animMaxFrames, curFrame);
		srcRect = (SDL_Rect) {ent->rect.w * (curFrame % 8), ent->rect.h * (curFrame / 8), ent->rect.w, ent->rect.h};
		//if(ent->animFrame > ent->animFrameMax*4) {ent->animFrame = 0;}
		return &srcRect;
	}
	return NULL;
}
void Entity::Draw(double dt) {
	this->rect.x = this->pos.x;
	this->rect.y = this->pos.y;
	int ret;
	//if(ent->type == TYPE_BALL || ent->type == TYPE_PLAYER) { // For some reason, it didn't like switching between the methods on a per-entity basis (using ent->ang != 0)
	//	ret = SDL_RenderCopyEx(renderer, ent->texture, EntityGetFrame(ent, dt), &ent->rect, ent->ang, NULL, SDL_FLIP_NONE);
	//} else{
		ret = SDL_RenderCopy(renderer, this->texture, EntityGetFrame(this, dt), &this->rect);
	//}
	if(ret != 0) {printf("Render failed: %s\n", SDL_GetError());}
}
void Entity::Update(double dt) {
	if(this->deathTime != 0 && this->deathTime < SDL_GetTicks()) {
		delete this;
		return;
	}
	switch(this->type) {
		case TYPE_PLAYER:
			if(abs(this->vel.y) > JUMP_THRESHOLD) {playerOnGround = 0;}
			
			Direction collideDir;
			for (int i=0; i < MAX_COLLISION_ITERATIONS; i++) {
				collideDir = (Direction) 0;
				Entity *hit = this->CollisionMovement(collideDir, dt);
				if(hit != NULL) {
					if(collideDir & DOWN) playerOnGround = 1;
				} else {
					break;
				}
			}
			this->Movement(dt);
			
			// Clamp movement to sides of level (for now)
			if((this->pos.x + this->rect.w) > WIDTH) {
				this->pos.x = WIDTH - this->rect.w;
			}
			else if(this->pos.x < 0) {
				this->pos.x = 0;
			}
			break;
	}
}
void Entity::Movement(double dt) {
	this->pos.x += this->vel.x * dt;
	this->pos.y += this->vel.y * dt;
}
Entity* Entity::TestCollision() {
	for(int i=0; i<entsC; i++) {
		if(ents[i] == NULL || ents[i]->collision == 0 || this == ents[i]) continue;
		
		// Rect collisions
		double maxX = max(ents[i]->pos.x, this->pos.x);
		double minX = min(ents[i]->pos.x + ents[i]->rect.w, this->pos.x + this->rect.w);
		double maxY = max(ents[i]->pos.y, this->pos.y);
		double minY = min(ents[i]->pos.y + ents[i]->rect.h, this->pos.y + this->rect.h);
		if(maxX < minX && maxY < minY) {return ents[i];}
		
		// Happy fun Circle collisions :D :D
		//if(this->Distance(ents[i]) < (ents[i]->collisionSize + this->collisionSize)) {return ents[i];}
	}
	return NULL;
}

int Entity::ContainsPoint(double x, double y) {
	return (x < (this->pos.x + this->rect.w) && 
		x > this->pos.x &&
		y < (this->pos.y + this->rect.h) &&
		y > this->pos.y);
}

Entity* Entity::CollisionMovement(Direction &collideDir, double dt) {
	static Vector points[9] = {
		{(double) this->rect.w/2, 0}, // Top (1 point)
		{(double) this->rect.w/4, (double) this->rect.h}, {(double) this->rect.w * (3.0d/4), (double) this->rect.h}, // Bottom (2 points)
		{0, (double) this->rect.h/4}, {0, (double) this->rect.h/2}, {0, (double) this->rect.h * (3.0d/4)}, // Left (3 points)
		{(double) this->rect.w, (double) this->rect.h/4}, {(double) this->rect.w, (double) this->rect.h/2}, {(double) this->rect.w, (double) this->rect.h * (3.0d/4)} // Right (3 points)
	};
	
	double originalMoveX, originalMoveY, nextMoveX, nextMoveY;
	originalMoveX = nextMoveX = this->vel.x * dt;
	originalMoveY = nextMoveY = this->vel.y * dt;

	Entity* hit;
	for (int enti = 0; enti < entsC; enti++) {
		if(ents[enti] == NULL || ents[enti]->collision == 0 || this == ents[enti]) continue;
		// Test the four possible directions of player movement individually (0 = top, 1 = bottom, 2 = left, 3 = right)
		for (int dir = 0; dir < 4; dir++) {
			// Skip the test if the expected direction of movement makes the test irrelevant
			if (dir == 0 && nextMoveY > 0) continue; // Moving Down
			if (dir == 1 && nextMoveY < 0) continue; // Moving Up
			if (dir == 2 && nextMoveX > 0) continue; // Moving Right
			if (dir == 3 && nextMoveX < 0) continue; // Moving Left

			// Traverse backwards in X or Y (but not both at the same time)
			// until the player is no longer colliding with the geometry
			// Note: This code also enables walking up gently sloping surfaces:
			// as the force of gravity pulls down on the player and causes surface contact,
			// the correction pushes the player away from the inside of the platform up to the surface.
			if(dir == 0) {
				while (ents[enti]->ContainsPoint(points[0].x + this->pos.x, points[0].y + this->pos.y + nextMoveY)) {
					nextMoveY++;
				}
			} else if (dir == 1) {
				while (ents[enti]->ContainsPoint(points[1].x + this->pos.x, points[1].y + this->pos.y + nextMoveY)
					|| ents[enti]->ContainsPoint(points[2].x + this->pos.x, points[2].y + this->pos.y + nextMoveY)) {
					nextMoveY--;
				}
			} else if (dir == 2) {
				while (ents[enti]->ContainsPoint(points[3].x + this->pos.x + nextMoveX, points[3].y + this->pos.y)
					|| ents[enti]->ContainsPoint(points[4].x + this->pos.x + nextMoveX, points[4].y + this->pos.y)
					|| ents[enti]->ContainsPoint(points[5].x + this->pos.x + nextMoveX, points[5].y + this->pos.y)) {
					nextMoveX++;
				}
			} else if (dir == 3) {
				while (ents[enti]->ContainsPoint(points[6].x + this->pos.x + nextMoveX, points[6].y + this->pos.y)
					|| ents[enti]->ContainsPoint(points[7].x + this->pos.x + nextMoveX, points[7].y + this->pos.y)
					|| ents[enti]->ContainsPoint(points[8].x + this->pos.x + nextMoveX, points[8].y + this->pos.y)) {
					nextMoveX--;
				}
			}
		}

		// Detect what type of contact has occurred based on a comparison of
		// the original expected movement vector and the new one
		if (nextMoveY > originalMoveY && originalMoveY < 0) {
			collideDir = collideDir | UP;
		}
		else if (nextMoveY < originalMoveY && originalMoveY > 0) {
			collideDir = collideDir | DOWN;
		}
		if (nextMoveX > originalMoveX && originalMoveX < 0) {
			collideDir = collideDir | RIGHT;
		}
		else if (nextMoveX < originalMoveX && originalMoveX > 0) {
			collideDir = collideDir | LEFT;
		}
		if(collideDir) { // If we hit anything at all
			hit = ents[enti];
			break;
		}
	}

	// If a contact has been detected, apply the re-calculated movement vector
	// and disable any further movement this frame (in either X or Y as appropriate)
	if(collideDir) {
		if (collideDir & (UP | DOWN)) { // ContactY +/-
			this->pos.y += nextMoveY;
			this->vel.y = 0;
		}
		if (collideDir & (LEFT | RIGHT)) { // ContactX +/-
			this->pos.x += nextMoveX;
			this->vel.x = 0;
		}
	}
	
	return hit;
}

void Entity::Damage(int damage) {
	if(this->health <= 0) return;
	this->health -= damage;
	if(this->health <= 0) {
		this->type = TYPE_EXPLOSION;
		this->texture = explosionTD.texture;
		this->rect.w = explosionTD.w; this->rect.h = explosionTD.h;
		this->pos.y -= 12;
		this->animDuration = explosionTD.animDuration;
		this->animMaxFrames = explosionTD.animMaxFrames;
		this->collision = 0;
		
		this->DeathClock(explosionTD.animDuration * 1000);
	}
}
void Entity::DeathClock(int delay) {
	this->deathTime = SDL_GetTicks() + delay;
}

double Entity::Distance(Entity *ent2) {
	return pow(pow(this->rect.x - ent2->rect.x, 2) + powf(this->rect.y - ent2->rect.y, 2), 0.5);
}

void Entity::use(){
	if(DEBUG){printf("Action executed: %d\n", action);}	
	switch(action){
		case PLY_HEALTH_UP:
			ply->health += 25;
			break;
	}
}

Entity* Entity::closestInteractable(int minDist) {
	Entity *closest = NULL;
	int dist = minDist;
	for(int i=0; i<entsC; i++) {
		if(ents[i] == NULL || this == ents[i] || ents[i]->action == NO_ACTION) {continue;}

		if(this->Distance(ents[i]) < dist) {
			dist = this->Distance(ents[i]);
			closest = ents[i];
		}
	}
	
	return closest;
}

void Entity::interact() {
	Entity *closest = closestInteractable(INTERACT_DIST);
	if(closest == NULL) {return;}
	if(closest->action == NO_ACTION) {return;}
	closest->use();
}

inline Direction operator|(Direction a, Direction b) {return static_cast<Direction>(static_cast<int>(a) | static_cast<int>(b));}
