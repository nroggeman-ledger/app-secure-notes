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
#include "buffer.h"

#include "notes_handlers.h"
#include "sign_tx.h"
#include "../sw.h"
#include "../globals.h"
#include "../app_notes.h"
#include "../ui/display.h"
#include "../transaction/types.h"
#include "../transaction/deserialize.h"

int handler_add_address(buffer_t *cdata) {
    explicit_bzero(&G_context, sizeof(G_context));
    G_context.req_type = CONFIRM_ADD_ADDRESS;
    G_context.state = STATE_NONE;

    // 32 bytes for address
    buffer_copy(cdata, (uint8_t *)sharedBuffer, 32);
    app_notesAddAddress((const char*)sharedBuffer);
    return io_send_sw(SW_OK);
}
