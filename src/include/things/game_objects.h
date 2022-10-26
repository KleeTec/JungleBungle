#pragma once

#include <SDL2/SDL.h>

#include "../asset.h"

typedef struct {
	int x;
	int y;
} JB_Vec;

/**
 * ein Objekt im Spiel mit HitBox usw.
 */
typedef struct JB_GameObject {
	/**
	 * die HitBox des Objekts als SDL_Rect
	 */
	SDL_Rect hitBox;
	/**
	 * Der Bewegungsvektor des Objekts
	 */
	JB_Vec motion;
	/**
	 * die Textur des Elements als TextureElement, da die Textur eine andere Größe haben kann als die HitBox
	 */
	JB_Asset* assets;
	/**
	 * verlinkte Liste zum nächsten Objekt
	 */
	struct JB_GameObject* next;
} JB_GameObject;