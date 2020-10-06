#ifndef __DEVICE_KEYBOARD_H
#define __DEVICE_KEYBOARD_H
#include "print.h"
#include "interrupt.h"
#include "io.h"
#include "global.h"
#include "iobuf.h"

#define KBD_BUF_PORT 0x60 

#define esc '\033'
#define backspace '\b'
#define tab ' '
#define enter '\r'
#define delete '\177'

#define char_invisible 0
#define ctrl_l_char char_invisible
#define ctrl_r_char char_invisible
#define shift_l_char char_invisible
#define shift_r_char char_invisible
#define alt_l_char char_invisible
#define alt_r_char char_invisible
#define caps_lock_char char_invisible

#define shift_l_make 0x2a 
#define shift_r_make 0x36 
#define alt_l_make 0x38 
#define alt_r_make 0xe038
#define alt_r_break 0xe0b8
#define ctrl_l_make 0x1d 
#define ctrl_r_make 0xe01d
#define ctrl_r_break 0xe09d 
#define caps_lock_make 0x3a 

static int ctrl_status, shift_status, alt_status, caps_lock_status, ext_scancode;

static char keymap[][2] = {
    {0,0},
    {esc,esc},
    {'1','!'},
    {'2','@'},
    {'3','#'},
    {'4','$'},
    {'5','%'},
    {'6','^'},
    {'7','&'},
    {'8','*'},
    {'9','('},
    {'0',')'},
    {'-','_'},
    {'=','+'},
    {backspace,backspace},
    {tab,tab},
    {'q','Q'},
    {'w','W'},
    {'e','E'},
    {'r','R'},
    {'t','T'},
    {'y','Y'},
    {'u','U'},
    {'i','I'},
    {'o','O'},
    {'p','P'},
    {'[','{'},
    {']','}'},
    {enter,enter},
    {ctrl_l_char,ctrl_l_char},
    {'a','A'},
    {'s','S'},
    {'d','D'},
    {'f','F'},
    {'g','G'},
    {'h','H'},
    {'j','J'},
    {'k','K'},
    {'l','L'},
    {';',':'},
    {'\'','"'},
    {'`','~'},
    {shift_l_char,shift_l_char},
    {'\\','|'},
    {'z','Z'},
    {'x','X'},
    {'c','C'},
    {'v','V'},
    {'b','B'},
    {'n','N'},
    {'m','M'},
    {',','<'},
    {'.','>'},
    {'/','?'},
    {shift_r_char,shift_r_char},
    {'*','*'},
    {alt_l_char,alt_l_char},
    {' ',' '},
    {caps_lock_char,caps_lock_char}
};



static void intr_keyboard_handler();

void keyboard_init();

extern struct iobuf kbd_buf;
#endif 