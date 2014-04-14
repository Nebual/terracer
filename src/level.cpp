#include <stdio.h>
#include <sys/time.h>
#include <math.h>
#include <getopt.h>

#include <SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include "json/json.h"

#include "entity.h"
#include "main.h"
#include "level.h"
#include "util.h"

int curLevel;
int displayWinText;
int displayLevelText;
char menuMode[50];

void checkWinLoss(){
	if(menuMode[0] != '\0'){return;}
}

void generateLevel(int level) {
	strcpy(menuMode, "");
	
	for(int enti=0; enti<entsC; enti++) {
		if(ents[enti] == NULL) continue;
		if(ents[enti]->type == TYPE_BLOCK){
			delete ents[enti];
		}
	}
	Entity::GC();

	loadJSONLevel(level);
	
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

void loadJSONLevel(int level) {
	Json::Value root;   // will contains the root value after parsing.
	Json::Reader reader;
	bool parsingSuccessful = reader.parse( "{\"hi\": 5}", root );
}

void drawBackground(SDL_Renderer *renderer, double dt) {
	const SDL_Rect rect = {0,0,WIDTH,HEIGHT};
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
	SDL_SetRenderDrawColor(renderer, 20,20,20, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, &rect);
}

void drawHud(double dt){
	if(menuMode[0] != '\0'){
		displayTextCentered(400, 200, menuMode);
	}
}


