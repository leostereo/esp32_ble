#include <stdio.h>
#include <string.h>
#include "rgb_led.h"
#define LEDOFF "apagar"
#define LEDON "prender"


const char * TAG = "rgb_led";

uint8_t  ledr = 2;
uint8_t  ledg = 2;
uint8_t  ledb = 2;

esp_err_t  init_rgb_led(uint8_t led){
    
    gpio_reset_pin(led);
    gpio_set_direction(led,GPIO_MODE_OUTPUT);

    ESP_LOGI(TAG,"init_rgb");
    return ESP_OK;

}

esp_err_t set_led_on(void)
{
    ESP_LOGI(TAG,"turn ON");
    gpio_set_level(ledr,1);
    return ESP_OK;
}

esp_err_t set_led_off(void)
{
    ESP_LOGI(TAG,"turn OFF");
    gpio_set_level(ledr,0);
    return ESP_OK;
}

esp_err_t handle_led(uint8_t * data){



    ESP_LOGI(TAG,"handle_led %s\n",data);
    
    if (strcmp((const char *)data, LEDOFF) == 0){
        ESP_LOGI(TAG," handle_led apagar");
        set_led_off();
    }else if (strcmp((const char *)data, LEDON) == 0){
        ESP_LOGI(TAG," handle_led prender");
        set_led_on();
    }


    return ESP_OK;

}
