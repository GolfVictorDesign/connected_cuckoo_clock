/*
 *  main.c
 *
 *  Created on: 29 mai 2024
 *      Author: Guillaume Varlet
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "sdkconfig.h"

#include "wifi/wifi_connection.h"
#include "front_panel/front_light.h"

extern "C" void app_main(void)
{
    WifiConnection wifi;
	configure_front_light();
	update_front_light(45, 0, 60);
    wifi.wifi_start();
    wifi.wifi_connect();
}
