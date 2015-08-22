/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo Garramuño

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
#include <fltk/events.h>

namespace mrv {
class HotkeyUI;
}

namespace fltk {

class Browser;

}


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
     
     bool match( unsigned rawkey )
     {
	bool ok = false;
	if ( ctrl )
	{
	   if ( fltk::event_key_state( fltk::LeftCtrlKey ) ||
		fltk::event_key_state( fltk::RightCtrlKey ) )
	      ok = true;
	   else
	      return false;
	}
        else
        {
            if ( fltk::event_key_state( fltk::LeftCtrlKey ) ||
                 fltk::event_key_state( fltk::RightCtrlKey ) )
                return false;
        }

	if ( shift )
	{ 
	   if ( fltk::event_key_state( fltk::LeftShiftKey ) ||
		fltk::event_key_state( fltk::RightShiftKey ) )
	      ok = true;
	   else
	      return false;
	}
        else
        {
            if ( fltk::event_key_state( fltk::LeftShiftKey ) ||
                 fltk::event_key_state( fltk::RightShiftKey ) )
                return false;
        }
	if ( alt )
	{
	   if ( fltk::event_key_state( fltk::LeftAltKey ) ||
		fltk::event_key_state( fltk::RightAltKey ) )
	      ok = true;
	   else
	      return false;
	}
        else
        {
            if ( fltk::event_key_state( fltk::LeftAltKey ) ||
                 fltk::event_key_state( fltk::RightAltKey ) )
                return false;
        }
	if ( meta )
	{ 
	   if ( fltk::event_key_state( fltk::LeftMetaKey ) ||
		fltk::event_key_state( fltk::RightMetaKey ) )
	      ok = true;
	   else
	      return false;
	}
        else
        {
            if ( fltk::event_key_state( fltk::LeftMetaKey ) ||
                 fltk::event_key_state( fltk::RightMetaKey ) )
                return false;
        }
	if (( rawkey != 0 ) && ( rawkey == key || 
				 rawkey == key2 ))
	{
	   ok = true;
	}
	else
	{
	   if ( text != "" && text == fltk::event_text() ) ok = true;
	   else ok = false;
	}

	return ok;
     }

     unsigned hotkey()
     {
	unsigned r = 0;
	if ( ctrl ) r += fltk::CTRL;
	if ( shift ) r += fltk::SHIFT;
	if ( meta ) r += fltk::META;
	if ( alt ) r += fltk::ALT;
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

extern Hotkey kOpenImage;
extern Hotkey kOpenSingleImage;
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

extern Hotkey kPreviousImage;
extern Hotkey kNextImage;

extern Hotkey kFirstFrame;
extern Hotkey kLastFrame;
extern Hotkey kToggleBG;

extern Hotkey kToggleTopBar;
extern Hotkey kTogglePixelBar;
extern Hotkey kToggleTimeline;
extern Hotkey kTogglePresentation;

extern Hotkey kScrub;

extern Hotkey kZDepthUp;
extern Hotkey kZDepthDown;

extern Hotkey kDensityUp;
extern Hotkey kDensityDown;

extern Hotkey kExposureMore;
extern Hotkey kExposureLess;
extern Hotkey kGammaMore;
extern Hotkey kGammaLess;

extern Hotkey kSetAsBG;
extern Hotkey kToggleLUT;

extern Hotkey kAttachAudio;
extern Hotkey kEditAudio;
extern Hotkey kDetachAudio;

extern Hotkey kCopyRGBAValues;
extern Hotkey kCloneImage;

extern Hotkey kClearCache;

extern Hotkey kSetInPoint;
extern Hotkey kSetOutPoint;

struct HotkeyEntry
{
     HotkeyEntry( const std::string n,
		  Hotkey& h ) :
     name(n),
     hotkey(h)
     {
     };

     std::string name;
     Hotkey& hotkey;
};

struct TableText
{
     int n;
     const char* text;
};

extern struct TableText table[];
extern HotkeyEntry hotkeys[];

void fill_ui_hotkeys( fltk::Browser* o );
void select_hotkey( mrv::HotkeyUI* m );

}


#endif // mrvHotkey_h
