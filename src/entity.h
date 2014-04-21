#ifndef __ENTITY_H
#define __ENTITY_H

#include <map>
#include <string>

typedef struct {
	double x, y;
} Vector;

typedef enum {
	TYPE_NONE,
	TYPE_BLOCK,
	TYPE_PLAYER,
	TYPE_BALL,
	TYPE_EXPLOSION,
	TYPE_MAX
} Type;

static std::string BLOCK_DIRT = "dirt";
static std::string BLOCK_STONE = "stone";

typedef struct {
	SDL_Texture *texture;
	short int animMaxFrames;
	short int animWidth;
	short int animDuration;
	int w;
	int h;
} TextureData;

enum Direction {
	UP = 1,
	DOWN = 2,
	LEFT = 4,
	RIGHT = 8
};

enum Action {
	NO_ACTION,
	PLY_HEALTH_UP,
	PLY_SPEED_UP,
	PLY_LIVES_UP
};

enum RenderLayer{
	RL_BACKGROUND,
	RL_FOREGROUND,
	RL_HUD,
	RL_MAX
};

Direction operator|(Direction a, Direction b);

struct Drawable {
	SDL_Rect rect;
	SDL_Texture *texture;
	
	double animTime;
	double animDuration;
	short int animMaxFrames;
	RenderLayer renderLayer;

	Drawable(TextureData texdata, int x, int y);
	~Drawable();
	virtual SDL_Rect* GetFrame(double dt);
	virtual void Draw(double dt);
};

struct Entity : Drawable {
	Vector vel;
	Vector pos;
	Uint32 deathTime;
	short int collision;
	short int collisionSize;	// Used by anything involved with collisions. Define size for circle collision check 
	short int damage;			// Projectiles
	short int health;			// Ships

	Type type;
	
	Action action;
	Direction facing;
	
	Entity (TextureData texdata, Type type, int x, int y);
	~Entity ();
	void Draw(double dt);
	void Update(double dt);
	void Movement(double dt);
	static void GC();
	int ContainsPoint(double x, double y);
	Entity* TestCollision();
	Entity* CollisionMovement(Direction &dir, double dt);
	void Damage(int damage);
	void DeathClock(int delay);
	double Distance(Entity *ent2);
	void use();
	Entity* closestInteractable(int dist);
	void interact();
	void face(Direction newDirection);
};


struct Hud{
	Drawable *hearts[5];
	
	Hud();
	~Hud();
	void Draw(double dt);
	void fillHearts();
};

extern std::map <std::string, TextureData> blockTDs;

void initTextures();

TextureData TextureDataCreate(const char texturePath[]);


#endif
