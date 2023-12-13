#ifdef UNIT_TEST

#include "_stdint.h"
 #ifdef USE_ESP32
 #include "esp32/rom/crc.h"
 #endif
 
namespace esphome {

static const uint16_t CRC16_A001_LE_LUT_L[] = {0x0000, 0xc0c1, 0xc181, 0x0140, 0xc301, 0x03c0, 0x0280, 0xc241,
                                               0xc601, 0x06c0, 0x0780, 0xc741, 0x0500, 0xc5c1, 0xc481, 0x0440};
static const uint16_t CRC16_A001_LE_LUT_H[] = {0x0000, 0xcc01, 0xd801, 0x1400, 0xf001, 0x3c00, 0x2800, 0xe401,
                                               0xa001, 0x6c00, 0x7800, 0xb401, 0x5000, 0x9c01, 0x8801, 0x4400};

#ifndef USE_ESP32
static const uint16_t CRC16_8408_LE_LUT_L[] = {0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
                                               0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7};
static const uint16_t CRC16_8408_LE_LUT_H[] = {0x0000, 0x1081, 0x2102, 0x3183, 0x4204, 0x5285, 0x6306, 0x7387,
                                               0x8408, 0x9489, 0xa50a, 0xb58b, 0xc60c, 0xd68d, 0xe70e, 0xf78f};

static const uint16_t CRC16_1021_BE_LUT_L[] = {0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
                                               0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef};
static const uint16_t CRC16_1021_BE_LUT_H[] = {0x0000, 0x1231, 0x2462, 0x3653, 0x48c4, 0x5af5, 0x6ca6, 0x7e97,
                                               0x9188, 0x83b9, 0xb5ea, 0xa7db, 0xd94c, 0xcb7d, 0xfd2e, 0xef1f};
#endif

/// Calculate a CRC-16 checksum of \p data with size \p len.
uint16_t crc16(const uint8_t *data, uint16_t len, uint16_t crc = 0xffff, uint16_t reverse_poly = 0xa001,
               bool refin = false, bool refout = false) {
#ifdef USE_ESP32
  if (reverse_poly == 0x8408) {
    crc = crc16_le(refin ? crc : (crc ^ 0xffff), data, len);
    return refout ? crc : (crc ^ 0xffff);
  }
#endif
  if (refin) {
    crc ^= 0xffff;
  }
#ifndef USE_ESP32
  if (reverse_poly == 0x8408) {
    while (len--) {
      uint8_t combo = crc ^ (uint8_t) *data++;
      crc = (crc >> 8) ^ CRC16_8408_LE_LUT_L[combo & 0x0F] ^ CRC16_8408_LE_LUT_H[combo >> 4];
    }
  } else
#endif
      if (reverse_poly == 0xa001) {
    while (len--) {
      uint8_t combo = crc ^ (uint8_t) *data++;
      crc = (crc >> 8) ^ CRC16_A001_LE_LUT_L[combo & 0x0F] ^ CRC16_A001_LE_LUT_H[combo >> 4];
    }
  } else {
    while (len--) {
      crc ^= *data++;
      for (uint8_t i = 0; i < 8; i++) {
        if (crc & 0x0001) {
          crc = (crc >> 1) ^ reverse_poly;
        } else {
          crc >>= 1;
        }
      }
    }
  }
  return refout ? (crc ^ 0xffff) : crc;
}

} // namespace esphome

#endif