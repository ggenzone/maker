#ifndef NETWORK_CONFIG_H
#define NETWORK_CONFIG_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "network.h"

/**
 * @brief WiFi credential entry
 */
typedef struct {
    char ssid[33];                   // Network SSID
    char password[64];               // Network password
    network_security_t security;     // Security type
    bool auto_connect;               // Auto-connect to this network
    int8_t priority;                 // Connection priority (higher = preferred)
    uint32_t last_used;              // Last connection timestamp
} network_credential_t;

/**
 * @brief Network profile containing all network settings
 */
typedef struct {
    // Connection settings
    uint32_t connection_timeout_ms;   // Connection timeout
    uint32_t scan_timeout_ms;         // Scan timeout
    bool auto_reconnect;             // Enable auto-reconnect
    uint32_t reconnect_max_attempts; // Max reconnect attempts
    uint32_t reconnect_delay_ms;     // Delay between reconnects
    
    // Power management
    uint8_t power_save_mode;         // WiFi power save mode
    bool sleep_on_idle;              // Sleep when no activity
    uint32_t idle_timeout_ms;        // Idle timeout before sleep
    
    // IP configuration
    bool use_static_ip;              // Use static IP instead of DHCP
    uint32_t static_ip;              // Static IP address
    uint32_t static_netmask;         // Static netmask
    uint32_t static_gateway;         // Static gateway
    uint32_t static_dns1;            // Primary DNS
    uint32_t static_dns2;            // Secondary DNS
    
    // Advanced settings
    char hostname[32];               // Device hostname
    uint8_t channel_preference;      // Preferred WiFi channel (0 = auto)
    bool fast_scan;                  // Enable fast scan mode
    bool pmf_required;               // Require Protected Management Frames
    
    // Credential management
    uint8_t credential_count;        // Number of stored credentials
    network_credential_t credentials[8]; // Stored network credentials
} network_profile_t;

/**
 * @brief Initialize network configuration system
 * @return ESP_OK on success
 */
esp_err_t network_config_init(void);

/**
 * @brief Deinitialize network configuration system
 * @return ESP_OK on success
 */
esp_err_t network_config_deinit(void);

/**
 * @brief Load network profile from storage
 * @return ESP_OK on success
 */
esp_err_t network_config_load(void);

/**
 * @brief Save network profile to storage
 * @return ESP_OK on success
 */
esp_err_t network_config_save(void);

/**
 * @brief Reset network configuration to factory defaults
 * @return ESP_OK on success
 */
esp_err_t network_config_reset_to_defaults(void);

/**
 * @brief Get current network profile
 * @param profile Pointer to profile structure to fill
 * @return ESP_OK on success
 */
esp_err_t network_config_get_profile(network_profile_t *profile);

/**
 * @brief Set network profile
 * @param profile Pointer to new profile
 * @return ESP_OK on success
 */
esp_err_t network_config_set_profile(const network_profile_t *profile);

/**
 * @brief Add or update network credentials
 * @param ssid Network SSID
 * @param password Network password (can be NULL for open networks)
 * @param auto_connect Auto-connect to this network
 * @param priority Connection priority (higher = preferred)
 * @return ESP_OK on success
 */
esp_err_t network_config_add_credentials(const char *ssid, const char *password, 
                                        bool auto_connect, int8_t priority);

/**
 * @brief Remove network credentials
 * @param ssid Network SSID to remove
 * @return ESP_OK on success
 */
esp_err_t network_config_remove_credentials(const char *ssid);

/**
 * @brief Get network credentials by SSID
 * @param ssid Network SSID
 * @param credential Pointer to credential structure to fill
 * @return ESP_OK if found
 */
esp_err_t network_config_get_credentials(const char *ssid, network_credential_t *credential);

/**
 * @brief Get all stored credentials
 * @param credentials Array to store credentials
 * @param max_count Maximum number of credentials to return
 * @param actual_count Pointer to store actual count returned
 * @return ESP_OK on success
 */
esp_err_t network_config_get_all_credentials(network_credential_t *credentials, 
                                           uint8_t max_count, uint8_t *actual_count);

/**
 * @brief Find best credentials for auto-connect
 * @param credential Pointer to credential structure to fill
 * @return ESP_OK if found
 */
esp_err_t network_config_find_best_credentials(network_credential_t *credential);

/**
 * @brief Update last used timestamp for credentials
 * @param ssid Network SSID
 * @return ESP_OK on success
 */
esp_err_t network_config_update_last_used(const char *ssid);

/**
 * @brief Clear all stored credentials
 * @return ESP_OK on success
 */
esp_err_t network_config_clear_all_credentials(void);

// Individual setting getters and setters
uint32_t network_config_get_connection_timeout(void);
esp_err_t network_config_set_connection_timeout(uint32_t timeout_ms);

bool network_config_get_auto_reconnect(void);
esp_err_t network_config_set_auto_reconnect(bool enabled);

uint8_t network_config_get_power_save_mode(void);
esp_err_t network_config_set_power_save_mode(uint8_t mode);

bool network_config_get_use_static_ip(void);
esp_err_t network_config_set_use_static_ip(bool enabled);

esp_err_t network_config_get_static_ip_info(uint32_t *ip, uint32_t *netmask, uint32_t *gateway,
                                           uint32_t *dns1, uint32_t *dns2);
esp_err_t network_config_set_static_ip_info(uint32_t ip, uint32_t netmask, uint32_t gateway,
                                           uint32_t dns1, uint32_t dns2);

esp_err_t network_config_get_hostname(char *hostname, size_t max_len);
esp_err_t network_config_set_hostname(const char *hostname);

#endif // NETWORK_CONFIG_H
