
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
static char workingName[CONTACT_NAME_LEN];
static char workingAddress[CONTACT_ADDRESS_MAX_LEN];
static bool isUnlocked = false;

/**********************
 *      VARIABLES
 **********************/
Note_t    currentNote;
Contact_t currentContact;

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

    currentNote.title      = workingTitle;
    currentNote.content    = workingContent;
    currentContact.name    = workingName;
    currentContact.address = workingAddress;
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
        if (N_nvram.data.usedNotes & (1 << i)) {
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
int app_notesGetNote(uint8_t index, Note_t *note)
{
    if (N_nvram.data.usedNotes & (1 << index)) {
        note->index   = index;
        note->title   = (char *) N_nvram.data.notes[index].title;
        note->content = (char *) N_nvram.data.notes[index].content;
        return 0;
    }
    return -1;
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
        if ((N_nvram.data.usedNotes & (1 << i)) == 0) {
            uint32_t mask = N_nvram.data.usedNotes | (1 << i);
            nvm_write((void *) &N_nvram.data.usedNotes, (void *) &mask, sizeof(uint32_t));
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
    uint32_t mask = N_nvram.data.usedNotes & ~(1 << index);
    nvm_write((void *) &N_nvram.data.usedNotes, (void *) &mask, sizeof(uint32_t));
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

/**
 * @brief Get the number of used addresses, and set the given array with all found used addresses
 *
 * @param contactsArray array of addresses to be filled (if NULL, only the number of slots is
 * retrieved)
 * @return number of Notes (number of used elements in noteArray)
 */
uint8_t app_notesGetContacts(Contact_t contactsArray[NB_MAX_CONTACTS])
{
    uint8_t i;
    uint8_t nbUsedSlots = 0;
    // try to parse all slots
    for (i = 0; i < NB_MAX_CONTACTS; i++) {
        if (N_nvram.data.usedContacts & (1 << i)) {
            if (contactsArray != NULL) {
                contactsArray[nbUsedSlots].index   = i;
                contactsArray[nbUsedSlots].name    = (char *) N_nvram.data.contacts[i].name;
                contactsArray[nbUsedSlots].address = (char *) N_nvram.data.contacts[i].address;
            }
            nbUsedSlots++;
        }
    }
    return nbUsedSlots;
}

/**
 * @brief Add the new address in any available slot
 *
 * @param name name to be applied (max @ref NOTE_TITLE_MAX_LEN bytes)
 * @param address address to be applied (max @ref NOTE_CONTENT_MAX_LEN bytes
 * @return index of the added address, or <0 if error
 */
int app_notesAddContact(const char *name, const char *address)
{
    uint8_t i;
    // try to find an unused slot
    for (i = 0; i < NB_MAX_NOTES; i++) {
        if ((N_nvram.data.usedContacts & (1 << i)) == 0) {
            uint32_t mask = N_nvram.data.usedContacts | (1 << i);
            nvm_write((void *) &N_nvram.data.usedContacts, (void *) &mask, sizeof(uint32_t));
            nvm_write((void *) &N_nvram.data.contacts[i].name, (void *) name, strlen(name) + 1);
            nvm_write(
                (void *) &N_nvram.data.contacts[i].address, (void *) address, strlen(address) + 1);
            return i;
        }
    }
    return -1;
}

/**
 * @brief Modify the contact at the given index
 *
 * @param index index of the contact to modify
 * @param name name to be applied (max @ref ADDRESS_NAME_MAX_LEN bytes)
 * @param address address to be applied (max @ref NOTE_CONTENT_MAX_LEN bytes
 * @return >= 0 if OK
 */
int app_notesModifyContact(uint8_t index, const char *name, const char *address)
{
    nvm_write((void *) &N_nvram.data.contacts[index].name, (void *) name, strlen(name) + 1);
    nvm_write(
        (void *) &N_nvram.data.contacts[index].address, (void *) address, strlen(address) + 1);
    return 0;
}

/**
 * @brief Delete the address at the given slot
 *
 * @param index index of the address to delete
 * @return >= 0 if OK
 */
int app_notesDeleteContact(uint8_t index)
{
    uint32_t mask = N_nvram.data.usedContacts & ~(1 << index);
    nvm_write((void *) &N_nvram.data.usedContacts, (void *) &mask, sizeof(uint32_t));
    return 0;
}
