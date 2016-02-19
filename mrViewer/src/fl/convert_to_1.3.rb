#!/usr/bin/ruby
# -*- coding: utf-8 -*-

if ARGV.size < 1
  $stderr.puts "Usage: "
  $stderr.puts 
  $stderr.puts "$0 fltk2file.fl" 
  exit 1
end

file  = ARGV[0]
lines = File.readlines( file )

lines.each { |x|
  x.sub!(/^(\s+)\{fltk::Window\}/, '\1Fl_Window' )
  x.sub!(/^(\s+)\{fltk::Group\}/, '\1Fl_Group' )
  x.sub!(/^(\s+)\{fltk::PackedGroup\}/, '\1Fl_Pack' )
  x.sub!(/^(\s+)\{fltk::Browser\}/, '\1Fl_Browser' )
  x.sub!(/^(\s+)\{fltk::Choice\}/, '\1Fl_Choice' )
  x.sub!(/^(\s+)\{fltk::ValueSlider\}/, '\1Fl_Value_Slider' )
  x.sub!(/^(\s+)\{fltk::PopupMenu\}/, '\1Fl_Menu_Button' )
  x.sub!(/^(\s+)\{fltk::Item\}/, '\1MenuItem' )
  x.sub!(/^(\s+)\{fltk::ItemGroup\}/, '\1Submenu' )
  x.sub!(/^(\s+)\{fltk::Button\}/, '\1Fl_Button' )
  x.sub!(/^(\s+)\{fltk::CheckButton\}/, '\1Fl_Check_Button' )
  x.sub!(/^(\s+)\{fltk::RadioButton\}/, '\1Fl_Radio_Button' )
  x.sub!(/^(\s+)\{fltk::ValueSlider\}/, '\1Fl_Value_Slider' )
  x.sub!(/^(\s+)\{fltk::Slider\}/, '\1Fl_Slider' )
  x.sub!(/fltk::ValueSlider*/, 'Fl_Value_Slider*' )
  x.sub!(/fltk::Slider*/, 'Fl_Slider*' )
  x.sub!(/^(\s+)\{fltk::ValueInput\}/, '\1Fl_Value_Input' )
  x.sub!(/fltk::ValueInput/, 'Fl_Value_Input' )
  x.sub!(/^(\s+)\{fltk::InvisibleBox\}/, '\1Fl_Box' )
  x.sub!(/^(\s+)\{Fl_TextDisplay\}/, '\1Fl_Text_Display' )
  x.sub!(/mrv\:\:ViewerUI/, 'ViewerUI' )
  x.sub!(/mrv\:\:PreferencesUI/, 'PreferencesUI' )
  x.sub!(/vertical\s+/, '' )
}

f = File.open( file, "w" )
f.puts lines
f.close
