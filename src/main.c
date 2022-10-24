#include <unistd.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "include/main.h"
#include "include/util.h"
#include "include/game_logic.h"
#include "include/modes.h"
#include "include/asset.h"
#include "include/things/game_objects.h"

/**
 *
 * @param argc ==> Menge an Argumenten (größer gleich 1)
 * @param argv ==> String-Array an Argumenten für das Programm
 * @return ==> nichts, da immer zuerst die JB_quit() aufgerufen wird
 */
int main(__attribute__((unused)) int argc, __attribute__((unused)) char** argv) {
	// Spiel laden, Texturen in das Spiel laden, usw.
	JB_init_game("Jungle Bungle");
	JB_Game.gameLogicThread = SDL_CreateThread((SDL_ThreadFunction) JB_EventHandler, "JB_EventHandler",
											   &JB_Game);

	// am Anfang wird einmal gerendert, danach nur, wenn die entsprechende Zeit
	// (Mindestzeit) abgelaufen ist, um Ressourcen zub schonen
	long t1 = currentTimeMillis();
	long t2 = t1;

	SDL_GetWindowSize(JB_Game.window, &JB_Game.windowSize.width, &JB_Game.windowSize.height);
	JB_Game.renderFunctions[JB_Game.modeType]();

	while(JB_Game.running) {
		// Das muss immer laufen. Ohne Verzögerung, ohne sonst irgendwas!
		SDL_PumpEvents();
		SDL_GetWindowSize(JB_Game.window, &JB_Game.windowSize.width, &JB_Game.windowSize.height);

		double delta_time = (double) ( t2 - t1 );
		double restTime = ( 1000.0 / MAX_FPS ) - delta_time;
		if(restTime <= 0) {
			JB_Game.fps = delta_time ? 1000.0 / delta_time : MAX_FPS;
			t1 = currentTimeMillis();

			SDL_SetRenderDrawColor(JB_Game.renderer, 0, 0, 0, 255);
			SDL_RenderClear(JB_Game.renderer);

			JB_Game.renderFunctions[JB_Game.modeType]();

			JB_renderFPS();
			SDL_RenderPresent(JB_Game.renderer);
		}
		t2 = currentTimeMillis();
	}
}


/**
 * Erstellt ein neues Spiel. Diese Funktion sollte nur einmal aufgerufen werden.
 * @param name ==> der Name des Spiels
 * @return Referenz zum neuen Spiel-Struct
 */
void JB_init_game(char* name) {
	/*
	 * Es soll nur ein Spiel geben können, weshalb auch nur eine (statische) Variable davon erstellt wird.
	 * Führt man diese Funktion mehrmals aus, wird der gleiche Zeiger zurückgegeben.
	 * Folgende Zeilen beschreiben einen normalen Aufbau eines SDL-Programms mit Erstellung eines Fensters, setzen des Names, usw...
	 */
	JB_Game.name = name;
	JB_Game.error_code = SDL_Init(SDL_INIT_EVERYTHING);
	if(JB_Game.error_code) JB_onError("SDL_Init");

	JB_Game.error_code = TTF_Init();
	if(JB_Game.error_code) JB_onError("TTF_Init");

	JB_Game.window = SDL_CreateWindow(name,
									  SDL_WINDOWPOS_CENTERED,
									  SDL_WINDOWPOS_CENTERED,
									  1920, 1080,
									  SDL_WINDOW_RESIZABLE);
	if(JB_Game.window == NULL) {
		JB_Game.error_code = 1;
		JB_onError("Create Window");
	}
	JB_Game.renderer = SDL_CreateRenderer(JB_Game.window, -1, SDL_RENDERER_ACCELERATED);
	if(JB_Game.renderer == NULL) {
		JB_Game.error_code = 1;
		JB_onError("Create Renderer");
	}

	/*
	 * Siehe include/game_logic.h
	 */
	JB_Game.renderFunctions[JB_MODE_ANIMATION] = JB_render_startingAnimation;
	JB_Game.renderFunctions[JB_MODE_MENU] = JB_render_menu;
	JB_Game.renderFunctions[JB_MODE_ROUND] = JB_render_round;
	JB_Game.renderFunctions[JB_MODE_LEVEL_EDITOR] = JB_render_levelEditor;
	JB_Game.eventHandlerFunctions[JB_MODE_ANIMATION] = JB_handleEvents_startingAnimation;
	JB_Game.eventHandlerFunctions[JB_MODE_MENU] = JB_handleEvents_menu;
	JB_Game.eventHandlerFunctions[JB_MODE_ROUND] = JB_handleEvents_round;
	JB_Game.eventHandlerFunctions[JB_MODE_LEVEL_EDITOR] = JB_handleEvents_levelEditor;

	/*
	 * Zuletzt wird ein Filter eingebaut, sodass das Spiel immer und jederzeit geschlossen werden kann.
	 * Die Medien (alle Assets) werden zum Schluss noch in den Zwischenspeicher geladen.
	 */
	SDL_SetEventFilter((SDL_EventFilter) JB_filterEvents, NULL);
	JB_loadMedia();
	JB_Game.running = true;
}

