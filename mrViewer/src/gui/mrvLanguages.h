#ifndef mrvLanguages_h
#define mrvLanguages_h


struct LanguageTable
{
    int index;
    const char* code;
};

extern LanguageTable kLanguages[17];

class PreferencesUI;
void check_language( PreferencesUI* uiPrefs, int& language_index );


#endif // mrvLanguages_h
