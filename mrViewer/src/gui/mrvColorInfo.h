/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo GarramuÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂ±o

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
 * @file   mrvColorInfo.h
 * @author gga
 * @date   Wed Nov 08 05:30:32 2006
 *
 * @brief  Color Info Text Display
 *
 *
 */

#ifndef mrvColorInfo_h
#define mrvColorInfo_h

#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include "core/mrvRectangle.h"
#include "gui/mrvBrowser.h"
#include "gui/mrvPopupMenu.h"


namespace mrv
{
class CMedia;
class ColorWidget;
class ColorBrowser;
class ImageView;

class ColorInfo : public Fl_Group
{
public:
    ColorInfo( int x, int y, int w, int h, const char* l = 0 );

    void main( ViewerUI* m );
    ImageView* view() const;

    virtual int handle( int event );

    void update();
    void update( const CMedia* img,
                 const mrv::Rectd& selection );

    static void selection_to_coord( const CMedia* img,
                                    const mrv::Rectd& selection,
                                    int& xmin, int& ymin, int& xmax,
                                    int& ymax, bool& right, bool& bottom );

protected:
    ColorWidget*    dcol;
    Fl_Box*         area;
    ColorBrowser*   browser;
    mrv::PopupMenu* uiColorB;
    static ViewerUI*   uiMain;
};

} // namespace mrv

#endif // mrvColorInfo_h

