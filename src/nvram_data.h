/**
 * @file nvram_data.h
 * @brief All definitions of the specific part of NVRAM structure for this application
 */
#pragma once

#include <stdint.h>
#include "app_notes.h"

typedef struct {
    bool    locked;
    uint8_t unused[2];
    uint8_t nbDigits;
    uint8_t digits[8];
} NvramSettings_t;

typedef struct {
    bool       used;
    const char title[NOTE_TITLE_MAX_LEN];
    const char content[NOTE_CONTENT_MAX_LEN];
} NvramNote_t;

/**
 * @brief Oldest supported version of the NVRAM (for conversion)
 *
 */
#define NVRAM_FIRST_SUPPORTED_VERSION 1

/**
 * @brief Current version of the NVRAM structure
 * @note Set it to 1 to generate the first version of NVRAM data to fetch after a first launch.
 * Then set it back to 2 to generate the second version, then load fetched NVRAM after a
 * first launch.
 *
 */
#define NVRAM_STRUCT_VERSION 1

/**
 * @brief Current version of the NVRAM data
 *
 */
#define NVRAM_DATA_VERSION 1

/**
 * @brief Structure defining the NVRAM
 *
 */
typedef struct Nvram_data_s {
    NvramSettings_t settings;
    NvramNote_t     notes[NB_MAX_NOTES];
} Nvram_data_t;
