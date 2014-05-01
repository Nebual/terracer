#include <stdio.h>
#include <sys/time.h>
#include <math.h>
#include <typeinfo>
#ifdef _WIN32
	#include <windows.h>
#endif

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#include "util.h"
#include "entity.h"
#include "level.h"
#include "player.h"

/* ================= */
/*  	Textures   	 */
/* ================= */


std::map <std::string, TextureData> blockTDs;
TextureData explosionTD;
TextureData playerTD;
TextureData heart_fullTD;
TextureData heart_emptyTD;
TextureData goombaTD;

void initTextures() {
	blockTDs["dirt"] = TextureDataCreate("res/dirt.png");
	blockTDs["stone"] = TextureDataCreate("res/stone.png");
	blockTDs["sand"] = TextureDataCreate("res/sand.png");
	blockTDs["sandstone"] = TextureDataCreate("res/sandstone.png");
	blockTDs["sandstone_bg"] = TextureDataCreate("res/sandstone_bg.png");

	for(auto it=blockTDs.begin(); it != blockTDs.end(); ++it) {
		SDL_SetTextureBlendMode(it->second.texture, SDL_BLENDMODE_NONE);
	}
	
	goombaTD = TextureDataCreate("res/goomba.png");

	explosionTD.texture = IMG_LoadTexture(renderer, "res/explosion_50.png");
	explosionTD.animMaxFrames = 36;
	explosionTD.w = 50; explosionTD.h = 50;
	explosionTD.animWidth = 8;
	explosionTD.animDuration = 2;
	
	playerTD = TextureDataCreate("res/player_right.png", "res/player_left.png", "res/player_right.png");
	
	heart_fullTD = TextureDataCreate("res/heart_full.png");
	heart_emptyTD = TextureDataCreate("res/heart_empty.png");
}

TextureData TextureDataCreate(const char texturePath[], const char leftPath[], const char rightPath[]) {
	TextureData data = {NULL, NULL, NULL, 0, 0, 0, 0, 0};
	data.texture = IMG_LoadTexture(renderer, texturePath);
	if (!data.texture) {fprintf(stderr, "Couldn't load %s: %s\n", texturePath, SDL_GetError());}
	data.left = IMG_LoadTexture(renderer, leftPath);
	if (!data.left) {data.left = data.texture;}
	data.right = IMG_LoadTexture(renderer, rightPath);
	if (!data.right) {data.right = data.texture;}
	
	SDL_SetTextureBlendMode(data.texture, SDL_BLENDMODE_BLEND);
	SDL_SetTextureBlendMode(data.left, SDL_BLENDMODE_BLEND);
	SDL_SetTextureBlendMode(data.left, SDL_BLENDMODE_BLEND);
	SDL_QueryTexture(data.texture, NULL, NULL, &data.w, &data.h);
	
	return data;
}

/* ================= */
/*  	Entity   	 */
/* ================= */


