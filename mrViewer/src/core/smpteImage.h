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
 * @file   smpteImage.h
 * @author gga
 * @date   Sun Oct 22 22:52:27 2006
 * 
 * @brief  A simple image generator of callibration images
 * 
 * 
 */


#ifndef smpteImage_h
#define smpteImage_h

#include "CMedia.h"


namespace mrv {

  class smpteImage : public CMedia
  {
  public:
    enum Type
      {
	kGammaChart,
	kLinearGradient,
	kLuminanceGradient,
	kCheckered,
      };

  public:
    smpteImage( const Type c = kGammaChart, const unsigned int dw = 720,
		const unsigned int dh = 480 );

    static bool test(const char* file) { return false; }

    virtual const char* const format() const { return "Built-in Image"; }

    bool fetch( const boost::int64_t frame );

  protected:
    void luminance_gradient();
    void linear_gradient();
    void checkered();

    void gamma_chart();
    void gamma_boxes( unsigned int x, unsigned int y, 
		      unsigned int w, unsigned int h,
		      float bg, float fg );
    void gamma_box( unsigned int x, unsigned int y, 
		    unsigned int w, unsigned int h );

    Pixel bg;
    Pixel fg;
    Type type_;
  };

} // namespace mrv


#endif // smpteImage_h
