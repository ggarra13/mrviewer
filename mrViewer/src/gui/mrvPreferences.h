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
    static int bgcolor;
    static int textcolor;
    static int selectioncolor;

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
