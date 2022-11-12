/**
 * Diese Datei beschreibt alle möglichen Funktionen zur Steuerung des Spiels.
 * Jede Render-Funktion wird jeden Tick ausgeführt. Sie dürfen auf keinen Fall durch zB SDL_Delay() verzögert werden!
 * Ansonsten passieren unschöne Renderfehler.
 * Damit man dennoch zB über eine Schleife einen Farbwert anpassen, indem man den Zähler in Game.mode.MODE.counter
 * speichert und am Ende der Funktion erhöht.
 * Alle Daten, die einer Funktion übergeben werden sollen oder in der Funktion genutzt werden sollen, müssen in game
 * gespeichert sein!
 */
#pragma once
#include "modes.h"

void JB_renderFPS();
void JB_setHover(enum JB_ModeType modeType, SDL_Event* event);

// Startanimation
void JB_render_startingAnimation();
void JB_render_startingAnimation_StageOne();
void JB_render_startingAnimation_StageTwo();
void JB_handleEvents_startingAnimation(SDL_Event* event);

// Menü
void JB_changeModeToMenu(bool pause);
void JB_render_menu();
void JB_handleEvents_menu(SDL_Event* event);
void JB_onTestButtonClick(JB_Button* this);

// Runde
void JB_changeModeToRound();
void JB_render_round();
void JB_handleEvents_round(SDL_Event* event);
void JB_generateBlock();

// Leveleditor
void JB_changeModeToLevelEditor();
void JB_render_levelEditor();
void JB_handleEvents_levelEditor(SDL_Event* event);
void JB_initDrag(JB_Button* this);
void JB_drag(SDL_MouseMotionEvent motion, bool aligned);

void JB_new_MenuButton(JB_Button* button, char* string);