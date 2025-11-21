#include "base32.h"
#include <string.h>
#include <ctype.h>

static int base32_char_to_value(char c) {
    if (c >= 'A' && c <= 'Z') {
        return c - 'A';
    }
    if (c >= '2' && c <= '7') {
        return c - '2' + 26;
    }
    return -1;
}

int base32_decode(const char *encoded, uint8_t *decoded, size_t decoded_len) {
    if (encoded == NULL || decoded == NULL) {
        return -1;
    }

    size_t encoded_len = strlen(encoded);
    size_t decoded_idx = 0;
    uint32_t buffer = 0;
    int bits_in_buffer = 0;

    for (size_t i = 0; i < encoded_len; i++) {
        char c = toupper(encoded[i]);
        
        // Skip padding and whitespace
        if (c == '=' || c == ' ' || c == '\n' || c == '\r') {
            continue;
        }

        int value = base32_char_to_value(c);
        if (value < 0) {
            return -1; // Invalid character
        }

        buffer = (buffer << 5) | value;
        bits_in_buffer += 5;

        if (bits_in_buffer >= 8) {
            if (decoded_idx >= decoded_len) {
                return -1; // Output buffer too small
            }
            decoded[decoded_idx++] = (buffer >> (bits_in_buffer - 8)) & 0xFF;
            bits_in_buffer -= 8;
        }
    }

    return decoded_idx;
}
