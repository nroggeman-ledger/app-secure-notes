
/**
 * @file app_notes_share.c
 * @brief Page to display the list of contacts and share a note
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
    NAV_TOKEN,
    ADD_CONTACT_TOKEN,
    CANCEL_TOKEN,
    BAR_TOUCHED_TOKEN,
};

// if no nav
#define CONTENT_AREA_HEIGHT (SCREEN_HEIGHT - TOUCHABLE_HEADER_BAR_HEIGHT)

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    uint8_t         nbUsedContacts;
    Contact_t       contacts[NB_MAX_CONTACTS];
    Note_t         *note;
    uint8_t         currentPage;
    uint8_t         nbPages;
    uint8_t         firstContactIndexInPage;
    uint8_t         selectedContactIndex;
    Note_t          receivedNote;
    nbgl_callback_t onBack;
} ShareContext_t;

/**********************
 *  STATIC VARIABLES
 **********************/
static ShareContext_t context;
static nbgl_layout_t *layoutContext;

/**********************
 *      VARIABLES
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void layoutTouchCallback(int token, uint8_t index);
static void buildScreen(void);

static uint8_t getNbNotesInPage(uint8_t nbNotes, uint16_t maxHeight)
{
    uint8_t  nbNotesInPage = 0;
    uint16_t currentHeight = 0;

    while (nbNotesInPage < nbNotes) {
        // value height
        currentHeight += TOUCHABLE_BAR_HEIGHT;
        if (currentHeight >= maxHeight) {
            break;
        }
        nbNotesInPage++;
    }
    return nbNotesInPage;
}

static uint8_t getNbPagesTotal(uint8_t nbTotalNotes)
{
    uint8_t nbPages          = 0;
    uint8_t nbRemainingNotes = nbTotalNotes;
    uint8_t nbNotesInPage;

    if (nbTotalNotes == 0) {
        return 1;
    }
    while (nbRemainingNotes > 0) {
        nbNotesInPage = getNbNotesInPage(nbRemainingNotes, CONTENT_AREA_HEIGHT);
        // if we already know that there are more than 1 page, or it at the first page
        // all notes cannot be displayed, it mean that there will be a nav bar
        if ((nbPages > 0) || (nbRemainingNotes > nbNotesInPage)) {
            nbNotesInPage
                = getNbNotesInPage(nbRemainingNotes, CONTENT_AREA_HEIGHT - SIMPLE_FOOTER_HEIGHT);
        }
        nbRemainingNotes -= nbNotesInPage;
        nbPages++;
    }
    return nbPages;
}

// gets the number of notes and the index of the first note fitting in the given page
static uint8_t getContactsForPage(uint8_t  nbTotalNotes,
                                  uint8_t  page,
                                  uint16_t maxHeight,
                                  uint8_t *firstParagraphIndexInPage)
{
    uint8_t nbPages          = 0;
    uint8_t nbRemainingNotes = nbTotalNotes;
    uint8_t nbNotesInPage;
    uint8_t i = 0;
    ;

    while (i < nbTotalNotes) {
        // upper margin
        nbNotesInPage = getNbNotesInPage(nbRemainingNotes, maxHeight);
        if (nbPages == page) {
            *firstParagraphIndexInPage = i;
            return nbNotesInPage;
        }
        i += nbNotesInPage;
        nbRemainingNotes -= nbNotesInPage;
        nbPages++;
    }
    return nbPages;
}

// gets the number of notes and the index of the first note fitting in the given page
static uint8_t getPageForContactIndex(uint8_t nbTotalNotes, uint8_t noteIndex, uint16_t maxHeight)
{
    uint8_t nbPages          = 0;
    uint8_t nbRemainingNotes = nbTotalNotes;
    uint8_t nbNotesInPage;
    uint8_t i = 0;
    ;

    while (i < nbTotalNotes) {
        // upper margin
        nbNotesInPage = getNbNotesInPage(nbRemainingNotes, maxHeight);
        i += nbNotesInPage;
        if (i >= noteIndex) {
            return nbPages;
        }
        nbRemainingNotes -= nbNotesInPage;
        nbPages++;
    }
    return nbPages;
}

static void onBackOnShare(void)
{
    app_notesShare(context.onBack, context.note);
}

static void onNoteReceptionChoice(bool confirm)
{
    if (confirm) {
        int status;
        // save note without content
        status = app_notesAddNote(context.receivedNote.title, context.receivedNote.content);
        if (status >= 0) {
            ui_menu_main();
        }
        else {
            // impossible to add, probably full
            // answer APDU here
            nbgl_useCaseStatus("Impossible to add Note", false, ui_menu_main);
        }
    }
    else {
        ui_menu_main();
    }
}

static void displayWaitingScreen(void)
{
    // display a screen to invite user to use phone of computer to add address
    nbgl_layoutDescription_t layoutDescription
        = {.modal = false, .withLeftBorder = false, .onActionCallback = &layoutTouchCallback};
    nbgl_layoutCenteredInfo_t centeredInfo
        = {.text1   = "Continue on your phone or computer",
           .text2   = tmpString,
           .text3   = NULL,
           .style   = LARGE_CASE_INFO,
           .icon    = &C_Phone_64px,
           .offsetY = 0};
    nbgl_layoutFooter_t footerDesc = {.type              = FOOTER_SIMPLE_TEXT,
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
             "Use Ledger Live to share this Note with %s.",
             currentContact.name);
    nbgl_layoutAddCenteredInfo(layoutContext, &centeredInfo);
    nbgl_layoutAddExtendedFooter(layoutContext, &footerDesc);

    nbgl_layoutDraw(layoutContext);
}

static void layoutTouchCallback(int token, uint8_t index)
{
    if (token == BACK_BUTTON_TOKEN) {
        context.selectedContactIndex = 0;
        ui_menu_main();
    }
    else if (token == NAV_TOKEN) {
        context.currentPage = index;
        buildScreen();
    }
    else if (token == ADD_CONTACT_TOKEN) {
        context.selectedContactIndex = context.nbUsedContacts;
        app_notesNewContact(onBackOnShare, &currentContact);
    }
    else if (token == CANCEL_TOKEN) {
        onBackOnShare();
    }
    else if (token >= BAR_TOUCHED_TOKEN) {
        context.selectedContactIndex = context.firstContactIndexInPage + token - BAR_TOUCHED_TOKEN;
        currentContact.index         = context.contacts[context.selectedContactIndex].index;
        strcpy(currentContact.name, context.contacts[context.selectedContactIndex].name);
        strcpy(currentContact.address, context.contacts[context.selectedContactIndex].address);
        displayWaitingScreen();
    }
}

static void buildScreen(void)
{
    nbgl_layoutDescription_t layoutDescription = {.modal                 = false,
                                                  .withLeftBorder        = true,
                                                  .onActionCallback      = &layoutTouchCallback,
                                                  .ticker.tickerCallback = NULL,
                                                  .tapActionText         = NULL};
    nbgl_layoutHeader_t      headerDesc        = {.type           = HEADER_EXTENDED_BACK,
                                                  .separationLine = true,
                                                  .extendedBack.backToken = BACK_BUTTON_TOKEN,
                                                  .extendedBack.tuneId    = TUNE_TAP_CASUAL,
                                                  .extendedBack.text = (char *) "Choose receiver",
#ifdef TARGET_STAX
                                      .extendedBack.actionIcon = &C_Plus_32px,
#else   // TARGET_STAX
                                      .extendedBack.actionIcon = &C_Plus_40px,
#endif  // TARGET_STAX
                                      .extendedBack.textToken = NBGL_INVALID_TOKEN};
    nbgl_layoutBar_t barLayout = {
        .centered  = false,
        .iconLeft  = NULL,
        .iconRight = &PUSH_ICON,
        .inactive  = false,
        .large     = false,
        .subText   = false,
        .tuneId    = NBGL_NO_TUNE,
    };

    layoutContext = nbgl_layoutGet(&layoutDescription);
    if (context.nbUsedContacts < NB_MAX_CONTACTS) {
        headerDesc.extendedBack.actionToken = ADD_CONTACT_TOKEN;
    }
    else {
        // if max number of notes is reached, deactivate it (grayed-out)
        headerDesc.extendedBack.actionToken = NBGL_INVALID_TOKEN;
    }
    nbgl_layoutAddHeader(layoutContext, &headerDesc);

    if (context.nbPages > 1) {
        nbgl_layoutNavigationBar_t navInfo = {.activePage         = context.currentPage,
                                              .nbPages            = context.nbPages,
                                              .token              = NAV_TOKEN,
                                              .tuneId             = NBGL_NO_TUNE,
                                              .withBackKey        = true,
                                              .withExitKey        = false,
                                              .withSeparationLine = true};
        nbgl_layoutAddNavigationBar(layoutContext, &navInfo);
    }
    // if content is not empty, display it as a list of touchable bars
    if (context.nbUsedContacts) {
        uint16_t maxHeight = CONTENT_AREA_HEIGHT;
        if (context.nbPages > 1) {
            maxHeight -= SIMPLE_FOOTER_HEIGHT;
        }
        uint8_t nbNotesInPage = getContactsForPage(context.nbUsedContacts,
                                                   context.currentPage,
                                                   maxHeight,
                                                   &context.firstContactIndexInPage);
        for (uint8_t i = 0; i < nbNotesInPage; i++) {
            barLayout.text  = context.contacts[context.firstContactIndexInPage + i].name;
            barLayout.token = BAR_TOUCHED_TOKEN + i;
            nbgl_layoutAddTouchableBar(layoutContext, &barLayout);
            nbgl_layoutAddSeparationLine(layoutContext);
        }
    }

    nbgl_layoutDraw(layoutContext);

    nbgl_refresh();
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * @brief Page to list the contacts to share a note
 *
 */
