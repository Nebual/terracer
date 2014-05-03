#ifndef __INPUT_H
#define __INPUT_H

#include "common.h"
#include "entity.h"

struct Player : PhysicsEntity {
	Player (TextureData &texdata, int x, int y);
	void Update(double dt);
	void HandleKeyboard(double dt);
	void HandleCollision(Entity* hit, Direction collideDir, double dt);
	void interact();
};

void initInput();

#endif
