
/**
 * @file app_notes_settings.c
 * @brief Page to display the settings of Notes app
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "nbgl_debug.h"
#include "nbgl_use_case.h"
#include "app_notes.h"

/*********************
 *      DEFINES
 *********************/
enum {
    BACK_BUTTON_TOKEN = 0,
    NAV_TOKEN,
    SWITCH_TOKEN,
};

#define NB_PAGES 2

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static nbgl_layout_t *layoutContext;
static uint8_t        currentPage;

/**********************
 *      VARIABLES
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void displaySettings(void);

static void onPinSuccess(void)
{
    app_notesSettingsSetLockAndPasscode(false, NULL, 0);
    app_notesSettings();
}

static void onPasscodeChoice(bool confirm)
{
    if (confirm) {
        if (!app_notesSettingsIsLocked()) {
            app_notesSetPasscode();
        }
        else {
            app_notesValidatePasscode(
                "Enter your Notes passcode\nto confirm", onPinSuccess, app_notesSettings);
        }
    }
    else {
        app_notesSettings();
    }
}

static void layoutTouchCallback(int token, uint8_t index)
{
    if (token == BACK_BUTTON_TOKEN) {
        ui_menu_main();
    }
    else if (token == NAV_TOKEN) {
        currentPage = index;
        displaySettings();
    }
    else if (token == SWITCH_TOKEN) {
        // ask confirmation
        nbgl_useCaseChoice(
            &C_Important_Circle_64px,
            app_notesSettingsIsLocked() ? "Remove Notes passcode?" : "Set a Notes passcode?",
            app_notesSettingsIsLocked()
                ? "Notes will be accessible as soon as the app is launched."
                : "If you forget this code, you won't be able to ever access your Notes.",
            app_notesSettingsIsLocked() ? "Remove passcode" : "Choose passcode",
            "Cancel",
            onPasscodeChoice);
    }
}

static void displaySettings(void)
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
                                                  .backAndText.text   = (char *) "Secure Notes"};
    nbgl_layoutSwitch_t      switchInfo        = {
                    .initState = app_notesSettingsIsLocked(),
                    .text      = "Lock Notes",
                    .subText = "Set a dedicated passcode as additional security on top of your device PIN.",
                    .token  = SWITCH_TOKEN,
                    .tuneId = NBGL_NO_TUNE,
    };

    layoutContext = nbgl_layoutGet(&layoutDescription);
    nbgl_layoutAddHeader(layoutContext, &headerDesc);

    nbgl_layoutNavigationBar_t navInfo = {.activePage         = currentPage,
                                          .nbPages            = NB_PAGES,
                                          .token              = NAV_TOKEN,
                                          .tuneId             = NBGL_NO_TUNE,
                                          .withBackKey        = true,
                                          .withExitKey        = false,
                                          .withSeparationLine = true};
    nbgl_layoutAddNavigationBar(layoutContext, &navInfo);

    if (currentPage == 0) {
        nbgl_layoutAddSwitch(layoutContext, &switchInfo);
        nbgl_layoutAddSeparationLine(layoutContext);
    }

    nbgl_layoutDraw(layoutContext);

    nbgl_refresh();
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * @brief Page to display the settings of Notes app
 *
 */
void app_notesSettings(void)
{
    currentPage = 0;

    displaySettings();
}