void app_notesShare(nbgl_callback_t onBack, Note_t *note)
{
    context.onBack         = onBack;
    context.note           = note;
    context.nbUsedContacts = app_notesGetContacts(context.contacts);
    // compute number of pages
    context.nbPages = getNbPagesTotal(context.nbUsedContacts);
    if (context.nbUsedContacts) {
        uint16_t maxHeight = CONTENT_AREA_HEIGHT;
        if (context.nbPages > 1) {
            maxHeight -= SIMPLE_FOOTER_HEIGHT;
        }
        context.currentPage = getPageForContactIndex(
            context.nbUsedContacts, context.selectedContactIndex, maxHeight);
    }
    buildScreen();
}

/**
 * @brief Function when receiving APDU for sharing emission
 *
 */
Note_t *app_notesGetSharedNote(void)
{
    if (context.note == NULL) {
        return NULL;
    }
    snprintf(tmpString,
             sizeof(tmpString),
             "Note sent\nNext, %s has to accept it.",
             currentContact.name);
    // display status
    nbgl_useCaseStatus(tmpString, true, app_notesList);
    return context.note;
}

/**
 * @brief Function when receiving APDU for sharing in reception
 *
 */
int app_notesReceiveSharedNote(const char *title, const char *content)
{
    if (title == NULL) {
        return -1;
    }
    if (content == NULL) {
        return -1;
    }
    context.receivedNote.title   = (char *) title;
    context.receivedNote.content = (char *) content;
    // display status
    nbgl_useCaseChoice(&C_Download_64px,
                       "Add shared Note?",
                       title,
                       "Add Note",
                       "Reject Note",
                       onNoteReceptionChoice);
    return 0;
}