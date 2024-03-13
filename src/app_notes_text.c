
/**
 * @file app_notes_text.c
 * @brief Functions to handle text edition in Notes app
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
    CONFIRM_BUTTON_TOKEN,
    KBD_TEXT_TOKEN,
    ERASE_TEXT_TOKEN
};

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static nbgl_callback_t onBackCallback;
static nbgl_callback_t onConfirmCallback;
static const char     *confirmButtonText;
static char           *enteredText;
static uint16_t        enteredTextLen;
static uint8_t         keyboardIndex, textIndex, buttonIndex;
static nbgl_layout_t  *layoutContext;

/**********************
 *      VARIABLES
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void layoutTouchCallback(int token, uint8_t index)
{
    UNUSED(index);

    if (token == CONFIRM_BUTTON_TOKEN) {
        char *enteredTextEnd = enteredText + strlen(enteredText) - 1;

        // trim trailing ' ' chars from entered name
        while (*enteredTextEnd == ' ') {
            *enteredTextEnd = '\0';
            enteredTextEnd--;
        }

        // save enteredText and exit
        onConfirmCallback();
    }
    else if (token == KBD_TEXT_TOKEN) {
        // when nothing entered yet, a touch on this area should clear the gray text
        if ((enteredTextLen == 0) && (strlen(enteredText) != 0)) {
            io_seproxyhal_play_tune(TUNE_TAP_CASUAL);
            enteredText[enteredTextLen] = 0;
            nbgl_layoutUpdateEnteredText(layoutContext, textIndex, false, 0, enteredText, true);
            nbgl_refreshSpecialWithPostRefresh(BLACK_AND_WHITE_REFRESH,
                                               POST_REFRESH_FORCE_POWER_ON);
        }
    }
    else if (token == BACK_BUTTON_TOKEN) {
        // go to previous screen or previous word
        onBackCallback();
    }
    else if (token == ERASE_TEXT_TOKEN) {
        // delete all chars
        enteredTextLen              = 0;
        enteredText[enteredTextLen] = 0;

        nbgl_layoutUpdateEnteredText(layoutContext, textIndex, false, 0, enteredText, true);
        nbgl_layoutUpdateKeyboard(layoutContext, keyboardIndex, 1 << 29, true, UPPER_CASE);

#ifdef HAVE_CONFIGURABLE_DISPLAY_FAST_MODE
        nbgl_post_refresh_t post_refresh;
        if (bolos_ux_settingsIsSmartFastModeEnabled()) {
            if (touchedKey != BACKSPACE_KEY) {
                // No backspace: fast mode
                post_refresh = POST_REFRESH_FORCE_POWER_ON;
            }
            else {
                post_refresh = POST_REFRESH_FORCE_POWER_OFF;
            }
        }
        else {
            post_refresh = POST_REFRESH_FORCE_POWER_ON;
        }

        nbgl_refreshSpecialWithPostRefresh(FULL_COLOR_PARTIAL_REFRESH, post_refresh);
#else   // HAVE_CONFIGURABLE_DISPLAY_FAST_MODE
        nbgl_refreshSpecialWithPostRefresh(FULL_COLOR_PARTIAL_REFRESH, POST_REFRESH_FORCE_POWER_ON);
#endif  // !HAVE_CONFIGURABLE_DISPLAY_FAST_MODE
    }
}

static void keyboardCallback(char touchedKey)
{
    bool                redrawKeyboard = false;
    bool                updateCasing   = false;
    uint32_t            keyMask;
    nbgl_refresh_mode_t refreshMode = BLACK_AND_WHITE_FAST_REFRESH;

    LOG_DEBUG(UX_LOGGER, "keyboardCallback(): touchedKey = %d\n", touchedKey);
    // if not Backspace
    if (touchedKey != BACKSPACE_KEY) {
        if (enteredTextLen < (NOTE_CONTENT_MAX_LEN - 1)) {
            io_seproxyhal_play_tune(TUNE_TAP_CASUAL);
            enteredText[enteredTextLen] = touchedKey;
            enteredTextLen++;
            // NULL terminated
            enteredText[enteredTextLen] = 0;
            if (nbgl_layoutUpdateEnteredText(layoutContext, textIndex, false, 0, enteredText, false)
                > 0) {
                // the text was too long for area so needs a clean refresh
                refreshMode = BLACK_AND_WHITE_REFRESH;
            }
            // reactivate all 'char' keys of keyboard if they were deactivated
            if (enteredTextLen == 1) {
                // when the first char is added, the default name in gray is removed
                // so normal refresh to avoid ghosting
                refreshMode    = BLACK_AND_WHITE_REFRESH;
                keyMask        = 0;
                redrawKeyboard = true;
                // activate "Confirm" button
                nbgl_layoutUpdateConfirmationButton(
                    layoutContext, buttonIndex, true, confirmButtonText);
            }
        }
    }
    else {  // backspace
        if (enteredTextLen > 0) {
            io_seproxyhal_play_tune(TUNE_TAP_CASUAL);
            enteredTextLen--;
            enteredText[enteredTextLen] = 0;

            if (enteredTextLen == 0) {
                keyMask        = 1 << 29;  // only SPACE key is inactive
                redrawKeyboard = true;
                updateCasing   = true;
                nbgl_layoutUpdateEnteredText(layoutContext, textIndex, false, 0, enteredText, true);
                refreshMode = FULL_COLOR_PARTIAL_REFRESH;
            }
            else {
                nbgl_layoutUpdateEnteredText(
                    layoutContext, textIndex, false, 0, enteredText, false);
                // do a normal refresh to avoid ghosting on removed char
                refreshMode = BLACK_AND_WHITE_REFRESH;
            }
        }
    }
    if (redrawKeyboard) {
        nbgl_layoutUpdateKeyboard(layoutContext, keyboardIndex, keyMask, updateCasing, UPPER_CASE);
    }
    else if (nbgl_layoutKeyboardNeedsRefresh(layoutContext, keyboardIndex)) {
        if (refreshMode == BLACK_AND_WHITE_FAST_REFRESH) {
            // do a normal refresh to avoid ghosting on keyboard
            refreshMode = BLACK_AND_WHITE_REFRESH;
        }
    }

#ifdef HAVE_CONFIGURABLE_DISPLAY_FAST_MODE
    nbgl_post_refresh_t post_refresh;
    if (bolos_ux_settingsIsSmartFastModeEnabled()) {
        if (touchedKey != BACKSPACE_KEY) {
            // No backspace: fast mode
            post_refresh = POST_REFRESH_FORCE_POWER_ON;
        }
        else {
            post_refresh = POST_REFRESH_FORCE_POWER_OFF;
        }
    }
    else {
        post_refresh = POST_REFRESH_FORCE_POWER_ON;
    }

    nbgl_refreshSpecialWithPostRefresh(refreshMode, post_refresh);
#else   // HAVE_CONFIGURABLE_DISPLAY_FAST_MODE
    nbgl_refreshSpecialWithPostRefresh(refreshMode, POST_REFRESH_FORCE_POWER_ON);
#endif  // !HAVE_CONFIGURABLE_DISPLAY_FAST_MODE
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * @brief launch page to edit the given text
 *
 * @param onBack function called if back key is pressed
 * @param onConfirm  function called if confirm button is pressed
 * @param headerText  text to set in header
 * @param confirmText  text to set in confirm button
 * @param text text to edit
 */
