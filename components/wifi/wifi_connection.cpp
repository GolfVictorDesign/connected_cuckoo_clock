/*
 * wifi_connection.c
 *
 *  Created on: 30 mai 2024
 *      Author: Guillaume Varlet
 */

#include <string.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_wifi_default.h"
#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "sdkconfig.h"

#include "wifi_connection.h"


 /* 
  * The event group allows multiple bits for each event, but we only care about two events:
  * - we are connected to the AP with an IP
  * - we failed to connect after the maximum amount of retries 
  */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

/**********************************************************************************************************************
 *                                              Constants declarations                                                *
 **********************************************************************************************************************/
const char* WifiConnection::m_pNetifDescSta = "wlan_esp32s3";
const wifi_init_config_t WifiConnection::m_wifiConfig = WIFI_INIT_CONFIG_DEFAULT();


static void wifi_station_event_handler(
    void* arg, 
    esp_event_base_t event_base,
    int32_t event_id, 
    void* event_data)
{
    static uint8_t num_retry = 0;
    EventGroupHandle_t*  p_wifi_event_group;
    esp_err_t result;

    p_wifi_event_group = (EventGroupHandle_t*)event_data;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        result = esp_wifi_connect();
        ESP_ERROR_CHECK(result);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (num_retry < CONFIG_WIFI_MAXIMUM_RETRY) {
            esp_wifi_connect();
            num_retry++;
            ESP_LOGI("WIFI_station_event", "retry to connect to the AP");
        }
        else {
            xEventGroupSetBits(*p_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI("WIFI_station_event", "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
        ESP_LOGI("WIFI_station_event", "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        num_retry = 0;
        xEventGroupSetBits(*p_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

WifiConnection::WifiConnection(void)
{
    m_pStaNetif = nullptr;

    /*
     * Initialize Non-Volatile Storage Library as the WiFi interface requires this to run 
     */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    /*
     * Create a WiFi event group to handle 
     */
    m_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

}

esp_err_t WifiConnection::close_connection(void)
{
    esp_err_t ret;

    ret = esp_event_loop_delete_default();
    ESP_ERROR_CHECK(ret);

    /*
     * Deinitialize Non-Volatile Storage Library as the WiFi interface requires this to run
     */
    ret = nvs_flash_deinit();
    ESP_ERROR_CHECK(ret);

    return (ret);
}

WifiConnection::~WifiConnection(void)
{
    m_pStaNetif = nullptr;
    close_connection();
}

/***********************************************************************************************************************
 *                                                WiFi Station class                                                   *
 **********************************************************************************************************************/

WifiStation::WifiStation(void)
{

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
            /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (password len => 8).
             * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
             * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
             * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
             */
            .threshold = {
                .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            },
            .sae_pwe_h2e = (wifi_sae_pwe_method_t)WPA3_SAE_PWE_UNSPECIFIED,
            .sae_h2e_identifier = "",
        },
    };

    m_pStaNetif = esp_netif_create_default_wifi_sta();
    init_wifi();

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_station_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_station_event_handler,
                                                        NULL,
                                                        &instance_got_ip));


    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );
}
