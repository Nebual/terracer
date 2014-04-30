#ifndef __ENTITY_H
#define __ENTITY_H

#include "common.h"

struct TextureData {
	SDL_Texture *texture;
	SDL_Texture *left;
	SDL_Texture *right;
	short int animMaxFrames;
	short int animWidth;
	short int animDuration;
	int w;
	int h;
};

struct Drawable {
	SDL_Rect rect;
	SDL_Texture *texture;
	TextureData *texdata;
	
	double animTime;
	double animDuration;
	short int animMaxFrames;
	RenderLayer renderLayer;

	Drawable(TextureData &texdata, int x, int y);
	~Drawable();
	virtual SDL_Rect* GetFrame(double dt);
	virtual void Draw(double dt);
};

struct Entity : Drawable {
	Vector pos;
	Uint32 deathTime;
	short int collision;
	short int collisionSize;	// Used by anything involved with collisions. Define size for circle collision check 
	short int damage;			// Projectiles
	short int health;			// Ships
	
	Action action;
	Direction facing;
	
	Entity (TextureData &texdata, int x, int y, RenderLayer rl=RL_FOREGROUND);
	~Entity ();
	void Draw(double dt);
	virtual void Update(double dt);
	static void GC();
	int ContainsPoint(double x, double y);
	Entity* TestCollision();
	void Damage(int damage);
	void DeathClock(int delay);
	double Distance(Entity *ent2);
	void use();
	Entity* closestInteractable(int dist);
	void interact();
	void face(Direction newDirection);
};

struct PhysicsEntity : Entity {
	Vector vel;
	int onGround;
	double jumpTime;
	int patrolling;
	Vector collisionPoints[9];
	
	PhysicsEntity(TextureData &texdata, int x, int y, RenderLayer rl=RL_FOREGROUND);
	Entity* CollisionMovement(Direction &dir, double dt);
	void Movement(double dt);
	void moveForward();
	virtual void Update(double dt);
	virtual void HandleCollision(Direction collideDir, double dt);
};

struct Hud{
	Drawable *hearts[5];
	
	Hud();
	~Hud();
	void Draw(double dt);
	void fillHearts();
};

void initTextures();
TextureData TextureDataCreate(const char texturePath[], const char leftPath[] = "", const char rightPath[] = "");
Direction operator|(Direction a, Direction b);


#endif
