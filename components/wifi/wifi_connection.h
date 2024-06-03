/*
 * wifi_connection.h
 *
 *  Created on: 3 juin 2024
 *      Author: Guillaume Varlet
 */

#ifndef COMPONENTS_WIFI_WIFI_CONNECTION_H_
#define COMPONENTS_WIFI_WIFI_CONNECTION_H_

class WifiConnection
{
    private:
        const static char*                  m_pTAG;
        const static char*                  m_pNetifDescSta;
        const static wifi_init_config_t     m_wifiConfig;

        esp_netif_t*        m_pStaNetif;
        SemaphoreHandle_t   m_semGetIpAddrs;

    public:

        WifiConnection(void);
        WifiConnection(WifiConnection& another);

        esp_err_t wifi_sta_do_connect(wifi_config_t wifi_config, bool wait);
        esp_err_t wifi_sta_do_disconnect(void);

        esp_err_t wifi_connect(void);
        void wifi_start(void);
        void wifi_stop(void);
        void wifi_shutdown(void);

        bool is_our_netif(void);

# if 0
        void handler_on_wifi_connect(
                void *esp_netif,
                esp_event_base_t event_base,
                int32_t event_id,
                void *event_data);
        void handler_on_wifi_disconnect(
                void *arg,
                esp_event_base_t event_base,
                int32_t event_id,
                void *event_data);
        void handler_on_sta_got_ip(
                void *arg,
                esp_event_base_t event_base,
                int32_t event_id,
                void *event_data);
#endif
};


#endif /* COMPONENTS_WIFI_WIFI_CONNECTION_H_ */
