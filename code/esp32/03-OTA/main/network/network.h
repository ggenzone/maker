#ifndef NETWORK_H
#define NETWORK_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_netif.h"

/**
 * @brief Network connection state
 */
typedef enum {
    NETWORK_STATE_DISCONNECTED = 0,
    NETWORK_STATE_CONNECTING,
    NETWORK_STATE_CONNECTED,
    NETWORK_STATE_RECONNECTING,
    NETWORK_STATE_FAILED,
    NETWORK_STATE_DISABLED
} network_state_t;

/**
 * @brief Network security type
 */
typedef enum {
    NETWORK_SECURITY_OPEN = 0,
    NETWORK_SECURITY_WEP,
    NETWORK_SECURITY_WPA_PSK,
    NETWORK_SECURITY_WPA2_PSK,
    NETWORK_SECURITY_WPA_WPA2_PSK,
    NETWORK_SECURITY_WPA3_PSK,
    NETWORK_SECURITY_UNKNOWN
} network_security_t;

/**
 * @brief WiFi access point information
 */
typedef struct {
    char ssid[33];                    // SSID string (max 32 + null)
    uint8_t bssid[6];                // MAC address
    int8_t rssi;                     // Signal strength in dBm
    network_security_t security;     // Security type
    uint8_t channel;                 // WiFi channel
    bool is_hidden;                  // Hidden network
} network_ap_info_t;

/**
 * @brief Network connection information
 */
typedef struct {
    char ssid[33];                   // Connected SSID
    uint8_t bssid[6];               // Connected BSSID
    int8_t rssi;                    // Current signal strength
    uint8_t channel;                // Current channel
    uint32_t ip_addr;               // IP address (network byte order)
    uint32_t netmask;               // Network mask
    uint32_t gateway;               // Gateway address
    uint32_t dns1;                  // Primary DNS
    uint32_t dns2;                  // Secondary DNS
    bool dhcp_enabled;              // DHCP status
} network_info_t;

/**
 * @brief Network statistics
 */
typedef struct {
    uint32_t connect_attempts;       // Total connection attempts
    uint32_t successful_connections; // Successful connections
    uint32_t failed_connections;     // Failed connections
    uint32_t disconnections;         // Unexpected disconnections
    uint32_t reconnections;          // Automatic reconnections
    uint32_t uptime_seconds;         // Total connected time
    uint32_t last_connect_time;      // Last connection timestamp
    uint32_t last_disconnect_time;   // Last disconnection timestamp
} network_stats_t;

/**
 * @brief Network event callback function
 * @param state Current network state
 * @param info Network information (can be NULL)
 */
typedef void (*network_event_cb_t)(network_state_t state, const network_info_t *info);

/**
 * @brief Scan complete callback function
 * @param ap_list Array of found access points
 * @param ap_count Number of access points found
 */
typedef void (*network_scan_cb_t)(const network_ap_info_t *ap_list, uint16_t ap_count);

/**
 * @brief Initialize the network subsystem
 * @return ESP_OK on success
 */
esp_err_t network_init(void);

/**
 * @brief Deinitialize the network subsystem
 * @return ESP_OK on success
 */
esp_err_t network_deinit(void);

/**
 * @brief Check if network subsystem is ready
 * @return true if ready for use
 */
bool network_is_ready(void);

/**
 * @brief Enable WiFi and start connection process
 * @return ESP_OK on success
 */
esp_err_t network_enable(void);

/**
 * @brief Disable WiFi and stop all network activity
 * @return ESP_OK on success
 */
esp_err_t network_disable(void);

/**
 * @brief Connect to a WiFi network
 * @param ssid Network SSID
 * @param password Network password (can be NULL for open networks)
 * @param timeout_ms Connection timeout in milliseconds (0 = use default)
 * @return ESP_OK on success
 */
esp_err_t network_connect(const char *ssid, const char *password, uint32_t timeout_ms);

/**
 * @brief Disconnect from current network
 * @return ESP_OK on success
 */
esp_err_t network_disconnect(void);

