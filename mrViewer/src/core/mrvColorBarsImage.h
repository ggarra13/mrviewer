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
 * @file   colorBarsImage.h
 * @author gga
 * @date   Sun Oct 22 22:52:27 2006
 * 
 * @brief  A simple 3-second color bar generator (with tone)
 * 
 * 
 */


#ifndef mrvColorBarsImage_h
#define mrvColorBarsImage_h

#include "CMedia.h"


namespace mrv {

  class ColorBarsImage : public CMedia
  {
  public:
    enum Type
      {
	kSMPTE_NTSC,
	kSMPTE_NTSC_HDTV,
	kPAL,
	kPAL_HDTV,
      };

  public:
    ColorBarsImage( const Type c = kSMPTE_NTSC_HDTV );

    static bool test(const char* file) { return false; }

    virtual const char* const format() const { return "Built-in Image"; }

    virtual bool fetch( const boost::int64_t frame );

  protected:
    void NTSC_color_bars();
    void PAL_color_bars();
    void NTSC_HDTV_color_bars();
    void PAL_HDTV_color_bars();
    
    void smpte_color_bars( const unsigned int X, const unsigned int W, 
			   const unsigned int H, const float pct );
    void smpte_bottom_bars( const unsigned int X, const unsigned int Y, 
			    const unsigned int W, const unsigned int H );
  };

} // namespace mrv


#endif // mrvColorBarsImage_h
