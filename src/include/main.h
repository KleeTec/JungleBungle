#pragma once
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include "asset.h"
#include "modes.h"
#include "things/game_objects.h"


#define JB_MAX_FPS 60.0
#define JB_MAX_MOTION_SPEED 9

/**
 * das Spiel als globales Struct
 */
extern struct JB_Game_Struct {
	/**
	 * Der Name des Spiels
	 */
	char* name;
	bool running;
	/**
	 * Sollte es einen Error geben, wird der Code hier drin gespeichert<br/>
	 * TODO: Dokumentieren, welcher Code für was steht
	 */
	int error_code;

	/**
	 * Frames per second
	 */
	double fps;

	/**
	 * Das Fenster, auf das gerendert wird
	 */
	SDL_Window* window;

	/**
 	* die Ausmaße des Fensters
 	*/
	SDL_Rect windowSize;
	double bgOffsetX;

	struct {
		bool aHeld;
		bool dHeld;
		bool spaceHeld;
	} controls;

	/**
	 * Der Renderer, der den Render-shit ganz am Rande rendert
	 */
	SDL_Renderer* renderer;

	/**
	 *  Welcher der Modi aktiv ist
	 */
	enum JB_ModeType modeType;
	union JB_GameMode mode;
	/**
	 * Daten, die zu dem jeweiligen Modus gehören, nicht allgemein zum Spiel
	 */
	struct JB_GameModeData data;
	int bestScore;
	int bananaScore;
	struct {
		JB_Asset* background1;
		JB_Asset* background2;
		JB_Asset* title;
		JB_Asset* fps;
		JB_Asset* pointCounter;
		JB_Asset* bananaCounter;
	} assetsHardcoded;
	JB_Asset* assets;
	JB_GameObject* bananas;
	JB_GameObject* gameObjects;

	struct {
		Mix_Music* music;
		Mix_Chunk* essen;
		Mix_Chunk* sterben[2];
	} sounds;
	JB_Button* buttons[sizeof(enum JB_ModeType)];

	struct {
		TTF_Font* defaultFont;
	} fonts;
	/**
	 * Die render-Funktionen, wovon die aktuelle jeden Tick ausgeführt wird
	 * @param game ==> das Spiel
	 */
	void (* renderFunctions[sizeof(enum JB_ModeType)])();
	void (* eventHandlerFunctions[sizeof(enum JB_ModeType)])(SDL_Event* event);
} Game;

/**
 * Zahlen werden im Big-Endian-Format gespeichert. Ein Integer nimmt hier 4 Bytes ein.
 */
struct JB_SaveData {
	unsigned char bestScore[4];
	unsigned char bananaScore[4];
};


void JB_init_game(char* name);
void JB_appendAsset(JB_Asset* asset);
void JB_appendGameObject(JB_GameObject* gameObject);
void JB_removeBanana(JB_GameObject* banana);
void JB_SaveData();
void JB_LoadData();
SDL_Texture* JB_loadImage(char* path);

void JB_quit();
void JB_exitWithError(char* position);
void JB_DestroyGameObjects(JB_GameObject* gameObject);
void JB_DestroyAssets(JB_Asset* assets);
void JB_checkBackground();

int JB_filterEvents(__attribute__((unused)) void* _, SDL_Event* event);
bool JB_checkCollision(SDL_Rect hitBox1, SDL_Rect hitBox2);