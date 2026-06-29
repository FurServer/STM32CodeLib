#include "u8g2.h"
#include "u8g2_stm32.h"

// 设备句柄
static u8g2_t u8g2;

int main(void)
{
	// 初始化
	u8g2Init(&u8g2);
	
	while(1)
    {
		// 绘制
		u8g2_FirstPage(&u8g2);
        do
        {
            draw_u8g2_example(&u8g2);
        }
        while (u8g2_NextPage(&u8g2));
    }
}