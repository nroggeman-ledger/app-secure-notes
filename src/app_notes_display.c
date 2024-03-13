
/**
 * @file app_notes_display.c
 * @brief Functions to display a note in Note App
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
    TAP_ACTION_TOKEN,
    NAV_TOKEN,
    TITLE_TOUCHED_TOKEN,
    ACTION_TOKEN,
    TEXT_TOUCHED_TOKEN,
};

// if no nav
#define CONTENT_AREA_HEIGHT (SCREEN_HEIGHT - TOUCHABLE_HEADER_BAR_HEIGHT - SIMPLE_FOOTER_HEIGHT)

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    nbgl_callback_t onBack;
    Note_t         *note;
    char           *paragraphs[NB_MAX_PARAGRAPHS];
    uint8_t         nbParagraphs;
    uint8_t         firstParagraphIndexInPage;
    uint8_t         modifiedParagraphIndex;
    uint8_t         currentPage;
    uint8_t         nbPages;
    bool            smallFont;
} DisplayContext_t;

/**********************
 *  STATIC VARIABLES
 **********************/
static nbgl_layout_t   *layoutContext;
static DisplayContext_t context;
static char             tmpString[NOTE_CONTENT_MAX_LEN];

/**********************
 *      VARIABLES
 **********************/

/**********************
 *  STATIC FUNCTIONS
 **********************/
static void displayNoteContent(Note_t *note);

static uint8_t getNbParagraphsInPage(uint8_t      nbParagraphs,
                                     const char **paragraphs,
                                     uint8_t      startIndex,
                                     uint16_t     maxHeight,
                                     bool         smallFont,
                                     bool        *tooLongToFit)
{
    uint8_t  nbParagraphsInPage = 0;
    uint16_t currentHeight      = 12;  // upper margin

    *tooLongToFit = false;
    while (nbParagraphsInPage < nbParagraphs) {
        const char *paragraph;

        // margin between paragraphs
        if (nbParagraphsInPage > 0) {
            currentHeight += 28;
        }
        // fetch tag/value pair strings.
        paragraph = paragraphs[startIndex + nbParagraphsInPage];

        // value height
        currentHeight += nbgl_getTextHeightInWidth(
            smallFont ? SMALL_REGULAR_FONT : LARGE_MEDIUM_FONT, paragraph, AVAILABLE_WIDTH, true);
        if (currentHeight >= maxHeight) {
            break;
        }
        nbParagraphsInPage++;
    }
    if ((nbParagraphsInPage == 0) && (currentHeight >= maxHeight)) {
        *tooLongToFit      = true;
        nbParagraphsInPage = 1;
    }
    return nbParagraphsInPage;
}

static uint8_t getNbPagesForArray(uint8_t      nbTotalParagraphs,
                                  const char **paragraphs,
                                  bool         smallFont)
{
    uint8_t nbPages               = 0;
    uint8_t nbRemainingParagraphs = nbTotalParagraphs;
    uint8_t nbParagraphsInPage;
    uint8_t i = 0;
    bool    tooLongToFit;

    while (i < nbTotalParagraphs) {
        // upper margin
        nbParagraphsInPage = getNbParagraphsInPage(
            nbRemainingParagraphs, paragraphs, i, CONTENT_AREA_HEIGHT, smallFont, &tooLongToFit);
        // if it is supposed to be the last page (of more than 1 page), let's try again with "Tep to
        // enter" in addition of nav bar
        if ((nbPages > 0) && (nbRemainingParagraphs == nbParagraphsInPage)) {
            nbParagraphsInPage = getNbParagraphsInPage(nbRemainingParagraphs,
                                                       paragraphs,
                                                       i,
                                                       CONTENT_AREA_HEIGHT - SIMPLE_FOOTER_HEIGHT,
                                                       smallFont,
                                                       &tooLongToFit);
        }
        i += nbParagraphsInPage;
        nbRemainingParagraphs -= nbParagraphsInPage;
        nbPages++;
    }
    return nbPages;
}

// gets the number of paragraphs and the index of the first paragraph fitting in the given page
static uint8_t getParagraphsForPage(uint8_t      nbTotalParagraphs,
                                    const char **paragraphs,
                                    uint8_t      page,
                                    bool         smallFont,
                                    uint8_t     *firstParagraphIndexInPage)
{
    uint8_t nbPages               = 0;
    uint8_t nbRemainingParagraphs = nbTotalParagraphs;
    uint8_t nbParagraphsInPage;
    uint8_t i = 0;
    bool    tooLongToFit;

    while (i < nbTotalParagraphs) {
        nbParagraphsInPage = getNbParagraphsInPage(
            nbRemainingParagraphs, paragraphs, i, CONTENT_AREA_HEIGHT, smallFont, &tooLongToFit);
        // if it is supposed to be the last page (of more than 1 page), let's try again with "Tep to
        // enter" in addition of nav bar
        if ((nbPages > 0) && (nbRemainingParagraphs == nbParagraphsInPage)) {
            nbParagraphsInPage = getNbParagraphsInPage(nbRemainingParagraphs,
                                                       paragraphs,
                                                       i,
                                                       CONTENT_AREA_HEIGHT - SIMPLE_FOOTER_HEIGHT,
                                                       smallFont,
                                                       &tooLongToFit);
        }
        if (nbPages == page) {
            *firstParagraphIndexInPage = i;
            return nbParagraphsInPage;
        }
        i += nbParagraphsInPage;
        nbRemainingParagraphs -= nbParagraphsInPage;
        nbPages++;
    }
    return nbPages;
}

