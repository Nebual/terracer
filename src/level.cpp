#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>
#include <fstream>
#include <algorithm>    // std::find
#include <sstream>
#include <typeinfo>

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include "json/json.h"

#include "entity.h"
#include "main.h"
#include "player.h"
#include "level.h"
#include "util.h"

int displayWinText;
int displayLevelText;
char menuMode[50];

Entity* posLookup[100][100];
SDL_Texture *backgroundRLTexture;

void checkWinLoss(){
	if(menuMode[0] != '\0'){return;}
}

void setEntityProperties(Entity* ent, Json::Value info) {
	if(!!info["collision"]) {ent->collision = info["collision"].asBool();}
	if(!!info["action"]) {
		if(info["action"].asString() == "PLAYERSPAWN") {
			ply = new Player(getTexture("player"), ent->pos.x, ent->pos.y - BLOCK_SIZE*3);
			camera.x = ent->pos.x; camera.y = ent->pos.y;
			delete ent;
			return;
		}
		ent->action = (Action) (std::find(actionLookup, actionLookup + MAX_ACTIONS, info["action"].asString()) - actionLookup);
		if(ent->action == MAX_ACTIONS) {ent->action = NO_ACTION; printf("Ent properties error: no such action '%s'!\n", info["action"].asCString());}
	}
	if(!!info["idata"]) {ent->iData = info["idata"].asInt();}
}

Level *curLevel;
Level::Level(int inLevel) {
	this->w = 0;
	this->h = 0;
	this->id = inLevel;
}

Entity* constructEntity(Json::Value tileinfo, int x, int y) {
	Entity *ent;
	// Note: Anything with RL_BACKGROUND will be compiled into backgroundRLTexture once at startup,
	// so be sure to set anything that moves to RL_FOREGROUND
	RenderLayer rl = tileinfo.get("static", true).asBool() ? RL_BACKGROUND : RL_FOREGROUND;
	std::string className = tileinfo.get("class","").asString();
	//std::ostringstream sPos; sPos << "(" << x << "," << y << ")";
	//if(className == "" && (!!tileset[blockC]["action"] || (!!customEntities[sPos.str()] && !!customEntities[sPos.str()]["action"]))) {className = "interactable";}
	if(className == "interactable") {
		ent = new Interactable(getTexture(tileinfo.get("texture","").asString()), x*BLOCK_SIZE, y*BLOCK_SIZE, rl);
	} else {
		ent = new Entity(getTexture(tileinfo.get("texture","").asString()), x*BLOCK_SIZE, y*BLOCK_SIZE, rl);
	}
	return ent;
}

void generateLevel(int level) {
	strcpy(menuMode, "");
	
	for(int enti=0; enti<entsC; enti++) {
		if(ents[enti] == NULL) continue;
		delete ents[enti];
	}
	Entity::GC();

	curLevel = new Level(level);

	Json::Value root;   // will contains the root value after parsing.
	if(!loadJSONLevel(level, root)) {
		printf("Error parsing JSON file for level %d!\n", level);
		return; // TODO: Load from a default?
	}
	Json::Value tileset = root["tileset"];
	Json::Value customEntities = root["entities"];
	
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
				
				ent = constructEntity(tileset[blockC], x, y);
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
	curLevel->w = max((biggestX+1)*BLOCK_SIZE, WIDTH);
	curLevel->h = max((biggestY+1)*BLOCK_SIZE, HEIGHT);

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

	compileBackground(renderer);
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

void compileBackground(SDL_Renderer *renderer) {
	SDL_DestroyTexture(backgroundRLTexture);
	
	backgroundRLTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, curLevel->w, curLevel->h);
	SDL_SetTextureBlendMode(backgroundRLTexture, SDL_BLENDMODE_NONE);
	SDL_SetRenderTarget(renderer, backgroundRLTexture);
	
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
	SDL_SetRenderDrawColor(renderer, 20,20,20, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, NULL);
	for(Drawable *ent : renderLayers[RL_BACKGROUND]) {
		if(ent == NULL) {continue;}
		ent->Drawable::Draw(0);
	}
	
	SDL_SetRenderTarget(renderer, NULL);
}

void drawBackground(SDL_Renderer *renderer, double dt) {
	SDL_RenderCopy(renderer, backgroundRLTexture, &camera, NULL);
}




