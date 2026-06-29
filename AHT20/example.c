#include "aht20.h"

int16_t temperature;
uint16_t humidity;

// u8g2 (可选)
static u8g2_t u8g2;
static char oled[16];

int main(void)
{
	while (1)
    {
		AHT20_Measure(&hi2c1);
		HAL_Delay(80);
		AHT20_Get_Data(&hi2c1,&temperature,&humidity);
		
		u8g2_FirstPage(&u8g2);
        do
        {
            u8g2_SetFontMode(&u8g2, 1);
            u8g2_SetFontDirection(&u8g2, 0);
            u8g2_SetFont(&u8g2, u8g2_font_t0_16_mr);
            sprintf(oled, "T:%02d.%02dC", temperature / 100, temperature % 100);
            u8g2_DrawStr(&u8g2, 0, 10, oled);
            sprintf(oled, "R:%02d.%02d%%", humidity / 100, humidity % 100);
            u8g2_DrawStr(&u8g2, 0, 24, oled);
        }
        while (u8g2_NextPage(&u8g2));
		
		
	}
}