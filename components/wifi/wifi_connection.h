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
    protected:
        const static char*                  m_pNetifDescSta;
        const static wifi_init_config_t     m_wifiConfig;

        esp_netif_t*        m_pStaNetif;

        EventGroupHandle_t  m_wifi_event_group;

        virtual esp_err_t close_connection(void);
        virtual esp_err_t init_wifi(void) { return esp_wifi_init(&m_wifiConfig); }

    public:
        WifiConnection(void);
        ~WifiConnection(void);


};

class WifiStation : WifiConnection
{
    protected:

    public:
        WifiStation(void);

};



#endif /* COMPONENTS_WIFI_WIFI_CONNECTION_H_ */
