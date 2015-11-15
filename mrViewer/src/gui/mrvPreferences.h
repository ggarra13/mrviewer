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

#include <ImfChromaticities.h>

namespace mrv {

  class ViewerUI;
  class PreferencesUI;

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

    enum LutAlgorithm
      {
	kLutPreferCTL,
	kLutOnlyCTL,
	kLutPreferICC,
	kLutOnlyICC
      };

  public:
    Preferences( PreferencesUI* ui );
    ~Preferences();

    static void run( mrv::ViewerUI* main );
    static void save();

    static std::string temporaryDirectory() { return tempDir; }

  protected:
    static bool set_theme();

  public:
      static bool native_file_chooser;
    static int bgcolor;
    static int textcolor;
    static int selectioncolor;
    static int selectiontextcolor;

    static std::string ODT_CTL_transform;
    static std::string ODT_ICC_profile;

    static CacheType           cache_type;
    static Imf::Chromaticities ODT_CTL_chromaticities;

    static float ODT_CTL_white_luminance;
    static float ODT_CTL_surround_luminance;

    static std::string CTL_8bits_save_transform;
    static std::string CTL_16bits_save_transform;
    static std::string CTL_32bits_save_transform;
    static std::string CTL_float_save_transform;

    static std::string root;
    static std::string tempDir;
  };


} // namespace mrv


#endif // mrvPreferences_h
