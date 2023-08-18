#include "speaker.h"
#include "lib.h"
#include "pit.h"
#include "rtc.h"
#include "bga.h"

char NOTES[] = "CcDdEFfGgAaB";

/* the frequency of all the notes in octave 0 */
static const float base_freq[] = {16.35, 17.32, 18.35, 19.45, 20.60, 21.83, 23.12, 24.50, 25.96, 27.50, 29.14, 30.87};

static void create_mario();
static void create_rickroll();
static void create_take_me_home();

/** note2freq
 * DESCRIPTION: converts the passed in note and octave into a frequency which can be sent to PC speaker
 * INPUTS:
 *      note - the note to play
 *      octave - the octave of the note
 * OUTPUTS: frequency of note
*/
uint32_t note2freq(char note, uint32_t octave)
{

    /** frequency-octave relationship found at: 
     * https://pages.mtu.edu/~suits/notefreqs.html 
    */

    // offset into the notes array for the current note
    int note_offset;
    char *tmp;
    int ret;

    /* if note is empty or not in allowed notes, return 0 */
	if (!note || (tmp = strchr(NOTES, note)) == NULL)
		return 0;

    /* only allow up to octave 8 */
    if (octave > 8) return 0;

    note_offset = tmp - &NOTES[0];

    float freq = base_freq[note_offset];

    static int i;
    for(i = 0; i < octave; i++)
    {
        freq *= 2;
    }

    ret = (int)freq;

    // round frequency to nearest whole number
    if(freq - (float)ret >= 0.5) ret++;

    return ret;

}

char* get_notes()
{
    return NOTES;
}


void play_song(uint32_t* song)
{
    int i;

    for(i = 0; song[i] != 0; i++)
    {
        if (stop_pressed)
        {
            break;
        }
        speaker_beep(song[i]);
        rtc_delay_inv(8);
    }
}

void click_sound()
{
    speaker_beep(note2freq('A', 3));
    speaker_beep(note2freq('A', 3));
}



void create_music()
{

    create_mario();
    create_rickroll();
    create_take_me_home();
}


static void create_mario()
{
    memset(super_mario_theme_notes, 0, SUPER_MARIO_SIZE * sizeof(uint32_t));

    super_mario_theme_notes[0] = note2freq('E', 5);
    super_mario_theme_notes[1] = note2freq('E', 5);
    super_mario_theme_notes[2] = note2freq('E', 5);
    super_mario_theme_notes[3] = note2freq('C', 5);
    super_mario_theme_notes[4] = note2freq('E', 5);
    super_mario_theme_notes[5] = note2freq('G', 5);
    super_mario_theme_notes[6] = note2freq('G', 4);

    super_mario_theme_notes[7] = note2freq('C', 5);
    super_mario_theme_notes[8] = note2freq('G', 4);
    super_mario_theme_notes[9] = note2freq('E', 4);
    super_mario_theme_notes[10] = note2freq('A', 4);
    super_mario_theme_notes[11] = note2freq('B', 4);
    super_mario_theme_notes[12] = note2freq('a', 4);
    super_mario_theme_notes[13] = note2freq('A', 4);
    super_mario_theme_notes[14] = note2freq('G', 4);
    super_mario_theme_notes[15] = note2freq('E', 5);
    super_mario_theme_notes[16] = note2freq('G', 5);
    super_mario_theme_notes[17] = note2freq('A', 5);
    super_mario_theme_notes[18] = note2freq('F', 5);
    super_mario_theme_notes[19] = note2freq('G', 5);
    super_mario_theme_notes[20] = note2freq('E', 5);
    super_mario_theme_notes[21] = note2freq('C', 5);
    super_mario_theme_notes[22] = note2freq('D', 5);
    super_mario_theme_notes[23] = note2freq('B', 4);

    super_mario_theme_notes[24] = note2freq('C', 5);
    super_mario_theme_notes[25] = note2freq('G', 4);
    super_mario_theme_notes[26] = note2freq('E', 4);
    super_mario_theme_notes[27] = note2freq('A', 4);
    super_mario_theme_notes[28] = note2freq('B', 4);
    super_mario_theme_notes[29] = note2freq('a', 4);
    super_mario_theme_notes[30] = note2freq('A', 4);
    super_mario_theme_notes[31] = note2freq('G', 4);
    super_mario_theme_notes[32] = note2freq('E', 5);
    super_mario_theme_notes[33] = note2freq('G', 5);
    super_mario_theme_notes[34] = note2freq('A', 5);
    super_mario_theme_notes[35] = note2freq('F', 5);
    super_mario_theme_notes[36] = note2freq('G', 5);
    super_mario_theme_notes[37] = note2freq('E', 5);
    super_mario_theme_notes[38] = note2freq('C', 5);
    super_mario_theme_notes[39] = note2freq('D', 5);
    super_mario_theme_notes[40] = note2freq('B', 4);

}

