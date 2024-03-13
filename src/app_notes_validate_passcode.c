
/**
 * @file app_notes_validate passcode.c
 * @brief Page to validate the existing passcode PIN
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
    TAP_TOKEN = 0,
    BACK_TOKEN
};

#define ALREADY_OPENED() ((layoutCtx != NULL) || (errorLayoutCtx != NULL))

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    uint8_t pinLen;
    char    pin[MAX_PIN_LENGTH];
} pinStruct_t;

/**********************
 *  STATIC VARIABLES
 **********************/
static nbgl_callback_t afterPinCallback;  // function called if pin succesfull
static nbgl_callback_t onBack;            // function called if back key is pressed
static nbgl_layout_t  *layoutCtx, *errorLayoutCtx;
static uint8_t         keypadIndex, hiddenDigitsIndex;
static const char     *messageStr;
static pinStruct_t     pinStr;

/**********************
 *      VARIABLES
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void buildScreen(void);

/**********************
 *   STATIC FUNCTIONS
 **********************/
// layout callback function for error screen
static void errorTouchCallback(int token, uint8_t index)
{
    UNUSED(index);
    nbgl_layoutRelease(errorLayoutCtx);
    errorLayoutCtx = NULL;
    if (token == TAP_TOKEN) {
        buildScreen();
    }
}

/**
 * @brief Function used to display the invalid pin screen, after invalid attempts, and before reboot
 *
 * @param retries number of attempts left
 */
static void buildInvalidPinScreen()
{
    nbgl_layoutCenteredInfo_t info;
    nbgl_layoutDescription_t  layoutDescription = {
         .onActionCallback      = &errorTouchCallback,
         .tapActionText         = NULL,
         .ticker.tickerCallback = NULL,
         .withLeftBorder        = false,
         .modal                 = true,
    };

    layoutDescription.tapActionText  = "Tap to continue";
    layoutDescription.tapActionToken = TAP_TOKEN;
    layoutDescription.tapTuneId      = TUNE_TAP_CASUAL;
    errorLayoutCtx                   = nbgl_layoutGet(&layoutDescription);

    info.style = LARGE_CASE_INFO;
    info.onTop = false;
    info.text3 = NULL;
    info.text1 = "Incorrect PIN";
    info.icon  = &C_Important_Circle_64px;
#ifdef TARGET_STAX
    info.offsetY = -40;
#else   // TARGET_STAX
    info.offsetY = -48;
#endif  // TARGET_STAX
    info.text2 = (char *) "Incorrect PIN, try again";
    nbgl_layoutHeader_t headerDesc
        = {.type = HEADER_EMPTY, .separationLine = false, .emptySpace.height = 64};
    nbgl_layoutAddHeader(errorLayoutCtx, &headerDesc);
    nbgl_layoutAddCenteredInfo(errorLayoutCtx, &info);

    nbgl_layoutDraw(errorLayoutCtx);
    nbgl_refreshSpecial(FULL_COLOR_REFRESH);
}

// layout callback function for nominal screen
static void touchCallback(int token, uint8_t index)
{
    UNUSED(index);
    if ((token == BACK_TOKEN) && (onBack != NULL)) {
        nbgl_layoutRelease(layoutCtx);
        layoutCtx = NULL;
        onBack();
    }
}

/**
 * @brief Function used to display the pin code markers, and configure pin pad
 *
 */
