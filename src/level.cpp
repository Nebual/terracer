#include <stdio.h>
#include <sys/time.h>
#include <math.h>
#include <getopt.h>

#include <SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#include "entity.h"
#include "main.h"
#include "level.h"
#include "util.h"

int curLevel;
int balls = 3;
Entity *ballInPlay;
int displayWinText;
int displayLevelText;
char menuMode[50];

void checkWinLoss(){
	if(menuMode[0] != '\0'){return;}
	
	if(balls <= 0 && ballInPlay == NULL){
		//printf("You've dropped the barr far too many times to bother trying again. You lose!\n");
		strcpy(menuMode, "GAME OVER! Play again? Y/N");
	} else if (! blocksRemain()){
		Mix_Chunk *victorySound = Mix_LoadWAV("res/sounds/victory.ogg");
		playSound(victorySound);
		
		displayWinText = 1;
		TimerCreate(0, 3000, 1, [](){displayWinText = 0;});
		
		displayLevelText = 1;
		TimerCreate(0, 3000, 1, [](){displayLevelText = 0;});		
		
		curLevel++;
		generateLevel(curLevel);
	}
}

int blocksRemain(){
	for(int i=0; i < entsC; i++){
		if(ents[i] == NULL) continue;
		if(ents[i]->type == TYPE_BLOCK){
			return 1;
		}
	}
	return 0;
}

void generateLevel(int level) {
	strcpy(menuMode, "");
	
	delete ballInPlay;
	ballInPlay = NULL;
	balls=3;
	
	for(int enti=0; enti<entsC; enti++) {
		if(ents[enti] == NULL) continue;
		if(ents[enti]->type == TYPE_BLOCK){
			delete ents[enti];
		}
	}
	Entity::GC();
	
	curLevel = level;
	char filename[14] = "";
	char line[18] = "";
	sprintf(filename, "levels/%d.lvl", level);
	if(DEBUG) printf("Reading file %s\n", filename);
	
	FILE *fp = fopen(filename, "r");
	if(!fp) {
		//printf("%s not found, you've completed the last level!\n", filename);
		strcpy(menuMode, "YOU DEFEATED! Play again? Y/N");
		return;
	}
	
	for(int y=0; fgets(line, sizeof(line), fp); y++) {
		if(DEBUG) printf("Read line: %s", line);
		for(int x=0; x<16; x++) {
			if(blockTDs[line[x]].texture != NULL) {
				if(DEBUG) printf("Spawning block(%d,%c)\n", line[x], line[x]);
				Entity *ent = new Entity(blockTDs[line[x]], TYPE_BLOCK, x*50, y*25);
				ent->blockType = (BlockType) line[x];
				switch(ent->blockType){
					case BLOCK_TOUGH:
						ent->health *= 2;
						break;
				}
					
			}
		}
		memset(line, '\0', sizeof(line));
	}
}

void drawBackground(SDL_Renderer *renderer, double dt) {
	const SDL_Rect rect = {0,0,WIDTH,HEIGHT};
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
	SDL_SetRenderDrawColor(renderer, 20,20,20, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, &rect);
}

void drawHud(double dt){
	char displayString[20];// = "Balls: ";
	sprintf(displayString, "Balls: %d", balls);
	displayText(0, 0, displayString);
	if(displayWinText){
		displayTextCentered(400, 100, "GOOD JOB!");
	}
	if(displayLevelText){
		char levelName[20];
		sprintf(levelName, "Level %d", curLevel);
		displayTextCentered(400, 200, levelName);
	}
	if(menuMode[0] != '\0'){
		displayTextCentered(400, 200, menuMode);
	}
}


