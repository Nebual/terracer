
const int PLAYER_AIR_ACCEL = 900;
const int PLAYER_MAX_SPEED = 300;
const float PLAYER_JUMP_TIME = 0.35;
const int GRAVITY_ACCEL = 1000;
const int JUMP_THRESHOLD = 150;

extern int playerOnGround;

void handleKeyboard(double dt, Entity *ply);
