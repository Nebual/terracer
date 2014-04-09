#ifndef __ENTITY_H
#define __ENTITY_H

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

typedef enum {
	BLOCK_NONE = ' ',
	BLOCK_NORMAL = '=',
	BLOCK_SPEEDUP = '+',
	BLOCK_SLOWDOWN = '-',
	BLOCK_RANDOM = 'X',
	BLOCK_TOUGH = '#'
} BlockType;

typedef struct {
	SDL_Texture *texture;
	short int animMaxFrames;
	short int animWidth;
	short int animDuration;
	int w;
	int h;
} TextureData;

struct Entity {
	Vector vel;
	Vector pos;
	SDL_Rect rect;
	SDL_Texture *texture;
	Uint32 deathTime;
	short int collision;
	short int collisionSize;	// Used by anything involved with collisions. Define size for circle collision check 
	short int damage;			// Projectiles
	
	double animTime;
	double animDuration;
	short int animMaxFrames;

	short int health;			// Ships

	Type type;
	BlockType blockType;
	
	Entity (TextureData texdata, Type type, int x, int y);
	~Entity ();
	void Draw(double dt);
	void Update(double dt);
	void Movement(double dt);
	static void GC();
	Entity* TestCollision();
	void Damage(int damage);
	void DeathClock(int delay);
	double Distance(Entity *ent2);
};

extern TextureData blockTDs[127];


void initTextures();
TextureData TextureDataCreate(const char texturePath[]);
void GenBall(Entity *ent);


#endif