Entity::Entity(TextureData &texdata, int x, int y, RenderLayer rl) : Drawable(texdata, x, y) {
	this->pos = (Vector) {(double) x,(double) y};
	this->collision = 1;
	this->collisionSize = (this->rect.w + this->rect.h) / 2; // Average of widthheight / 2
	this->damage = 0;
	this->health = 100;
	this->deathTime = 0;
	this->facing = RIGHT;
	
	this->renderLayer = rl;
	
	ents[entsC++] = this;
	renderLayers[this->renderLayer][renderLayersC[this->renderLayer]++] = this;
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
	
	for(int rl = 0; rl < RL_MAX; rl++){
		newC = 0;
		for(int i = 0; i < renderLayersC[rl]; i++){
			if(renderLayers[rl][i] != NULL){
				renderLayers[rl][newC] = renderLayers[rl][i];
				if(i != newC) { renderLayers[rl][i] = NULL;}
				newC++;
			}
		}
		renderLayersC[rl] = newC;
	}
	
}
void Entity::Draw(double dt) {
	this->rect.x = this->pos.x - camera.x;
	this->rect.y = this->pos.y - camera.y;
	static int ret;
	//if(this->type == TYPE_BALL || this->type == TYPE_PLAYER) { // For some reason, it didn't like switching between the methods on a per-entity basis (using ent->ang != 0)
	//	ret = SDL_RenderCopyEx(renderer, this->texture, this->GetFrame(dt), &this->rect, this->ang, NULL, SDL_FLIP_NONE);
	//} else{
		ret = SDL_RenderCopy(renderer, this->texture, this->GetFrame(dt), &this->rect);
	//}
	if(ret != 0) {printf("Render failed: %s\n", SDL_GetError());}
}
void Entity::Update(double dt) {
	if(this->deathTime != 0 && this->deathTime < SDL_GetTicks()) {
		delete this;
		return;
	}
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

void Entity::Damage(int damage) {
	if(this->health <= 0) return;
	this->health -= damage;
	if(this->health <= 0) {
		//this->type = TYPE_EXPLOSION;
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
	return pow(pow(this->pos.x - ent2->pos.x, 2) + powf(this->pos.y - ent2->pos.y, 2), 0.5);
}

Interactable* Entity::closestInteractable(int minDist) {
	Interactable *closest = NULL;
	int dist = minDist;
	for(int i=0; i<entsC; i++) {
		if(ents[i] == NULL || this == ents[i]) {continue;}
		if(dynamic_cast<Interactable*>(ents[i]) == NULL){continue;} //not an interactable object
				
		if(this->Distance(ents[i]) < dist) {
			dist = this->Distance(ents[i]);
			closest = (Interactable*) ents[i];
		}
	}
	
	return closest;
}

void Entity::interact() {
	int displacement = INTERACT_RANGE;
	
	if(this->facing == RIGHT){
		displacement = INTERACT_RANGE + INTERACT_DISPLACEMENT;
	}else if(this->facing == LEFT){
		displacement = INTERACT_RANGE - INTERACT_DISPLACEMENT;
	}
	
	Interactable *closest = closestInteractable(displacement);
	if(closest == NULL) {return;}
	closest->use();
											
}

void Entity::face(Direction newDirection) {
	if(facing == newDirection) {
		return;
	}
	facing = newDirection;
	
	switch(newDirection) {
		case LEFT:
			this->texture = this->texdata->left;
			break;
		case RIGHT:
			this->texture = this->texdata->right;
			break;
	}
}

/* ================= */
/*  PhysicsEntity 	 */
/* ================= */


PhysicsEntity::PhysicsEntity(TextureData &texdata, int x, int y, RenderLayer rl) : Entity(texdata, x, y, rl) {
	this->vel = (Vector) {0,0};
	this->onGround = 0;
	this->jumpTime = 0;
	this->patrolling = 0;
	static Vector genericCollisionPoints[9] = {
					{0.50d, 0}, 			// Top (1 point)
		{0.25d, 1}, 			{0.75d, 1}, // Bottom (2 points)
		{0, 0.25d}, {0, 0.50d}, {0, 2/3.0d},// Left (3 points)
		{1, 0.25d}, {1, 0.50d}, {1, 2/3.0d} // Right (3 points)
	};
	for(int i=0; i<9; i++) {
		this->collisionPoints[i].x = genericCollisionPoints[i].x * this->rect.w;
		this->collisionPoints[i].y = genericCollisionPoints[i].y * this->rect.h;
	}
}

void PhysicsEntity::Movement(double dt) {
	this->pos.x += this->vel.x * dt;
	this->pos.y += this->vel.y * dt;
	this->vel.y += GRAVITY_ACCEL*dt;
}

Entity* PhysicsEntity::CollisionMovement(Direction &collideDir, double dt) {
	double originalMoveX, originalMoveY, nextMoveX, nextMoveY;
	originalMoveX = nextMoveX = this->vel.x * dt;
	originalMoveY = nextMoveY = this->vel.y * dt;

	Entity* hit = NULL;
	for (int enti = 0; enti < entsC; enti++) {
		if(ents[enti] == NULL || ents[enti]->collision == 0 || this == ents[enti]) continue;
		if(ents[enti]->Distance(this) > (ents[enti]->collisionSize + this->collisionSize)) continue;
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
				while (ents[enti]->ContainsPoint(collisionPoints[0].x + this->pos.x, collisionPoints[0].y + this->pos.y + nextMoveY)) {
					nextMoveY++;
				}
			} else if (dir == 1) {
				while (ents[enti]->ContainsPoint(collisionPoints[1].x + this->pos.x, collisionPoints[1].y + this->pos.y + nextMoveY)
					|| ents[enti]->ContainsPoint(collisionPoints[2].x + this->pos.x, collisionPoints[2].y + this->pos.y + nextMoveY)) {
					nextMoveY--;
				}
			} else if (dir == 2) {
				while (ents[enti]->ContainsPoint(collisionPoints[3].x + this->pos.x + nextMoveX, collisionPoints[3].y + this->pos.y)
					|| ents[enti]->ContainsPoint(collisionPoints[4].x + this->pos.x + nextMoveX, collisionPoints[4].y + this->pos.y)
					|| ents[enti]->ContainsPoint(collisionPoints[5].x + this->pos.x + nextMoveX, collisionPoints[5].y + this->pos.y)) {
					nextMoveX++;
				}
			} else if (dir == 3) {
				while (ents[enti]->ContainsPoint(collisionPoints[6].x + this->pos.x + nextMoveX, collisionPoints[6].y + this->pos.y)
					|| ents[enti]->ContainsPoint(collisionPoints[7].x + this->pos.x + nextMoveX, collisionPoints[7].y + this->pos.y)
					|| ents[enti]->ContainsPoint(collisionPoints[8].x + this->pos.x + nextMoveX, collisionPoints[8].y + this->pos.y)) {
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

void PhysicsEntity::Update(double dt) {
	if(abs(this->vel.y) > JUMP_THRESHOLD) {this->onGround = 0;}
			
	Direction collideDir;
	for (int i=0; i < MAX_COLLISION_ITERATIONS; i++) {
		collideDir = (Direction) 0;
		Entity *hit = this->CollisionMovement(collideDir, dt);
		if(hit == NULL) { break; }
		this->HandleCollision(collideDir, dt);
	}
	this->Movement(dt);
	if(patrolling){this->moveForward();}
	
	// Clamp movement to sides of level
	if((this->pos.x + this->rect.w) > curLevel->w) {
		this->pos.x = curLevel->w - this->rect.w;
	}
	else if(this->pos.x < 0) {
		this->pos.x = 0;
	}
	
	Entity::Update(dt);
}

void PhysicsEntity::moveForward(){
	if(! this->onGround){return;}
	if(this->facing == LEFT){
		this->vel.x = -100;
	}else{
		this->vel.x = 100;
	}
}

void PhysicsEntity::HandleCollision(Direction collideDir, double dt) {
	if(collideDir & DOWN) this->onGround = 1;	
	if(this->patrolling){
		if(! this->onGround || collideDir == LEFT || collideDir == RIGHT){ //we're falling, or we hit a wall
			
			//flip direction
			if(this->facing == RIGHT){
				this->face(LEFT);
			}else if(this->facing == LEFT){
				this->face(RIGHT);
			}
		}
	}
}


inline Direction operator|(Direction a, Direction b) {return static_cast<Direction>(static_cast<int>(a) | static_cast<int>(b));}

/* ================= */
/*  Drawable	  	 */
/* ================= */

Drawable::Drawable(TextureData &texdata, int x, int y) {
	this->texture = texdata.texture;
	this->texdata = &texdata;
	this->rect = (SDL_Rect) {x,y,texdata.w,texdata.h};
	this->animTime = 0;
	this->animDuration = 0;
	this->animMaxFrames = 0;
	this->renderLayer = RL_FOREGROUND;
}

Drawable::~Drawable() {
	for(int i=0; i<renderLayersC[this->renderLayer]; i++) {
		if(renderLayers[this->renderLayer][i] == this) {renderLayers[this->renderLayer][i] = NULL; break;}
	}
}

SDL_Rect* Drawable::GetFrame(double dt) {
	if(this->animDuration != 0) {
		static SDL_Rect srcRect;
		static int curFrame;
		
		this->animTime += dt;
		curFrame = this->animMaxFrames * (this->animTime / this->animDuration);
		//printf("Test (%.4f, %.4f, %.4f, %d, %d)\n", ent->animTime, ent->animDuration, ent->animTime / ent->animDuration, ent->animMaxFrames, curFrame);
		srcRect = (SDL_Rect) {this->rect.w * (curFrame % 8), this->rect.h * (curFrame / 8), this->rect.w, this->rect.h};
		//if(ent->animFrame > ent->animFrameMax*4) {ent->animFrame = 0;}
		return &srcRect;
	}
	return NULL;
}
void Drawable::Draw(double dt) {
	int ret = SDL_RenderCopy(renderer, this->texture, this->GetFrame(dt), &this->rect);
	if(ret != 0) {printf("Render failed: %s\n", SDL_GetError());}
}

/* ================= */
/*  HUD			   	 */
/* ================= */

Hud::Hud() {
	for(int i = 0; i < MAX_HP; i++){
		hearts[i] = new Drawable(heart_emptyTD, i*50, 0);
	}
	
	fillHearts();
}

Hud::~Hud(){
	for(int i = 0; i < MAX_HP; i++){
		delete this->hearts[i];
	}
}

void Hud::fillHearts(){
	if(ply->health <= 0){return;}
	double healthPercent = ply->health/((double) MAX_HP);
	
	for(int i = 0; i < MAX_HP; i++){
		double currentPercent = i/((double) MAX_HP);
		
		if(currentPercent < healthPercent){
			hearts[i]->texture = heart_fullTD.texture;
		}else{
			hearts[i]->texture = heart_emptyTD.texture;
		}
	}	
}

void Hud::Draw(double dt){
	for(int i = 0; i < MAX_HP; i++) {
		this->hearts[i]->Draw(dt);
	}
}

/* ================= */
/*  Interactable   	 */
/* ================= */

Interactable::Interactable(TextureData &texdata, int x, int y, RenderLayer rl) : Entity(texdata, x, y, rl){
	this->action = NO_ACTION;
	this->target = NULL;
}

void Interactable::use(){
	ply->health += 1;
	hud->fillHearts();
	
}
