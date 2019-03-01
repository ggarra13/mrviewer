/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo GarramuÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂ±o

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

#ifndef mrvHotkey_h
#define mrvHotkey_h

#include <string>
#include <iostream>
#include <gui/mrvBrowser.h>

class HotkeyUI;

namespace mrv {

struct Hotkey
{
    Hotkey() :
        ctrl( false ),
        meta( false ),
        alt( false ),
        shift( false ),
        key(0),
        text(""),
        key2(0)
    {
    }

    Hotkey( const bool c, const bool m,
            const bool a, const bool s,
            const unsigned k, std::string t = "", const unsigned k2=0 ) :
        ctrl( c ),
        meta( m ),
        alt( a ),
        shift( s ),
        key( k ),
        text( t ),
        key2( k2 )
    {
    };

    Hotkey( const Hotkey& h ) :
        ctrl( h.ctrl ),
        meta( h.meta ),
        alt( h.alt ),
        shift( h.shift ),
        key( h.key ),
        text( h.text ),
        key2( h.key2 )
    {
    };

    bool match( unsigned rawkey );


    unsigned hotkey()
    {
        unsigned r = 0;
        if ( ctrl ) r += FL_CTRL;
        if ( shift ) r += FL_SHIFT;
        if ( meta ) r += FL_META;
        if ( alt ) r += FL_ALT;
        r += key;
        return r;
    }

public:
    bool ctrl;
    bool meta;
    bool alt;
    bool shift;
    unsigned key;
    std::string text;
    unsigned key2;
};

extern Hotkey kOpenDirectory;
extern Hotkey kOpenImage;
extern Hotkey kOpenSingleImage;
extern Hotkey kOpenStereoImage;
extern Hotkey kOpenClipXMLMetadata;
extern Hotkey kSaveReel;
extern Hotkey kSaveImage;
extern Hotkey kSaveSnapshot;
extern Hotkey kSaveSequence;
extern Hotkey kSaveClipXMLMetadata;
extern Hotkey kIccProfile;
extern Hotkey kIDTScript;
extern Hotkey kLookModScript;
extern Hotkey kCTLScript;
extern Hotkey kMonitorCTLScript;
extern Hotkey kMonitorIccProfile;

extern Hotkey kZoomMin;
extern Hotkey kZoomMax;

extern Hotkey kZoomIn;
extern Hotkey kZoomOut;
extern Hotkey kFullScreen;
extern Hotkey kCenterImage;
extern Hotkey kFitScreen;
extern Hotkey kSafeAreas;
extern Hotkey kDisplayWindow;
extern Hotkey kDataWindow;
extern Hotkey kWipe;
extern Hotkey kFlipX;
extern Hotkey kFlipY;

extern Hotkey kFrameStepBack;
extern Hotkey kFrameStepFPSBack;
extern Hotkey kFrameStepFwd;
extern Hotkey kFrameStepFPSFwd;
extern Hotkey kPlayBack;
extern Hotkey kPlayBackTwiceSpeed;
extern Hotkey kPlayBackHalfSpeed;
extern Hotkey kPlayFwd;
extern Hotkey kPlayFwdTwiceSpeed;
extern Hotkey kPlayFwdHalfSpeed;
extern Hotkey kStop;

extern Hotkey kPreviousVersionImage;
extern Hotkey kNextVersionImage;

extern Hotkey kPreviousImage;
extern Hotkey kNextImage;

extern Hotkey kSwitchChannels;
extern Hotkey kPreviousChannel;
extern Hotkey kNextChannel;

extern Hotkey kFirstFrame;
extern Hotkey kLastFrame;
extern Hotkey kToggleBG;

extern Hotkey kToggleTopBar;
extern Hotkey kTogglePixelBar;
extern Hotkey kToggleTimeline;
extern Hotkey kTogglePresentation;


extern Hotkey kZDepthUp;
extern Hotkey kZDepthDown;

extern Hotkey kDensityUp;
extern Hotkey kDensityDown;

extern Hotkey kDrawMode;
extern Hotkey kEraseMode;
extern Hotkey kScrubMode;
extern Hotkey kTextMode;
extern Hotkey kAreaMode;
extern Hotkey kMoveSizeMode;

extern Hotkey kPenSizeMore;
extern Hotkey kPenSizeLess;

extern Hotkey kExposureMore;
extern Hotkey kExposureLess;
extern Hotkey kGammaMore;
extern Hotkey kGammaLess;

extern Hotkey kSwitchFGBG;
extern Hotkey kSetAsBG;
extern Hotkey kToggleLUT;

extern Hotkey kAddIPTCMetadata;
extern Hotkey kRemoveIPTCMetadata;

extern Hotkey kSOPSatNodes;

extern Hotkey kAttachAudio;
extern Hotkey kEditAudio;
extern Hotkey kDetachAudio;

extern Hotkey kCopyRGBAValues;
extern Hotkey kCloneImage;

extern Hotkey kPreloadCache;
extern Hotkey kClearCache;
extern Hotkey kClearSingleFrameCache;

extern Hotkey kSetInPoint;
extern Hotkey kSetOutPoint;

extern Hotkey kHudToggle;

// OCIO Hotkets
extern Hotkey kOCIOInputColorSpace;
extern Hotkey kOCIODisplay;
extern Hotkey kOCIOView;

// Windows hotkeys
extern Hotkey kToggleReel;
extern Hotkey kToggleMediaInfo;
extern Hotkey kToggleColorInfo;
extern Hotkey kToggleAction;
extern Hotkey kToggleStereoOptions;
extern Hotkey kToggleEDLEdit;
extern Hotkey kTogglePreferences;
extern Hotkey kToggle3dView;
extern Hotkey kToggleHistogram;
extern Hotkey kToggleVectorscope;
extern Hotkey kToggleWaveform;
extern Hotkey kToggleICCProfiles;
extern Hotkey kToggleConnections;
extern Hotkey kToggleHotkeys;
extern Hotkey kToggleLogs;
extern Hotkey kToggleAbout;

extern Hotkey kTogglePixelRatio;
extern Hotkey kToggleLut;

extern Hotkey kRotatePlus90;
extern Hotkey kRotateMinus90;

extern Hotkey kToggleICS;

struct HotkeyEntry
{
    HotkeyEntry( const std::string n,
                 Hotkey& h ) :
        name(n),
        hotkey(h)
    {
    };

    std::string name;
    Hotkey hotkey;
};

struct TableText
{
    int n;
    const char* text;
};

extern struct TableText table[];
extern HotkeyEntry hotkeys[];

void fill_ui_hotkeys( mrv::Browser* o );
void select_hotkey( HotkeyUI* m );

}


#endif // mrvHotkey_h