// gets the page containing the given paragraph index
static uint8_t getPageForParagraphs(uint8_t      nbTotalParagraphs,
                                    const char **paragraphs,
                                    uint8_t      paragraphIndex,
                                    uint16_t     maxHeight,
                                    bool         smallFont)
{
    uint8_t nbPages               = 0;
    uint8_t nbRemainingParagraphs = nbTotalParagraphs;
    uint8_t nbParagraphsInPage;
    uint8_t i = 0;
    bool    tooLongToFit;

    while (i < nbTotalParagraphs) {
        // upper margin
        nbParagraphsInPage = getNbParagraphsInPage(
            nbRemainingParagraphs, paragraphs, i, maxHeight, smallFont, &tooLongToFit);
        i += nbParagraphsInPage;
        if (i >= paragraphIndex) {
            return nbPages;
        }
        nbRemainingParagraphs -= nbParagraphsInPage;
        nbPages++;
    }
    return nbPages;
}

// convert (in place) flat (with \n) representation to the paragraph representation
static void content2paragraphs(Note_t *note)
{
    char *currentChar = note->content;

    // if empty, return as is
    if (*currentChar == '\0') {
        context.nbParagraphs = 0;
        return;
    }
    context.paragraphs[0] = currentChar;
    context.nbParagraphs  = 1;
    // split on '\n', replacing by '\0'
    while (*currentChar) {
        if (*currentChar == '\n') {
            *currentChar                             = '\0';
            context.paragraphs[context.nbParagraphs] = currentChar + 1;
            context.nbParagraphs++;
        }
        currentChar++;
    }
}

// convert (in place) the paragraph representation to a flat (with \n) representation (the one
// actually saved)
static void paragraphs2content(void)
{
    uint8_t i;
    // replace all '\0' at the end of paragraphs, except the last one
    for (i = 1; i < context.nbParagraphs; i++) {
        *(char *) (context.paragraphs[i] - 1) = '\n';
    }
}

// called when a new paragraph is added
static void onNewParagraphConfirmed(void)
{
    // tmpString is used as a temporary buffer
    // it needs to be added as last paragraph
    if (context.nbParagraphs > 0) {
        context.paragraphs[context.nbParagraphs]
            = context.paragraphs[context.nbParagraphs - 1]
              + strlen(context.paragraphs[context.nbParagraphs - 1]) + 1;
    }
    else {
        context.paragraphs[context.nbParagraphs] = context.note->content;
    }
    strcpy(context.paragraphs[context.nbParagraphs], tmpString);
    context.nbParagraphs++;
    paragraphs2content();
    // save this note
    app_notesModifyNote(context.note->index, context.note->title, context.note->content);
    app_notesDisplay(context.onBack, context.note);
}

// called when a paragraph is modified
static void onParagraphModified(void)
{
    size_t  newLen     = strlen(tmpString);
    size_t  currentLen = strlen(context.paragraphs[context.modifiedParagraphIndex]);
    uint8_t i;

    if (newLen == 0) {
        // remove paragraph by shifting all paragraphs after it
        for (i = context.modifiedParagraphIndex; i < (context.nbParagraphs - 1); i++) {
            memmove(context.paragraphs[i],
                    context.paragraphs[i + 1],
                    strlen(context.paragraphs[i + 1]) + 1);
        }
        context.paragraphs[context.nbParagraphs - 1] = NULL;
        context.nbParagraphs--;
    }
    else if (newLen != currentLen) {
        size_t diff = newLen - currentLen;
        // all paragraphs after modified one need to me moved to avoid being overwritten, if the
        // length has changed
        if (context.modifiedParagraphIndex < (context.nbParagraphs - 1)) {
            memmove(context.paragraphs[context.modifiedParagraphIndex] + newLen + 1,
                    context.paragraphs[context.modifiedParagraphIndex + 1],
                    &context.paragraphs[context.nbParagraphs - 1]
                                       [strlen(context.paragraphs[context.nbParagraphs - 1]) + 1]
                        - context.paragraphs[context.modifiedParagraphIndex + 1]);
            for (i = context.modifiedParagraphIndex + 1; i < context.nbParagraphs; i++) {
                context.paragraphs[i] += diff;
            }
        }
        strcpy(context.paragraphs[context.modifiedParagraphIndex], tmpString);
    }
    else {
        // same len, sut replace bytes
        strcpy(context.paragraphs[context.modifiedParagraphIndex], tmpString);
    }
    paragraphs2content();
    // save modified
    app_notesModifyNote(context.note->index, context.note->title, context.note->content);
    app_notesDisplay(context.onBack, context.note);
}