void JB_appendAsset(JB_Asset* asset) {
	if(JB_Game.assets == NULL) {
		JB_Game.assets = asset;
		return;
	}
	JB_Asset* current = JB_Game.assets;
	while(current->next != NULL) current = current->next;
	current->next = asset;
}

void JB_appendGameObject(JB_GameObject* gameObject) {
	if(JB_Game.gameObjects == NULL) {
		JB_Game.gameObjects = gameObject;
		return;
	}
	JB_GameObject* current = JB_Game.gameObjects;
	while(current->next != NULL) current = current->next;
	current->next = gameObject;
}

/**
 * Lädt die Texturen in das Spiel
 * @param JB_Game ==> Referenz zum Spiel
 */
void JB_loadMedia() {
	JB_Game.error_code = !IMG_Init(IMG_INIT_PNG);
	if(JB_Game.error_code) JB_onError("IMG init");
	JB_Game.fonts.defaultFont = JB_loadFont("assets/roboto.ttf", 24);

	JB_Game.assetsHardcoded.background = JB_new_Image("assets/background.png");
	JB_Game.assetsHardcoded.title = JB_new_Image("assets/title.png");
	JB_Game.assetsHardcoded.fps = JB_new_Text("FPS: 0", (SDL_Colour) { 255, 255, 255 }, JB_Game.fonts.defaultFont);
	SDL_Rect r = { 0, 0, 100, 100 };
	JB_updateAsset(JB_Game.assetsHardcoded.fps,
				   (JB_Asset) { .fontFitRect=true, .rect=&r },
				   JB_AssetUpdate_fontFitRect | JB_AssetUpdate_rect);
}

/**
 * Lädt ein Bild, da der relative Pfad abhängig vom Betriebssystem unterschiedlich ist.
 * @param JB_Game ==> Referenz zum Spiel
 * @param path ==> relativer Pfad zum Bild (z.B. assets/background.png)
 * @return ==> Referenz zur Textur
 */
SDL_Texture* JB_loadImage(char* path) {
	SDL_Texture* img;
	img = IMG_LoadTexture(JB_Game.renderer, path);
	if(img == NULL) {
		img = IMG_LoadTexture(JB_Game.renderer, appendChar("../", path));
		if(img == NULL) JB_onError("Texture loading");
	}
	return img;
}

/**
 *
 * @param JB_Game ==> Das Spiel
 * @param event ==> das zu überprüfende Ereignis
 * @return <ul>
 *		<li> 0, wenn das Ereignis ein WindowEvent ist</li>
 *		<li> 1, wenn das Ereignis der Schlange angefügt werden soll</li>
 * </ul>
 */
