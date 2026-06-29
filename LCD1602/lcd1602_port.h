/**
 * @file    lcd1602_port.h
 * @brief   LCD1602 移植层
 */

#ifndef LCD1602_PORT_H
#define LCD1602_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lcd1602.h"

void lcd1602_port_init(lcd1602_t *lcd);

#ifdef __cplusplus
}
#endif

#endif /* LCD1602_PORT_H */
