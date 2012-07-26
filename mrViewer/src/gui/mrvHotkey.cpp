
#include <cstdio>
#include <iostream>

#include <fltk/events.h>
#include <fltk/run.h>

#include <fltk/Browser.h>

#include "keyboard_ui.h"
#include "mrViewer.h"

#include "mrvHotkey.h"


namespace mrv {

Hotkey kOpenImage( true, false, false, false, 'o' );
Hotkey kSaveImage( true, false, false, false, 's' );
Hotkey kIccProfile( true, false, false, false, 'i' );
Hotkey kCTLScript( true, false, false, false, 't' );
Hotkey kMonitorCTLScript( true, false, false, false, 'm' );
Hotkey kMonitorIccProfile( true, false, false, false, 'n' );

Hotkey kZoomMin( false, false, false, false, '0' );
Hotkey kZoomMax( false, false, false, false, '9' );

Hotkey kZoomIn( false, false, false, false, '.' );
Hotkey kZoomOut( false, false, false, false, ',' );
Hotkey kFullScreen( false, false, false, false, ' ' );
Hotkey kFitScreen( false, false, false, false, 'f' );
Hotkey kSafeAreas( false, false, false, false, 's' );
Hotkey kWipe( false, false, false, false, 'w' );
Hotkey kFlipX( false, false, false, false, 'x' );
Hotkey kFlipY( false, false, false, false, 'y' );

Hotkey kFrameStepBack( false, false, false, false, fltk::LeftKey, "", 
		       fltk::Keypad4 );
Hotkey kFrameStepFwd( false, false, false, false, fltk::RightKey, "", 
		      fltk::Keypad6 );
Hotkey kPlayBack( false, false, false, false, fltk::UpKey, "", fltk::Keypad8 );
Hotkey kPlayFwd( false, false, false, false, fltk::DownKey, "", fltk::Keypad2 );
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


HotkeyEntry hotkeys[] = {
HotkeyEntry("Open Image", kOpenImage),
HotkeyEntry("Save Image", kSaveImage),
HotkeyEntry("Image Icc Profile", kIccProfile ),
HotkeyEntry("Image CTL script", kCTLScript ),
HotkeyEntry("Monitor Icc Profile", kMonitorIccProfile ),
HotkeyEntry("Monitor CTL script", kMonitorCTLScript ),
HotkeyEntry("Zoom Minimum", kZoomMin),
HotkeyEntry("Zoom Maximum", kZoomMax),
HotkeyEntry("Fit Screen", kFitScreen),
HotkeyEntry("Safe Areas", kSafeAreas),
HotkeyEntry("Wipe", kWipe),
HotkeyEntry("Flip X", kFlipX),
HotkeyEntry("Flip Y", kFlipY),
HotkeyEntry("Frame Step Backwards", kFrameStepBack),
HotkeyEntry("Frame Step Forwards", kFrameStepFwd),
HotkeyEntry("Play Backwards", kPlayBack),
HotkeyEntry("Play Forwards", kPlayFwd),
HotkeyEntry("Stop", kStop),
HotkeyEntry("Previous Image", kPreviousImage ),
HotkeyEntry("Next Image", kNextImage ),
HotkeyEntry("First Frame", kFirstFrame ),
HotkeyEntry("Last Frame", kLastFrame ),
HotkeyEntry("Toggle Background", kToggleBG ),
HotkeyEntry("Toggle Top Bar", kToggleTopBar ),
HotkeyEntry("Toggle Pixel Bar", kTogglePixelBar ),
HotkeyEntry("Toggle Bottom Bar", kToggleTimeline ),
HotkeyEntry("Toggle Full Screen", kFullScreen),
HotkeyEntry("Toggle Presentation", kTogglePresentation ),
HotkeyEntry("Scrub", kScrub),
HotkeyEntry("Exposure More", kExposureMore),
HotkeyEntry("Exposure Less", kExposureLess),
HotkeyEntry("Gamma More", kGammaMore),
HotkeyEntry("Gamma Less", kGammaLess),
HotkeyEntry("END", kGammaLess),
};


struct TableText table[] = {
  {fltk::EscapeKey, "Escape"},
  {fltk::BackSpaceKey, "BackSpace"},
  {fltk::TabKey, "Tab"},
  {fltk::ReturnKey, "Return"},
  {fltk::PrintKey, "Print"},

  {fltk::ScrollLockKey, "ScrollLock"},
  {fltk::PauseKey, "Pause"},
  {fltk::InsertKey, "Insert"},
  {fltk::HomeKey, "Home"},
  {fltk::PageUpKey, "PageUp"},

  {fltk::DeleteKey, "Delete"},
  {fltk::EndKey, "End"},
  {fltk::PageDownKey, "PageDown"},
  {fltk::LeftKey, "Left"},
  {fltk::UpKey, "Up"},

  {fltk::RightKey, "Right"},
  {fltk::DownKey, "Down"},
  {fltk::LeftShiftKey, "LeftShift"},
  {fltk::RightShiftKey, "RightShift"},
  {fltk::LeftCtrlKey, "LeftCtrl"},

  {fltk::RightCtrlKey, "RightCtrl"},
  {fltk::CapsLockKey, "CapsLock"},
  {fltk::LeftAltKey, "LeftAlt"},
  {fltk::RightAltKey, "RightAlt"},
  {fltk::LeftMetaKey, "LeftMeta"},

  {fltk::RightMetaKey, "RightMeta"},
  {fltk::MenuKey, "Menu"},
  {fltk::NumLockKey, "NumLock"},
  {fltk::KeypadEnter, "KeypadEnter"},
  {fltk::MultiplyKey, "Multiply"},

  {fltk::AddKey, "Add"},
  {fltk::SubtractKey, "Subtract"},
  {fltk::DecimalKey, "Decimal"},
  {fltk::DivideKey, "Divide"},
  {fltk::Keypad0, "Keypad0"},

  {fltk::Keypad1, "Keypad1"},
  {fltk::Keypad2, "Keypad2"},
  {fltk::Keypad3, "Keypad3"},
  {fltk::Keypad4, "Keypad4"},
  {fltk::Keypad5, "Keypad5"},

  {fltk::Keypad6, "Keypad6"},
  {fltk::Keypad7, "Keypad7"},
  {fltk::Keypad8, "Keypad8"},
  {fltk::Keypad9, "Keypad9"},
  {fltk::SpaceKey,"Space (' ')"}
};


void fill_uiFunction( fltk::Browser* b )
{
   const char* labels[] = {"Function", "Hotkey", NULL};
   b->column_labels( labels );
   const int widths[] = {200, -1, -1};
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

      std::string row( h.name );
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

