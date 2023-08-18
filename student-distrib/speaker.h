/** speaker.h
 *  driver for the PC speaker
*/

#include "lib.h"


uint32_t note2freq(char note, uint32_t octave);
char* get_notes();
void play_song(uint32_t* song);
void create_music();
void click_sound();

#define SUPER_MARIO_SIZE 42
#define RICK_ROLL_SIZE   42
#define TAKE_ME_HOME_SIZE 65

uint32_t super_mario_theme_notes[SUPER_MARIO_SIZE];
uint32_t never_gonna_give_you_up[RICK_ROLL_SIZE];
uint32_t take_me_home[TAKE_ME_HOME_SIZE];


