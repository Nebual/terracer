#ifndef __COMMON_H
#define __COMMON_H

#include <map>
#include <string>

#include <SDL.h>

/* ================= */
/* Forward declares  */
/* ================= */

class Drawable;
class Entity;
class PhysicsEntity;
class Player;

class TextureData;
class Timer;
class Level;
class Hud;
class Interactable;
class Door;


/* ================= */
/* Constants         */
/* ================= */

const int BLOCK_SIZE = 24;
const int MAX_ENTITIES = 8192;
const int MAX_COLLISION_ITERATIONS = 3;
const int INTERACT_RANGE = 50;
const int INTERACT_DISPLACEMENT = 20;
const int MAX_HP = 5;

const int PLAYER_AIR_ACCEL = 900;
const int PLAYER_MAX_SPEED = 300;
const float PLAYER_JUMP_TIME = 0.35;
const int GRAVITY_ACCEL = 1000;
const int JUMP_THRESHOLD = 150;
const int CAM_DEADZONE = 50;
const int CAM_SPEED = 3;


/* ================= */
/* Structs and Enums */
/* ================= */

typedef struct {
	double x, y;
} Vector;

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
	SWITCH_LEVEL,
	MAX_ACTIONS
};
static const std::string actionLookup[] = {
	"NO_ACTION",
	"PLY_HEALTH_UP",
	"PLY_SPEED_UP",
	"PLY_LIVES_UP",
	"SWITCH_LEVEL"
};

enum RenderLayer{
	RL_BACKGROUND,
	RL_FOREGROUND,
	RL_FOREGROUND2,
	RL_HUD,
	RL_MAX
};


/* ================= */
/* Varbles           */
/* ================= */

extern Entity *ents[MAX_ENTITIES];
extern int entsC;
extern Drawable *renderLayers[RL_MAX][MAX_ENTITIES];
extern int renderLayersC[RL_MAX];

extern SDL_Renderer *renderer;
extern Player *ply;
extern Hud *hud;
extern SDL_Rect camera;
extern Level *curLevel;

extern int WIDTH, HEIGHT, WINDOW_WIDTH, WINDOW_HEIGHT, WIDTH_OFFSET, HEIGHT_OFFSET;

extern std::map <std::string, TextureData> blockTDs;
extern TextureData goombaTD;
extern TextureData playerTD;

#ifndef DEBUG
#define DEBUG 0
#endif

#endif
