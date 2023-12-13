#ifndef TEST_INCLUDES_H_
#define TEST_INCLUDES_H_

#ifdef UNIT_TEST

#include <_stdint.h>
#include <stddef.h>
#include <crc.h>

#ifndef ESP_LOGI
#define ESP_LOGI
#endif

#ifndef ESP_LOGD
#define ESP_LOGD
#endif

#ifndef ESP_LOGV
#define ESP_LOGV
#endif

#endif // UNIT_TEST

#endif // TEST_INCLUDES_H_