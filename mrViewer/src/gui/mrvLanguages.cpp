#include <FL/fl_ask.H>

#include <gui/mrvLanguages.h>
#include <mrvPreferencesUI.h>


#ifdef _WIN32
#  define execv _execv
#endif

namespace {
const char* kModule = "lang";
}


const char* kLanguages[16] = {
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
    "zh.UTF-8",
    "sv.UTF-8",
    "gr.UTF-8",
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
            setenv( "LC_CTYPE", "UTF-8", 1 );
            setenv( "LANGUAGE", language, 1 );

            char buf[128];
            // We change the system language environment variable so that
            // the next time we start we start with the same language.
            // Saving the setting in the preferences is not enough on Windows.
            sprintf( buf, "setx LANGUAGE %s", language );

            STARTUPINFO si;
            PROCESS_INFORMATION pi;

            ZeroMemory( &si, sizeof(si) );
            si.cb = sizeof(si);
            ZeroMemory( &pi, sizeof(pi) );

            // Start the child process.
            if( !CreateProcess( NULL,   // No module name (use command line)
                                buf,    // Command line
                                NULL,   // Process handle not inheritable
                                NULL,   // Thread handle not inheritable
                                FALSE,  // Set handle inheritance to FALSE
                                CREATE_NO_WINDOW,  // No console window
                                NULL,   // Use parent's environment block
                                NULL,   // Use parent's starting directory
                                &si,    // Pointer to STARTUPINFO structure
                                &pi )   // Pointer to PROCESS_INFORMATION struct
            )
            {
                LOG_ERROR( "CreateProcess failed" );
                return;
            }

            // Wait until child process exits.
            WaitForSingleObject( pi.hProcess, INFINITE );

            // Close process and thread handles.
            CloseHandle( pi.hProcess );
            CloseHandle( pi.hThread );

#else
            setenv( "LANGUAGE", language, 1 );
#endif

            Fl_Preferences base( mrv::prefspath().c_str(), "filmaura",
                                 "mrViewer" );

            // Save ui preferences
            Fl_Preferences ui( base, "ui" );
            ui.set( "language", language_index );

            base.flush();

            // deleete ViewerUI
            delete mrv::Preferences::uiMain;

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
