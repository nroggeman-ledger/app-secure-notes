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
#define APPVERSION              "1.0.0"
#define NB_MAX_NOTES            10
#define NOTE_TITLE_MAX_LEN      128
#define NOTE_CONTENT_MAX_LEN    512
#define CONTACT_NAME_LEN        32
#define CONTACT_ADDRESS_MAX_LEN 32
#define NB_MAX_CONTACTS         16

#define NB_MAX_PARAGRAPHS 10

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

typedef struct {
    uint8_t index;
    char   *name;
    char   *address;
} Contact_t;

/**********************
 *      VARIABLES
 **********************/
// current working note
extern Note_t currentNote;
// current working contact
extern Contact_t currentContact;

extern char             tmpString[NOTE_CONTENT_MAX_LEN];

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void    ui_menu_main(void);
void    app_notesNew(nbgl_callback_t onBack, Note_t *note);
void    app_notesSettings(void);
void    app_notesSetPasscode(void);
void    app_notesValidatePasscode(const char     *message,
                                  nbgl_callback_t pinSuccessCallback,
                                  nbgl_callback_t backCallback);
void    app_notesList(void);
void    app_notesEditText(nbgl_callback_t onBack,
                          nbgl_callback_t onConfirm,
                          const char     *headerText,
                          const char     *confirmText,
                          char           *text);
void    app_notesDisplay(nbgl_callback_t onBack, Note_t *note);
void    app_notesActionOnNote(nbgl_callback_t onBack, Note_t *note);
void    app_notesShare(nbgl_callback_t onBack, Note_t *note);
void    app_notesNewContact(nbgl_callback_t onBack, Contact_t *contact);
void    app_notesAddAddress(const char *address);
Note_t *app_notesGetSharedNote(void);
int     app_notesReceiveSharedNote(const char *title, const char *content);

void    app_notesInit(void);
uint8_t app_notesGetAll(Note_t noteArray[NB_MAX_NOTES]);
int     app_notesGetNote(uint8_t index, Note_t *note);
int     app_notesAddNote(const char *title, const char *content);
int     app_notesModifyNote(uint8_t index, const char *title, const char *content);
int     app_notesDeleteNote(uint8_t index);

bool app_notesSettingsIsLocked(void);
bool app_notesSettingsCheckPasscode(uint8_t *digits, uint8_t nbDigits);
void app_notesSettingsSetLockAndPasscode(bool lock, uint8_t *digits, uint8_t nbDigits);
bool app_notesIsSessionUnlocked(void);
void app_notesSessionLock(void);

uint8_t app_notesGetContacts(Contact_t contactsArray[NB_MAX_CONTACTS]);
int     app_notesAddContact(const char *name, const char *address);
int     app_notesModifyContact(uint8_t index, const char *name, const char *address);
int     app_notesDeleteContact(uint8_t index);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* APP_NOTES_H */
