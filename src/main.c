#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

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

//	Game.data.round.showHitboxes = true;
	char buffer[1024];
	getcwd(buffer, sizeof buffer);
	SDL_Log("CWD: %s", buffer);

	while(Game.running) {
		double delta_time = (double) ( t2 - t1 );
		double restTime = ( 1000.0 / JB_MAX_FPS ) - delta_time;

		/**
 		* Ereignisabhandlung
 		*/
		{
			SDL_GetWindowSize(Game.window, &Game.windowSize.w, &Game.windowSize.h);
			if (SDL_PollEvent(&event)) Game.eventHandlerFunctions[Game.modeType](&event);
		}

		/**
		 * Ressourcen schonen
		 */
		if (restTime > 0){
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
		if (Game.modeType == JB_MODE_ROUND){
			/**
			 * Musik abspielen
			 */
			{
				if (Game.sounds.music){
					if (!Mix_PlayingMusic()) Mix_PlayMusic(Game.sounds.music, -1);
					if (Mix_PausedMusic()) Mix_ResumeMusic();
				}
			}
			JB_GameObject* player = Game.data.round.player;
			/**
			 * Spieler stirbt
			 */
			if (player->hitBox.y > Game.windowSize.h){
				int channel = Mix_PlayChannel(-1, Game.sounds.sterben[rand() & 1], false);
				Game.data.round.fallSpeed = 0;
				Game.controls.aHeld = false;
				Game.controls.dHeld = false;
				player->motion.x = 0;
				player->motion.y = 0;
				Game.data.round.windowAdjustment = 0;
				JB_DestroyGameObjects(Game.gameObjects);
				JB_DestroyGameObjects(Game.bananas);
				Game.gameObjects = NULL;
				Game.bananas = NULL;

				if (Game.bestScore < Game.data.round.counter){
					Game.bestScore = Game.data.round.counter;
				}
				JB_SaveData();
				// warten, bis das Sterben-Geräusch zu Ende gespielt wurde
				while(Mix_Playing(channel));
				JB_changeModeToMenu(false);
				continue;
			}

			// wenn Knopf gedrückt
			if (Game.controls.dHeld && player->motion.x < JB_MAX_MOTION_SPEED) player->motion.x++;
			if (Game.controls.aHeld && player->motion.x > -JB_MAX_MOTION_SPEED) player->motion.x--;
			if (Game.controls.spaceHeld && player->motion.y == 0){
				if (Game.data.round.grounded){
					Game.data.round.player->motion.y -= 20;
					Game.data.round.grounded = false;
				}
			}

			// wenn Knopf losgelassen
			if (!Game.controls.dHeld && player->motion.x > 0) player->motion.x--;
			if (!Game.controls.aHeld && player->motion.x < 0) player->motion.x++;

			// Position aufgrund der Bewegung bestimmen
			int newX = player->hitBox.x + player->motion.x;
			int newY = player->hitBox.y + player->motion.y + Game.data.round.fallSpeed++;
			SDL_Rect newHitBox = { newX, newY, player->hitBox.w, player->hitBox.h };

			JB_GameObject* currentObj = Game.gameObjects;
			bool grounded = false;
			int objOut = 0;
			while(currentObj != NULL) {
				if (currentObj->hitBox.x > Game.windowSize.w){
					objOut++;
				}

				/**
				 * Kollision mit anderen Objekten
				 */
				if (!JB_checkCollision(newHitBox, currentObj->hitBox)){
					currentObj = currentObj->next;
					continue;
				}

				/**
				 * Kollision links am Objekt
				 */
				if (currentObj->hitBox.x >= player->hitBox.x + player->hitBox.w){
					player->motion.x = 0;
					newX = currentObj->hitBox.x - player->hitBox.w;
					currentObj = currentObj->next;
					continue;
				}

				/**
				 * Kollision rechts am Objekt
				 */
				if (currentObj->hitBox.x + currentObj->hitBox.w <= player->hitBox.x){
					player->motion.x = 0;
					newX = currentObj->hitBox.x + currentObj->hitBox.w;
					currentObj = currentObj->next;
					continue;
				}

				/**
				 * Kollision oben am Objekt
				 */
				if (currentObj->hitBox.y >= player->hitBox.y + player->hitBox.h){
					grounded = true;
					Game.data.round.fallSpeed = 0;
					player->motion.y = 0;
					newY = currentObj->hitBox.y - player->hitBox.h;
				}

				/**
				 * Kollision unten am Objekt
				 */
				if (currentObj->hitBox.y + currentObj->hitBox.h <= player->hitBox.y){
					player->motion.y = 0;
					newY = currentObj->hitBox.y + currentObj->hitBox.h;
				}

				currentObj = currentObj->next;
			}
			Game.data.round.grounded = grounded;

			if (objOut <= 1){
				JB_generateBlock();
			}

			currentObj = Game.bananas;
			while(currentObj) {
				/**
				 * Kollision mit anderen Objekten
				 */
				if (!JB_checkCollision(newHitBox, currentObj->hitBox)){
					currentObj = currentObj->next;
					continue;
				}

				/**
				 * Entfernen der Banane
				 */
				JB_removeBanana(currentObj);
				Mix_PlayChannel(-1, Game.sounds.essen, 0);
				Game.bananaScore++;
				break;
			}

			/**
			 * Simuliert die vertikale Bewegung des Hintergrundes
			 */
			int offY = Game.assetsHardcoded.background1->rect->y + 30;
			if (newY < player->hitBox.y){
				/**
				 * Wenn sich der Spieler nach oben bewegt, wird der Hintergrund anhand einer Funktion nach unten bewegt
				 * y=-(1/400)(x-120)^2+36
				 */
				Game.bgOffsetX++;
				double y = ( -0.0025 ) * pow(Game.bgOffsetX - 120, 2) + 36;
				/**
				 * Zu kleine Bewegungen ausschließen
				 */
				y++;

				SDL_Rect br = { Game.assetsHardcoded.background1->rect->x, -30 + (int) y,
								Game.assetsHardcoded.background1->rect->w, Game.assetsHardcoded.background1->rect->h };
				JB_updateAsset(Game.assetsHardcoded.background1, (JB_Asset) { .rect=&br }, JB_AssetUpdate_rect);
			} else if (offY > 0){
				/**
				 * Wenn der Spieler nach unten fällt:<br/>
				 * Das selbe wie bei der Bewegung nach oben nur umgekehrt
				 */
				Game.bgOffsetX--;
				double y = ( -0.0025 ) * pow(Game.bgOffsetX - 120, 2) + 36;
				SDL_Rect br = { Game.assetsHardcoded.background1->rect->x, -30 + (int) y,
								Game.assetsHardcoded.background1->rect->w, Game.assetsHardcoded.background1->rect->h };
				JB_updateAsset(Game.assetsHardcoded.background1, (JB_Asset) { .rect=&br }, JB_AssetUpdate_rect);
			}

			/**
			 * Damit der Spieler sich nicht aus dem Bildschirm bewegen kann, wird der Spieler anhand einer Funktion relativ in die Mitte des Bildschirms gehalten.<br/>
			 * Die Grundfunktionsgleichung ist:<br/>y=(JB_MAX_MOTION_SPEED * 10/-x) + JB_MAX_MOTION_SPEED + 1
			 */
			int altNewX = newX;
			if (Game.data.round.windowAdjustment >= 0 &&
				(( Game.controls.dHeld && !Game.controls.aHeld ) || player->motion.x > 0 )){
				/**
				 * Grundbedingung: Der Spieler hat sich davor nicht bewegt oder er bewegt sich nach rechts.<br/>
				 * Dazu müssen noch Nebenbedingungen erfüllt sein, damit es zu keinen Fehlern oder überschneidungen kommt
				 */
				Game.data.round.windowAdjustment++;

				/**
				 * Überprüfung, ob der Spieler davor stand oder nicht. Das ist nötig, da die Grundfunktion bei werten unter JB_MAX_MOTION_SPEED + 1 im negativen Bereich liegt
				 */
				Game.data.round.windowAdjustment = ( Game.data.round.windowAdjustment == 1 ) ? JB_MAX_MOTION_SPEED + 1
																							 : Game.data.round.windowAdjustment;

				/**
				 * Berechnung der tatsächlich Dargestellten Bewegung des Spielers. windowAdjustment wird dabei als x-Koordinate verwendet
				 */
				int adjY = ( JB_MAX_MOTION_SPEED * 10 / -Game.data.round.windowAdjustment ) + JB_MAX_MOTION_SPEED + 1;

				/**
				 * Wenn der Spieler im Bild schon zum erlaubten Bereich bewegt wurde, bleibt die Position konstant
				 */
				if (adjY > JB_MAX_MOTION_SPEED) adjY = JB_MAX_MOTION_SPEED;
				if (adjY > newX - player->hitBox.x) adjY = newX - player->hitBox.x;

				/**
				 * Die Bewegung des Spielers wird durchgeführt
				 */
				newX -= adjY;
			} else if (Game.data.round.windowAdjustment <= 0 &&
					   (( Game.controls.aHeld && !Game.controls.dHeld ) || player->motion.x < 0 )){
				/**
				 * Das selbe wie bei der Bewegung nach rechts nur umgekehrt
				 */
				Game.data.round.windowAdjustment--;
				Game.data.round.windowAdjustment = ( Game.data.round.windowAdjustment == -1 ) ? -1 - JB_MAX_MOTION_SPEED
																							  : Game.data.round.windowAdjustment;
				int adjY = ( JB_MAX_MOTION_SPEED * 10 / Game.data.round.windowAdjustment ) + JB_MAX_MOTION_SPEED + 1;
				if (adjY > JB_MAX_MOTION_SPEED) adjY = JB_MAX_MOTION_SPEED;
				if (adjY > player->hitBox.x - newX) adjY = player->hitBox.x - newX;
				newX += adjY;
			} else if (Game.data.round.windowAdjustment != 0){
				/**
				 * Wenn der Spieler nicht mehr bewegt wird, wird windowAdjustment nach und nach wieder auf 0 gesetzt
				 */
				int center = ( Game.windowSize.w - player->hitBox.w ) / 2 - player->hitBox.x;
				if (center <= 5 && center >= -5) Game.data.round.windowAdjustment = 0;
				if (center > 0) newX = ( Game.windowSize.w - player->hitBox.w ) / 2 - center + 5;
				else if (center < 0) newX = ( Game.windowSize.w - player->hitBox.w ) / 2 - center - 5;
			}

			/**
			 * Die Bewegung des Spielers wird auf die Objekte angewendet um die Illusion einer Bewegung des Spielers zu erzeugen
			 */
			currentObj = Game.gameObjects;
			while(currentObj != NULL) {
				currentObj->hitBox.x += newX - altNewX;
				currentObj->assetsBox.x = currentObj->hitBox.x;
				currentObj = currentObj->next;
			}

			currentObj = Game.bananas;
			while(currentObj != NULL) {
				currentObj->hitBox.x += newX - altNewX;
				currentObj->assetsBox.x = currentObj->hitBox.x;
				currentObj = currentObj->next;
			}

			/**
			 * Die Bewegung wird nun auf die x-Koordinate des Hintergrundes angewendet. Allerdings bewegt sich dieser halb so schnell wie der Spieler
			 */
			double x = Game.assetsHardcoded.background1->rect->x + (double) ( newX - altNewX ) / 2;
			SDL_Rect br1 = { (int) x, Game.assetsHardcoded.background1->rect->y,
							 Game.assetsHardcoded.background1->rect->w, Game.assetsHardcoded.background1->rect->h };
			SDL_Rect br2 = { Game.assetsHardcoded.background2->rect->x, Game.assetsHardcoded.background1->rect->y,
							 Game.assetsHardcoded.background1->rect->w, Game.assetsHardcoded.background1->rect->h };

			/**
			 * Wenn der Haupt-Hintergrund außerhalb des Bildschirmes ist, wird der zweite Hintergrund mit dem Haupt-Hintergrund ausgetauscht
			 */
			if (Game.assetsHardcoded.background1->rect->x > Game.windowSize.w ||
				Game.assetsHardcoded.background1->rect->x + Game.assetsHardcoded.background1->rect->w < 0){
				br1.x = br2.x;
			}

			/**
			 * Je nach der Position des Haupt-Hintergrundes wird der zweite Hintergrund an die richtige Stelle gesetzt
			 */
			if (Game.assetsHardcoded.background1->rect->x > -( Game.assetsHardcoded.background1->rect->w / 2 )){
				br2.x = br1.x - br2.w;
			} else {
				br2.x = br1.x + br2.w;
			}

			/**
			 * Die Hintergründe werden nun auf die neuen Koordinaten gesetzt
			 */
			JB_updateAsset(Game.assetsHardcoded.background1, (JB_Asset) { .rect=&br1 }, JB_AssetUpdate_rect);
			JB_updateAsset(Game.assetsHardcoded.background2, (JB_Asset) { .rect=&br2 }, JB_AssetUpdate_rect);

			if (newX < Game.windowSize.w - player->hitBox.w && newX > 0){
				player->hitBox.x = newX;
				player->assetsBox.x = newX;
				// player->assets->rect->x = newX;
			}

			if (newY > 0){
				player->hitBox.y = newY;
				player->assetsBox.y = newY;
				// player->assets->rect->y = newY;
			}
		}

		/**
		 * rendern
		 */
		{
			SDL_SetRenderDrawColor(Game.renderer, 0, 0, 0, 0);
			SDL_RenderClear(Game.renderer);
			JB_renderAssets(Game.assetsHardcoded.background1);
			JB_renderAssets(Game.assetsHardcoded.background2);

			Game.renderFunctions[Game.modeType]();

			// JB_renderFPS();
			SDL_RenderPresent(Game.renderer);
		}
		t2 = currentTimeMillis();
	}
	return 0;
}

