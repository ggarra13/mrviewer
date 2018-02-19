// "$Id:"
//
// Preferences database for the Fast Light toolkit Interface Designer (fluid)
//
// Copyright 1998-2006 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems on the following page:
//
//    http://www.fltk.org/str.php

#ifndef _fltk_PrefsData_h
#define _fltk_PrefsData_h
#include <fltk/Preferences.h>
#include <fltk/Color.h>
#include <fltk/string.h>

// Properties are not implemented in C++ so here's so macros defining them
// Note: a fluid property is persistent, 
#define FluidProperty(type, p, name) \
public:	    type p() const {return p##_;} \
	    type * p##Ptr() {return &p##_;} \
            void p(const type v) {p##_=v; if(autosave_) set(name, p##_);} \
private:    type p##_

#define FluidPropVect(type, size, p, name) \
public:	    const type* p() const {return p##_;} \
            void p(const type* v) {memcpy(p##_,v,size);if(autosave_) set(name, p##_);} \
private:    type p##_[size];

#define FluidPropertyGroup(group, type, p, name) \
public:	    type p() const {return p##_;} \
	    type * p##Ptr() {return &p##_;} \
            void p(const type v) {p##_=v; if(autosave_) {fltk::Preferences g(this, group); g.set(name, p##_);}} \
private:    type p##_

#define FluidPropVectGroup(type, size, p, name) \
public:	    const type* p() const {return p##_;} \
    void p(const type* v) {memcpy(p##_,v,size); if(autosave_) {fltk::Preferences g(this, group); g.set(name, p##_);}} \
private:    type p##_[size];

/** \class PrefsData
  Fluid unified preferences data structure
  This class permit to unify all often used persistent 
  fluid data. It features the notion of 'property' which is 
  not only an attribute, but has members and the 'set' part is persistent
  so that 
  it makes not necessary to reload all the time the same data from the prefs
  as every 'set' is written automatically to the prefs file
  So not performance gain occurs now (it was quite acceptable before), 
  it also greatly simplifies the design.
  All important persistant fluid data should be declared here.
*/
class PrefsData : public fltk::Preferences {
    // 1. define your persistent property here
    FluidProperty(int, gridx,"gridx");
    FluidProperty(int, gridy,"gridy");
    FluidProperty(int, snap,"snap");
    FluidProperty(int, show_tooltip,"show_tooltips");
    FluidProperty(int, open_previous_file,"open_previous_file");
    FluidProperty(int, prev_window_pos,"prev_window_pos");
    FluidProperty(int, show_completion_dialogs,"show_completion_dialogs");
    FluidProperty(int, recent_files,"recent_files");
    FluidPropVect(char, 1024, shell_command,"shell_command");
    FluidProperty(int, shell_writecode,"shell_writecode");
    FluidProperty(int, shell_writemsgs,"shell_writemsgs");
    FluidProperty(int, shell_savefl,"shell_savefl");
    FluidProperty(int, show_statusbar,"show_statusbar");
    FluidProperty(int, show_splash,"show_splash");
    
    FluidPropertyGroup("sourceview", int, sv_autorefresh,"autorefresh");
    FluidPropertyGroup("sourceview", int, sv_autoposition,"autoposition");
    FluidPropertyGroup("sourceview", int, sv_tab,"tab");

    FluidPropertyGroup("tabs", int, tabcolor,"tabcolor");
    FluidPropertyGroup("tabs", int, tabcolor1,"tabcolor1");
    FluidPropertyGroup("tabs", int, tabcolor2,"tabcolor2");
    FluidPropertyGroup("tabs", int, tabcolor3,"tabcolor3");

    // 2. initialize your persistent property here
    void get_all_properties() {
      get("gridx",gridx_, 5);
      get("gridy",gridy_, 5);
      get("snap",snap_, 3);
      get("show_tooltip",show_tooltip_, 1);
      get("open_previous_file", open_previous_file_, 0);
      get("prev_window_pos", prev_window_pos_, 1);
      get("show_completion_dialogs", show_completion_dialogs_, 1);
      get("recent_files", recent_files_, 5);
      get("shell_command", shell_command_, "", sizeof(shell_command_));
      get("shell_writecode", shell_writecode_, 1);
      get("shell_writemsgs", shell_writemsgs_, 0);
      get("shell_savefl", shell_savefl_, 1);
      get("show_statusbar", show_statusbar_, 1);
      get("show_splash",show_splash_,1);

      fltk::Preferences s(this, "sourceview");
      s.get("autorefresh",sv_autorefresh_,1);
      s.get("autoposition",sv_autoposition_,1);
      s.get("tab",sv_tab_,0);

      fltk::Preferences g(this,"tabs");
      g.get("tabcolor",tabcolor_, 1);
      g.get("tabcolor1",tabcolor1_, 0x14aef700);
      g.get("tabcolor2",tabcolor2_, 0xd4b38d00);
      g.get("tabcolor3",tabcolor3_, 0x93d49c00);

    }

    bool    autosave_;

private:
    void init() {autosave_=true; get_all_properties();}

public:
    PrefsData( Root root, const char *vendor, const char *application ):
	Preferences(root, vendor, application) {init();}
    //!avoid saving in the prefs when writing (for per-file prefs purpose)
    void autosave(bool a) {autosave_=a;}
};

extern PrefsData prefs; // FLUID preferences

#endif

//
// End of "$Id"
//
