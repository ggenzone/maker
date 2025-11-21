#ifndef TOTP_PARSER_H
#define TOTP_PARSER_H

#include "totp_storage.h"
#include "esp_err.h"

/**
 * @brief Parse otpauth:// URI and extract TOTP parameters
 * @param uri Full otpauth://totp/... string
 * @param service Output service structure
 * @return ESP_OK on success
 * 
 * Example URI formats:
 * - otpauth://totp/GitHub:user@email.com?secret=JBSWY3DPEHPK3PXP&issuer=GitHub
 * - otpauth://totp/user@email.com?secret=JBSWY3DPEHPK3PXP&issuer=Google
 * - otpauth://totp/ServiceName?secret=JBSWY3DPEHPK3PXP
 */
esp_err_t totp_parse_uri(const char *uri, totp_service_t *service);

#endif // TOTP_PARSER_H
