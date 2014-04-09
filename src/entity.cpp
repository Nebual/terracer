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

TextureData ballTD;
TextureData blockTDs[127];
TextureData explosionTD;
static Mix_Chunk *bounceSound;
static Mix_Chunk *hitSounds[3];
static Mix_Chunk *failSound;
void initTextures() {
	ballTD = TextureDataCreate("res/ball.png");
	blockTDs[BLOCK_NORMAL] = TextureDataCreate("res/block.png");
	blockTDs[BLOCK_SPEEDUP] = TextureDataCreate("res/block_speedup.png");
	blockTDs[BLOCK_SLOWDOWN] = TextureDataCreate("res/block_slowdown.png");
	blockTDs[BLOCK_RANDOM] = TextureDataCreate("res/block_random.png");
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
	switch(type) {
		case TYPE_PLAYER:
			this->collision = 1;
			break;
		case TYPE_BLOCK:
			this->collision = 1;
			this->health = 100;
			break;
		case TYPE_BALL:
			this->collision = 1;
			this->damage = 100;
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
		case TYPE_BALL: {
			this->Movement(dt);
			
			if((this->pos.x + this->rect.w) > WIDTH) {
				if(this->vel.x > 0) {
					this->vel.x *= -1;
				}
			} else if(this->pos.x < 0) {
				if(this->vel.x < 0) {
					this->vel.x *= -1;
				}
			}
			if((this->pos.y + this->rect.h) > HEIGHT) {
				ballInPlay = NULL;
				playSound(failSound);
				delete this;
				return;
			} else if (this->pos.y < 0){
				if(this->vel.y < 0){
					this->vel.y *= -1;
				}
			}
			
			Entity *hit = this->TestCollision();
			if(hit != NULL) {
				if(DEBUG) printf("Collision Occured: PosX %.3f, PosY: %.3f, Block (%.2f, %.2f), CollisionSizes (%d, %d), distance: %.2f\n", this->pos.x, this->pos.y, hit->pos.x, hit->pos.y, this->collisionSize, hit->collisionSize, this->Distance(hit));
				this->vel.y = abs(this->vel.y) * sign(this->pos.y - hit->pos.y);
				
				if(hit->type == TYPE_PLAYER){
					this->vel.x += hit->vel.x * 0.25;
					playSound(bounceSound);
				} else if(hit->type == TYPE_BLOCK) {
					playSound(hitSounds[rand() % 3]);
					switch(hit->blockType) {
						case BLOCK_SPEEDUP: 
							this->vel.x *= 1.25;
							this->vel.y *= 1.1;
							break;
						case BLOCK_SLOWDOWN: 
							this->vel.x /= 1.25;
							this->vel.y /= 1.1;
							break;
						case BLOCK_RANDOM:
							this->vel.x = this->vel.x * 0.5 + random_range(-250, 250);
							break;
					}
				}
				hit->Damage(this->damage);
			}
			
			break;
		} case TYPE_PLAYER:
			this->Movement(dt);
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
		if(ents[i]->type == TYPE_BALL) continue;
		
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

void GenBall(Entity *ent){
	if(ballInPlay != NULL || balls <= 0) return;
	ballInPlay = new Entity(ballTD, TYPE_BALL, ent->pos.x, (ent->pos.y - 50));
	ballInPlay->vel.x = random_range(-150, 150);
	ballInPlay->vel.y = -300;
	balls--;
}