void app_notesEditText(nbgl_callback_t onBack,
                       nbgl_callback_t onConfirm,
                       const char     *headerText,
                       const char     *confirmText,
                       char           *text)
{
    onBackCallback                             = onBack;
    nbgl_layoutDescription_t layoutDescription = {.modal                 = false,
                                                  .withLeftBorder        = true,
                                                  .onActionCallback      = &layoutTouchCallback,
                                                  .ticker.tickerCallback = NULL};
    nbgl_layoutKbd_t         kbdInfo           = {.callback    = keyboardCallback,
                                                  .lettersOnly = false,  // all types of chars are allowed
                                                  .mode        = MODE_LETTERS};
    nbgl_layoutHeader_t      headerDesc        = {
                    .type                        = HEADER_BACK_TEXT_AND_ACTION,
                    .separationLine              = true,
                    .backTextAndAction.backToken = BACK_BUTTON_TOKEN,
                    .backTextAndAction.tuneId    = TUNE_TAP_CASUAL,
                    .backTextAndAction.text      = (char *) headerText,
#ifdef TARGET_STAX
        .backTextAndAction.actionIcon = &C_Close_32px,
#else   // TARGET_STAX
        .backTextAndAction.actionIcon = &C_Close_40px
#endif  // TARGET_STAX
    };
    int status;

    confirmButtonText = confirmText;
    onBackCallback    = onBack;
    onConfirmCallback = onConfirm;

    enteredText    = text;
    enteredTextLen = strlen(enteredText);

    layoutContext = nbgl_layoutGet(&layoutDescription);
    if (enteredTextLen > 0) {
        headerDesc.backTextAndAction.actionToken = ERASE_TEXT_TOKEN;
    }
    else {
        headerDesc.backTextAndAction.actionToken = 0xFF;
    }
    nbgl_layoutAddHeader(layoutContext, &headerDesc);

    // add keyboard
    // only SPACE key is inactive if text is empty
    kbdInfo.keyMask = (enteredTextLen > 0) ? 0 : 1 << 29;
    kbdInfo.casing  = (enteredTextLen > 0) ? LOWER_CASE : UPPER_CASE;
    status          = nbgl_layoutAddKeyboard(layoutContext, &kbdInfo);
    if (status < 0) {
        return;
    }
    keyboardIndex = (uint8_t) status;
    // add confirmation button
    status = nbgl_layoutAddConfirmationButton(layoutContext,
                                              enteredTextLen > 0,
                                              confirmButtonText,
                                              CONFIRM_BUTTON_TOKEN,
                                              TUNE_TAP_CASUAL);
    if (status < 0) {
        return;
    }
    buttonIndex = (uint8_t) status;
    // add entered text with current text
    status = nbgl_layoutAddEnteredText(layoutContext,
                                       false,
                                       0,
                                       enteredText,
                                       false,
#ifdef TARGET_STAX
                                       60,
#else   // TARGET_STAX
                                       24,
#endif  // TARGET_STAX
                                       KBD_TEXT_TOKEN);
    if (status < 0) {
        return;
    }
    textIndex = (uint8_t) status;
    nbgl_layoutDraw(layoutContext);

    // Ensure a clean refresh is used in order
    // to properly render the gray 'Confirm name' button
#ifdef HAVE_CONFIGURABLE_DISPLAY_FAST_MODE
    if (bolos_ux_settingsIsSmartFastModeEnabled()) {
        nbgl_refreshSpecialWithPostRefresh(FULL_COLOR_CLEAN_REFRESH, POST_REFRESH_FORCE_POWER_OFF);
    }
    else {
        nbgl_refreshSpecialWithPostRefresh(FULL_COLOR_CLEAN_REFRESH, POST_REFRESH_FORCE_POWER_ON);
    }
#else   // HAVE_CONFIGURABLE_DISPLAY_FAST_MODE
    nbgl_refreshSpecialWithPostRefresh(FULL_COLOR_CLEAN_REFRESH, POST_REFRESH_FORCE_POWER_ON);
#endif  // !HAVE_CONFIGURABLE_DISPLAY_FAST_MODE
}
