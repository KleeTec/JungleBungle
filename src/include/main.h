#pragma once
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "asset.h"
#include "modes.h"
#include "things/game_objects.h"


#define MAX_FPS 60.0

/**
 * das Spiel als globales Struct
 */
struct {
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
	struct { int width, height; } windowSize;

	/**
	 * Der Renderer, der den Render-shit ganz am Rande rendert
	 */
	SDL_Renderer* renderer;

	/**
	 * Der Thread, der sich damit befasst, alle die
	 */
	SDL_Thread* gameLogicThread;

	/**
	 *  Welcher der Modi aktiv ist
	 */
	enum JB_ModeType modeType;
	union JB_GameMode mode;
	/**
	 * Daten, die zu dem jeweiligen Modus gehören, nicht allgemein zum Spiel
	 */
	struct JB_GameModeData data;
	/**
	 *
	 */
	struct {
		JB_Asset* background;
		JB_Asset* title;
		JB_Asset* fps;
	} assetsHardcoded;
	JB_Asset* assets;
	JB_GameObject* gameObjects;
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
} JB_Game;


void JB_init_game(char* name);
void JB_appendAsset(JB_Asset* asset);
void JB_appendGameObject(JB_GameObject* gameObject);
SDL_Texture* JB_loadImage(char* path);

void JB_quit();
void JB_onError(char* position);
void JB_loadMedia();

int JB_filterEvents(__attribute__((unused)) void* _, SDL_Event* event);