#include "esp_err.h"
#include "driver/gpio.h"
#include "esp_log.h"

esp_err_t  init_rgb_led(uint8_t led);
esp_err_t set_led_on(void);
esp_err_t set_led_off(void);
