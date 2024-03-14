
/**
 * @file app_notes_new_contact.c
 * @brief Functions to create new contact in Notes App
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
    CANCEL_TOKEN = 0
};

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static Contact_t      *newContact;
static nbgl_callback_t onBackCallback;
static nbgl_layout_t  *layoutContext;

/**********************
 *      VARIABLES
 **********************/

/**********************
 *  STATIC FUNCTIONS
 **********************/

static void layoutTouchCallback(int token, uint8_t index)
{
    UNUSED(index);
    if (token == CANCEL_TOKEN) {
        onBackCallback();
    }
}

static void onNameConfirmed(void)
{
    // display a screen to invite user to use phone of computer to add address
    nbgl_layoutDescription_t layoutDescription
        = {.modal = false, .withLeftBorder = false, .onActionCallback = &layoutTouchCallback};
    nbgl_layoutCenteredInfo_t centeredInfo = {.text1   = "Continue on your phone or computer",
                                              .text2   = tmpString,
                                              .text3   = NULL,
                                              .style   = LARGE_CASE_INFO,
                                              .icon    = &C_Phone_64px,
                                              .offsetY = 0};
    nbgl_layoutFooter_t       footerDesc   = {.type              = FOOTER_SIMPLE_TEXT,
                                              .simpleText.text   = "Cancel",
                                              .simpleText.token  = CANCEL_TOKEN,
                                              .simpleText.tuneId = TUNE_TAP_CASUAL};

    layoutContext = nbgl_layoutGet(&layoutDescription);
#ifndef TARGET_STAX
    nbgl_layoutHeader_t headerDesc
        = {.type = HEADER_EMPTY, .separationLine = false, .emptySpace.height = 40};
    nbgl_layoutAddHeader(layoutContext, &headerDesc);
#endif  // TARGET_STAX
    snprintf(tmpString,
             sizeof(tmpString),
             "Use Ledger Live to get %s's info and send them this Note.",
             newContact->name);
    nbgl_layoutAddCenteredInfo(layoutContext, &centeredInfo);
    nbgl_layoutAddExtendedFooter(layoutContext, &footerDesc);

    nbgl_layoutDraw(layoutContext);
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * @brief Function to create new contact in Notes App
 *
 */
void app_notesNewContact(nbgl_callback_t onBack, Contact_t *contact)
{
    onBackCallback = onBack;
    newContact     = contact;
    // reset title & content
    strcpy(contact->name, "");
    strcpy(contact->address, "");

    // TODO: Add a max len for text
    app_notesEditText(
        onBack, onNameConfirmed, "Name your new trusted contact", "Confirm name", contact->name);
}

/**
 * @brief Function when receiving APDU with address
 *
 */
void app_notesAddAddress(const char *address)
{
    int status;

    // save contact without address
    status = app_notesAddContact(newContact->name, address);
    if (status >= 0) {
        newContact->index = (uint8_t) status;
    }
    onBackCallback();
}