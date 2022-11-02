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


struct JB_Game_Struct Game = {};

/**
 *
 * @param argc ==> Menge an Argumenten (größer gleich 1)
 * @param argv ==> String-Array an Argumenten für das Programm
 * @return ==> nichts, da immer zuerst die JB_quit() aufgerufen wird
 */
int main(__attribute__((unused)) int argc, __attribute__((unused)) char** argv) {
	// Spiel laden, Texturen in das Spiel laden, usw.
	JB_init_game("Jungle Bungle");
	// am Anfang wird einmal gerendert, danach nur, wenn die entsprechende Zeit
	// (Mindestzeit) abgelaufen ist, um Ressourcen zub schonen
	long t1 = currentTimeMillis();
	long t2 = t1;
	double oldFPS = JB_MAX_FPS;

	SDL_Event event;
	Game.renderFunctions[Game.modeType]();

	while(Game.running) {

		double delta_time = (double) ( t2 - t1 );
		double restTime = ( 1000.0 / JB_MAX_FPS ) - delta_time;

		/**
 		* Ereignisabhandlung
 		*/
		{
			SDL_GetWindowSize(Game.window, &Game.windowSize.w, &Game.windowSize.h);
			if(SDL_PollEvent(&event)) Game.eventHandlerFunctions[Game.modeType](&event);
		}

		/**
		 * Ressourcen schonen
		 */
		if(restTime > 0) {
			t2 = currentTimeMillis();
			continue;
		}

		/**
		 * FPS berechnen und setzen
		 */
		{
			Game.fps = delta_time ? 1000.0 / delta_time : oldFPS;
			oldFPS = Game.fps;
			t1 = currentTimeMillis();
		}

		/**
		 * Positionsberechnung der GameObjects
		 */
		if(Game.modeType == JB_MODE_ROUND) {
			JB_GameObject* player = Game.data.round.player;

			// wenn Knopf gedrückt
			if(Game.controls.dHeld && player->motion.x < JB_MAX_MOTION_RIGHT) player->motion.x++;
			if(Game.controls.aHeld && player->motion.x > -JB_MAX_MOTION_RIGHT) player->motion.x--;
			// wenn Knopf losgelassen
			if(!Game.controls.dHeld && player->motion.x > 0) player->motion.x--;
			if(!Game.controls.aHeld && player->motion.x < 0) player->motion.x++;

			JB_GameObject* currentObj = Game.gameObjects;
			while(currentObj != NULL) {
				// TODO: Kollisionswahrnehmung
				currentObj = currentObj->next;
			}

			// Position aufgrund der Bewegung bestimmen
			int newX = player->hitBox.x + player->motion.x;
			int newY = player->hitBox.y + player->motion.y + Game.data.round.fallspeed++;
			if(newX < Game.windowSize.w - player->hitBox.w && newX > 0) {
				player->hitBox.x = newX;
				player->assets->rect->x = newX;
			}
			if(newY > 0) {
				player->hitBox.y = newY;
				player->assets->rect->y = newY;
			}
		}

		/**
		 * rendern
		 */
		{
			SDL_SetRenderDrawColor(Game.renderer, 0, 0, 0, 255);
			SDL_RenderClear(Game.renderer);
			Game.renderFunctions[Game.modeType]();
			JB_renderFPS();
			SDL_RenderPresent(Game.renderer);
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
	/**
	 * Folgende Zeilen beschreiben einen normalen Aufbau eines SDL-Programms mit Erstellung eines Fensters, setzen des Names, usw...
	 */
	{
		// TODO: Window Size automatisch vom Monitor übernehmen
		Game.name = name;
		if(( Game.error_code = SDL_Init(SDL_INIT_EVERYTHING))) JB_onError("SDL_Init");
		if(( Game.error_code = TTF_Init())) JB_onError("TTF_Init");
		Game.window = SDL_CreateWindow(name,
									   SDL_WINDOWPOS_CENTERED,
									   SDL_WINDOWPOS_CENTERED, 1920, 1080,
									   SDL_WINDOW_RESIZABLE/* | SDL_WINDOW_FULLSCREEN*/);
		if(Game.window == NULL) JB_onError("Create Window");
		if(!( Game.renderer = SDL_CreateRenderer(Game.window, -1, SDL_RENDERER_ACCELERATED)))
			JB_onError("Create Renderer");
	}

	/**
	 * Siehe include/game_logic.h
	 */
	{
		Game.renderFunctions[JB_MODE_ANIMATION] = JB_render_startingAnimation;
		Game.renderFunctions[JB_MODE_MENU] = JB_render_menu;
		Game.renderFunctions[JB_MODE_ROUND] = JB_render_round;
		Game.renderFunctions[JB_MODE_LEVEL_EDITOR] = JB_render_levelEditor;
		Game.eventHandlerFunctions[JB_MODE_ANIMATION] = JB_handleEvents_startingAnimation;
		Game.eventHandlerFunctions[JB_MODE_MENU] = JB_handleEvents_menu;
		Game.eventHandlerFunctions[JB_MODE_ROUND] = JB_handleEvents_round;
		Game.eventHandlerFunctions[JB_MODE_LEVEL_EDITOR] = JB_handleEvents_levelEditor;
	}

	/**
 	* Medien laden
 	*/
	{
		Game.error_code = !IMG_Init(IMG_INIT_PNG);
		if(Game.error_code) JB_onError("IMG init");
		Game.fonts.defaultFont = JB_loadFont("assets/default_font.ttf", 24);

		Game.assetsHardcoded.background = JB_new_Image("assets/sprites/background.png");
		JB_updateAsset(Game.assetsHardcoded.background, (JB_Asset) { .rect=&Game.windowSize }, JB_AssetUpdate_rect);
		Game.assetsHardcoded.title = JB_new_Image("assets/title.png");
		Game.assetsHardcoded.fps = JB_new_Text("FPS: 0", (SDL_Colour) { 255, 255, 255, 255 }, Game.fonts.defaultFont);
		static SDL_Rect r = { 10, 10, 0, 0 };
		JB_updateAsset(Game.assetsHardcoded.fps,
					   (JB_Asset) { .fontFitRect=true, .rect=&r },
					   JB_AssetUpdate_fontFitRect | JB_AssetUpdate_rect);
	}

	/**
	 * Zuletzt wird ein Filter eingebaut, sodass das Spiel immer und jederzeit geschlossen werden kann.
	 * Die Medien (alle Assets) werden zum Schluss noch in den Zwischenspeicher geladen.
	 */
	{
		SDL_SetEventFilter(JB_filterEvents, NULL);
		Game.running = true;
	}
}

void JB_appendAsset(JB_Asset* asset) {
	if(Game.assets == NULL) {
		Game.assets = asset;
		return;
	}
	JB_Asset* current = Game.assets;
	while(current->next != NULL) current = current->next;
	current->next = asset;
}

void JB_appendGameObject(JB_GameObject* gameObject) {
	if(Game.gameObjects == NULL) {
		Game.gameObjects = gameObject;
		return;
	}
	JB_GameObject* current = Game.gameObjects;
	while(current->next != NULL) current = current->next;
	current->next = gameObject;
}


/**
 * Lädt ein Bild, da der relative Pfad abhängig vom Betriebssystem unterschiedlich ist.
 * @param path ==> relativer Pfad zum Bild (z.B. assets/sprites/background.png)
 * @return ==> Referenz zur Textur
 */
SDL_Texture* JB_loadImage(char* path) {
	SDL_Texture* img = IMG_LoadTexture(Game.renderer, path);
	if(img == NULL) {
		img = IMG_LoadTexture(Game.renderer, appendChar("../", path));
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

void JB_DestroyAssets(JB_Asset* assets) {
	if(!assets) return;
	if(assets->next) JB_DestroyAssets(assets->next);
	SDL_DestroyTexture(assets->texture);
	TTF_CloseFont(assets->font);
	free(assets);
}

void JB_DestroyGameObjects(JB_GameObject* gameObject) {
	if(gameObject == NULL) return;
	if(gameObject->next != NULL) JB_DestroyGameObjects(gameObject->next);
	JB_DestroyAssets(gameObject->assets);
}

/**
 * Schließt das Spiel, zerstört alle Texturen usw und beendet das Programm
 * @param JB_Game ==> das Spiel
 */
void JB_quit() {
	Game.running = false;

	// destroy resources
	if(Game.gameObjects) JB_DestroyGameObjects(Game.gameObjects);
	if(Game.assets) JB_DestroyAssets(Game.assets);
	if(Game.assetsHardcoded.background) JB_DestroyAssets(Game.assetsHardcoded.background);
	if(Game.assetsHardcoded.title) JB_DestroyAssets(Game.assetsHardcoded.title);
	if(Game.assetsHardcoded.fps) JB_DestroyAssets(Game.assetsHardcoded.fps);
	SDL_Log("Ressourcen erfolgreich gelöscht");

	// exit
	IMG_Quit();
	TTF_Quit();
	SDL_Quit();
	// Game ist auf dem Stack (statische Variable) und muss nicht befreit werden
	exit(Game.error_code);
}

/**
 * Schmeißt den SDL-Error auf die Konsole
 * @param JB_Game ==> das Spiel
 * @param position ==> die Position im Kodex, an der der Error auftrat, da C keine Stacktrace anbietet...
 */
void JB_onError(char* position) {
	SDL_LogError(Game.error_code, "Error %d %s: %s", Game.error_code, position, SDL_GetError());
	JB_quit();
}