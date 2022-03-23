#include <FL/fl_ask.H>

#include <gui/mrvLanguages.h>
#include <mrvPreferencesUI.h>

const char* kLanguages[14] = {
    "cs.UTF-8",
    "de.UTF-8",
    "C",
    "es.UTF-8",
    "fr.UTF-8",
    "it.UTF-8",
    "ja.UTF-8",
    "ko.UTF-8",
    "pl.UTF-8",
    "pt.UTF-8",
    "ro.UTF-8",
    "ru.UTF-8",
    "tr.UTF-8",
    "zh.UTF-8"
};

void check_language( PreferencesUI* uiPrefs, int& language_index )
{
    if ( uiPrefs->uiLanguage->value() != language_index )
    {
        int ok = fl_choice( _("Need to reboot mrViewer to change language.  "
                              "Are you sure you want to continue?" ),
                            _("No"),  _("Yes"), NULL, NULL );
        if ( ok )
        {
            language_index = uiPrefs->uiLanguage->value();
            const char* language = kLanguages[language_index];

#ifdef _WIN32
            char* buf = new char[64];
            sprintf( buf, "LC_ALL=%s", language );
            putenv( buf );
            buf = new char[64];
            sprintf( buf, "LANGUAGE=%s", language );
            putenv( buf );
#else
            setenv( "LANGUAGE", language, 1 );
#endif

            Fl_Preferences base( mrv::prefspath().c_str(), "filmaura",
                                 "mrViewer" );

            // Save ui preferences
            Fl_Preferences ui( base, "ui" );
            ui.set( "language", language_index );

            base.flush();



            std::string root = getenv( "MRV_ROOT" );
            root += "/bin/mrViewer";

            const char *const parmList[] = {root.c_str(), NULL};
            execv( root.c_str(), (char* const*) parmList );
        }
        else
        {
            uiPrefs->uiLanguage->value( language_index );
        }
    }

}
