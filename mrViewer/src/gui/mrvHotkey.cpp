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

#include <cstdio>
#include <iostream>

#include <fltk/events.h>
#include <fltk/run.h>

#include <fltk/Browser.h>

#include "keyboard_ui.h"
#include "mrViewer.h"

#include "core/mrvI8N.h"

#include "mrvHotkey.h"


namespace mrv {
// ctrl, meta, alt, shift, key
Hotkey kOpenImage( true, false, false, false, 'o' );
Hotkey kOpenSingleImage( true, false, true, false, 'o' );
Hotkey kOpenClipXMLMetadata( true, false, false, false, 'x' );
Hotkey kSaveReel( true, false, false, false, 'r' );
Hotkey kSaveImage( true, false, false, false, 's' );
Hotkey kSaveSequence( true, false, false, true, 's' );
Hotkey kSaveSnapshot( false, false, true, false, 's' );
Hotkey kSaveClipXMLMetadata( false, false, true, false, 'x' );
Hotkey kIccProfile( true, false, false, false, 'i' );
Hotkey kIDTScript( true, false, false, true, 'i' );
Hotkey kLookModScript( true, false, false, false, 'l' );
Hotkey kCTLScript( true, false, false, false, 't' );
Hotkey kMonitorCTLScript( true, false, false, false, 'm' );
Hotkey kMonitorIccProfile( true, false, false, false, 'n' );

Hotkey kZoomMin( false, false, false, false, '0' );
Hotkey kZoomMax( false, false, false, false, '9' );

Hotkey kZoomIn( false, false, false, false, '.' );
Hotkey kZoomOut( false, false, false, false, ',' );
Hotkey kFullScreen( false, false, false, false, fltk::F11Key );
Hotkey kFitScreen( false, false, false, false, 'f' );
Hotkey kSafeAreas( false, false, false, false, 's' );
Hotkey kDisplayWindow( false, false, false, false, 'd' );
Hotkey kDataWindow( true, false, false, false, 'd' );
Hotkey kWipe( false, false, false, false, 'w' );
Hotkey kFlipX( false, false, false, false, 'x' );
Hotkey kFlipY( false, false, false, false, 'y' );
Hotkey kCenterImage( false, false, false, false, 'h' );

Hotkey kFrameStepBack( false, false, false, false, fltk::LeftKey, "", 
		       fltk::Keypad4 );
Hotkey kFrameStepFPSBack( true, false, false, false, fltk::LeftKey, "", 
			  fltk::Keypad4 );
Hotkey kFrameStepFwd( false, false, false, false, fltk::RightKey, "", 
		      fltk::Keypad6 );
Hotkey kFrameStepFPSFwd( true, false, false, false, fltk::RightKey, "", 
			 fltk::Keypad6 );
Hotkey kPlayBackTwiceSpeed( true, false, false, false, 
			    fltk::UpKey, "", fltk::Keypad8 );
Hotkey kPlayBackHalfSpeed( false, false, true, false, 
			   fltk::UpKey, "", fltk::Keypad8 );
Hotkey kPlayBack( false, false, false, false, fltk::UpKey, "", fltk::Keypad8 );
Hotkey kPlayFwd( false, false, false, false, fltk::SpaceKey, "", 
		 fltk::Keypad2 );
Hotkey kPlayFwdTwiceSpeed( true, false, false, false, 
			   fltk::DownKey, "", fltk::Keypad2 );
Hotkey kPlayFwdHalfSpeed( false, false, true, false, 
			  fltk::DownKey, "", fltk::Keypad2 );
Hotkey kStop( false, false, false, false, fltk::ReturnKey, "", fltk::Keypad5 );

Hotkey kPreviousImage( false, false, false, false, fltk::PageUpKey );
Hotkey kNextImage( false, false, false, false, fltk::PageDownKey );

Hotkey kFirstFrame( false, false, false, false, fltk::HomeKey );
Hotkey kLastFrame( false, false, false, false, fltk::EndKey );
Hotkey kToggleBG( false, false, false, false, fltk::TabKey );


Hotkey kToggleTopBar( false, false, false, false, fltk::F1Key );
Hotkey kTogglePixelBar( false, false, false, false, fltk::F2Key );
Hotkey kToggleTimeline( false, false, false, false, fltk::F3Key );
Hotkey kTogglePresentation( false, false, false, false, fltk::F12Key );

Hotkey kScrub( false, false, false, false, fltk::LeftShiftKey, "",
	       fltk::RightShiftKey );


Hotkey kExposureMore( false, false, false, false, 0, "]" );
Hotkey kExposureLess( false, false, false, false, 0, "[" );
Hotkey kGammaMore( false, false, false, false, 0, ")" );
Hotkey kGammaLess( false, false, false, false, 0, "(" );

Hotkey kSetAsBG( false, false, false, false, 0 );
Hotkey kToggleLUT( false, false, false, false, 0 );


Hotkey kZDepthUp( false, false, false, false, 's' );
Hotkey kZDepthDown( false, false, false, false, 'a' );

Hotkey kDensityUp( false, false, false, false, 'c' );
Hotkey kDensityDown( false, false, false, false, 'd' );

Hotkey kAttachAudio( false, false, false, false, 0 );
Hotkey kEditAudio( false, false, false, false, 0 );
Hotkey kDetachAudio( false, false, false, false, 0 );
Hotkey kCopyRGBAValues( true, false, false, false, 'c' );
Hotkey kCloneImage( false, false, false, false, 0 );

Hotkey kClearCache( false, false, false, false, 'k' );

Hotkey kSetInPoint( false, false, false, false, 'i' );
Hotkey kSetOutPoint( false, false, false, false, 'o' );

HotkeyEntry hotkeys[] = {
HotkeyEntry( _("Open Movie or Sequence"), kOpenImage),
HotkeyEntry( _("Open Single Image"), kOpenSingleImage),
HotkeyEntry( _("Open Clip XML Metadata"), kOpenClipXMLMetadata),
HotkeyEntry( _("Save Reel"), kSaveReel),
HotkeyEntry( _("Save Image"), kSaveImage),
HotkeyEntry( _("Save GL Snapshot"), kSaveSnapshot),
HotkeyEntry( _("Save Sequence"), kSaveSequence),
HotkeyEntry( _("Save Clip XML Metadata"), kSaveClipXMLMetadata),
HotkeyEntry( _("Image Icc Profile"), kIccProfile ),
HotkeyEntry( _("Image CTL script"), kCTLScript ),
HotkeyEntry( _("Monitor Icc Profile"), kMonitorIccProfile ),
HotkeyEntry( _("Monitor CTL script"), kMonitorCTLScript ),
//HotkeyEntry( _("Zoom Minimum"), kZoomMin),
//HotkeyEntry( _("Zoom Maximum"), kZoomMax),
HotkeyEntry( _("Center Image"), kCenterImage ),
HotkeyEntry( _("Fit Screen"), kFitScreen),
HotkeyEntry( _("Safe Areas"), kSafeAreas),
HotkeyEntry( _("Display Window"), kDisplayWindow),
HotkeyEntry( _("Data Window"), kDataWindow),
HotkeyEntry( _("Wipe"), kWipe),
HotkeyEntry( _("Flip X"), kFlipX),
HotkeyEntry( _("Flip Y"), kFlipY),
HotkeyEntry( _("Frame Step Backwards"), kFrameStepBack),
HotkeyEntry( _("Frame Step FPS Backwards"), kFrameStepFPSBack),
HotkeyEntry( _("Frame Step Forwards"), kFrameStepFwd),
HotkeyEntry( _("Frame Step FPS Forwards"), kFrameStepFPSFwd),
HotkeyEntry( _("Play Backwards"), kPlayBack),
HotkeyEntry( _("Play Backwards X2  Speed"), kPlayBackTwiceSpeed),
HotkeyEntry( _("Play Backwards 1/2 Speed"), kPlayBackHalfSpeed),
HotkeyEntry( _("Play Forwards"), kPlayFwd),
HotkeyEntry( _("Play Forwards X2  Speed"), kPlayFwdTwiceSpeed),
HotkeyEntry( _("Play Forwards 1/2 Speed"), kPlayFwdHalfSpeed),
HotkeyEntry( _("Clear Image Cache"), kClearCache),
HotkeyEntry( _("Stop"), kStop),
HotkeyEntry( _("Previous Image"), kPreviousImage ),
HotkeyEntry( _("Next Image"), kNextImage ),
HotkeyEntry( _("First Frame"), kFirstFrame ),
HotkeyEntry( _("Last Frame"), kLastFrame ),
HotkeyEntry( _("Toggle Background"), kToggleBG ),
HotkeyEntry( _("Toggle Top Bar"), kToggleTopBar ),
HotkeyEntry( _("Toggle Pixel Bar"), kTogglePixelBar ),
HotkeyEntry( _("Toggle Bottom Bar"), kToggleTimeline ),
HotkeyEntry( _("Toggle Full Screen"), kFullScreen),
HotkeyEntry( _("Toggle Presentation"), kTogglePresentation ),
HotkeyEntry( _("Scrub"), kScrub),
HotkeyEntry( _("Exposure More"), kExposureMore),
HotkeyEntry( _("Exposure Less"), kExposureLess),
HotkeyEntry( _("Gamma More"), kGammaMore),
HotkeyEntry( _("Gamma Less"), kGammaLess),
HotkeyEntry( _("Set As BG Image"), kSetAsBG),
HotkeyEntry( _("Toggle LUT"), kToggleLUT),
HotkeyEntry( _("Attach Audio File"), kAttachAudio),
HotkeyEntry( _("Edit Audio Frame Offset"), kEditAudio),
HotkeyEntry( _("Copy RGBA Values"), kCopyRGBAValues),
HotkeyEntry( _("Clone Image"), kCloneImage),
HotkeyEntry( _("3dView Z Depth Up"), kZDepthUp),
HotkeyEntry( _("3dView Z Depth Down"), kZDepthDown),
HotkeyEntry( _("3dView Density Up"), kDensityUp),
HotkeyEntry( _("3dView Density Down"), kDensityDown),
HotkeyEntry( _("Set In Point"), kSetInPoint),
HotkeyEntry( _("Set Out Point"), kSetOutPoint),
HotkeyEntry( N_("END"), kGammaLess),
};


struct TableText table[] = {
{fltk::EscapeKey, _("Escape")},
{fltk::BackSpaceKey, _("BackSpace")},
{fltk::TabKey, _("Tab")},
{fltk::ReturnKey, _("Return")},
{fltk::PrintKey, _("Print")},

{fltk::ScrollLockKey, _("ScrollLock")},
{fltk::PauseKey, _("Pause")},
{fltk::InsertKey, _("Insert")},
{fltk::HomeKey, _("Home")},
{fltk::PageUpKey, _("PageUp")},

{fltk::DeleteKey, _("Delete")},
{fltk::EndKey, _("End")},
{fltk::PageDownKey, _("PageDown")},
{fltk::LeftKey, _("Left")},
{fltk::UpKey, _("Up")},

{fltk::RightKey, _("Right")},
{fltk::DownKey, _("Down")},
{fltk::LeftShiftKey, _("LeftShift")},
{fltk::RightShiftKey, _("RightShift")},
{fltk::LeftCtrlKey, _("LeftCtrl")},

{fltk::RightCtrlKey, _("RightCtrl")},
{fltk::CapsLockKey, _("CapsLock")},
{fltk::LeftAltKey, _("LeftAlt")},
{fltk::RightAltKey, _("RightAlt")},
{fltk::LeftMetaKey, _("LeftMeta")},

{fltk::RightMetaKey, _("RightMeta")},
{fltk::MenuKey, _("Menu")},
{fltk::NumLockKey, _("NumLock")},
{fltk::KeypadEnter, _("KeypadEnter")},
{fltk::MultiplyKey, _("Multiply")},

{fltk::AddKey, _("Add")},
{fltk::SubtractKey, _("Subtract")},
{fltk::DecimalKey, _("Decimal")},
{fltk::DivideKey, _("Divide")},
{fltk::Keypad0, _("Keypad0")},

{fltk::Keypad1, _("Keypad1")},
{fltk::Keypad2, _("Keypad2")},
{fltk::Keypad3, _("Keypad3")},
{fltk::Keypad4, _("Keypad4")},
{fltk::Keypad5, _("Keypad5")},

{fltk::Keypad6, _("Keypad6")},
{fltk::Keypad7, _("Keypad7")},
{fltk::Keypad8, _("Keypad8")},
{fltk::Keypad9, _("Keypad9")},
{fltk::SpaceKey,_("Space (' ')")}
};


void fill_uiFunction( fltk::Browser* b )
{
   if (!b) return;

   const char* labels[] = { _("Function"), _("Hotkey"), NULL};
   b->column_labels( labels );
   const int widths[] = {240, -1, 0};
   b->column_widths( widths );
   
   b->clear();


   for ( int i = 0; hotkeys[i].name != "END"; ++i )
   {
      HotkeyEntry& h = hotkeys[i];
      std::string key;
      if ( h.hotkey.ctrl ) key += "Ctrl+";
      if ( h.hotkey.alt ) key += "Alt+";
      if ( h.hotkey.meta ) key += "Meta+";
      if ( h.hotkey.shift ) key += "Shift+";

      unsigned k = h.hotkey.key;

      bool special = false;
      for ( int j = 0; j < 45; ++j )
	 if ( k == table[j].n )
	 {
	    key += table[j].text;
	    special = true;
	    break;
	 }

      if ( !special )
      {
	 if (k >= fltk::F0Key && k <= fltk::LastFunctionKey) {
	    char buf[64];
	    sprintf(buf, "F%d", k - fltk::F0Key);
	    key += buf;
	 }
	 else
	 {
	    if ( h.hotkey.key != 0 ) key += (char) h.hotkey.key;
	    if ( h.hotkey.key == 0 && h.hotkey.text != "" ) 
	       key += h.hotkey.text;
	 }
      }

      std::string row( _(h.name.c_str()) );
      row += "\t" + key;

      b->add( row.c_str() );
   }
}



void select_hotkey( HotkeyUI* b )
{
   int idx = b->uiFunction->value();
   
   Hotkey& hk = hotkeys[idx].hotkey;

   ChooseHotkey* h = new ChooseHotkey(hk);
   h->make_window();
   h->fill();

   fltk::Window* window = h->uiMain;
   window->child_of( b->uiMain );
   window->show();

   while ( window->visible() )
      fltk::check();

   hk = h->hk;

   delete h;

   fill_uiFunction( b->uiFunction );
}


} // namespace mrv