static void create_rickroll()
{
    memset(never_gonna_give_you_up, 0, RICK_ROLL_SIZE * sizeof(uint32_t));

    never_gonna_give_you_up[0] = note2freq('A', 4);
    never_gonna_give_you_up[1] = note2freq('B', 4);
    never_gonna_give_you_up[2] = note2freq('D', 5);
    never_gonna_give_you_up[3] = note2freq('B', 4);
    never_gonna_give_you_up[4] = note2freq('f', 5);
    never_gonna_give_you_up[5] = note2freq('f', 5);
    never_gonna_give_you_up[6] = note2freq('E', 5);

    never_gonna_give_you_up[7] = note2freq('A', 4);
    never_gonna_give_you_up[8] = note2freq('B', 4);
    never_gonna_give_you_up[9] = note2freq('D', 5);
    never_gonna_give_you_up[10] = note2freq('B', 4);
    never_gonna_give_you_up[11] = note2freq('E', 5);
    never_gonna_give_you_up[12] = note2freq('E', 5);
    never_gonna_give_you_up[13] = note2freq('D', 5);
    never_gonna_give_you_up[14] = note2freq('c', 5);
    never_gonna_give_you_up[15] = note2freq('B', 4);

    never_gonna_give_you_up[16] = note2freq('A', 4);
    never_gonna_give_you_up[17] = note2freq('B', 4);
    never_gonna_give_you_up[18] = note2freq('D', 5);
    never_gonna_give_you_up[19] = note2freq('B', 4);
    never_gonna_give_you_up[20] = note2freq('D', 5);
    never_gonna_give_you_up[21] = note2freq('E', 5);
    never_gonna_give_you_up[22] = note2freq('c', 5);
    never_gonna_give_you_up[23] = note2freq('A', 4);
    never_gonna_give_you_up[24] = note2freq('A', 4);
    never_gonna_give_you_up[25] = note2freq('E', 5);
    never_gonna_give_you_up[26] = note2freq('D', 5);


}

static void create_take_me_home()
{
    memset(take_me_home, 0, TAKE_ME_HOME_SIZE * sizeof(uint32_t));

    take_me_home[0] = note2freq('E', 4);
    take_me_home[1] = note2freq('E', 4);
    take_me_home[2] = note2freq('f', 4);
    take_me_home[3] = note2freq('E', 4);
    take_me_home[4] = note2freq('f', 4);
    take_me_home[5] = note2freq('E', 4);
    take_me_home[6] = note2freq('f', 4);
    take_me_home[7] = note2freq('A', 4);
    take_me_home[8] = note2freq('B', 4);
    take_me_home[9] = note2freq('B', 4);
    take_me_home[10] = note2freq('c', 5);
    take_me_home[11] = note2freq('B', 4);
    take_me_home[12] = note2freq('f', 4);
    take_me_home[13] = note2freq('f', 4);
    take_me_home[14] = note2freq('f', 4);
    take_me_home[15] = note2freq('E', 4);
    take_me_home[16] = note2freq('f', 4);
    take_me_home[17] = note2freq('A', 4);
    take_me_home[18] = note2freq('E', 4);
    take_me_home[19] = note2freq('E', 4);
    take_me_home[20] = note2freq('f', 4);
    take_me_home[21] = note2freq('E', 4);
    take_me_home[22] = note2freq('E', 4);
    take_me_home[23] = note2freq('f', 4);
    take_me_home[24] = note2freq('A', 4);
    take_me_home[25] = note2freq('c', 5);
    take_me_home[26] = note2freq('c', 5);
    take_me_home[27] = note2freq('B', 4);
    take_me_home[28] = note2freq('B', 4);
    take_me_home[29] = note2freq('B', 4);
    take_me_home[30] = note2freq('B', 4);
    take_me_home[31] = note2freq('c', 5);
    take_me_home[32] = note2freq('B', 4);
    take_me_home[33] = note2freq('f', 4);
    take_me_home[34] = note2freq('A', 4);
    take_me_home[35] = note2freq('A', 4);
    take_me_home[36] = note2freq('B', 4);
    take_me_home[37] = note2freq('A', 4);

    take_me_home[38] = note2freq('A', 4);
    take_me_home[39] = note2freq('B', 4);
    take_me_home[40] = note2freq('c', 5);
    take_me_home[41] = note2freq('c', 5);
    take_me_home[42] = note2freq('A', 4);
    take_me_home[43] = note2freq('B', 4);
    take_me_home[44] = note2freq('c', 5);
    take_me_home[45] = note2freq('B', 4);
    take_me_home[46] = note2freq('A', 4);
    take_me_home[47] = note2freq('c', 5);
    take_me_home[48] = note2freq('E', 5);
    take_me_home[49] = note2freq('f', 5);
    take_me_home[50] = note2freq('f', 5);
    take_me_home[51] = note2freq('f', 5);
    take_me_home[52] = note2freq('E', 5);
    take_me_home[53] = note2freq('c', 5);
    take_me_home[54] = note2freq('c', 5);
    take_me_home[55] = note2freq('A', 4);
    take_me_home[56] = note2freq('B', 4);
    take_me_home[57] = note2freq('c', 5);
    take_me_home[58] = note2freq('c', 5);
    take_me_home[59] = note2freq('B', 4);
    take_me_home[60] = note2freq('A', 4);
    take_me_home[61] = note2freq('A', 4);
    take_me_home[62] = note2freq('B', 4);
    take_me_home[63] = note2freq('A', 4);


}
