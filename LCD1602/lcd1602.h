/**
 * @file    lcd1602.h
 * @brief   LCD1602 (HD44780) 驱动
 */

#ifndef LCD1602_H
#define LCD1602_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>


// 消息类型

// 硬件初始化 (port 层可在此初始化 GPIO)
#define LCD1602_MSG_INIT          0

// 设置 RS (寄存器选择) 线
// arg_int = 0 (指令) / 1 (数据)
#define LCD1602_MSG_SET_RS        1

// 写数据到数据线
// arg_int = 数据值 (8-bit模式:完整字节; 4-bit模式:高nibble或低nibble)
#define LCD1602_MSG_WRITE_DATA    2

// 产生E脉冲 (高→低)
#define LCD1602_MSG_PULSE_EN      3

// 毫秒延时 (uint8_t)
#define LCD1602_MSG_DELAY_MS      4

// 背光控制)
// arg_int = 亮度(0~255)
#define LCD1602_MSG_BACKLIGHT     5


typedef struct lcd1602_t lcd1602_t;


/**
 * @brief  消息回调
 * @param  lcd      LCD1602 句柄
 * @param  msg      消息类型
 * @param  arg_int  消息参数
 * @param  arg_ptr  消息参数指针
 * @retval  1=成功, 0=失败
 */
typedef uint8_t (*lcd1602_msg_cb)(lcd1602_t *lcd, uint8_t msg, uint8_t arg_int, void *arg_ptr);
// 如果你试图移植到8051且使用SDCC编译器, 请替换为下面这一行 :)
// typedef uint8_t (*lcd1602_msg_cb)(lcd1602_t *lcd, uint8_t msg, uint8_t arg_int, void *arg_ptr) __reentrant;


// 接口模式
typedef enum {
    LCD1602_MODE_4BIT = 0,
    LCD1602_MODE_8BIT = 1
} lcd1602_mode_t;

// 显示尺寸
#define LCD1602_COLS  16
#define LCD1602_ROWS  2

// LCD1602 句柄
struct lcd1602_t {
    lcd1602_msg_cb  msg_cb;        // 消息回调
    lcd1602_mode_t  mode;          // 接口模式
    uint8_t         display_ctrl;  // Display ON/OFF/Cursor/Blink
    uint8_t         entry_mode;    // Entry Mode 缓存
    uint8_t         col;           // 当前光标列 (0~15)
    uint8_t         row;           // 当前光标行 (0~1)
    uint8_t         wrap;          // 自动换行: 0=关闭, 1=开启
};

// API
void lcd1602_init(lcd1602_t *lcd);  // 初始化
void lcd1602_clear(lcd1602_t *lcd); // 清屏, 光标归位
void lcd1602_home(lcd1602_t *lcd);  // 光标归位
void lcd1602_set_cursor(lcd1602_t *lcd, uint8_t col, uint8_t row); // 设置光标位置 (col:0~15, row:0~1)

void lcd1602_display_on(lcd1602_t *lcd);  // 开启显示
void lcd1602_display_off(lcd1602_t *lcd); // 关闭显示 (保留数据)
void lcd1602_cursor_on(lcd1602_t *lcd);   // 显示下划线光标
void lcd1602_cursor_off(lcd1602_t *lcd);  // 隐藏光标
void lcd1602_blink_on(lcd1602_t *lcd);    // 开启字符闪烁
void lcd1602_blink_off(lcd1602_t *lcd);   // 关闭字符闪烁

// 将5×8点阵图案写入CGRAM, 索引 0~7
// 自定义字符 (索引 0)
// const uint8_t heart[8] = {0b00000,0b01010,0b10101,0b10001,0b10001,0b01010,0b00100,0b00000};
// lcd1602_create_char(&lcd, 0, heart);
// lcd1602_put_char(&lcd, 0);
void lcd1602_create_char(lcd1602_t *lcd, uint8_t idx, const uint8_t pattern[8]);

void lcd1602_put_char(lcd1602_t *lcd, char ch);      // 在当前位置写入单个字符, 光标自动后移; 若开启自动换行则自动换行
void lcd1602_print(lcd1602_t *lcd, const char *str); // 写入字符串, 支持 \n 换行, 配合自动换行使用

void lcd1602_set_wrap(lcd1602_t *lcd, uint8_t enable); // 设置自动换行: 0=关闭, 非0=开启 (默认关闭)

void lcd1602_shift_left(lcd1602_t *lcd);  // 显示内容左移一列
void lcd1602_shift_right(lcd1602_t *lcd); // 显示内容右移一列

void lcd1602_backlight(lcd1602_t *lcd, uint8_t brightness); // 背光亮度 0~255

#ifdef __cplusplus
}
#endif

#endif /* LCD1602_H */
