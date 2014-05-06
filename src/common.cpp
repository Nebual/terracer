#include <cmath>

#include "common.h"

Entity *ents[MAX_ENTITIES];
int entsC = 0;
Drawable *renderLayers[RL_MAX][MAX_ENTITIES];
int renderLayersC[RL_MAX];

SDL_Renderer *renderer;
Player *ply;
Hud *hud;
SDL_Rect camera = {0,0,0,0};

int WIDTH, HEIGHT, WINDOW_WIDTH, WINDOW_HEIGHT, WIDTH_OFFSET, HEIGHT_OFFSET;

Vector::Vector(const double x, const double y) {
	this->x = x;
	this->y = y;
}
Vector& Vector::operator+=(const Vector &rhs) {
	this->x += rhs.x;
	this->y += rhs.y;
	return *this;
}
Vector& Vector::operator-=(const Vector &rhs) {
	this->x -= rhs.x;
	this->y -= rhs.y;
	return *this;
}

double Vector::Distance(const Vector &other) const {
	return pow(pow(this->x - other.x, 2) + powf(this->y - other.y, 2), 0.5);
}
