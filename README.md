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

MPU6050

> 精简的MPU6050库, 解算出四元数和姿态角, 包含一个使用示例

printf

> #include printf.h
>
> 以便将printf重定向至串口

Switch_I2C1

> Switch_I2C1(uint8_t stat)  使能或禁用I2C1
>
> STM32F103系列48pin芯片设计缺陷, 当使能I2C1时, PB5(SMBA)只能作为GPIO使用
>
> 受影响的功能: SPI1重映射(MOSI), TIM3CH2重映射
>
> 通过使能或禁用I2C1时钟, 实现与其他功能分时复用



