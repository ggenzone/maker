#ifndef BASE32_H
#define BASE32_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Decode Base32 encoded string
 * @param encoded Base32 encoded string (uppercase, no padding)
 * @param decoded Output buffer for decoded data
 * @param decoded_len Size of output buffer
 * @return Length of decoded data, or -1 on error
 */
int base32_decode(const char *encoded, uint8_t *decoded, size_t decoded_len);

#endif // BASE32_H
