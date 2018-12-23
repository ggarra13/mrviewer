// This theme plugin causes fltk to do nothing, thus leaving the theme
// at the fltk default appearance, with all the styles at their compiled-in
// settings.
//
// This is different than the default behavior of fltk. If you don't
// specify a theme, fltk tries to load one called "default". If that
// fails, it will call fl_get_system_colors() which reads the Windows
// preferences or the xrdb database to try to get some colors.

extern "C" bool fltk_theme() {return true;}
