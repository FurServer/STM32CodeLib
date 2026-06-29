#include "lcd1602_port.h"

// 设备句柄和缓冲区
static lcd1602_t lcd;
static char lcd_buf[17];

int main(void)
{
	// 初始化
	lcd1602_port_init(&lcd);
	
	while(1){
	
	// 显示下划线光标
    lcd1602_print(&lcd, "cursor_on");
    HAL_Delay(500);
    lcd1602_cursor_on(&lcd);
    HAL_Delay(500);

    // 设置光标位置
    lcd1602_clear(&lcd); lcd1602_print(&lcd, "set_cursor");
    lcd1602_set_cursor(&lcd, 2, 1);
    HAL_Delay(500);

    // 开启字符闪烁
    lcd1602_clear(&lcd); lcd1602_print(&lcd, "blink_on");
    lcd1602_blink_on(&lcd);
    HAL_Delay(500);

    // 关闭字符闪烁
    lcd1602_clear(&lcd); lcd1602_print(&lcd, "blink_off");
    lcd1602_blink_off(&lcd);
    HAL_Delay(500);

    // 清屏
    lcd1602_clear(&lcd); lcd1602_print(&lcd, "clear");
    HAL_Delay(500);
    lcd1602_clear(&lcd);
    HAL_Delay(500);

    // 光标归位
    lcd1602_print(&lcd, "home");
    lcd1602_set_cursor(&lcd, 5, 0);
    HAL_Delay(500);
    lcd1602_home(&lcd);
    HAL_Delay(500);

    // 关闭显示
    lcd1602_clear(&lcd); lcd1602_print(&lcd, "display_off");
    HAL_Delay(500);
    lcd1602_display_off(&lcd);
    HAL_Delay(500);

    // 开启显示
    lcd1602_clear(&lcd); lcd1602_print(&lcd, "display_on");
    lcd1602_display_on(&lcd);
    HAL_Delay(500);

    // 隐藏光标
    lcd1602_clear(&lcd); lcd1602_print(&lcd, "cursor_off");
    lcd1602_cursor_off(&lcd);
    HAL_Delay(500);

    // 自定义字符
    lcd1602_clear(&lcd); lcd1602_print(&lcd, "set_cursor");
    {
        const uint8_t heart[8] = {
            0b00000,
            0b01010,
            0b10101,
            0b10001,
            0b01010,
            0b00100,
            0b00000,
            0b00000
        };
        lcd1602_create_char(&lcd, 0, heart);
    }
    HAL_Delay(500);

    // 显示自定义字符
    lcd1602_clear(&lcd); lcd1602_print(&lcd, "put_char");
    HAL_Delay(500);
    lcd1602_set_cursor(&lcd, 0, 1);
    lcd1602_put_char(&lcd, 0);
    HAL_Delay(500);

    // 开启自动换行
    lcd1602_clear(&lcd); lcd1602_print(&lcd, "set_wrap=1");
    lcd1602_set_wrap(&lcd, 1);
    HAL_Delay(500);
    
    // 测试自动换行
    lcd1602_clear(&lcd);
    for (uint8_t i = 0; i < 55; i++) {
        lcd1602_put_char(&lcd, 'A' + (i % 26));
        HAL_Delay(10);
    }
    HAL_Delay(500);

    // 长字符串自动换行测试
    lcd1602_clear(&lcd); lcd1602_print(&lcd, "print>16");
    HAL_Delay(500);
    lcd1602_clear(&lcd);
    lcd1602_print(&lcd, "Hello LCD1602 Auto Wrap Test");
    HAL_Delay(800);

    // 显示内容左移3列
    lcd1602_clear(&lcd); lcd1602_print(&lcd, "shift_left x3");
    HAL_Delay(500);
    lcd1602_shift_left(&lcd);
    HAL_Delay(200);
    lcd1602_shift_left(&lcd);
    HAL_Delay(200);
    lcd1602_shift_left(&lcd);
    HAL_Delay(500);

    // 显示内容右移3列
    lcd1602_clear(&lcd); lcd1602_print(&lcd, "shift_right x3");
    HAL_Delay(500);
    lcd1602_shift_right(&lcd);
    HAL_Delay(200);
    lcd1602_shift_right(&lcd);
    HAL_Delay(200);
    lcd1602_shift_right(&lcd);
    HAL_Delay(500);

    // finish
    lcd1602_clear(&lcd);
    lcd1602_set_wrap(&lcd, 0);
    lcd1602_print(&lcd, "finish");
    HAL_Delay(2000);
	
    }
}