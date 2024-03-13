
/**
 * @file app_notes_utils.c
 * @brief non-UI functions to manages notes (get/add) in Note App
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <string.h>
#include "nbgl_debug.h"
#include "nbgl_use_case.h"
#include "app_notes.h"
#include "nvram_struct.h"
#include "os_nvm.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static char workingTitle[NOTE_TITLE_MAX_LEN];
static char workingContent[NOTE_CONTENT_MAX_LEN];
static bool isUnlocked = false;

/**********************
 *      VARIABLES
 **********************/
Note_t currentNote;

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void app_notesInit(void)
{
    static Nvram_data_t storage = {0};

    // If the NVRAM content is not initialized or of a too old version, let's init it from scratch
    if (!nvram_is_initalized() || (nvram_get_struct_version() < NVRAM_FIRST_SUPPORTED_VERSION)) {
        nvram_init();
        nvm_write((void *) &N_nvram.data, (void *) &storage, sizeof(Nvram_data_t));
    }
    else if (nvram_get_struct_version() < NVRAM_STRUCT_VERSION) {
        // if the version is supported and not current, let's convert it
    }

    currentNote.title   = workingTitle;
    currentNote.content = workingContent;
}

/**
 * @brief Get the number of used Notes, and set the given array with all found used notes
 *
 * @param noteArray array of notes to be filled (if NULL, only the number of slots is retrieved)
 * @return number of Notes (number of used elements in noteArray)
 */
uint8_t app_notesGetAll(Note_t noteArray[NB_MAX_NOTES])
{
    uint8_t i;
    uint8_t nbUsedSlots = 0;
    // try to parse all slots
    for (i = 0; i < NB_MAX_NOTES; i++) {
        if (N_nvram.data.notes[i].used == true) {
            if (noteArray != NULL) {
                noteArray[nbUsedSlots].index   = i;
                noteArray[nbUsedSlots].title   = (char *) N_nvram.data.notes[i].title;
                noteArray[nbUsedSlots].content = (char *) N_nvram.data.notes[i].content;
            }
            nbUsedSlots++;
        }
    }
    return nbUsedSlots;
}

/**
 * @brief Get note Title and Content at the given index
 *
 * @param index index to the note to be retrieved
 * @param note structure to fill with info
 * @return >= 0 if OK
 */
int app_notesGetNote(uint8_t index, Note_t note)
{
    UNUSED(index);
    UNUSED(note);
    return 0;
}

/**
 * @brief Add the new note in any available slot
 *
 * @param title title to be applied (max @ref NOTE_TITLE_MAX_LEN bytes)
 * @param content content to be applied (max @ref NOTE_CONTENT_MAX_LEN bytes
 * @return index of the added note, or <0 if error
 */
int app_notesAddNote(const char *title, const char *content)
{
    uint8_t i;
    // try to find an unused slot
    for (i = 0; i < NB_MAX_NOTES; i++) {
        if (N_nvram.data.notes[i].used != true) {
            bool value = true;
            nvm_write((void *) &N_nvram.data.notes[i].used, (void *) &value, sizeof(bool));
            nvm_write((void *) &N_nvram.data.notes[i].title, (void *) title, strlen(title) + 1);
            nvm_write(
                (void *) &N_nvram.data.notes[i].content, (void *) content, strlen(content) + 1);
            return i;
        }
    }
    return -1;
}

/**
 * @brief Modify the note at the given index
 *
 * @param index index of the note to modify
 * @param title title to be applied (max @ref NOTE_TITLE_MAX_LEN bytes)
 * @param content content to be applied (max @ref NOTE_CONTENT_MAX_LEN bytes
 * @return >= 0 if OK
 */
int app_notesModifyNote(uint8_t index, const char *title, const char *content)
{
    nvm_write((void *) &N_nvram.data.notes[index].title, (void *) title, strlen(title) + 1);
    nvm_write((void *) &N_nvram.data.notes[index].content, (void *) content, strlen(content) + 1);
    return 0;
}

/**
 * @brief Delete the note at the given slot
 *
 * @param index index of the note to delete
 * @return >= 0 if OK
 */
int app_notesDeleteNote(uint8_t index)
{
    nvm_erase((void *) &N_nvram.data.notes[index], sizeof(NvramNote_t));
    return 0;
}

/**
 * @brief Check lock state in Settings in NVRAM
 *
 * @return true if data are locked
 */
bool app_notesSettingsIsLocked(void)
{
    return N_nvram.data.settings.locked;
}

/**
 * @brief Check passcode from value Settings in NVRAM
 *
 * @return true if passcode is OK
 */
bool app_notesSettingsCheckPasscode(uint8_t *digits, uint8_t nbDigits)
{
    isUnlocked = ((N_nvram.data.settings.nbDigits == nbDigits)
                  && (!memcmp((void *) N_nvram.data.settings.digits, digits, nbDigits)));
    return isUnlocked;
}

/**
 * @brief set tne lock status in Settings in NVRAM
 *
 */
void app_notesSettingsSetLockAndPasscode(bool lock, uint8_t *digits, uint8_t nbDigits)
{
    nvm_write((void *) &N_nvram.data.settings.locked, (void *) &lock, 1);
    if (lock) {
        nvm_write((void *) &N_nvram.data.settings.nbDigits, (void *) &nbDigits, 1);
        nvm_write((void *) &N_nvram.data.settings.digits, (void *) digits, nbDigits);
        isUnlocked = true;
    }
}

bool app_notesIsSessionUnlocked(void)
{
    return isUnlocked;
}

void app_notesSessionLock(void)
{
    isUnlocked = false;
}
