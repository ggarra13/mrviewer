#!/usr/bin/ruby

if ARGV.size < 1
  $stderr.puts "Usage: "
  $stderr.puts 
  $stderr.puts "$0 fltk2file.fl" 
  exit 1
end

file  = ARGV[0]
lines = File.readlines( file )

lines.each { |x|
  x.sub!(/\{fltk::Window\}/, 'Fl_Window' )
  x.sub!(/\{fltk::Group\}/, 'Fl_Group' )
  x.sub!(/\{fltk::PackedGroup\}/, 'Fl_Pack' )
  x.sub!(/\{fltk::Browser\}/, 'Fl_Browser' )
  x.sub!(/\{fltk::PopupMenu\}/, 'Fl_Menu_Button' )
  x.sub!(/\{fltk::Item\}/, 'MenuItem' )
  x.sub!(/\{fltk::ItemGroup\}/, 'Submenu' )
  x.sub!(/\{fltk::Button\}/, 'Fl_Button' )
  x.sub!(/\{fltk::CheckButton\}/, 'Fl_Check_Button' )
  x.sub!(/\{fltk::RadioButton\}/, 'Fl_Radio_Button' )
  x.sub!(/\{fltk::ValueSlider\}/, 'Fl_Value_Slider' )
  x.sub!(/vertical\s+/, '' )
}

puts lines
