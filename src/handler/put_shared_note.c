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

int handler_put_shared_note(buffer_t *cdata) {
    uint8_t title_len = 0;
    uint8_t content_len;

    explicit_bzero(&G_context, sizeof(G_context));
    G_context.req_type = CONFIRM_PUT_NOTE;
    G_context.state = STATE_NONE;
            PRINTF("LC = %d\n", cdata->size);

     // get content and title from shared buffer
    // TODO: should be a decryption with private key
   if (!buffer_read_u8(cdata, &title_len) ||
        !buffer_read_nu8(cdata, sharedBuffer, (size_t) title_len)) {
            PRINTF("Wrong title len %d\n", title_len);
        return io_send_sw(SW_WRONG_DATA_LENGTH);
    }
    sharedBuffer[title_len] = 0;
    if (!buffer_read_u8(cdata, &content_len) ||
        !buffer_read_nu8(cdata, &sharedBuffer[title_len+1], (size_t) content_len)) {
            PRINTF("Wrong content len\n");
        return io_send_sw(SW_WRONG_DATA_LENGTH);
    }
    sharedBuffer[title_len+content_len+1] = 0;
    app_notesReceiveSharedNote((const char*)&sharedBuffer[0], (const char*)&sharedBuffer[title_len+1]);
    return io_send_sw(SW_OK);
}