void JB_removeBanana(JB_GameObject* banana) {
	JB_GameObject* currentObj = Game.bananas;
	if (currentObj == banana){
		Game.bananas = currentObj->next;
		free(currentObj);
	} else {
		while(currentObj) {
			if (currentObj->next == banana){
				currentObj->next = currentObj->next->next;
				free(banana);
				return;
			}
			currentObj = currentObj->next;
		}
	}
}

/**
 * Daten werden im Big-Endian-Format gespeichert.
 * Ein Integer ist 4 Bytes groß, also quasi 4 char.
 * Das erste Byte wird mit den größten 8 bit des integers behandelt.
 */
void JB_SaveData() {
	struct JB_SaveData saveData;
	saveData.bananaScore[0] = (char) ( Game.bananaScore >> 24 );
	saveData.bananaScore[1] = (char) ( Game.bananaScore >> 16 );
	saveData.bananaScore[2] = (char) ( Game.bananaScore >> 8 );
	saveData.bananaScore[3] = (char) Game.bananaScore;
	saveData.bestScore[0] = (char) ( Game.bestScore >> 24 );
	saveData.bestScore[1] = (char) ( Game.bestScore >> 16 );
	saveData.bestScore[2] = (char) ( Game.bestScore >> 8 );
	saveData.bestScore[3] = (char) Game.bestScore;

	// Die Spielstände sollen im gleichen Ordner wie die Assets gespeichert werden.
	char* path = "saves";
	DIR* assetsDir = opendir("assets");
	if (!assetsDir) path = "../saves";
	// ist der saves-Ordner noch nicht vorhanden, wird er erstellt.
	DIR* dir = opendir(path);
#if defined(_WIN32)
	if (!dir) mkdir(path);
#elif defined(__unix__)
	if (!dir) mkdir(path, 0777);
#endif

	// Die Datei wird automatisch erstellt, wenn sie nicht existiert.
	// Der Dateimodus ist wb+ ==> write binary, create if not existing
	char name[] = "/data.jb";
	char dataJBPath[(sizeof path) + (sizeof name) -1] = {0};
	strcat(dataJBPath, path);
	strcat(dataJBPath, name);
	FILE* file = fopen(dataJBPath, "wb+");
	fwrite(&saveData, sizeof saveData, 1, file);
	fclose(file);
}

