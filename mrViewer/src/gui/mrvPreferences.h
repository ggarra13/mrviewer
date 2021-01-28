/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2020  Gonzalo Garramuño

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @file   mrvPreferences.h
 * @author gga
 * @date   Sun Jul  1 19:24:28 2007
 *
 * @brief  Read/Save preferences for mrViewer
 *
 *
 */

#ifndef mrvPreferences_h
#define mrvPreferences_h

#include <string>
#include <ImfChromaticities.h>
#include "mrvColorSchemes.h"
// OpenColorIO
#ifdef _WIN32
#pragma warning( disable: 4275 )
#endif
#include <OpenColorIO/OpenColorIO.h>
namespace OCIO = OCIO_NAMESPACE;

class ViewerUI;
class PreferencesUI;

namespace mrv {

class Preferences
{
public:
    enum CacheType
    {
        kCacheAsLoaded,
        kCache8bits,
        kCacheHalf,
        kCacheFloat
    };

    enum MissingFrameType
    {
        kBlackFrame = 0,
        kRepeatFrame,
        kScratchedRepeatFrame
    };


public:
    Preferences( PreferencesUI* ui );
    ~Preferences();

    static void run( ViewerUI* main );
    static void save();

    static std::string temporaryDirectory() {
        return tempDir;
    }

    static OCIO::ConstConfigRcPtr OCIOConfig()
        {
            return config;
        }

public:
    static ViewerUI* uiMain;
    static bool use_ocio;
    static OCIO::ConstConfigRcPtr config;
    static bool native_file_chooser;
    static int bgcolor;
    static int textcolor;
    static int selectioncolor;
    static int selectiontextcolor;
    static int R3dScale;
    static int BRAWScale;
    static int switching_images;

    static int64_t max_memory;

    static std::string OCIO_Display;
    static std::string OCIO_View;


    static CacheType           cache_type;

    static MissingFrameType missing_frame;
    static std::string video_threads;


    static std::string root;
    static std::string tempDir;
    static std::string hotkeys_file;
    static ColorSchemes schemes;
    static int debug;
};


} // namespace mrv


#endif // mrvPreferences_h
