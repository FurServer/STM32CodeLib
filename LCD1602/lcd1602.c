/**
 * @file    lcd1602.c
 * @brief   LCD1602 (HD44780) 驱动
 */

#include "lcd1602.h"
#include <stddef.h>   /* NULL */

/* ================================================================== */
/* HD44780 指令常量                                                      */
/* ================================================================== */

#define LCD_CMD_CLEAR           0x01
#define LCD_CMD_HOME            0x02
#define LCD_CMD_ENTRY_MODE      0x04
#define LCD_CMD_DISPLAY         0x08
#define LCD_CMD_SHIFT           0x10
#define LCD_CMD_FUNCTION        0x20
#define LCD_CMD_CGRAM_ADDR      0x40
#define LCD_CMD_DDRAM_ADDR      0x80

#define LCD_ENTRY_INC           0x02
#define LCD_ENTRY_SHIFT         0x01

#define LCD_DISPLAY_ON          0x04
#define LCD_CURSOR_ON           0x02
#define LCD_BLINK_ON            0x01

#define LCD_SHIFT_DISPLAY       0x08
#define LCD_SHIFT_RIGHT         0x04

#define LCD_FUNC_8BIT           0x10
#define LCD_FUNC_2LINE          0x08
#define LCD_FUNC_5X10           0x04

#define LCD_DDRAM_ROW0          0x00
#define LCD_DDRAM_ROW1          0x40
#define LCD_DDRAM_ROW2          0x14
#define LCD_DDRAM_ROW3          0x54



/** @brief 检查 msg_cb 是否有效 */
static uint8_t lcd_ok(lcd1602_t *lcd)
{
    return lcd && lcd->msg_cb;
}

/**
 * @brief  写一个完整字节到 HD44780
 * @param  data  8 位数据
 * @param  rs    0=指令, 1=数据
 *
 * 8-bit 模式: RS → 数据 → E 脉冲
 * 4-bit 模式: RS → 高 nibble → E → 低 nibble → E
 */
static void lcd_write_byte(lcd1602_t *lcd, uint8_t data, uint8_t rs)
{
    if (!lcd_ok(lcd)) return;

    /* 设置 RS */
    lcd->msg_cb(lcd, LCD1602_MSG_SET_RS, rs, NULL);

    if (lcd->mode == LCD1602_MODE_8BIT) {
        /* 8-bit: 一次写完 */
        lcd->msg_cb(lcd, LCD1602_MSG_WRITE_DATA, data, NULL);
        lcd->msg_cb(lcd, LCD1602_MSG_PULSE_EN, 0, NULL);
    } else {
        /*
         * 4-bit: 先高 nibble, 再低 nibble
         */
        lcd->msg_cb(lcd, LCD1602_MSG_WRITE_DATA, (data >> 4), NULL);
        lcd->msg_cb(lcd, LCD1602_MSG_PULSE_EN, 0, NULL);
        lcd->msg_cb(lcd, LCD1602_MSG_WRITE_DATA, (data & 0x0F), NULL);
        lcd->msg_cb(lcd, LCD1602_MSG_PULSE_EN, 0, NULL);
    }
}

/** @brief 发送指令 (RS=0) */
static void lcd_command(lcd1602_t *lcd, uint8_t cmd)
{
    lcd_write_byte(lcd, cmd, 0);
}

/** @brief 发送数据 (RS=1) */
static void lcd_data(lcd1602_t *lcd, uint8_t data)
{
    lcd_write_byte(lcd, data, 1);
}

/**
 * @brief  4-bit 初始化专用: 以 8-bit 接口时序发一个 nibble
 *         用于初始化阶段，此时 HD44780 尚处于 8-bit 模式（未切换）
 */
static void lcd_nibble_8bit(lcd1602_t *lcd, uint8_t nibble, uint8_t rs)
{
    if (!lcd_ok(lcd)) return;
    lcd->msg_cb(lcd, LCD1602_MSG_SET_RS, rs, NULL);
    lcd->msg_cb(lcd, LCD1602_MSG_WRITE_DATA, nibble, NULL);
    lcd->msg_cb(lcd, LCD1602_MSG_PULSE_EN, 0, NULL);
}

/** @brief 毫秒延时 */
static void lcd_delay(lcd1602_t *lcd, uint8_t ms)
{
    if (lcd_ok(lcd)) {
        lcd->msg_cb(lcd, LCD1602_MSG_DELAY_MS, ms, NULL);
    }
}

