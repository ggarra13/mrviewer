#ifndef mrvLanguages_h
#define mrvLanguages_h

#include <string>
#include "gui/mrvPopupMenu.h"

struct LanguageTable
{
    int index;
    const char* code;
};

extern LanguageTable kLanguages[18];

class PreferencesUI;
void check_language( PreferencesUI* uiPrefs, int& language_index );

char* select_character( const char* p, bool colon = false );
void select_character( mrv::PopupMenu* w , bool colon = false);


#endif // mrvLanguages_h