void JB_LoadData() {
	FILE* file;
	struct JB_SaveData saveData;
	file = fopen("saves/data.jb", "r");
	if (file == NULL){
		SDL_Log("Konnte Datei nicht öffnen");
		return;
	}
	fread(&saveData, sizeof saveData, 1, file);
	fclose(file);
	Game.bestScore =
			(int) ( saveData.bestScore[0] << 24 )
			+ ( saveData.bestScore[1] << 16 )
			+ ( saveData.bestScore[2] << 8 )
			+ ( saveData.bestScore[3] );
	Game.bananaScore =
			(int) ( saveData.bananaScore[0] << 24 )
			+ ( saveData.bananaScore[1] << 16 )
			+ ( saveData.bananaScore[2] << 8 )
			+ ( saveData.bananaScore[3] );
}

bool JB_checkCollision(SDL_Rect hitBox1, SDL_Rect hitBox2) {
	if (hitBox1.x + hitBox1.w < hitBox2.x) return false;
	if (hitBox1.x > hitBox2.x + hitBox2.w) return false;
	if (hitBox1.y + hitBox1.h < hitBox2.y) return false;
	if (hitBox1.y > hitBox2.y + hitBox2.h) return false;
	return true;
}

/**
 * Erstellt ein neues Spiel. Diese Funktion sollte nur einmal aufgerufen werden.
 * @param name ==> der Name des Spiels
 */
