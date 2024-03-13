/**
 * @file app_notes.h
 * @brief Header for Notes app
 *
 */

#ifndef APP_NOTES_H
#define APP_NOTES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nbgl_types.h"
#include "nbgl_use_case.h"
#include "glyphs.h"
#include "menu.h"
#include "constants.h"

/*********************
 *      DEFINES
 *********************/
#define NB_MAX_NOTES         10
#define NOTE_TITLE_MAX_LEN   128
#define NOTE_CONTENT_MAX_LEN 512
#define NB_MAX_PARAGRAPHS    10

#define MAX_PIN_LENGTH 8
#define MIN_PIN_LENGTH 4

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    uint8_t index;
    char   *title;
    char   *content;
} Note_t;

/**********************
 *      VARIABLES
 **********************/
// current working note
extern Note_t currentNote;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void app_notesNew(nbgl_callback_t onBack, Note_t *note);
void app_notesSettings(void);
void app_notesSetPasscode(void);
void app_notesValidatePasscode(const char     *message,
                               nbgl_callback_t pinSuccessCallback,
                               nbgl_callback_t backCallback);
void app_notesList(void);
void app_notesEditText(nbgl_callback_t onBack,
                       nbgl_callback_t onConfirm,
                       const char     *headerText,
                       const char     *confirmText,
                       char           *text);
void app_notesDisplay(nbgl_callback_t onBack, Note_t *note);

void    app_notesInit(void);
uint8_t app_notesGetAll(Note_t noteArray[NB_MAX_NOTES]);
int     app_notesGetNote(uint8_t index, Note_t note);
int     app_notesAddNote(const char *title, const char *content);
int     app_notesModifyNote(uint8_t index, const char *title, const char *content);

bool app_notesSettingsIsLocked(void);
bool app_notesSettingsCheckPasscode(uint8_t *digits, uint8_t nbDigits);
void app_notesSettingsSetLockAndPasscode(bool lock, uint8_t *digits, uint8_t nbDigits);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* APP_NOTES_H */
