#ifndef __INPUT_H
#define __INPUT_H

// Forward declarations
class PhysicsEntity;

const int PLAYER_AIR_ACCEL = 900;
const int PLAYER_MAX_SPEED = 300;
const float PLAYER_JUMP_TIME = 0.35;
const int GRAVITY_ACCEL = 1000;
const int JUMP_THRESHOLD = 150;
const int CAM_DEADZONE = 50;
const int CAM_SPEED = 3;

struct Player : PhysicsEntity {
	Player (TextureData &texdata, int x, int y);
	void Update(double dt);
	void HandleKeyboard(double dt);
	void HandleCollision(Direction collideDir, double dt);
};

#endif