void JB_init_game(char* name) {
	SDL_Rect screenSize;
	/**
	 * Folgende Zeilen beschreiben einen normalen Aufbau eines SDL-Programms mit Erstellung eines Fensters, setzen des Names, usw...
	 */
	{
		Game.name = name;
		if ((Game.error_code = SDL_Init(SDL_INIT_EVERYTHING))) JB_exitWithError("SDL_Init");
		if (( Game.error_code = TTF_Init())) JB_exitWithError("TTF_Init");
		SDL_GetDisplayUsableBounds(0, &screenSize);
		Game.window = SDL_CreateWindow(name,
									   SDL_WINDOWPOS_CENTERED,
									   SDL_WINDOWPOS_CENTERED, screenSize.w, screenSize.h,
									   SDL_WINDOW_RESIZABLE);

		if (Game.window == NULL) JB_exitWithError("Create Window");
		if (!( Game.renderer = SDL_CreateRenderer(Game.window, -1, SDL_RENDERER_ACCELERATED)))
			JB_exitWithError("Create Renderer");
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
		if (Game.error_code) JB_exitWithError("IMG init");
		Game.fonts.defaultFont = JB_loadFont("assets/default_font.ttf", 24);

		Game.assetsHardcoded.background1 = JB_new_Image("assets/sprites/background.png");
		static SDL_Rect br1 = { -10, -30, 4224,0 };
		br1.h = screenSize.h;
		JB_updateAsset(Game.assetsHardcoded.background1, (JB_Asset) { .rect=&br1 }, JB_AssetUpdate_rect);

		Game.assetsHardcoded.background2 = JB_new_Image("assets/sprites/background.png");
		static SDL_Rect br2 = { 4224 - 10, -30, 4224, 1188 };
		JB_updateAsset(Game.assetsHardcoded.background2, (JB_Asset) { .rect=&br2 }, JB_AssetUpdate_rect);

		Game.assetsHardcoded.title = JB_new_Image("assets/title.png");

/*		Game.assetsHardcoded.fps = JB_new_Text("FPS: 0", (SDL_Colour) { 255, 255, 255, 255 }, Game.fonts.defaultFont);
		static SDL_Rect r1 = { 10, 10, 0, 0 };
		JB_updateAsset(Game.assetsHardcoded.fps,
					   (JB_Asset) { .fontFitRect=true, .rect=&r1 },
					   JB_AssetUpdate_fontFitRect | JB_AssetUpdate_rect);*/

		Game.assetsHardcoded.pointCounter = JB_new_Text("Points: 0", (SDL_Colour) { 255, 255, 255, 255 },
														Game.fonts.defaultFont);
		static SDL_Rect r2 = { 10, 10, 0, 0 };
		JB_updateAsset(Game.assetsHardcoded.pointCounter,
					   (JB_Asset) { .fontFitRect=true, .rect=&r2 },
					   JB_AssetUpdate_fontFitRect | JB_AssetUpdate_rect);

		Game.assetsHardcoded.bananaCounter = JB_new_Text("Bananas: 0", (SDL_Colour) { 255, 255, 255, 255 },
														 Game.fonts.defaultFont);
		static SDL_Rect r3 = { 10, 40, 0, 0 };
		JB_updateAsset(Game.assetsHardcoded.bananaCounter,
					   (JB_Asset) { .fontFitRect=true, .rect=&r3 },
					   JB_AssetUpdate_fontFitRect | JB_AssetUpdate_rect);
	}

	/**
	 * Musik laden
	 */
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
	Game.sounds.music = Mix_LoadMUS("assets/sounds/JungleBungle.mp3");
	if (Game.sounds.music == NULL) SDL_Log("Musik konnte nicht geladen werden,");

	Game.sounds.essen = Mix_LoadWAV("assets/sounds/essen.wav");
	if (Game.sounds.essen == NULL) SDL_Log("essem-Geräusch konnte nicht geladen werden,");

	Game.sounds.sterben[0] = Mix_LoadWAV("assets/sounds/sterben.wav");
	if (Game.sounds.essen == NULL) SDL_Log("sterben1-Geräusch konnte nicht geladen werden,");

	Game.sounds.sterben[1] = Mix_LoadWAV("assets/sounds/sterben2.wav");
	if (Game.sounds.essen == NULL) SDL_Log("sterben2-Geräusch konnte nicht geladen werden,");

	srand(time(NULL));
	JB_LoadData();

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
	if (Game.assets == NULL){
		Game.assets = asset;
		return;
	}
	JB_Asset* current = Game.assets;
	while(current->next != NULL) current = current->next;
	current->next = asset;
}