static void setPinCodeText(bool add)
{
    bool enableValidate, enableBackspace, enableDigits;
    bool redrawKeypad = false;

    enableDigits    = (pinStr.pinLen < MAX_PIN_LENGTH);
    enableValidate  = (pinStr.pinLen > (MIN_PIN_LENGTH - 1));
    enableBackspace = (pinStr.pinLen > 0);
    if (add) {
        if ((pinStr.pinLen == MIN_PIN_LENGTH) ||  // activate "validate" button on keypad
            (pinStr.pinLen == MAX_PIN_LENGTH) ||  // deactivate "digits" on keypad
            (pinStr.pinLen == 1)) {               // activate "backspace"
            redrawKeypad = true;
        }
    }
    else {                                              // remove
        if ((pinStr.pinLen == 0) ||                     // deactivate "backspace" button on keypad;
            (pinStr.pinLen == (MIN_PIN_LENGTH - 1)) ||  // deactivate "validate" button on keypad
            (pinStr.pinLen == (MAX_PIN_LENGTH - 1))) {  // reactivate "digits" on keypad
            redrawKeypad = true;
        }
    }
    nbgl_layoutUpdateHiddenDigits(layoutCtx, hiddenDigitsIndex, pinStr.pinLen);
    if (redrawKeypad) {
        nbgl_layoutUpdateKeypad(
            layoutCtx, keypadIndex, enableValidate, enableBackspace, enableDigits);
    }

    if (!add) {
        if (pinStr.pinLen == 0) {
            // Full refresh to fully clean the bullets when reaching 0 digits
#ifndef HAVE_CONFIGURABLE_DISPLAY_FAST_MODE
            nbgl_refreshSpecialWithPostRefresh(FULL_COLOR_REFRESH, POST_REFRESH_FORCE_POWER_ON);
#else
            if (bolos_ux_settingsIsSmartFastModeEnabled()) {
                nbgl_refreshSpecialWithPostRefresh(FULL_COLOR_REFRESH,
                                                   POST_REFRESH_FORCE_POWER_OFF);
            }
            else {
                nbgl_refreshSpecialWithPostRefresh(FULL_COLOR_REFRESH, POST_REFRESH_FORCE_POWER_ON);
            }
#endif
        }
        else {
#ifndef HAVE_CONFIGURABLE_DISPLAY_FAST_MODE
            nbgl_refreshSpecialWithPostRefresh(BLACK_AND_WHITE_REFRESH,
                                               POST_REFRESH_FORCE_POWER_ON);
#else   // HAVE_CONFIGURABLE_DISPLAY_FAST_MODE
            if (bolos_ux_settingsIsSmartFastModeEnabled()) {
                nbgl_refreshSpecialWithPostRefresh(BLACK_AND_WHITE_REFRESH,
                                                   POST_REFRESH_FORCE_POWER_OFF);
            }
            else {
                nbgl_refreshSpecialWithPostRefresh(BLACK_AND_WHITE_REFRESH,
                                                   POST_REFRESH_FORCE_POWER_ON);
            }
#endif  // !HAVE_CONFIGURABLE_DISPLAY_FAST_MODE
        }
    }
    else {
        nbgl_refreshSpecialWithPostRefresh(BLACK_AND_WHITE_FAST_REFRESH,
                                           POST_REFRESH_FORCE_POWER_ON);
    }
}

/**
 * @brief Callback when a key of the keypad is pressed
 *
 * @param touchedKey char corresponding to the digit (from '0' to '9'). @ref BACKSPACE_KEY is used
 * for backspace
 */
static void keypadCallback(char touchedKey)
{
    // if a digit in ASCII
    if ((touchedKey >= 0x30) && (touchedKey < 0x40)) {
        if (pinStr.pinLen < MAX_PIN_LENGTH) {
            pinStr.pin[pinStr.pinLen] = touchedKey;
            pinStr.pinLen++;
        }
        setPinCodeText(true);
    }
    else if (touchedKey == BACKSPACE_KEY) {  // backspace
        if (pinStr.pinLen > 0) {
            pinStr.pinLen--;
            pinStr.pin[pinStr.pinLen] = 0;
        }
        setPinCodeText(false);
    }
    else if (touchedKey == VALIDATE_KEY) {  // validate
        // Gray out keyboard / buttons as a first user feedback
        nbgl_layoutUpdateKeypad(layoutCtx, keypadIndex, false, false, true);
        nbgl_refreshSpecialWithPostRefresh(BLACK_AND_WHITE_FAST_REFRESH,
                                           POST_REFRESH_FORCE_POWER_ON);

        // Free layout
        nbgl_layoutRelease(layoutCtx);
        layoutCtx = NULL;

        // is pin OK?
        if (app_notesSettingsCheckPasscode((uint8_t *) pinStr.pin, pinStr.pinLen)) {
            io_seproxyhal_play_tune(TUNE_TAP_CASUAL);
            if (afterPinCallback != NULL) {
                afterPinCallback();
            }
        }
        else {
            // reset the pin buffer
            memset(&pinStr, 0, sizeof(pinStruct_t));

            // Trigger neutral or error sound
            io_seproxyhal_play_tune(TUNE_NEUTRAL);

            buildInvalidPinScreen();
        }
    }
}