int JB_filterEvents(__attribute__((unused)) void* _, SDL_Event* event) {
	if(event->type == SDL_QUIT) JB_quit();
	if(event->type == SDL_WINDOWEVENT) {
		switch(event->window.event) {
			case SDL_WINDOWEVENT_NONE:
				SDL_Log("SDL_WINDOWEVENT_NONE");
				break;
			case SDL_WINDOWEVENT_SHOWN:
				SDL_Log("SDL_WINDOWEVENT_SHOWN");
				break;
//			case SDL_WINDOWEVENT_HIDDEN:
//				SDL_Log("SDL_WINDOWEVENT_HIDDEN");
//				break;
//			case SDL_WINDOWEVENT_EXPOSED:
//				SDL_Log("SDL_WINDOWEVENT_EXPOSED");
//				break;
//			case SDL_WINDOWEVENT_MOVED:
//				SDL_Log("SDL_WINDOWEVENT_MOVED");
//				break;
//			case SDL_WINDOWEVENT_RESIZED:
//				SDL_Log("SDL_WINDOWEVENT_RESIZED");
//				break;
//			case SDL_WINDOWEVENT_SIZE_CHANGED:
//				SDL_Log("SDL_WINDOWEVENT_SIZE_CHANGED");
//				break;
//			case SDL_WINDOWEVENT_MINIMIZED:
//				SDL_Log("SDL_WINDOWEVENT_MINIMIZED");
//				break;
//			case SDL_WINDOWEVENT_MAXIMIZED:
//				SDL_Log("SDL_WINDOWEVENT_MAXIMIZED");
//				break;
//			case SDL_WINDOWEVENT_RESTORED:
//				SDL_Log("SDL_WINDOWEVENT_RESTORED");
//				break;
//			case SDL_WINDOWEVENT_ENTER:
//				SDL_Log("SDL_WINDOWEVENT_ENTER");
//				break;
//			case SDL_WINDOWEVENT_LEAVE:
//				SDL_Log("SDL_WINDOWEVENT_LEAVE");
//				break;
//			case SDL_WINDOWEVENT_FOCUS_GAINED:
//				SDL_Log("SDL_WINDOWEVENT_FOCUS_GAINED");
//				break;
//			case SDL_WINDOWEVENT_FOCUS_LOST:
//				SDL_Log("SDL_WINDOWEVENT_FOCUS_LOST");
//				break;
			case SDL_WINDOWEVENT_CLOSE:
				SDL_Log("SDL_WINDOWEVENT_CLOSE");
				break;
//			case SDL_WINDOWEVENT_TAKE_FOCUS:
//				SDL_Log("SDL_WINDOWEVENT_TAKE_FOCUS");
//				break;
//			case SDL_WINDOWEVENT_HIT_TEST:
//				SDL_Log("SDL_WINDOWEVENT_HIT_TEST");
//				break;
		}
		return 0;
	}
	return 1;
}

static void JB_DestroyAssets(JB_Asset* pt) {
	if(pt == NULL) return;
	if(pt->next != NULL) JB_DestroyAssets(pt->next);
	SDL_DestroyTexture(pt->texture);
	TTF_CloseFont(pt->font);
	free(pt);
}

static void JB_DestroyGameObjects(JB_GameObject* gameObject) {
	if(gameObject == NULL) return;
	if(gameObject->next != NULL) JB_DestroyGameObjects(gameObject->next);
	JB_DestroyAssets(gameObject->textureElements);
	free(gameObject);
}

/**
 * Schließt das Spiel, zerstört alle Texturen usw und beendet das Programm
 * @param JB_Game ==> das Spiel
 */
void JB_quit() {
	JB_Game.running = false;
	// JB_quit JB_Game logic thread
	if(JB_Game.gameLogicThread) SDL_DetachThread(JB_Game.gameLogicThread);
	SDL_Log("Spiellogik-Thread erfolgreich geschlossen.");

	// destroy resources
	JB_DestroyGameObjects(JB_Game.gameObjects);
	JB_DestroyAssets(JB_Game.assets);
	if(JB_Game.assetsHardcoded.background) JB_DestroyAssets(JB_Game.assetsHardcoded.background);
	if(JB_Game.assetsHardcoded.title) JB_DestroyAssets(JB_Game.assetsHardcoded.title);
	if(JB_Game.assetsHardcoded.fps) JB_DestroyAssets(JB_Game.assetsHardcoded.fps);

	if(JB_Game.renderer) SDL_DestroyRenderer(JB_Game.renderer);
	if(JB_Game.window) SDL_DestroyWindow(JB_Game.window);
	SDL_Log("Ressourcen erfolgreich gelöscht");

	// exit
	IMG_Quit();
	TTF_Quit();
	SDL_Quit();
	// JB_Game ist auf dem Stack (statische Variable) und muss nicht befreit werden
	exit(JB_Game.error_code);
}


/**
 * Schmeißt den SDL-Error auf die Konsole
 * @param JB_Game ==> das Spiel
 * @param position ==> die Position im Kodex, an der der Error auftrat, da C keine Stacktrace anbietet...
 */
void JB_onError(char* position) {
	SDL_LogError(JB_Game.error_code, "Error %d %s: %s", JB_Game.error_code, position, SDL_GetError());
	JB_quit();
}