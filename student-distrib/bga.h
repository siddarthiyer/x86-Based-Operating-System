#include "lib.h"

#define VBE_DISPI_IOPORT_INDEX  0x01CE
#define VBE_DISPI_IOPORT_DATA   0x01CF
#define VBE_DISPI_INDEX_ID 0
#define VBE_DISPI_INDEX_XRES 1
#define VBE_DISPI_INDEX_YRES 2
#define VBE_DISPI_INDEX_BPP 3
#define VBE_DISPI_INDEX_ENABLE 4
#define VBE_DISPI_INDEX_BANK 5
#define VBE_DISPI_INDEX_VIRT_WIDTH 6
#define VBE_DISPI_INDEX_VIRT_HEIGHT 7
#define VBE_DISPI_INDEX_X_OFFSET 8
#define VBE_DISPI_INDEX_Y_OFFSET 9

#define VBE_DISPI_DISABLED 0x00
#define VBE_DISPI_ENABLED 0x01
#define VBE_DISPI_LFB_ENABLED 0x40
#define VBE_DISPI_NOCLEARMEM 0x81

#define FONT_WIDTH 8

#define RED     36
#define BLACK   0
#define BLUE    9
#define YELLOW  54
#define ORANGE  52
#define WHITE   63
#define PURPLE  13
#define LGREEN  18
#define GREEN   2
#define GRAY    56
#define NAVYBLUE 8
#define DGREEN  16
#define PINK    37
#define BROWN   20
#define LAVENDER 15
#define LGRAY   7

int cur_term_esp;
int cur_term_ebp;

int lockscreenflag;
int useridx;
int passidx;
char ubuf[16];
char pbuf[16];
int mode;
int updated;
int LS_enter;

int lockscreen_flag;

int stop_pressed;

uint16_t isBGAAvailable();
void BgaWriteRegister(unsigned short IndexValue, unsigned short DataValue);
unsigned short BgaReadRegister(unsigned short IndexValue);
void BgaSetVideoMode(unsigned int Width, unsigned int Height, unsigned int BitDepth, int UseLinearFrameBuffer, int ClearVideoMemory);
void BgaSetBank(unsigned short BankNumber);
void putpixel(int pos_x, int pos_y, unsigned char VGA_COLOR, int bgflag);
void putpixelbackup(int pos_x, int pos_y, unsigned char VGA_COLOR, int bgflag, int termidx);
void drawrect(int pos_x, int pos_y, int width, int height, unsigned char VGA_COLOR);
void drawrectbackup(int pos_x, int pos_y, int width, int height, unsigned char VGA_COLOR, int termidx);
void printstring(int pos_x, int pos_y, char * str, unsigned char color);
void printlargestring(int pos_x, int pos_y, char * str, unsigned char color);
void printchar(int pos_x, int pos_y, char c, unsigned char color);
void printcharbackup(int pos_x, int pos_y, char c, unsigned char color, int termidx);
void scroll();
void scrollbackup(int termidx);
void refresh();
void printTime();
void printLockCursor(int on);
void printcursor(int pos_x, int pos_y, unsigned char outline, unsigned char color, int flag);
void drawexit (int pos_x, int pos_y);
void renderHeader(char * str);
void renderCalcIcon(int pos_x, int pos_y);
void renderIcon(int pos_x, int pos_y, unsigned char icon[30][30], char * str);
void drawPaint(int pos_x, int pos_y, int size, unsigned char color);
void drawSizeIcon(int pos_x, int pos_y, int size);
void fillBackground(unsigned char color);
void renderTaskbar();
void renderDesktop();
void initVGA();
void runGUI();
void piano_change_octave(int octave);

int pianomode;
int musicmode;
int exit_pressed;
int cur_octave;

#define BG BLUE //43

#define BP 15
#define C1 54
#define C2 2
#define C3 36
#define FG 9
#define BR 14 
#define HD 20
#define MT 7


#define FF 63 
#define BB 44
