
/**
 * @file app_notes_enter passcode.c
 * @brief Page to set the passcode PIN
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
#define SET_PIN_STEP     0
#define CONFIRM_PIN_STEP 1

// index of children in screenChildren array
enum {
    BACK_BUTTON_TOKEN,
    RETRY_BUTTON_TOKEN
};

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
static nbgl_layout_t *layoutCtx;
static pinStruct_t    pinStr;
// pin that has been set in compose step
static uint8_t setPin[MAX_PIN_LENGTH];
static uint8_t setPinLen = 0;
// current step in page
static uint8_t step;
static uint8_t keypadIndex, hiddenDigitsIndex;

/**********************
 *      VARIABLES
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void buildScreen(void);
static void layoutTouchCallback(int token, uint8_t index);

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * @brief Page displaying an info under an icon, with a Retry button
 *
 * @param onRetryCallback function to be called when Retry button is pressed
 */
static void retryScreen(void)
{
    nbgl_layoutDescription_t layoutDescription
        = {.modal = false, .withLeftBorder = false, .onActionCallback = &layoutTouchCallback};
    nbgl_layoutCenteredInfo_t centeredInfo = {.text1   = "The PINs don't match",
                                              .text2   = "Try again.",
                                              .text3   = NULL,
                                              .style   = LARGE_CASE_INFO,
                                              .icon    = &C_Important_Circle_64px,
                                              .offsetY = 0};
    nbgl_layoutButton_t       button       = {.fittingContent = false,
                                              .icon           = NULL,
                                              .onBottom       = true,
                                              .style          = BLACK_BACKGROUND,
                                              .text           = "Choose my PIN",
                                              .token          = RETRY_BUTTON_TOKEN,
                                              .tuneId         = TUNE_TAP_CASUAL};

    layoutCtx = nbgl_layoutGet(&layoutDescription);
#ifndef TARGET_STAX
    nbgl_layoutHeader_t headerDesc
        = {.type = HEADER_EMPTY, .separationLine = false, .emptySpace.height = 64};
    nbgl_layoutAddHeader(layoutCtx, &headerDesc);
#endif  // TARGET_STAX
    nbgl_layoutAddCenteredInfo(layoutCtx, &centeredInfo);
    nbgl_layoutAddButton(layoutCtx, &button);

    nbgl_layoutDraw(layoutCtx);
}

/**
 * @brief Function used to display the pin code markers, and configure pin pad
 *
 */
static void setPinCodeText(bool add)
{
    bool    enableValidate, enableBackspace, enableDigits;
    bool    redrawKeypad = false;
    uint8_t minLen, maxLen;

    // if first step, pin can be validated when min 4 digits
    // if second step, pin can be validated when min the number of digits entered in step 1
    minLen          = (step == SET_PIN_STEP) ? MIN_PIN_LENGTH : setPinLen;
    maxLen          = (step == SET_PIN_STEP) ? MAX_PIN_LENGTH : setPinLen;
    enableDigits    = (pinStr.pinLen < maxLen);
    enableValidate  = (pinStr.pinLen >= minLen);
    enableBackspace = (pinStr.pinLen > 0);
    if (add) {
        if ((pinStr.pinLen == minLen) ||  // activate "validate" button on keypad
            (pinStr.pinLen == maxLen) ||  // deactivate digits
            (pinStr.pinLen == 1)) {       // activate "backspace"
            redrawKeypad = true;
        }
    }
    else {                                      // remove
        if ((pinStr.pinLen == 0) ||             // deactivate "backspace" button on keypad
            (pinStr.pinLen == (minLen - 1)) ||  // deactivate "validate" button on keypad
            (pinStr.pinLen == (maxLen - 1))) {  // reactivate digits
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
            nbgl_refreshSpecialWithPostRefresh(FULL_COLOR_REFRESH, POST_REFRESH_FORCE_POWER_ON);
        }
        else {
            nbgl_refreshSpecialWithPostRefresh(BLACK_AND_WHITE_REFRESH,
                                               POST_REFRESH_FORCE_POWER_ON);
        }
    }
    else {
        nbgl_refreshSpecialWithPostRefresh(BLACK_AND_WHITE_FAST_REFRESH,
                                           POST_REFRESH_FORCE_POWER_ON);
    }
}