/** @brief 更新 Display 寄存器 (使用缓存值) */
static void lcd_update_display(lcd1602_t *lcd)
{
    lcd_command(lcd, LCD_CMD_DISPLAY | lcd->display_ctrl);
}

/* ================================================================== */
/* 初始化                                                                */
/* ================================================================== */

void lcd1602_init(lcd1602_t *lcd)
{
    if (!lcd_ok(lcd)) return;

    /* 初始化追踪字段 */
    lcd->col        = 0;
    lcd->row        = 0;
    lcd->wrap       = 0;  // 默认关闭自动换行
    lcd->disp_shift = 0;  // 显示窗口偏移归零

    lcd_delay(lcd, 50);  /* 上电稳定 (> 40ms) */

    if (lcd->mode == LCD1602_MODE_8BIT) {

        /* 8-bit 初始化 (HD44780 Figure 24) */
        lcd_write_byte(lcd, LCD_CMD_FUNCTION | LCD_FUNC_8BIT, 0);
        lcd_delay(lcd, 5);
        lcd_write_byte(lcd, LCD_CMD_FUNCTION | LCD_FUNC_8BIT, 0);
        lcd_delay(lcd, 1);
        lcd_write_byte(lcd, LCD_CMD_FUNCTION | LCD_FUNC_8BIT, 0);
        lcd_delay(lcd, 1);

        lcd->entry_mode = LCD_ENTRY_INC;
        lcd_command(lcd, LCD_CMD_FUNCTION | LCD_FUNC_8BIT | LCD_FUNC_2LINE);

    } else {

        /* 4-bit 初始化 (HD44780 Figure 23)
         * 前 4 次以 8-bit 接口时序发 nibble，因为 HD44780 还不知道要切 4-bit */
        lcd_nibble_8bit(lcd, 0x30, 0);  lcd_delay(lcd, 5);
        lcd_nibble_8bit(lcd, 0x30, 0);  lcd_delay(lcd, 1);
        lcd_nibble_8bit(lcd, 0x30, 0);  lcd_delay(lcd, 1);
        lcd_nibble_8bit(lcd, 0x20, 0);  /* 切换为 4-bit 模式 */
        lcd_delay(lcd, 1);

        /* 之后 HD44780 已进入 4-bit 模式，用 lcd_command 发剩余指令 */
        lcd->entry_mode = LCD_ENTRY_INC;
        lcd_command(lcd, LCD_CMD_FUNCTION | LCD_FUNC_2LINE);
    }

    /* Display OFF */
    lcd->display_ctrl = 0;
    lcd_command(lcd, LCD_CMD_DISPLAY);

    /* Clear */
    lcd_command(lcd, LCD_CMD_CLEAR);
    lcd_delay(lcd, 2);

    /* Entry Mode */
    lcd_command(lcd, LCD_CMD_ENTRY_MODE | lcd->entry_mode);

    /* Display ON */
    lcd->display_ctrl = LCD_DISPLAY_ON;
    lcd_command(lcd, LCD_CMD_DISPLAY | lcd->display_ctrl);
}

/* ================================================================== */
/* 显示控制                                                              */
/* ================================================================== */

void lcd1602_clear(lcd1602_t *lcd)
{
    if (!lcd) return;
    lcd_command(lcd, LCD_CMD_CLEAR);
    lcd->col        = 0;
    lcd->row        = 0;
    lcd->disp_shift = 0;
    lcd_delay(lcd, 2);
}

void lcd1602_home(lcd1602_t *lcd)
{
    if (!lcd) return;
    lcd_command(lcd, LCD_CMD_HOME);
    lcd->col        = 0;
    lcd->row        = 0;
    lcd->disp_shift = 0;
    lcd_delay(lcd, 2);
}

void lcd1602_set_cursor(lcd1602_t *lcd, uint8_t col, uint8_t row)
{
    static const uint8_t row_offset[] = {
        LCD_DDRAM_ROW0, LCD_DDRAM_ROW1,
        LCD_DDRAM_ROW2, LCD_DDRAM_ROW3
    };
    if (!lcd || row >= sizeof(row_offset)) return;
    lcd->col = col;
    lcd->row = row;
    /* DDRAM 地址 = 行基址 + (col + disp_shift) % 40, 在行内回绕 */
    lcd_command(lcd, LCD_CMD_DDRAM_ADDR |
        (row_offset[row] + ((uint8_t)(col + lcd->disp_shift) % LCD1602_DDRAM_LINE_SIZE)));
}

