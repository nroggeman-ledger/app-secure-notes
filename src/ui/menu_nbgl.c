
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
    if (app_notesSettingsIsLocked() && !app_notesIsSessionUnlocked()) {
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
    uint8_t nbUsedNotes;
    const nbgl_icon_details_t *icon = NULL;

    app_notesInit();

    nbUsedNotes = app_notesGetAll(NULL);
    if (app_notesSettingsIsLocked()) {

        if (app_notesIsSessionUnlocked()) {
#ifdef TARGET_STAX
            icon = &C_Check_32px;
#else   // TARGET_STAX
            icon = &C_Check_40px;
#endif  // TARGET_STAX
        }
        else {
#ifdef TARGET_STAX
            icon = &C_Lock_32px;
#else   // TARGET_STAX
            icon = &C_Lock_40px;
#endif  // TARGET_STAX
        }
    }
    nbgl_useCaseHomeExt2(APPNAME,
                         &C_app_securenotes_64px,
                         "Create private notes and share them with trusted people",
                         true,
                         (nbUsedNotes == 0) ? "Create my first Note" : "Go to my Notes",
                         icon,
                         onActionButton,
                     app_notesSettings,
                     app_quit);
}


#endif
