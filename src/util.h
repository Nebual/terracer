#ifndef __UTIL_H
#define __UTIL_H

extern SDL_Color WHITE;

void initFonts();
int curtime();
int curtime_u();
short int sign(double i);
#ifndef min
int min(int a, int b);
int max(int a, int b);
#endif
double clamp(double val, double min, double max);
double angnorm(double ang);
void fpsCounter();
void playSound(Mix_Chunk *snd);
int random_range(int min, int max);
void displayText(int x, int y, const char text[], SDL_Color color=WHITE);
SDL_Texture* readyText(int x, int y, const char text[], SDL_Color color);
void displayTextCentered(int x, int y, const char text[], SDL_Color color=WHITE);

struct Timer {
	int id;
	Uint32 endTime;
	Uint32 delay;
	short int reps;
	void (*callback)();
};

void TimerRun();
/** Creates a new Timer and runs it
 * @param id A unique identifier for the timer, so you can later modify the timer. Feel free to leave as 0
 * @param delay Amount of delay, in milliseconds
 * @param reps Number of repetitions. 0 for "keep doing until manually TimerDestroy'd"
 * @param callback The function to run once the timer is up
 * @example TimerCreate(5, 2000, 1, [](){ply->pos.y = 500;});
 */
void TimerCreate(int id, Uint32 delay, short int reps, void (*callback)());
void TimerDestroy(int id);

#endif