void lcd1602_display_on(lcd1602_t *lcd)
    { if (!lcd) return; lcd->display_ctrl |= LCD_DISPLAY_ON;  lcd_update_display(lcd); }
void lcd1602_display_off(lcd1602_t *lcd)
    { if (!lcd) return; lcd->display_ctrl &= ~LCD_DISPLAY_ON; lcd_update_display(lcd); }
void lcd1602_cursor_on(lcd1602_t *lcd)
    { if (!lcd) return; lcd->display_ctrl |= LCD_CURSOR_ON;   lcd_update_display(lcd); }
void lcd1602_cursor_off(lcd1602_t *lcd)
    { if (!lcd) return; lcd->display_ctrl &= ~LCD_CURSOR_ON;  lcd_update_display(lcd); }
void lcd1602_blink_on(lcd1602_t *lcd)
    { if (!lcd) return; lcd->display_ctrl |= LCD_BLINK_ON;    lcd_update_display(lcd); }
void lcd1602_blink_off(lcd1602_t *lcd)
    { if (!lcd) return; lcd->display_ctrl &= ~LCD_BLINK_ON;   lcd_update_display(lcd); }

/* ================================================================== */
/* 写入数据                                                              */
/* ================================================================== */

void lcd1602_put_char(lcd1602_t *lcd, char ch)
{
    if (!lcd) return;

    /* 自动换行检查: 当前行已满 (col >= LCD1602_COLS) */
    if (lcd->wrap && lcd->col >= LCD1602_COLS) {
        lcd->col = 0;
        lcd->row++;
        /* 如果超出总行数，回到第 0 行 */
        if (lcd->row >= LCD1602_ROWS) {
            lcd->row = 0;
        }
        lcd1602_set_cursor(lcd, lcd->col, lcd->row);
    }

    lcd_data(lcd, (uint8_t)ch);
    lcd->col++;
}

void lcd1602_print(lcd1602_t *lcd, const char *str)
{
    if (!lcd || !str) return;

    while (*str) {
        if (*str == '\n') {
            /* 换行: 跳到下一行开头 */
            lcd->col = 0;
            lcd->row++;
            if (lcd->row >= LCD1602_ROWS) {
                lcd->row = 0;
            }
            lcd1602_set_cursor(lcd, lcd->col, lcd->row);
            str++;
            continue;
        }
        lcd1602_put_char(lcd, *str++);
    }
}

void lcd1602_set_wrap(lcd1602_t *lcd, uint8_t enable)
{
    if (!lcd) return;
    lcd->wrap = enable ? 1 : 0;
}

/* ================================================================== */
/* 自定义字符 (CGRAM)                                                     */
/* ================================================================== */

void lcd1602_create_char(lcd1602_t *lcd, uint8_t idx, const uint8_t pattern[8])
{
    uint8_t i;
    if (!lcd || !pattern || idx > 7) return;
    lcd_command(lcd, LCD_CMD_CGRAM_ADDR | (idx << 3));
    for (i = 0; i < 8; i++) {
        lcd_data(lcd, pattern[i] & 0x1F);
    }
}

/* ================================================================== */
/* 显示移位                                                              */
/* ================================================================== */

void lcd1602_shift_left(lcd1602_t *lcd)
{
    if (!lcd) return;
    lcd_command(lcd, LCD_CMD_SHIFT | LCD_SHIFT_DISPLAY);
    /* 窗口左移 = 起始地址右移, 在 40 字节行内回绕 */
    lcd->disp_shift = (lcd->disp_shift + 1) % LCD1602_DDRAM_LINE_SIZE;
}

void lcd1602_shift_right(lcd1602_t *lcd)
{
    if (!lcd) return;
    lcd_command(lcd, LCD_CMD_SHIFT | LCD_SHIFT_DISPLAY | LCD_SHIFT_RIGHT);
    /* 窗口右移 = 起始地址左移, 在 40 字节行内回绕 */
    lcd->disp_shift = (lcd->disp_shift == 0)
        ? (LCD1602_DDRAM_LINE_SIZE - 1)
        : (lcd->disp_shift - 1);
}

/* ================================================================== */
/* 背光                                                                  */
/* ================================================================== */

void lcd1602_backlight(lcd1602_t *lcd, uint8_t brightness)
    { if (lcd_ok(lcd)) lcd->msg_cb(lcd, LCD1602_MSG_BACKLIGHT, brightness, NULL); }