static void onPinMismatch(void)
{
    app_notesSetPasscode();
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
        uint8_t i;
        // if we are in set-pin step, go to confirm-pin step
        if (step == SET_PIN_STEP) {
            io_seproxyhal_play_tune(TUNE_TAP_CASUAL);
            for (i = 0; i < pinStr.pinLen; i++) {
                setPin[i] = pinStr.pin[i];
            }
            setPinLen = pinStr.pinLen;
            step      = CONFIRM_PIN_STEP;
            buildScreen();
        }
        else if (step == CONFIRM_PIN_STEP) {
            // verify pin
            for (i = 0; i < pinStr.pinLen; i++) {
                // if any of the entered confirmed pin is wrong, let's retry from
                // beginning
                if (setPin[i] != pinStr.pin[i]) {
                    LOG_DEBUG(UX_LOGGER, "keypadCallback(): wrong pin\n");
                    io_seproxyhal_play_tune(TUNE_NEUTRAL);
                    retryScreen();
                    // Ensure clean refresh to avoid ghosting effect after pinpad
                    nbgl_refreshSpecial(FULL_COLOR_REFRESH);
                    return;
                }
            }
            // change state
            app_notesSettingsSetLockAndPasscode(true, setPin, setPinLen);
            app_notesSettings();
        }
    }
}

// called when back key is pressed
static void layoutTouchCallback(int token, uint8_t index)
{
    UNUSED(index);
    nbgl_layoutRelease(layoutCtx);
    layoutCtx = NULL;
    if (token == BACK_BUTTON_TOKEN) {
        // if we are in the pin set step (first), back should exit PIN
        // otherwise we shoul move to pin set step
        if (step == SET_PIN_STEP) {
            app_notesSettings();
        }
        else {
            step      = SET_PIN_STEP;
            setPinLen = 0;
            buildScreen();
        }
    }
    else if (token == RETRY_BUTTON_TOKEN) {
        onPinMismatch();
    }
}

static void buildScreen(void)
{
    nbgl_layoutDescription_t  layoutDescription = {.onActionCallback      = layoutTouchCallback,
                                                   .tapActionText         = NULL,
                                                   .ticker.tickerCallback = NULL,
                                                   .modal                 = false,
                                                   .withLeftBorder        = false};
    nbgl_layoutCenteredInfo_t centeredInfo      = {.text1 = NULL,
                                                   .text3 = NULL,
                                                   .style = LARGE_CASE_INFO,
                                                   .icon  = NULL,
#ifdef TARGET_STAX
                                              .offsetY = -12,  // in addition to regular 24px
#else                                                          // TARGET_STAX
                                              .offsetY = -8,  // in addition to regular 32px
#endif                                                         // TARGET_STAX
                                              .onTop = true};
    int status;

    // reset the pin buffer
    memset(&pinStr, 0, sizeof(pinStruct_t));

    layoutCtx = nbgl_layoutGet(&layoutDescription);
    if (step == SET_PIN_STEP) {
        centeredInfo.text2 = "Choose a code that is different from your device PIN";
    }
    else if (step == CONFIRM_PIN_STEP) {
        centeredInfo.text2 = "Re-enter your Notes passcode";
    }
    // add back key
    nbgl_layoutHeader_t headerDesc = {.type               = HEADER_BACK_AND_TEXT,
                                      .separationLine     = false,
                                      .backAndText.token  = BACK_BUTTON_TOKEN,
                                      .backAndText.tuneId = TUNE_TAP_CASUAL,
                                      .backAndText.text   = NULL};
    nbgl_layoutAddHeader(layoutCtx, &headerDesc);

    // add description
    nbgl_layoutAddCenteredInfo(layoutCtx, &centeredInfo);

    status = nbgl_layoutAddKeypad(layoutCtx, keypadCallback, true);
    if (status < 0) {
        return;
    }
    keypadIndex = status;
    // add hidden digits
    status = nbgl_layoutAddHiddenDigits(layoutCtx,
                                        (step == CONFIRM_PIN_STEP) ? setPinLen : MAX_PIN_LENGTH);
    if (status < 0) {
        return;
    }
    hiddenDigitsIndex = status;

    nbgl_layoutDraw(layoutCtx);
    nbgl_refreshSpecialWithPostRefresh(FULL_COLOR_CLEAN_REFRESH, POST_REFRESH_FORCE_POWER_ON);
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * @brief Page to display the settings of Notes app
 *
 */
void app_notesSetPasscode(void)
{
    step = SET_PIN_STEP;
    buildScreen();
}
