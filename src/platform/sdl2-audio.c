#include <string.h>

#include <SDL.h>
#include <SDL_mixer.h>
#include <espeak-ng/speak_lib.h>

#include "sdl2-audio.h"

int sdlAudioInit() {
    espeak_Initialize(AUDIO_OUTPUT_RETRIEVAL, 0, NULL, 0);
}

void sdlPlaySpeech(char *text) {
    espeak_Synth(text, strlen(text), 0, 0, 0, espeakCHARS_UTF8, NULL, NULL);
}
