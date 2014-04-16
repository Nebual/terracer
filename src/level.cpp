#include <stdio.h>
#include <sys/time.h>
#include <math.h>
#include <getopt.h>
#include <fstream>

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

	Json::Value root;   // will contains the root value after parsing.
	if(!loadJSONLevel(level, root)) {
		printf("Error parsing JSON file for level %d!\n", level);
		return; // TODO: Load from a default?
	}
	Json::Value tileset = root["tileset"];
	
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
	
	char blockC[] = "-";
	for(int y=0; fgets(line, sizeof(line), fp); y++) {
		if(DEBUG) printf("Read line: %s", line);
		for(int x=0; x<16; x++) {
			if(line[x] <= 32) continue;
			blockC[0] = line[x];
			if(!!tileset[blockC]) {
				if(DEBUG) printf("Spawning block(%d,%c)\n", line[x], line[x]);
				Entity *ent = new Entity(blockTDs[tileset[blockC]["texture"].asString()], TYPE_BLOCK, x*50, y*25);
			} else {
				printf("Unknown Block: '%c' (%d)\n", line[x], line[x]);
			}
		}
		memset(line, '\0', sizeof(line));
	}
	Entity *testInteract = new Entity(blockTDs[BLOCK_STONE], TYPE_BLOCK, 400, 200);
	testInteract->action = PLY_HEALTH_UP;
}

bool loadJSONLevel(int level, Json::Value &root) {
	char filename[15] = "";
	sprintf(filename, "levels/%d.json", level);
	if(DEBUG) printf("Reading JSON file %s\n", filename);
	
	std::ifstream in(filename);
	Json::Reader reader;
	bool parsingSuccessful = reader.parse( in, root );
	if(!parsingSuccessful) {printf("JSON Parsing Error: %s\n", reader.getFormattedErrorMessages().c_str());}
	return parsingSuccessful;
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


