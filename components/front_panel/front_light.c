/*
 * front_light.c
 *
 *  Created on: 29 mai 2024
 *      Author: Guillaume Varlet
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"
#include "front_light.h"

static led_strip_handle_t led_strip;

void update_front_light(uint8_t led_red, uint8_t led_green, uint8_t led_blue)
{
    ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, 0, led_red, led_green, led_blue));
    /* Refresh the strip to send data */
    led_strip_refresh(led_strip);
}

void configure_front_light(void)
{
    /* LED strip initialization with the GPIO and pixels number*/
    static const led_strip_config_t strip_config =
    {
        .strip_gpio_num = GPIO_FRONT_LIGHT,
        .max_leds = 1,
    };

    static const led_strip_rmt_config_t rmt_config =
    {
        .resolution_hz = 10 * 1000 * 1000,
        .flags.with_dma = false,
    };

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
}

