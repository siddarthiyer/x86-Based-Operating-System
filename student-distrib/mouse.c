#include "mouse.h"
#include "bga.h"
#include "scheduler.h"
#include "syscall.h"


volatile char mouse_data[3];    //signed char
volatile int mouse_x_coord = 320;
volatile int mouse_y_coord = 240;
volatile int lclick = 0;
volatile int rclick = 1;
int intr_done = 1;
int cur_idx = 0;
int firstintrflag = 0;
extern int paintmode;

int cursorInBounds(int xmin, int xmax, int ymin, int ymax)
{
  if (mouse_x_coord >= xmin && mouse_x_coord < xmax && mouse_y_coord >= ymin && mouse_y_coord < ymax)
  {
    return 1;
  }
  
  return 0;
}

//Mouse functions
void mouse_handler()
{
  cli();
  if (intr_done)
  {
    intr_done = 0;
    cur_idx = 0;
  }
  else
  {
    cur_idx++;
  }

  switch(cur_idx)
  {
    case 0:
      mouse_data[2]=inb(0x60);
      break;
    case 1:
      mouse_data[0]=inb(0x60);
      lclick = (0 != (mouse_data[0] & 1));
      rclick = (0 != (mouse_data[0] & 2));
      if (term_flag && lclick && cursorInBounds(600, 640, 0, 28))
      {
        term_flag = 0;
      }
      if (musicmode && lclick && cursorInBounds(600, 640, 0, 28))
      {
        stop_pressed = 1;
        exit_pressed = 1;
      }
      break;
    case 2:
      mouse_data[1]=inb(0x60);
      if (paintmode == 1)
      {
        mouse_x_coord+=(mouse_data[1]/4);
        mouse_y_coord-=(mouse_data[2]/4);
      }
      else
      {
        mouse_x_coord+=mouse_data[1];
        mouse_y_coord-=mouse_data[2];
      }
      
      if (mouse_x_coord < 0)
      {
        mouse_x_coord = 0;
      }
      else if (mouse_x_coord > 628)
      {
        mouse_x_coord = 628;
      }
      if (mouse_y_coord < 0)
      {
        mouse_y_coord = 0;
      }
      else if (mouse_y_coord > 459)
      {
        mouse_y_coord = 459;
      }
      printcursor(mouse_x_coord, mouse_y_coord, BLACK, WHITE, firstintrflag);
      if (!firstintrflag) firstintrflag = 1;
      intr_done = 1;
      if (lclick && term_flag && cursorInBounds(95, 185, 440, 480))
      {
        switch_terminal(0);
      }
      if (lclick && term_flag && cursorInBounds(187, 277, 440, 480))
      {
        switch_terminal(1);
      }
      if (lclick && term_flag && cursorInBounds(279, 369, 440, 480))
      {
        switch_terminal(2);
      }
      if (lclick && musicmode && cursorInBounds(588, 630, 40, 82))
      {
        stop_pressed = 1;
      }
      
      break;
    default:
      intr_done = 1;
      break;
  }
  


  // printf("%x %x %x \n", mouse_data[0], mouse_data[1], mouse_data[2]);
  send_eoi(12);
  sti();
}

void mouse_wait(uint8_t a_type) //unsigned char
{
  uint32_t _time_out=100000; //unsigned int
  if(a_type==0)
  {
    while(_time_out--) //Data
    {
      if((inb(0x64) & 1)==1)
      {
        return;
      }
      
    }
    return;
  }
  else
  {
    while(_time_out--) //Signal
    {
      if((inb(0x64) & 2)==0)
      {
        return;
      }
    }
    return;
  }
}

void mouse_write(uint8_t a_write) 
{
  mouse_wait(1);
  outb(0xD4, 0x64);
  mouse_wait(1);
  outb(a_write, 0x60);
}

uint8_t mouse_read()
{
  mouse_wait(0);
  return inb(0x60);
}

void mouse_init()
{
  uint8_t status;
  outb(0xA8, 0x64);
 
  outb(0x20, 0x64);
  status = (inb(0x60) | 2);
  outb(0x60, 0x64);
  outb(status, 0x60);
 
  mouse_write(0xF6);
  mouse_read();
  mouse_write(0xF4);
  mouse_read();
  enable_irq(12);
}
