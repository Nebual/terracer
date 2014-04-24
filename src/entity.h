#ifndef __ENTITY_H
#define __ENTITY_H

#include <map>
#include <string>

typedef struct {
	double x, y;
} Vector;

const int BLOCK_SIZE = 24;
const int MAX_ENTITIES = 8192;
const int MAX_COLLISION_ITERATIONS = 3;
const int INTERACT_RANGE = 50;
const int INTERACT_DISPLACEMENT = 20;
const int MAX_HP = 5;

typedef struct {
	SDL_Texture *texture;
	SDL_Texture *left;
	SDL_Texture *right;
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
	PLY_LIVES_UP,
	MAX_ACTIONS
};
static const std::string actionLookup[] = {
	"NO_ACTION",
	"PLY_HEALTH_UP",
	"PLY_SPEED_UP",
	"PLY_LIVES_UP"
};

enum RenderLayer{
	RL_BACKGROUND,
	RL_FOREGROUND,
	RL_FOREGROUND2,
	RL_HUD,
	RL_MAX
};

Direction operator|(Direction a, Direction b);

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
	
	Entity (TextureData &texdata, int x, int y, RenderLayer rl=RL_BACKGROUND);
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
	
	PhysicsEntity(TextureData &texdata, int x, int y, RenderLayer rl=RL_BACKGROUND);
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

extern std::map <std::string, TextureData> blockTDs;
extern TextureData goombaTD;
extern TextureData playerTD;

void initTextures();

TextureData TextureDataCreate(const char texturePath[], const char leftPath[] = "", const char rightPath[] = "");


#endif
