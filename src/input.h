#ifndef __INPUT_H
#define __INPUT_H

// Forward declarations
class Entity;

const int PLAYER_AIR_ACCEL = 900;
const int PLAYER_MAX_SPEED = 300;
const float PLAYER_JUMP_TIME = 0.35;
const int GRAVITY_ACCEL = 1000;
const int JUMP_THRESHOLD = 150;
const int CAM_DEADZONE = 50;
const int CAM_SPEED = 3;

extern int playerOnGround;

void handleKeyboard(double dt, Entity *ply);

#endif
