#include "keyboard.h"
#include "print.h"
#include "interrupt.h"
#include "io.h"
#include "global.h"
#include "stdint.h"
#include "iobuf.h"

struct iobuf kbd_buf;

static void intr_keyboard_handler()
{
    int ctrl_down_last = ctrl_status;
    int shift_down_last = shift_status;
    int caps_lock_last = caps_lock_status;
    int break_code;
    uint16_t scancode = inb(KBD_BUF_PORT);
    if(scancode == 0xe0)
    {
        ext_scancode = 1;
        return;
    }
    if(ext_scancode)
    {
        scancode = ((0xe000) | scancode);
        ext_scancode = 0;
    }
    break_code = ((scancode & 0x0080) != 0);
    if(break_code)
    {
        uint16_t make_code = (scancode &= 0xff7f);                    //现在是断码, 但是用通码判断, 因为使用通码判断比较简单. 由于断码第八位为1, 通码第八位为0, 所以用断码&0xff7f就是通码
        if(make_code == ctrl_l_make || make_code == ctrl_r_make)
        {
            ctrl_status = 0;
        }
        else if(make_code == shift_l_make || make_code == shift_r_make)
        {
            shift_status = 0;
        }
        else if(make_code == alt_l_make || make_code == alt_r_make)
        {
            alt_status = 0;
        }
        return;
    }
    else if((scancode > 0x00 && scancode < 0x3b) || (scancode == alt_r_make) || (scancode == ctrl_r_make))
    {
        int shift = 0;
        if((scancode < 0x0e) || (scancode == 0x29) || (scancode == 0x1a) || (scancode == 0x1b) || (scancode == 0x2b) || (scancode == 0x27) || (scancode == 0x28) || (scancode == 0x33) || (scancode == 0x34) || (scancode == 0x35))
        {
            if(shift_down_last)
            {
                shift = 1;
            }
        }
        else 
        {
            if(shift_down_last && caps_lock_last)
            {
                shift = 0;
            }
            else if(shift_down_last || caps_lock_last)
            {
                shift = 1;
            }
            else
            {
                shift = 0;
            }
        }
        uint8_t index = (scancode &= 0x00ff);
        char cur_char = keymap[index][shift];
        if(cur_char)
        {
            //put_char(cur_char);
            if(!iob_full(&kbd_buf))
            {
                put_char(cur_char);
                iob_putchar(&kbd_buf, cur_char);
            }
            return;
        }
        if(scancode == ctrl_l_make || scancode == ctrl_r_make)
        {
            ctrl_status = 1;
        }
        else if(scancode == shift_l_make || scancode == shift_r_make)
        {
            shift_status = 1;
        }
        else if(scancode == alt_l_make || scancode == alt_r_make)
        {
            alt_status = 1;
        }
        else if(scancode == caps_lock_make)
        {
            caps_lock_status = !caps_lock_status;
        }
    }
    else 
    {
        put_str("unknown key\n");
    }
}

void keyboard_init()
{
    put_str("keyboard init start\n");
    iobuf_init(&kbd_buf);
    register_handler(0x21, intr_keyboard_handler);
    put_str("keyboard init done\n");
}