#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>
#include <fstream>
#include <algorithm>    // std::find

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

Entity* posLookup[100][100];
SDL_Texture *backgroundRLTexture;

std::vector<WorldTip*> worldtips;

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
	if(!!info["sdata"]) {ent->sData = info["sdata"].asString();}
	if(!!info["target"]) {
		char sPos[20];
		strncpy(sPos, info["target"].asCString(), sizeof(sPos)-1);
		if(Entity *target = posLookup[atoi(strtok(sPos+1, ","))][atoi(strtok(NULL, ")"))]) {
			if(Interactable *interactable = dynamic_cast<Interactable*>(ent)) {
				interactable->target = target;
			} else {printf("setEntityProperties Error: Tried to set the target of a non-Interactable.\n");}
		}
	}
	if(!!info["msg"]) {
		Direction dir = DOWN;
		if(!!info["msgdir"]) { dir = (Direction) (1 << std::find(directionLookup, directionLookup + 4, info["msgdir"].asString()) - directionLookup); }

		worldtips.push_back(new WorldTip(ent, info["msg"].asCString(), dir));
	}
}

WorldTip::WorldTip(const Entity *ent, const char *text, Direction dir) : Drawable(RL_FOREGROUND2)  {
	SDL_Texture *txt_tex = readyText(text, BLACK);
	SDL_Rect rect = {0, 0, 0, 0};
	SDL_QueryTexture(txt_tex, NULL, NULL, &rect.w, &rect.h);
	
	SDL_Texture *tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, rect.w + 10, rect.h + 10);
	SDL_SetRenderTarget(renderer, tex);
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_Rect draw_rect = {2, 2, rect.w + 6, rect.h + 6};
	SDL_RenderFillRect(renderer, &draw_rect);
	draw_rect = {5, 5, rect.w, rect.h};
	SDL_RenderCopy(renderer, txt_tex, NULL, &draw_rect);
	SDL_SetRenderTarget(renderer, NULL);
	SDL_DestroyTexture(txt_tex);

	rect.w += 10; rect.h += 10; // Grow the rect, to encompass the border
	rect.x = (int) ent->pos.x + ent->rect.w / 2; rect.y = (int) ent->pos.y + ent->rect.h / 2;
	switch(dir) {
		case UP:
			rect.x -= rect.w / 2;
			rect.y -= rect.h + 50;
			break;
		case DOWN:
			rect.x -= rect.w / 2;
			rect.y += 50;
			break;
		case LEFT:
			rect.x -= rect.w + 50;
			rect.y -= rect.h / 2;
			break;
		case RIGHT:
			rect.x += rect.w + 50;
			rect.y -= rect.h / 2;
			break;
	}

	this->rect = rect;
	this->texdata->texture = tex;
	this->texture = tex;
	this->origin = Vector(ent->pos.x, ent->pos.y);
}
WorldTip::~WorldTip() {
	SDL_DestroyTexture(this->texture);
}
void WorldTip::Draw(double dt) {
	if(ply->pos.Distance(this->origin) < 200) {
		SDL_Rect rect = {this->rect.x - camera.x, this->rect.y - camera.y, this->rect.w, this->rect.h};
		if(SDL_RenderCopy(renderer, this->texture, NULL, &rect) != 0) {printf("Worldtip Render failed: %s\n", SDL_GetError());}
	}
}

Level *curLevel;
Level::Level(const std::string inLevel, Json::Value *root) {
	this->w = 0;
	this->h = 0;
	this->id = inLevel;
	this->json = root;
}

Entity* constructEntity(Json::Value tileinfo, int x, int y) {
	Entity *ent;
	// Note: Anything with RL_BACKGROUND will be compiled into backgroundRLTexture once at startup,
	// so be sure to set anything that moves to RL_FOREGROUND
	RenderLayer rl = tileinfo.get("static", true).asBool() ? RL_BACKGROUND : RL_FOREGROUND;
	std::string className = tileinfo.get("class","").asString();
	//std::ostringstream sPos; sPos << "(" << x << "," << y << ")";
	//if(className == "" && (!!tileset[blockC]["action"] || (!!customEntities[sPos.str()] && !!customEntities[sPos.str()]["action"]))) {className = "interactable";}
	if(className == "INTERACTABLE") {
		ent = new Interactable(getTexture(tileinfo.get("texture","").asString()), x*BLOCK_SIZE, y*BLOCK_SIZE, RL_FOREGROUND);
	} else if(className == "DOOR") {
		ent = new Door(getTexture(tileinfo.get("texture","").asString()), x*BLOCK_SIZE, y*BLOCK_SIZE, RL_FOREGROUND);
	} else {
		ent = new Entity(getTexture(tileinfo.get("texture","").asString()), x*BLOCK_SIZE, y*BLOCK_SIZE, rl);
	}
	return ent;
}

// Run when switching to a new level or on shutdown
void cleanLevel() {
	for(int enti=0; enti<entsC; enti++) {
		if(ents[enti] == NULL) continue;
		delete ents[enti];
	}
	for(auto &worldtip : worldtips) {
		delete worldtip;
	}
	worldtips.clear();
	Entity::GC();
	
	delete curLevel;
}

void generateLevel(const std::string &level) {
	cleanLevel();

	Json::Value root;   // will contains the root value after parsing.
	curLevel = new Level(level, &root);
	if(!loadJSONLevel(level, root)) {
		printf("Error parsing JSON file for level %s!\n", level.c_str());
		generateLevel("worldmap");
		return; // TODO: Load from a default?
	}
	Json::Value tileset = root["tileset"];
	Json::Value customEntities = root["entities"];
	
	char line[201] = "";
	std::string filename = "levels/"+level+".lvl";
	if(DEBUG) printf("Reading file %s\n", filename.c_str());
	
	FILE *fp = fopen(filename.c_str(), "r");
	if(!fp) {
		printf("ERROR: Level %s not found!\n", filename.c_str());
		generateLevel("worldmap");
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

bool loadJSONLevel(const std::string &level, Json::Value &root) {
	std::string filename = ("levels/" + level + ".json");
	if(DEBUG) printf("Reading JSON file %s\n", filename.c_str());
	
	std::ifstream in(filename.c_str());
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
	
	//SDL_RenderFillRect(renderer, NULL); // Clear the screen
	SDL_Texture *bgTex = IMG_LoadTexture(renderer, ("res/backgrounds/" + curLevel->json->get("background","").asString() + ".jpg").c_str());
	SDL_Rect bgRect = {0,0,curLevel->w,curLevel->h};
	SDL_RenderCopy(renderer, bgTex, NULL, &bgRect);
	
	for(Drawable *ent : renderLayers[RL_BACKGROUND]) {
		if(ent == NULL) {continue;}
		ent->Drawable::Draw(0);
	}
	
	SDL_SetRenderTarget(renderer, NULL);
}

void drawBackground(SDL_Renderer *renderer, double dt) {
	SDL_RenderCopy(renderer, backgroundRLTexture, &camera, NULL);
}

void handleAction(Entity* ent, Action action){
	switch(action){
		case HEALTH_UP:
			ent->setHealth(ent->health + 1);
			break;
		
		case OPEN_DOOR:
			if(Door *d = dynamic_cast<Door*>(ent)) {
				d->setOpen(); //flip open status
			} else{ printf("handleAction error: Tried to open a non-door!\n"); }
			break;
			
		case SCRAP_UP:
			ply->setScrap(ply->scrapCount + 1);
			break;
	}
}




