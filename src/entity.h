#ifndef __ENTITY_H
#define __ENTITY_H

#include "common.h"

struct TextureData {
	SDL_Texture *texture;
	SDL_Texture *left;
	SDL_Texture *right;
	short int animMinFrame;
	short int animMaxFrame;
	short int animWidth;
	double animDuration;
	int w;
	int h;
	CollisionType collisionType;
};

struct Drawable {
	int id;
	bool isKilled;
	SDL_Rect rect;
	SDL_Texture *texture;
	TextureData *texdata;
	
	double animTime;
	double animDuration;
	short int animMinFrame;
	short int animMaxFrame;
	RenderLayer renderLayer;

	Drawable(RenderLayer rl=RL_FOREGROUND);
	Drawable(TextureData &texdata, int x, int y, RenderLayer rl=RL_FOREGROUND);
	~Drawable();
	virtual SDL_Rect* GetFrame(double dt);
	virtual void Draw(double dt);
	void kill();
};

struct Entity : Drawable {
	Vector pos;
	Uint32 deathTime;
	short int collision;
	short int collisionSize;	// Used by anything involved with collisions. Define size for circle collision check 
	short int damage;			// Projectiles
	short int health;			// Ships
	
	Action action;
	int iData;
	std::string sData;
	Direction facing;
	
	Entity (TextureData &texdata, int x, int y, RenderLayer rl);
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
	Interactable* closestInteractable(const Vector &vec);
	void face(Direction newDirection);
	virtual void setHealth(int newValue);
};

struct PhysicsEntity : Entity {
	Vector vel;
	int onGround;
	double jumpTime;
	int patrolling;
	static Vector genericCollisionPoints[9];
	Vector collisionPoints[9];
	
	PhysicsEntity(TextureData &texdata, int x, int y, RenderLayer rl=RL_FOREGROUND);
	Entity* CollisionMovement(Direction &dir, double dt);
	void Movement(double dt);
	void moveForward();
	virtual void Update(double dt);
	virtual void HandleCollision(Entity* hit, Direction collideDir, double dt);
	virtual void SetAnimation(Animation newAnim);
};

struct Hud{
	Drawable *hearts[5];
	Drawable *scrap;
	
	Hud();
	~Hud();
	void update();
	void fillHearts();
	void setScrapText();
};

struct Door : Entity{
	int isOpen;
	
	Door(TextureData &texdata, int x, int y, RenderLayer rl=RL_FOREGROUND);
	void setOpen(int setTo=-1);
};

struct Interactable : Entity{
	Entity *target;
	
	Interactable(TextureData &texdata, int x, int y, RenderLayer rl=RL_FOREGROUND);
	void use();
};

extern std::map <std::string, TextureData> blockTDs;

void initTextures();
TextureData& getTexture(const std::string k);
TextureData TextureDataCreate(const char texturePath[], const char leftPath[] = "", const char rightPath[] = "");
Direction operator|(Direction a, Direction b);


#endif
