#include <stdio.h>
#include <sys/time.h>
#include <math.h>
#include <getopt.h>
#include <fstream>
#include <algorithm>    // std::find

#include <SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include "json/json.h"

#include "entity.h"
#include "main.h"
#include "input.h"
#include "level.h"
#include "util.h"

int displayWinText;
int displayLevelText;
char menuMode[50];

Entity* posLookup[100][100];

void checkWinLoss(){
	if(menuMode[0] != '\0'){return;}
}

void setEntityProperties(Entity* ent, Json::Value info) {
	if(!!info["collision"]) {ent->collision = info["collision"].asBool();}
	if(!!info["action"]) {
		ent->action = (Action) (std::find(actionLookup, actionLookup + MAX_ACTIONS, info["action"].asString()) - actionLookup);
		if(ent->action == MAX_ACTIONS) {ent->action = NO_ACTION; printf("Ent properties error: no such action '%s'!\n", info["action"].asCString());}
	}
}

Level *curLevel;
Level::Level(int inLevel) {
	this->w = 0;
	this->h = 0;
	this->id = inLevel;
}

void generateLevel(int level) {
	strcpy(menuMode, "");
	
	for(int enti=0; enti<entsC; enti++) {
		if(ents[enti] == NULL) continue;
		if(dynamic_cast<const Player*>(ents[enti]) == 0){ // Delete everything but the player
			delete ents[enti];
		}
	}
	Entity::GC();

	curLevel = new Level(level);

	Json::Value root;   // will contains the root value after parsing.
	if(!loadJSONLevel(level, root)) {
		printf("Error parsing JSON file for level %d!\n", level);
		return; // TODO: Load from a default?
	}
	Json::Value tileset = root["tileset"];
	
	char filename[14] = "";
	char line[101] = "";
	sprintf(filename, "levels/%d.lvl", level);
	if(DEBUG) printf("Reading file %s\n", filename);
	
	FILE *fp = fopen(filename, "r");
	if(!fp) {
		//printf("%s not found, you've completed the last level!\n", filename);
		strcpy(menuMode, "YOU DEFEATED! Play again? Y/N");
		return;
	}
	
	char blockC[] = "-";
	int x=0, y=0, biggestX=0, biggestY=0;
	Entity *ent;
	for(y=0; fgets(line, sizeof(line), fp); y++) {
		if(DEBUG) printf("Read line: %s", line);
		for(x=0; line[x]; x++) {
			if(line[x] <= 32) continue;
			blockC[0] = line[x];
			if(!!tileset[blockC]) {
				if(DEBUG) printf("Spawning block(%d,%d) type(%d,%c)\n", x, y, line[x], line[x]);
				ent = new Entity(blockTDs[tileset[blockC]["texture"].asString()], x*50, y*25);
				posLookup[x][y] = ent; // TODO: Remove from list, ensure consistency across block movements
				setEntityProperties(ent, tileset[blockC]);

				if(x > biggestX) biggestX = x;
				if(y > biggestY) biggestY = y;
			} else {
				printf("Unknown Block: '%c' (%d)\n", line[x], line[x]);
			}
		}
		memset(line, '\0', sizeof(line));
	}
	curLevel->w = max((biggestX+1)*50, WIDTH);
	curLevel->h = max((biggestY+1)*25, HEIGHT);
	
	Json::Value customEntities = root["entities"];
	char sPos[20];
	for(Json::ValueIterator iter = customEntities.begin(); iter != customEntities.end(); iter++ ) {
		strncpy(sPos, iter.memberName(), sizeof(sPos)-1);
		x = atoi(strtok(sPos+1, ","));
		y = atoi(strtok(NULL, ")"));
		if((ent = posLookup[x][y]) == NULL) {
			printf("Level setup error: Entity(%d,%d) not found!\n", x, y);
			continue;
		}
		if(DEBUG) printf("Applying Entity(%d,%d) specifics...\n", x, y);
		setEntityProperties(ent, *iter);
	}
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




