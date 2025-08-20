#include "gui_screen_settings_system.h"
#include "../navigation/gui_navigator.h"
#include "../widgets/gui_widget_header.h"
#include "../gui_common.h"
#include "../gui_styles.h"
#include "network/ota_update.h"
#include "storage/app_config.h"
#include "esp_app_desc.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_heap_caps.h"
#include "esp_mac.h"
#include "lwip/ip4_addr.h"
#include "esp_log.h"

static const char *TAG = "GUI_SCREEN_SYSTEM";

// Helper functions for getting system information
static void get_mac_address_string(char *mac_str, size_t size) {
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    snprintf(mac_str, size, "%02X:%02X:%02X:%02X:%02X:%02X", 
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

static void get_ip_address_string(char *ip_str, size_t size) {
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif) {
        esp_netif_ip_info_t ip_info;
        if (esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
            if (ip_info.ip.addr != 0) {
                snprintf(ip_str, size, IPSTR, IP2STR(&ip_info.ip));
                return;
            }
        }
    }
    strcpy(ip_str, "Not connected");
}

static size_t get_free_heap_size(void) {
    return esp_get_free_heap_size();
}

static size_t get_minimum_free_heap_size(void) {
    return esp_get_minimum_free_heap_size();
}

static void get_flash_size_string(char *flash_str, size_t size) {
    uint32_t flash_size = 0;
    esp_flash_get_size(NULL, &flash_size);
    if (flash_size >= 1024 * 1024) {
        snprintf(flash_str, size, "%.1f MB", flash_size / (1024.0 * 1024.0));
    } else {
        snprintf(flash_str, size, "%ld KB", flash_size / 1024);
    }
}

static void get_chip_info_string(char *chip_str, size_t size) {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    snprintf(chip_str, size, "%s rev%d (%d cores)", 
             CONFIG_IDF_TARGET, chip_info.revision, chip_info.cores);
}

static void get_wifi_rssi_string(char *rssi_str, size_t size) {
    wifi_ap_record_t ap_info;
    esp_err_t ret = esp_wifi_sta_get_ap_info(&ap_info);
    if (ret == ESP_OK) {
        snprintf(rssi_str, size, "%d dBm", ap_info.rssi);
    } else {
        strcpy(rssi_str, "N/A");
    }
}

static lv_obj_t* create_info_panel(lv_obj_t *parent, const char *title) {
    // Create panel container
    lv_obj_t *panel = gui_create_clean_container(parent);
    lv_obj_set_size(panel, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(panel, 15, 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_10, 0);
    lv_obj_set_style_bg_color(panel, lv_color_white(), 0);
    lv_obj_set_style_radius(panel, 8, 0);
    
    // Title label
    lv_obj_t *title_label = lv_label_create(panel);
    lv_label_set_text(title_label, title);
    lv_obj_align(title_label, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_14, 0);
    //lv_obj_set_style_text_color(title_label, lv_color_hex(0x333333), 0);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0x1976D2), 0);
    
    return panel;
}

static void add_info_line(lv_obj_t *panel, lv_obj_t *prev_obj, const char *label, const char *value) {
    lv_obj_t *info_label = lv_label_create(panel);
    lv_label_set_text_fmt(info_label, "%s: %s", label, value);
    
    if (prev_obj) {
        lv_obj_align_to(info_label, prev_obj, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    } else {
        // First item, position below title
        lv_obj_t *title = lv_obj_get_child(panel, 0);
        lv_obj_align_to(info_label, title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    }
    
    lv_obj_set_style_text_font(info_label, &lv_font_montserrat_14, 0);
}

static void ota_update_btn_clicked(lv_event_t *e) {
    ESP_LOGI(TAG, "OTA update initiated from system settings");
    perform_ota_update();
}

void gui_screen_settings_system_show(void *user_data) {
    ESP_LOGI(TAG, "Showing system settings screen");
    
    // Create main container
    lv_obj_t *container = gui_create_clean_container(lv_scr_act());
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_center(container);
    
    // Configure flex layout vertical
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(container, 8, 0);
    lv_obj_set_style_pad_all(container, 0, 0);

    // Header with back button
    gui_widget_header_create_with_back(container, "System Info", false);

    // Get system information
    const esp_app_desc_t *app_desc = esp_app_get_description();
    char mac_str[18], ip_str[16], flash_str[16], chip_str[64];
    char heap_str[32], min_heap_str[32], rssi_str[16];
    
    get_mac_address_string(mac_str, sizeof(mac_str));
    get_ip_address_string(ip_str, sizeof(ip_str));
    get_flash_size_string(flash_str, sizeof(flash_str));
    get_chip_info_string(chip_str, sizeof(chip_str));
    get_wifi_rssi_string(rssi_str, sizeof(rssi_str));
    
    size_t free_heap = get_free_heap_size();
    size_t min_heap = get_minimum_free_heap_size();
    snprintf(heap_str, sizeof(heap_str), "%.1f KB", free_heap / 1024.0);
    snprintf(min_heap_str, sizeof(min_heap_str), "%.1f KB", min_heap / 1024.0);

    // === FIRMWARE INFO PANEL ===
    lv_obj_t *fw_panel = create_info_panel(container, "Firmware");
    lv_obj_t *last_obj = NULL;
    
    add_info_line(fw_panel, last_obj, "App", app_desc->project_name);
    last_obj = lv_obj_get_child(fw_panel, lv_obj_get_child_cnt(fw_panel) - 1);
    
    add_info_line(fw_panel, last_obj, "Version", app_desc->version);
    last_obj = lv_obj_get_child(fw_panel, lv_obj_get_child_cnt(fw_panel) - 1);
    
    add_info_line(fw_panel, last_obj, "Build Date", app_desc->date);
    last_obj = lv_obj_get_child(fw_panel, lv_obj_get_child_cnt(fw_panel) - 1);
    
    add_info_line(fw_panel, last_obj, "ESP-IDF", app_desc->idf_ver);

    // === HARDWARE INFO PANEL ===
    lv_obj_t *hw_panel = create_info_panel(container, "Hardware");
    last_obj = NULL;
    
    add_info_line(hw_panel, last_obj, "Chip", chip_str);
    last_obj = lv_obj_get_child(hw_panel, lv_obj_get_child_cnt(hw_panel) - 1);
    
    add_info_line(hw_panel, last_obj, "Flash Size", flash_str);
    last_obj = lv_obj_get_child(hw_panel, lv_obj_get_child_cnt(hw_panel) - 1);
    
    add_info_line(hw_panel, last_obj, "Free Heap", heap_str);
    last_obj = lv_obj_get_child(hw_panel, lv_obj_get_child_cnt(hw_panel) - 1);
    
    add_info_line(hw_panel, last_obj, "Min Heap", min_heap_str);

    // === NETWORK INFO PANEL ===
    lv_obj_t *net_panel = create_info_panel(container, "Network");
    last_obj = NULL;
    
    add_info_line(net_panel, last_obj, "MAC Address", mac_str);
    last_obj = lv_obj_get_child(net_panel, lv_obj_get_child_cnt(net_panel) - 1);
    
    add_info_line(net_panel, last_obj, "IP Address", ip_str);
    last_obj = lv_obj_get_child(net_panel, lv_obj_get_child_cnt(net_panel) - 1);
    
    add_info_line(net_panel, last_obj, "WiFi Signal", rssi_str);


    // === OTA UPDATE SECTION ===
    // Check if WiFi is enabled before showing OTA option
    if (app_config_get_wifi_enabled()) {
        // OTA Update item
        lv_obj_t *ota_item = gui_create_setting_item(container, "Update Firmware", LV_SYMBOL_REFRESH);
        lv_obj_add_event_cb(ota_item, ota_update_btn_clicked, LV_EVENT_CLICKED, NULL);
        lv_obj_add_flag(ota_item, LV_OBJ_FLAG_CLICKABLE);
        
        // Add some visual feedback for the OTA button
        lv_obj_set_style_bg_color(ota_item, lv_color_hex(0x2196F3), LV_STATE_PRESSED);
    } else {
        // Show message that WiFi is required for OTA
        lv_obj_t *wifi_required_label = lv_label_create(container);
        lv_label_set_text(wifi_required_label, "Enable WiFi to update firmware");
        lv_obj_set_style_text_align(wifi_required_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_color(wifi_required_label, lv_color_hex(0x999999), 0);
        lv_obj_set_style_text_font(wifi_required_label, &lv_font_montserrat_14, 0);
    }

    // Spacer to push everything up
    lv_obj_t *bottom_spacer = gui_create_clean_container(container);
    lv_obj_set_size(bottom_spacer, 1, 1);
    lv_obj_set_flex_grow(bottom_spacer, 1);
    lv_obj_set_style_bg_opa(bottom_spacer, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(bottom_spacer, LV_OBJ_FLAG_CLICKABLE);
    
    ESP_LOGI(TAG, "System info screen displayed");
}
