#pragma once



#define EXAMPLE_LCD_H_RES     466 
#define EXAMPLE_LCD_V_RES     466 

//#define LVGL_LCD_BUFFER_SIZE  EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES // full frame buffer is slightly faster
#define LVGL_LCD_BUFFER_SIZE  EXAMPLE_LCD_H_RES * 10 
#define SEND_BUF_SIZE         EXAMPLE_LCD_H_RES * 10

