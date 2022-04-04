#ifndef mrvLanguages_h
#define mrvLanguages_h

#include <string>
#include "gui/mrvPopupMenu.h"

struct LanguageTable
{
    int index;
    const char* code;
};

extern LanguageTable kLanguages[17];

class PreferencesUI;
void check_language( PreferencesUI* uiPrefs, int& language_index );

void select_character( mrv::PopupMenu* w );


#endif // mrvLanguages_h
