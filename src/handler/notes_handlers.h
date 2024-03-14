#pragma once

#include <stdint.h>   // uint*_t
#include <stdbool.h>  // bool

#include "buffer.h"

extern uint8_t sharedBuffer[256];
/**
 * Handler for ADD_ADDRESS command. Send APDU response with version
 * of the application.
 *
 *
 * @return zero or positive integer if success, negative integer otherwise.
 *
 */
int handler_add_address(buffer_t *cdata);

/**
 * Handler for GET_NOTE command. Send APDU response with version
 * of the application.
 *
 *
 * @return zero or positive integer if success, negative integer otherwise.
 *
 */
int handler_get_shared_note(void);

/**
 * Handler for PUT_NOTE command. Send APDU response with version
 * of the application.
 *
 *
 * @return zero or positive integer if success, negative integer otherwise.
 *
 */
int handler_put_shared_note(buffer_t *cdata);
