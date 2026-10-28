#include "power.h"
#include "pins.h"
#include <esp_sleep.h>
#include <driver/rtc_io.h>


void powerSleep(){
    esp_sleep_enable_ext0_wakeup((gpio_num_t)BTN_POWER, 0);
    rtc_gpio_pullup_en((gpio_num_t)BTN_POWER);
    rtc_gpio_pulldown_dis((gpio_num_t)BTN_POWER);
    esp_deep_sleep_start();
}