static void buildScreen(void)
{
    nbgl_layoutDescription_t  layoutDescription = {.onActionCallback      = touchCallback,
                                                   .tapActionText         = NULL,
                                                   .ticker.tickerCallback = NULL,
                                                   .modal                 = true,
                                                   .withLeftBorder        = false};
    nbgl_layoutCenteredInfo_t centeredInfo      = {.text1 = NULL,
                                                   .text2 = messageStr,
                                                   .text3 = NULL,
                                                   .style = LARGE_CASE_INFO,
                                                   .icon  = NULL,
#ifdef TARGET_STAX
                                              .offsetY = 48,
#else   // TARGET_STAX
                                              .offsetY = 24,
#endif  // TARGET_STAX
                                              .onTop = true};
    nbgl_layoutHeader_t headerDesc = {.type               = HEADER_BACK_AND_TEXT,
                                      .separationLine     = false,
                                      .backAndText.token  = BACK_TOKEN,
                                      .backAndText.tuneId = TUNE_TAP_CASUAL,
                                      .backAndText.text   = NULL};
    int                 status;

    // reset the pin buffer
    memset(&pinStr, 0, sizeof(pinStruct_t));

    layoutCtx = nbgl_layoutGet(&layoutDescription);
    // add back key if necessary, and only if PIN is not invalidated yet
    nbgl_layoutAddHeader(layoutCtx, &headerDesc);
#ifdef TARGET_STAX
    centeredInfo.offsetY = 0;
#else   // TARGET_STAX
    centeredInfo.offsetY = -8;
#endif  // TARGET_STAX

    // add description
    nbgl_layoutAddCenteredInfo(layoutCtx, &centeredInfo);

    // add keypad
    status = nbgl_layoutAddKeypad(layoutCtx, keypadCallback, true);
    if (status < 0) {
        return;
    }
    keypadIndex = status;
    // add hidden digits
    status = nbgl_layoutAddHiddenDigits(layoutCtx, MAX_PIN_LENGTH);
    if (status < 0) {
        return;
    }
    hiddenDigitsIndex = status;

    nbgl_layoutDraw(layoutCtx);

#ifndef HAVE_CONFIGURABLE_DISPLAY_FAST_MODE
    nbgl_refreshSpecialWithPostRefresh(FULL_COLOR_CLEAN_REFRESH, POST_REFRESH_FORCE_POWER_ON);
#else   // HAVE_CONFIGURABLE_DISPLAY_FAST_MODE
    if (bolos_ux_settingsIsSmartFastModeEnabled()) {
        nbgl_refreshSpecialWithPostRefresh(FULL_COLOR_CLEAN_REFRESH, POST_REFRESH_FORCE_POWER_OFF);
    }
    else {
        nbgl_refreshSpecialWithPostRefresh(FULL_COLOR_CLEAN_REFRESH, POST_REFRESH_FORCE_POWER_ON);
    }
#endif  // !HAVE_CONFIGURABLE_DISPLAY_FAST_MODE
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * @brief Page to validate the existing passcode PIN
 *
 */
void app_notesValidatePasscode(const char     *message,
                               nbgl_callback_t pinSuccessCallback,
                               nbgl_callback_t backCallback)
{
    afterPinCallback = pinSuccessCallback;
    onBack           = backCallback;
    messageStr       = message;
    buildScreen();
}
