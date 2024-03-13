
/**
 * @file app_notes_new.c
 * @brief Functions to create new note in Note App
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <string.h>
#include "nbgl_debug.h"
#include "nbgl_use_case.h"
#include "app_notes.h"

/*********************
 *      DEFINES
 *********************/
enum {
    BACK_BUTTON_TOKEN = 0,
    TAP_ACTION_TOKEN
};

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static Note_t         *newNote;
static nbgl_callback_t onBackCallback;

/**********************
 *      VARIABLES
 **********************/

/**********************
 *  STATIC FUNCTIONS
 **********************/

static void onTitleConfirmed(void)
{
    int status;
    // save note without content
    status = app_notesAddNote(newNote->title, newNote->content);
    if (status >= 0) {
        newNote->index = (uint8_t) status;
    }
    app_notesDisplay(app_notesList, newNote);
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * @brief Stellar application start page
 *
 */
void app_notesNew(nbgl_callback_t onBack, Note_t *note)
{
    onBackCallback = onBack;
    newNote        = note;
    // reset title & content
    strcpy(note->title, "");
    strcpy(note->content, "");

    // TODO: Add a max len for text
    app_notesEditText(onBack, onTitleConfirmed, "New note", "Confirm title", note->title);
}
