
/**
 * @file app_notes_action.c
 * @brief Page to display the different actions on a note
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
    DELETE_TOKEN,
    SHARE_TOKEN
};

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static Note_t         *concernedNote;
static nbgl_callback_t onBackCallback;
static nbgl_layout_t  *layoutContext;

/**********************
 *      VARIABLES
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void buildScreen(void);

static void onDeleteChoice(bool confirm)
{
    if (confirm) {
        app_notesDeleteNote(concernedNote->index);
        nbgl_useCaseStatus("Note deleted", true, app_notesList);
    }
    else {
        buildScreen();
    }
}

static void layoutTouchCallback(int token, uint8_t index)
{
    UNUSED(index);
    if (token == BACK_BUTTON_TOKEN) {
        onBackCallback();
    }
    else if (token == DELETE_TOKEN) {
        // ask confirmation
        nbgl_useCaseChoice(&C_Important_Circle_64px,
                           "Permanently delete\nthis Note?",
                           concernedNote->title,
                           "Delete Note",
                           "Keep Note",
                           onDeleteChoice);
    }
    else if (token >= SHARE_TOKEN) {
    }
}

static void buildScreen(void)
{
    nbgl_layoutDescription_t layoutDescription = {.modal                 = false,
                                                  .withLeftBorder        = true,
                                                  .onActionCallback      = &layoutTouchCallback,
                                                  .ticker.tickerCallback = NULL,
                                                  .tapActionText         = NULL};
    nbgl_layoutHeader_t      headerDesc        = {.type               = HEADER_BACK_AND_TEXT,
                                                  .separationLine     = true,
                                                  .backAndText.token  = BACK_BUTTON_TOKEN,
                                                  .backAndText.tuneId = TUNE_TAP_CASUAL,
                                                  .backAndText.text   = ""};
    nbgl_layoutBar_t         barLayout         = {
                        .centered  = false,
                        .iconLeft  = NULL,
                        .iconRight = &PUSH_ICON,
                        .inactive  = false,
                        .large     = false,
                        .subText   = false,
                        .tuneId    = NBGL_NO_TUNE,
    };

    layoutContext = nbgl_layoutGet(&layoutDescription);
    nbgl_layoutAddHeader(layoutContext, &headerDesc);

    barLayout.text     = "Delete Note...";
    barLayout.token    = DELETE_TOKEN;
    barLayout.iconLeft = &C_Trash_32px;
    nbgl_layoutAddTouchableBar(layoutContext, &barLayout);
    nbgl_layoutAddSeparationLine(layoutContext);

    barLayout.text     = "Share Note";
    barLayout.token    = SHARE_TOKEN;
    barLayout.iconLeft = &C_Share_32px;
    nbgl_layoutAddTouchableBar(layoutContext, &barLayout);
    nbgl_layoutAddSeparationLine(layoutContext);

    nbgl_layoutDraw(layoutContext);

    nbgl_refresh();
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * @brief Page to display the different actions on a note
 *
 */
void app_notesActionOnNote(nbgl_callback_t onBack, Note_t *note)
{
    onBackCallback = onBack;
    concernedNote  = note;
    buildScreen();
}
