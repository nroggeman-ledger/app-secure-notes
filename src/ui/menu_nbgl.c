
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

#ifdef HAVE_NBGL

#include "os.h"
#include "glyphs.h"
#include "nbgl_use_case.h"

#include "../globals.h"
#include "menu.h"
#include "app_notes.h"

//  -----------------------------------------------------------
//  ----------------------- HOME PAGE -------------------------
//  -----------------------------------------------------------
static void onNewNote(void)
{
    app_notesNew(ui_menu_main, &currentNote);
}

static void onPasscodeSuccess(void)
{
    uint8_t nbUsedNotes = app_notesGetAll(NULL);
    if (nbUsedNotes == 0) {
        onNewNote();
    }
    else {
        app_notesList();
    }
}

static void onActionButton(void)
{
    if (app_notesSettingsIsLocked()) {
        app_notesValidatePasscode("Enter your Notes passcode", onPasscodeSuccess, ui_menu_main);
    }
    else {
        onPasscodeSuccess();
    }
}

void app_quit(void) {
    // exit app here
    os_sched_exit(-1);
}

// home page definition
void ui_menu_main(void) {
// This parameter shall be set to false if the settings page contains only information
// about the application (version , developer name, ...). It shall be set to
// true if the settings page also contains user configurable parameters related to the
// operation of the application.
#define SETTINGS_BUTTON_ENABLED (true)

    app_notesInit();
    uint8_t nbUsedNotes = app_notesGetAll(NULL);
    nbgl_useCaseHomeExt2("Notes",
                         &C_app_securenotes_64px,
                         "Create private notes, and share them in-person with trusted people.",
                         true,
                         (nbUsedNotes == 0) ? "Create my first Note" : "Go to my Notes",
#ifdef TARGET_STAX
                         app_notesSettingsIsLocked() ? &C_Lock_32px : NULL,
#else   // TARGET_STAX
                         app_notesSettingsIsLocked() ? &C_Lock_40px : NULL,
#endif  // TARGET_STAX
                         onActionButton,
                     app_notesSettings,
                     app_quit);
}


#endif
