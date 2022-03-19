#include <libintl.h>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>

#include <gui/mrvLanguages.h>
#include <mrvPreferencesUI.h>
#include <mrViewer.h>

void redraw_all_windows(ViewerUI* ui)
{
    delete ui;
    ui = new ViewerUI;
}

const char* kLanguages[] = {
    "cs",
    "de",
    "C",
    "es",
    "fr",
    "it",
    "ja",
    "ko",
    "pl",
    "pt",
    "ro",
    "ru",
    "tr",
    "zh"
};


void change_language( int i, ViewerUI* ui )
{
    const char* language = kLanguages[i];

    std::cerr << "change to language " << language << std::endl;
    setenv( "LC_MESSAGES", language, 1 );
    setenv( "LC_NUMERIC", language, 1 );
    setlocale( LC_MESSAGES, language );
    setlocale( LC_NUMERIC, language );

    redraw_all_windows(ui);
}
