#ifndef CRC_H_
#define CRC_H_

#include "_stdint.h"

namespace esphome {

/// Calculate a CRC-16 checksum of \p data with size \p len.
uint16_t crc16(const uint8_t *data, uint16_t len, uint16_t crc = 0xffff, uint16_t reverse_poly = 0xa001,
               bool refin = false, bool refout = false);

} // namespace esphome

#endif // CRC_H_