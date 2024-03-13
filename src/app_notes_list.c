
/**
 * @file app_notes_list.c
 * @brief Page to display the list of notes
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
    ADD_NOTE_TOKEN,
    BAR_TOUCHED_TOKEN,
};

// if no nav
#define CONTENT_AREA_HEIGHT (SCREEN_HEIGHT - TOUCHABLE_HEADER_BAR_HEIGHT)

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    uint8_t nbUsedNotes;
    Note_t  noteArray[NB_MAX_NOTES];
    uint8_t currentPage;
    uint8_t nbPages;
    uint8_t firstNoteIndexInPage;
} ListContext_t;

/**********************
 *  STATIC VARIABLES
 **********************/
static ListContext_t  context;
static nbgl_layout_t *layoutContext;

/**********************
 *      VARIABLES
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void displayNoteList(void);

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
    uint8_t i = 0;

    while (i < nbTotalNotes) {
        // upper margin
        nbNotesInPage = getNbNotesInPage(nbRemainingNotes, CONTENT_AREA_HEIGHT);
        // if it is supposed to be the last page (of more than 1 page)
        // nav bar
        if ((nbPages > 0) && (nbRemainingNotes == nbNotesInPage)) {
            nbNotesInPage = getNbNotesInPage(nbRemainingNotes, CONTENT_AREA_HEIGHT - 40);
        }
        i += nbNotesInPage;
        nbRemainingNotes -= nbNotesInPage;
        nbPages++;
    }
    return nbPages;
}

// gets the number of notes and the index of the first note fitting in the given page
static uint8_t getNotesForPage(uint8_t  nbTotalNotes,
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

static void layoutTouchCallback(int token, uint8_t index)
{
    if (token == BACK_BUTTON_TOKEN) {
        ui_menu_main();
    }
    else if (token == NAV_TOKEN) {
        context.currentPage = index;
        displayNoteList();
    }
    else if (token == ADD_NOTE_TOKEN) {
        app_notesNew(app_notesList, &currentNote);
    }
    else if (token >= BAR_TOUCHED_TOKEN) {
        index             = context.firstNoteIndexInPage + token - BAR_TOUCHED_TOKEN;
        currentNote.index = context.noteArray[index].index;
        strcpy(currentNote.title, context.noteArray[index].title);
        strcpy(currentNote.content, context.noteArray[index].content);
        app_notesDisplay(app_notesList, &currentNote);
    }
}

static void displayNoteList(void)
{
    nbgl_layoutDescription_t layoutDescription = {.modal                 = false,
                                                  .withLeftBorder        = true,
                                                  .onActionCallback      = &layoutTouchCallback,
                                                  .ticker.tickerCallback = NULL,
                                                  .tapActionText         = NULL};
    nbgl_layoutHeader_t      headerDesc        = {.type           = HEADER_BACK_TEXT_AND_ACTION,
                                                  .separationLine = true,
                                                  .backTextAndAction.backToken = BACK_BUTTON_TOKEN,
                                                  .backTextAndAction.tuneId    = TUNE_TAP_CASUAL,
                                                  .backTextAndAction.text      = (char *) "My Notes",
#ifdef TARGET_STAX
                                      .backTextAndAction.actionIcon = &C_Plus_32px,
#else   // TARGET_STAX
                                      .backTextAndAction.actionIcon = &C_Plus_40px,
#endif  // TARGET_STAX
                                      .backTextAndAction.textToken   = 0xFF,
                                      .backTextAndAction.actionToken = ADD_NOTE_TOKEN};
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
    // if content is not empty, display it as paragraphs
    if (context.nbUsedNotes) {
        uint16_t maxHeight = CONTENT_AREA_HEIGHT;
        if (context.nbPages > 1) {
            maxHeight -= SIMPLE_FOOTER_HEIGHT;
        }
        uint8_t nbNotesInPage = getNotesForPage(
            context.nbUsedNotes, context.currentPage, maxHeight, &context.firstNoteIndexInPage);
        for (uint8_t i = 0; i < nbNotesInPage; i++) {
            barLayout.text  = context.noteArray[context.firstNoteIndexInPage + i].title;
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
 * @brief Page to display the list of notes
 *
 */
void app_notesList(void)
{
    context.nbUsedNotes = app_notesGetAll(context.noteArray);
    context.currentPage = 0;
    // compute number of pages
    context.nbPages = getNbPagesTotal(context.nbUsedNotes);

    displayNoteList();
}