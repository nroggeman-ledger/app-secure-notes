/*****************************************************************************
 *   Ledger App Boilerplate.
 *   (c) 2020 Ledger SAS.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *****************************************************************************/

#include <stdint.h>   // uint*_t
#include <stdbool.h>  // bool
#include <stddef.h>   // size_t
#include <string.h>   // memset, explicit_bzero

#include "os.h"
#include "cx.h"
#include "io.h"
#include "buffer.h"
#include "crypto_helpers.h"

#include "notes_handlers.h"
#include "../globals.h"
#include "../app_notes.h"
#include "../types.h"
#include "../sw.h"
#include "../ui/display.h"
#include "../helper/send_response.h"

uint8_t sharedBuffer[256];

int handler_get_shared_note(void) {
    explicit_bzero(&G_context, sizeof(G_context));
    G_context.req_type = CONFIRM_GET_NOTE;
    G_context.state = STATE_NONE;

    Note_t *note = app_notesGetSharedNote();
    if (note == NULL) {
        PRINTF("Nothing to share\n");
        return io_send_sw(SW_NO_SHARED_NOTE);
    }
    else {
        uint8_t *ptr = sharedBuffer;
        uint8_t title_len = strlen(note->title);
        uint8_t content_len = strlen(note->content);

        // convert note title+content in a buffer
        // TODO: encrypt with public key of contact here
        *ptr++ = title_len;
        memcpy(ptr, note->title, title_len);
        ptr += title_len;
        *ptr++ = content_len;
        memcpy(ptr, note->content, content_len);
        return io_send_response_pointer(sharedBuffer, title_len+content_len+2, SW_OK);
    }
}