// called when the title is modified
static void onTitleModified(void)
{
    strcpy(context.note->title, tmpString);
    paragraphs2content();
    // save modified
    app_notesModifyNote(context.note->index, context.note->title, context.note->content);
    app_notesDisplay(context.onBack, context.note);
}

static void backFromDisplay(void)
{
    paragraphs2content();
    app_notesDisplay(context.onBack, context.note);
}

static void layoutTouchCallback(int token, uint8_t index)
{
    if (token == BACK_BUTTON_TOKEN) {
        context.modifiedParagraphIndex = 0;
        context.onBack();
    }
    else if (token == NAV_TOKEN) {
        context.currentPage = index;
        displayNoteContent(context.note);
    }
    else if (token == TAP_ACTION_TOKEN) {
        if (context.nbParagraphs < NB_MAX_PARAGRAPHS) {
            strcpy(tmpString, "");
            context.modifiedParagraphIndex = context.nbParagraphs;
            app_notesEditText(
                backFromDisplay, onNewParagraphConfirmed, "New paragraph", "Confirm", tmpString);
        }
    }
    else if (token == TITLE_TOUCHED_TOKEN) {
        strcpy(tmpString, context.note->title);
        app_notesEditText(backFromDisplay, onTitleModified, "Change title", "Confirm", tmpString);
    }
    else if (token == ACTION_TOKEN) {
        // launch a new page to act on this note
        app_notesActionOnNote(backFromDisplay, context.note);
    }
    else {
        context.modifiedParagraphIndex
            = context.firstParagraphIndexInPage + (token - TEXT_TOUCHED_TOKEN);
        strcpy(tmpString, context.paragraphs[context.modifiedParagraphIndex]);
        app_notesEditText(
            backFromDisplay, onParagraphModified, "New paragraph", "Confirm", tmpString);
    }
}

static void displayNoteContent(Note_t *note)
{
    nbgl_layoutDescription_t layoutDescription = {.modal                 = false,
                                                  .withLeftBorder        = true,
                                                  .onActionCallback      = &layoutTouchCallback,
                                                  .ticker.tickerCallback = NULL,
                                                  .tapActionToken        = TAP_ACTION_TOKEN};
    nbgl_layoutHeader_t      headerDesc        = {.type           = HEADER_BACK_TEXT_AND_ACTION,
                                                  .separationLine = true,
                                                  .backTextAndAction.backToken = BACK_BUTTON_TOKEN,
                                                  .backTextAndAction.tuneId    = TUNE_TAP_CASUAL,
                                                  .backTextAndAction.text      = (char *) note->title,
#ifdef TARGET_STAX
                                      .backTextAndAction.actionIcon = &C_Dots_32px,
#else   // TARGET_STAX
                                      .backTextAndAction.actionIcon = &C_Dots_40px,
#endif  // TARGET_STAX
                                      .backTextAndAction.actionToken = ACTION_TOKEN,
                                      .backTextAndAction.textToken   = TITLE_TOUCHED_TOKEN};
    // do not enable to add new paragraph if not at the last page (or single page)
    if ((context.nbPages > 1) && (context.currentPage < (context.nbPages - 1))) {
        layoutDescription.tapActionText = NULL;
    }
    else {
        if (context.nbParagraphs < NB_MAX_PARAGRAPHS) {
            layoutDescription.tapActionText = "Tap anywhere to edit";
        }
        else {
            layoutDescription.tapActionText = "No more space";
        }
    }

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
    if (context.nbParagraphs) {
        uint8_t nbParagraphInPage      = getParagraphsForPage(context.nbParagraphs,
                                                         (const char **) context.paragraphs,
                                                         context.currentPage,
                                                         context.smallFont,
                                                         &context.firstParagraphIndexInPage);
        context.modifiedParagraphIndex = context.firstParagraphIndexInPage;
        for (uint8_t i = 0; i < nbParagraphInPage; i++) {
            nbgl_layoutAddTouchableText(layoutContext,
                                        context.paragraphs[context.firstParagraphIndexInPage + i],
                                        TEXT_TOUCHED_TOKEN + i,
                                        10,
                                        context.smallFont,
                                        NBGL_NO_TUNE);
        }
    }

    nbgl_layoutDraw(layoutContext);

    nbgl_refresh();
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * @brief display a note in Note App
 *
 */
void app_notesDisplay(nbgl_callback_t onBack, Note_t *note)
{
    // save context
    context.onBack = onBack;
    context.note   = note;

    context.smallFont = (strlen(context.note->content) > 50);

    content2paragraphs(note);
    if (context.nbParagraphs) {
        context.nbPages = getNbPagesForArray(
            context.nbParagraphs, (const char **) context.paragraphs, context.smallFont);
        context.modifiedParagraphIndex
            = MIN(context.modifiedParagraphIndex, (context.nbParagraphs - 1));
        // go to proper page
        context.currentPage = getPageForParagraphs(context.nbParagraphs,
                                                   (const char **) context.paragraphs,
                                                   context.modifiedParagraphIndex,
                                                   CONTENT_AREA_HEIGHT,
                                                   context.smallFont);
    }

    displayNoteContent(note);
}
