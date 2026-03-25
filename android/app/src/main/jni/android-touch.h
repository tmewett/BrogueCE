#ifndef ANDROID_TOUCH_H
#define ANDROID_TOUCH_H

#include <SDL.h>
#include "Rogue.h"

/*
 * Process an SDL event for Android touch gestures.
 * Returns true and fills `out` if a rogueEvent was produced.
 */
boolean androidTouchEvent(SDL_Event *event, rogueEvent *out);

/*
 * Extract bundled assets from the APK to internal storage so that
 * fopen-based game code can access them as regular files.
 * `destDir` is the app's internal files path (from SDL_GetPrefPath).
 */
void androidExtractAssets(const char *destDir);

/* Clear pending touch state. Call on game state transitions to prevent
 * events from one screen leaking into the next. */
void androidResetTouchState(void);

/* Show/hide the native Android inventory UI.
 * json is a JSON array of item objects. */
void androidShowInventory(const char *json);
void androidHideInventory(void);

/* Zoom level for pinch-to-zoom. 1.0 = full grid, >1.0 = zoomed in. */
extern float androidZoomLevel;

/* Pan offset in pixels, applied when zoomed in. */
extern float androidPanX, androidPanY;

/* True while user is two-finger dragging — suppresses auto-center. */
extern boolean androidPanOverride;

#endif
