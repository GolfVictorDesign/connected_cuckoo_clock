/*
 * wifi_connection.c
 *
 *  Created on: 30 mai 2024
 *      Author: Guillaume Varlet
 */

#include <string.h>
#include "esp_log.h"
#include "sdkconfig.h"
#include "esp_err.h"

#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_wifi_default.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#include "wifi_connection.h"

#if CONFIG_EXAMPLE_WIFI_CONNECT_AP_BY_SIGNAL
#define EXAMPLE_WIFI_CONNECT_AP_SORT_METHOD WIFI_CONNECT_AP_BY_SIGNAL
#elif CONFIG_EXAMPLE_WIFI_CONNECT_AP_BY_SECURITY
#define EXAMPLE_WIFI_CONNECT_AP_SORT_METHOD WIFI_CONNECT_AP_BY_SECURITY
#endif

#if CONFIG_EXAMPLE_WIFI_AUTH_OPEN
#define EXAMPLE_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_EXAMPLE_WIFI_AUTH_WEP
#define EXAMPLE_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_EXAMPLE_WIFI_AUTH_WPA_PSK
#define EXAMPLE_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_EXAMPLE_WIFI_AUTH_WPA2_PSK
#define EXAMPLE_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_EXAMPLE_WIFI_AUTH_WPA_WPA2_PSK
#define EXAMPLE_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_EXAMPLE_WIFI_AUTH_WPA2_ENTERPRISE
#define EXAMPLE_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_ENTERPRISE
#elif CONFIG_EXAMPLE_WIFI_AUTH_WPA3_PSK
#define EXAMPLE_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_EXAMPLE_WIFI_AUTH_WPA2_WPA3_PSK
#define EXAMPLE_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_EXAMPLE_WIFI_AUTH_WAPI_PSK
#define EXAMPLE_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif

#define CONFIG_WIFI_CONNECTION_MAX_RETRY    4


/*********************************************************************************************************************
 *                                              Constant declarations
 *********************************************************************************************************************/
const char* WifiConnection::m_pNetifDescSta = "wlan_esp32s3";
const char* WifiConnection::m_pTAG = "WiFi_Connection";
const wifi_init_config_t WifiConnection::m_wifiConfig = WIFI_INIT_CONFIG_DEFAULT();


static void handler_on_wifi_disconnect(
        void *arg,
        esp_event_base_t event_base,
        int32_t event_id,
        void *event_data)
{
    SemaphoreHandle_t* pSemGetIpAddrs;
    uint8_t retry_num = 0;

    pSemGetIpAddrs = (SemaphoreHandle_t*)event_data;
    retry_num++;

    if( retry_num > CONFIG_WIFI_CONNECTION_MAX_RETRY )
    {
        ESP_LOGI("wifi_handler", "WiFi Connect failed %d times, stop reconnect.", retry_num);
        /* let wifi_sta_do_connect() return */
        if( *pSemGetIpAddrs )
        {
            xSemaphoreGive( *pSemGetIpAddrs );
        }

        return;
    }

    ESP_LOGI("wifi_handler", "Wi-Fi disconnected, trying to reconnect...");
    esp_err_t err = esp_wifi_connect();

    if( err == ESP_ERR_WIFI_NOT_STARTED )
    {
        return;
    }

    ESP_ERROR_CHECK(err);
}

static void handler_on_wifi_connect(
        void *esp_netif,
        esp_event_base_t event_base,
        int32_t event_id,
        void *event_data)
{
    ESP_LOGI("wifi_handler", "WiFi connected");
}

static void handler_on_sta_got_ip(
        void *arg,
        esp_event_base_t event_base,
        int32_t event_id,
        void *event_data)
{
    SemaphoreHandle_t* pSemGetIpAddrs;
    pSemGetIpAddrs = (SemaphoreHandle_t*)event_data;

    if( false /*is_our_netif()*/ )
    {
        return;
    }

    if( *pSemGetIpAddrs )
    {
        xSemaphoreGive( *pSemGetIpAddrs );
    }
    else
    {
        ESP_LOGI("wifi_handler", "- IPv4 address");
    }
}


/**
 * @brief Checks the netif description if it contains specified prefix.
 * All netifs created within common connect component are prefixed with the module TAG,
 * so it returns true if the specified netif is owned by this module
 */
