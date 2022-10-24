#pragma once

#include "things/game_objects.h"

/**
 * Der Modus des Spiels.<br/>
 * Dieser Spaß wird aufgeteilt, um Speicher zu sparen und die Logik ein bisschen einfacher zu halten.<br/>
 * Ist das Spiel gerade im Runden-Modus, ist (JB_Game.modeType == JB_MODE_ROUND)  und dann wird weiter unterteilt,
 * ob zB im Runden-Modus gerade ein Menü angezeigt wird, oder eine Animation stattfindet usw...
 */
enum JB_ModeType {
	JB_MODE_ANIMATION,
	JB_MODE_MENU,
	JB_MODE_ROUND,
	JB_MODE_LEVEL_EDITOR,
};

/**
 * Die verschiedenen Modi, in denen sich das Spiel befinden kann.
 * Da das Spiel maximal einen dieser Modi gleichzeitig haben kann, wird (um Platz zu sparen)
 * eine Union anstatt eines Structs verwendet.
 */
union JB_GameMode {
	/**
	 * Wenn sich das Spiel im Startanimation-Modus befindet
	 */
	struct {
		enum {
			JB_MODE_STARTING_0,
			JB_MODE_STARTING_1,
		} state;
	} startAnimation;
	/**
	 *  Wenn sich das Spiel im Menü-Modus befindet
	 */
	struct {
		enum { JB_MODE_MENU_MAIN } state;
	} menu;

	/**
	 *  Wenn sich das Spiel im Runden-Modus befindet
	 */
	struct {
		enum { JB_MODE_ROUND_WALK } state;
	} round;

	struct {

	} levelEditor;
};

struct JB_GameModeData {
	struct {
		int counter;
	} startAnimation;
	struct {
		int counter;
	} menu;
	struct {
		/**
		 * der Affe :)
		 */
		JB_GameObject* player;
	} round;
	struct {
		/**
		* Das anklickbare Object, was gerade ausgewählt ist und zB verschoben wird
 		*/
		struct JB_Button* selectedClickableObject;
		struct { int x, y; } oldMousePos;
	} levelEditor;
};

/**
 * Ein Feld aus verlinkten Listen, die anklickbare Objekte enthalten
 */
typedef struct JB_Button {
	/**
	 * die Ausmaße des Objekts
	 */
	SDL_Rect rect;
	SDL_Rect alignment;
	/**
	 * alle Texturen, die auf das Objekt angewendet werden
	 * zB. ein Text "Play" und ein Hintergrund des Textes
	 */
	JB_Asset* textureElements;
	/**
	 * ob gerade über das Objekt geschwebt wird
	 */
	bool hover;
	/**
	 * die Funktion, welche bei einem Klick ausgeführt werden soll
	 * @param game ==> das Spiel
	 */
	void (* onclick)();
	/**
	 * verlinkte Liste zum nächste anklickbaren Objekt
	 */
	struct JB_Button* next;
} JB_Button;