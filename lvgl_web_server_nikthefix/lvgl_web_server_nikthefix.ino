/*
nikthefix 26.05.25

LVGL Web Interface using websockets / HTML Canvas and Mouse control
Based on the great work by Blake Felt (https://github.com/Molorius) and converted to Arduino


Dependencies:

ESP32_Arduino boards package version 3.2
LVGL 8.3.11


Notes:

Set your router SSID and Password at line 63/64
Build and run sketch
Check serial monitor for dynamically assigned local IP address
Go to this address in browser
The canvas size will change automatically to fit the LVGL project dimensions
The canvas position in the viewport can be changed in index.h and the mouse coordinates will automatically reference the top left of the canvas
Press F12 to open the web console and monitor the relative mouse coordinates

Since the ESP32-s3 is rendering the LVGL frame buffer into HTML Canvas using WebSockets, the stronger your wifi connection the faster the draw will be.



Please set in lv_conf.h:  --->   #if 1                                  (line 15)
                          --->   #define LV_COLOR_16_SWAP 1             (line 30)
                          --->   #define LV_MEM_CUSTOM 1                (line 49)
                          --->   #define LV_TICK_CUSTOM 1               (line 88)
                          --->   #define LV_FONT_MONTSERRAT_14 1        (line 367)


Build options:

Select board ESP32S3 Dev Module
Select USB CDC On Boot "Enabled"
Select Flash Size 16M
Select Partition Scheme "custom" - partitions.csv in sketch folder will be used
Select PSRAM "OPI PSRAM"



Troubleshooting:

In case of build failure you can try deleting the cached sketch object files located here -
"C:\Users\your_user_name\AppData\Local\arduino\sketches"

*/


#include <Arduino.h>
#include <WiFi.h>
#include "SPI.h"
#include "pins_config.h"
#include "lvgl.h"
#include "src/ui/ui.h"
#include "globals.h"
#include "esp_system.h"
#include "websocket_driver.h"

#define AP_SSID "xxxx"
#define AP_PASSWORD "xxxx"

uint16_t count = 0;
bool automate = false;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t *buf;



void setup()
{
    Serial.begin(115200);
    Serial.printf("psram size : %d MB\r\nflash size : %d MB\r\n", ESP.getPsramSize() / 1024 / 1024, ESP.getFlashChipSize() / 1024 / 1024);

    WiFi.begin(AP_SSID, AP_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) 
    {
    delay(500);
    Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    websocket_driver_init();

    lv_init();
    buf = (lv_color_t *)heap_caps_malloc(sizeof(lv_color_t) * LVGL_LCD_BUFFER_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, LVGL_LCD_BUFFER_SIZE);
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = EXAMPLE_LCD_H_RES;
    disp_drv.ver_res = EXAMPLE_LCD_V_RES;
    disp_drv.flush_cb = websocket_driver_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);
    
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = websocket_driver_read;
    lv_indev_drv_register(&indev_drv);     

    ui_init();     
}



void loop()
{
    lv_timer_handler();
    delay(1); 

    if(lv_obj_has_state(ui_Button1, LV_STATE_CHECKED)) automate = true;
    else automate = false;
   
   
    if(automate == true)
    {
    lv_arc_set_value(ui_Arc1, count);
    lv_label_set_text_fmt(ui_Label1, "%d", count);
    count++;
    if (count == 1000) count = 0; 
    }    

    // if (pressed == 1)
    // {
    // Serial.printf("X: %d   Y: %d\n", mouse_x, mouse_y); //for testing
    // }

}



void check_for_memory_leaks() 
{
    Serial.print(F("Total heap  ")); Serial.println(ESP.getHeapSize());
    Serial.print(F("Free heap   ")); Serial.println(ESP.getFreeHeap());
    Serial.print(F("Total psram ")); Serial.println(ESP.getPsramSize());
    Serial.print(F("Free psram  ")); Serial.println(ESP.getFreePsram());
    Serial.println(F(" "));
}