void JB_appendGameObject(JB_GameObject* gameObject) {
	if (Game.gameObjects == NULL){
		Game.gameObjects = gameObject;
		return;
	}
	JB_GameObject* current = Game.gameObjects;
	while(current->next != NULL) current = current->next;
	current->next = gameObject;
}


/**
 * Lädt ein Bild, da der relative Pfad abhängig vom Betriebssystem unterschiedlich ist.
 * @param path ==> relativer Pfad zum Bild (z.B. assets/sprites/background1.png)
 * @return ==> Referenz zur Textur
 */
SDL_Texture* JB_loadImage(char* path) {
	SDL_Texture* img = IMG_LoadTexture(Game.renderer, path);
	if (img == NULL){
		img = IMG_LoadTexture(Game.renderer, appendChar("../", path));
		if (img == NULL) JB_exitWithError("Texture loading");
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
	if (event->type == SDL_QUIT) JB_quit();
	if (event->type == SDL_WINDOWEVENT){
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
	if (!assets) return;
	if (assets->next) JB_DestroyAssets(assets->next);
	SDL_DestroyTexture(assets->texture);
	TTF_CloseFont(assets->font);
	free(assets);
}

void JB_DestroyGameObjects(JB_GameObject* gameObject) {
	if (gameObject == NULL) return;
	if (gameObject->next != NULL) JB_DestroyGameObjects(gameObject->next);
	JB_DestroyAssets(gameObject->assets);
}

/**
 * Schließt das Spiel, zerstört alle Texturen usw und beendet das Programm
 * @param JB_Game ==> das Spiel
 */
void JB_quit() {
	Game.running = false;

	// destroy resources
	if (Game.gameObjects) JB_DestroyGameObjects(Game.gameObjects);
	if (Game.assets) JB_DestroyAssets(Game.assets);
	if (Game.assetsHardcoded.background1) JB_DestroyAssets(Game.assetsHardcoded.background1);
	if (Game.assetsHardcoded.title) JB_DestroyAssets(Game.assetsHardcoded.title);
	if (Game.assetsHardcoded.fps) JB_DestroyAssets(Game.assetsHardcoded.fps);
	if (Game.assetsHardcoded.pointCounter) JB_DestroyAssets(Game.assetsHardcoded.pointCounter);
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
void JB_exitWithError(char* position) {
	SDL_LogError(Game.error_code, "Error %d %s: %s", Game.error_code, position, SDL_GetError());
	JB_quit();
}