/**
 * @brief Force reconnection to current network
 * @return ESP_OK on success
 */
esp_err_t network_reconnect(void);

/**
 * @brief Start WiFi scan for available networks
 * @param callback Callback function to receive scan results
 * @param active_scan true for active scan, false for passive
 * @return ESP_OK on success
 */
esp_err_t network_scan_start(network_scan_cb_t callback, bool active_scan);

/**
 * @brief Stop ongoing WiFi scan
 * @return ESP_OK on success
 */
esp_err_t network_scan_stop(void);

/**
 * @brief Get current network state
 * @return Current network state
 */
network_state_t network_get_state(void);

/**
 * @brief Get current network information
 * @param info Pointer to structure to fill with network info
 * @return ESP_OK if connected and info retrieved
 */
esp_err_t network_get_info(network_info_t *info);

/**
 * @brief Get network statistics
 * @param stats Pointer to structure to fill with statistics
 * @return ESP_OK on success
 */
esp_err_t network_get_stats(network_stats_t *stats);

/**
 * @brief Reset network statistics
 * @return ESP_OK on success
 */
esp_err_t network_reset_stats(void);

/**
 * @brief Register event callback
 * @param callback Callback function for network events
 * @return ESP_OK on success
 */
esp_err_t network_register_event_callback(network_event_cb_t callback);

/**
 * @brief Unregister event callback
 * @return ESP_OK on success
 */
esp_err_t network_unregister_event_callback(void);

/**
 * @brief Set auto-reconnect behavior
 * @param enabled true to enable auto-reconnect
 * @param max_attempts Maximum reconnection attempts (0 = unlimited)
 * @param delay_ms Delay between attempts in milliseconds
 * @return ESP_OK on success
 */
esp_err_t network_set_auto_reconnect(bool enabled, uint32_t max_attempts, uint32_t delay_ms);

/**
 * @brief Configure static IP
 * @param ip_addr IP address (network byte order, 0 = disable static IP)
 * @param netmask Network mask
 * @param gateway Gateway address
 * @param dns1 Primary DNS server
 * @param dns2 Secondary DNS server (can be 0)
 * @return ESP_OK on success
 */
esp_err_t network_set_static_ip(uint32_t ip_addr, uint32_t netmask, uint32_t gateway, 
                                uint32_t dns1, uint32_t dns2);

/**
 * @brief Enable/disable DHCP
 * @param enabled true to enable DHCP, false for static IP
 * @return ESP_OK on success
 */
esp_err_t network_set_dhcp_enabled(bool enabled);

/**
 * @brief Set network hostname
 * @param hostname Hostname string (max 32 characters)
 * @return ESP_OK on success
 */
esp_err_t network_set_hostname(const char *hostname);

/**
 * @brief Get network hostname
 * @param hostname Buffer to store hostname
 * @param max_len Maximum length of hostname buffer
 * @return ESP_OK on success
 */
esp_err_t network_get_hostname(char *hostname, size_t max_len);

/**
 * @brief Set power saving mode
 * @param mode Power saving mode (WIFI_PS_NONE, WIFI_PS_MIN_MODEM, WIFI_PS_MAX_MODEM)
 * @return ESP_OK on success
 */
esp_err_t network_set_power_save_mode(wifi_ps_type_t mode);

/**
 * @brief Get signal strength of current connection
 * @return RSSI in dBm, or 0 if not connected
 */
int8_t network_get_rssi(void);

/**
 * @brief Check if network is connected
 * @return true if connected
 */
bool network_is_connected(void);

/**
 * @brief Get IP address as string
 * @param ip_str Buffer to store IP string (min 16 bytes)
 * @return ESP_OK on success
 */
esp_err_t network_get_ip_string(char *ip_str);

/**
 * @brief Convert network security type to string
 * @param security Security type
 * @return String representation of security type
 */
const char* network_security_to_string(network_security_t security);

/**
 * @brief Convert network state to string
 * @param state Network state
 * @return String representation of state
 */
const char* network_state_to_string(network_state_t state);

#endif // NETWORK_H
