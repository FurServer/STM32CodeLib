芝士泡泡学STM32时写的一些以后可能会用到的库
---

**库列表:**

AHT20

> AHT20_Measure(I2C_HandleTypeDef* hi2c)
>
> 单纯的发送读取指令, 需等待80ms测量完毕
>
> AHT20_Get_Data(I2C_HandleTypeDef* hi2c, int16_t* St, uint16_t* Srh)
>
> 单纯的读取温湿度, 返回单位为0.01(°C/%)的值