bool WifiConnection::is_our_netif(void)
{
    if (strncmp(m_pNetifDescSta, esp_netif_get_desc(m_pStaNetif), strlen(m_pNetifDescSta) - 1) == 0)
    {
        return (true);
    }
    else
    {
      return (false);
    }
}

WifiConnection::WifiConnection(void)
{
    m_pStaNetif = nullptr;
    m_semGetIpAddrs = nullptr;
}

void WifiConnection::wifi_start(void)
{
    ESP_ERROR_CHECK(esp_wifi_init(&m_wifiConfig));

    esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_INHERENT_DEFAULT_WIFI_STA();
    /* Warning: the interface desc is used in tests to capture actual connection details (IP, gw, mask) */
    esp_netif_config.if_desc = m_pNetifDescSta;
    esp_netif_config.route_prio = 128;
    m_pStaNetif = esp_netif_create_wifi(WIFI_IF_STA, &esp_netif_config);
    esp_wifi_set_default_wifi_sta_handlers();

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void WifiConnection::wifi_stop(void)
{
    esp_err_t err = esp_wifi_stop();
    if( err == ESP_ERR_WIFI_NOT_INIT )
    {
        return;
    }
    ESP_ERROR_CHECK(err);
    ESP_ERROR_CHECK(esp_wifi_deinit());
    ESP_ERROR_CHECK(esp_wifi_clear_default_wifi_driver_and_handlers(m_pStaNetif));
    esp_netif_destroy(m_pStaNetif);
    m_pStaNetif = NULL;
}

esp_err_t WifiConnection::wifi_sta_do_connect(wifi_config_t wifi_config, bool wait)
{
    if( wait )
    {
        m_semGetIpAddrs = xSemaphoreCreateBinary();
        if( m_semGetIpAddrs == NULL )
        {
            return ESP_ERR_NO_MEM;
        }
    }

    uint8_t retry_num = 0;
    ESP_ERROR_CHECK(
            esp_event_handler_register(
                    WIFI_EVENT,
                    WIFI_EVENT_STA_DISCONNECTED,
                    &handler_on_wifi_disconnect,
                    &m_semGetIpAddrs));
    ESP_ERROR_CHECK(
            esp_event_handler_register(
                    IP_EVENT,
                    IP_EVENT_STA_GOT_IP,
                    &handler_on_sta_got_ip,
                    &m_semGetIpAddrs));
    ESP_ERROR_CHECK(
            esp_event_handler_register(
                    WIFI_EVENT,
                    WIFI_EVENT_STA_CONNECTED,
                    &handler_on_wifi_connect,
                    m_pStaNetif));

    ESP_LOGI(m_pTAG, "Connecting to %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    esp_err_t ret = esp_wifi_connect();

    if( ret != ESP_OK )
    {
        ESP_LOGE(m_pTAG, "WiFi connect failed! ret:%x", ret);
        return ret;
    }

    if( wait )
    {
        ESP_LOGI(m_pTAG, "Waiting for IP(s)");

        xSemaphoreTake(m_semGetIpAddrs, portMAX_DELAY);

        if( retry_num > CONFIG_WIFI_CONNECTION_MAX_RETRY )
        {
            return ESP_FAIL;
        }
    }
    return (ESP_OK);
}

esp_err_t WifiConnection::wifi_sta_do_disconnect(void)
{
    ESP_ERROR_CHECK(
            esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &handler_on_wifi_disconnect));
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &handler_on_sta_got_ip));
    ESP_ERROR_CHECK(
            esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &handler_on_wifi_connect));

    if( m_semGetIpAddrs )
    {
        vSemaphoreDelete(m_semGetIpAddrs);
    }
    return esp_wifi_disconnect();
}

void WifiConnection::wifi_shutdown(void)
{
    wifi_sta_do_disconnect();
    wifi_stop();
}

esp_err_t WifiConnection::wifi_connect(void)
{
    ESP_LOGI(m_pTAG, "Start WiFi connection.");
    wifi_start();
    const static wifi_config_t wifi_config =
    { .sta =
        {
            .ssid = "",
            .password = "",
            .scan_method = WIFI_ALL_CHANNEL_SCAN,
            .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
            .threshold.rssi = -127,
            .threshold.authmode = WIFI_AUTH_OPEN,
        },
    };
    return wifi_sta_do_connect(wifi_config, true);
}
