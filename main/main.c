#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "front_panel/front_light.h"
#include "sdkconfig.h"

void app_main(void)
{
	configure_front_light();
	update_front_light();

}
