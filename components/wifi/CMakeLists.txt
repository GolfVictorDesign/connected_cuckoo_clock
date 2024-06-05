
set(srcs "wifi_connection.cpp")


idf_component_register(SRCS "${srcs}"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES esp_netif driver esp_wifi vfs nvs_flash)
                    
                    