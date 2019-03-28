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
 * @file   mrvImageBrowser.h
 * @author gga
 * @date   Wed Jul 11 18:51:10 2007
 *
 * @brief
 *
 *
 */

#ifndef mrvImageBrowser_h
#define mrvImageBrowser_h

#include <vector>
#include <string>

#include <FL/Fl_Button.H>
#include <FL/Fl_Tree.H>

#include "core/Sequence.h"
#include "core/mrvServer.h"
#include "gui/mrvReelList.h"
#include "gui/mrvBrowser.h"
#include "gui/mrvChoice.h"
#include "gui/mrvMedia.h"
#include "video/mrvGLShape.h"


class ViewerUI;

namespace mrv
{
class Element;
class Timeline;
class EDLGroup;
class ImageView;


void start_button_cb(Fl_Button* o, ViewerUI* v);
void end_button_cb(Fl_Button* o, ViewerUI* v);

class ImageBrowser : public Fl_Tree
{
public:
    typedef std::vector< boost::thread* > thread_pool_t;

    struct LThreadData
    {
        mrv::ImageView* view;

        LThreadData( mrv::ImageView* v ) :
            view( v )
        {
        }
    };

public:
    ImageBrowser(int x, int y, int w, int h);
    ~ImageBrowser();

    void clone_all_current();
    void clone_current();
    void open_directory();
    void open();
    void open_stereo();
    void open_single();
    void save();
    void save_sequence();
    void save_reel();
    void load_reel( const char* name );
    void remove_reel();

    size_t number_of_reels() const {
        return _reels.size();
    }
    mrv::Reel new_reel( const char* name = "reel" );
    mrv::Reel current_reel();
    mrv::Reel reel( const char* name );
    mrv::Reel reel( unsigned int idx );
    mrv::Reel reel_at( unsigned int idx );
    unsigned reel_index() { return _reel; }

    mrv::media current_image();

    void reel_choice( mrv::Choice* c ) {
        _reel_choice = c;
    }

    void attach_profile();

    void remove_current();

    void last_image();

    void change_image( int i );

    void image_version( int sum );
    void previous_image_version();
    void next_image_version();

    void previous_image();
    void next_image();

    void match_tree_order();

    void insert( unsigned idx, mrv::media m );

    // @todo: these should be factored to a database helper class
#if 0
    void add_image( const mrv::media& m );
    void add_video( const mrv::media& m );
    void add_audio( const mrv::media& m );
#endif

    mrv::media add( mrv::media& m );
    mrv::media add( CMedia* img );
    mrv::media add( const char* filename,
                    const int64_t start = -999999,
                    const int64_t end = -999999 );

    void debug_images() const;

    void load( const LoadList& files, const bool stereo = false,
               std::string bgfile = "",
	       const bool edl = false,
               const bool progressBar = false );
    void load( const stringArray& files, const bool seqs = true,
               const bool stereo = false,
               const std::string bgfile = "",
	       const bool edl = false,
               const bool progressBar = false );

    
    void replace( int idx, mrv::media m );

    void remove( mrv::media m );
    void remove( int idx );

    void refresh( mrv::media img );

    void seek( const int64_t f );

    void frame( const int64_t f );
    
    void clear_edl();
    void set_edl();
    void toggle_edl();

    void clear_bg();
    void set_bg( mrv::media bg );
    void change_background();

    void attach_icc_profile();

    void attach_ctl_script();


    void handle_dnd();

    void value( int idx ) { _value = idx; }
    int value() const { return _value; }

    virtual void draw();
    virtual int handle( int event );

    void main( ViewerUI* m ) {
        uiMain = m;
    }
    ViewerUI* main() {
        return uiMain;
    }

    void send_image( int idx );

    void exchange( int oldsel, int sel );

public:
    static mrv::Element* new_item(mrv::media img);

protected:

    std::string media_to_pathname( const mrv::media m );
    
    void send_reel( const mrv::Reel& r );
    void send_images( const mrv::Reel& r);
    void send_current_image( const mrv::media& m );

    void change_reel();
    void change_image();
    void adjust_timeline();
    void load_stereo( mrv::media& fg,
                      const char* name,
                      const int64_t first, const int64_t last,
                      const int64_t start, const int64_t end );
    mrv::media load_image( const char* name,
                           const int64_t first, const int64_t last,
                           const int64_t start, const int64_t end,
                           const bool avoid_seq = false );


    int mouseDrag( int x, int y );
    int mousePush( int x, int y );
    int mouseRelease( int x, int y );

    mrv::Timeline* timeline();
    mrv::EDLGroup* edl_group() const;

    mrv::ImageView* view() const;

    thread_pool_t  _load_threads;    //!< loading threads if any

    unsigned       _reel;
    mrv::ReelList  _reels;
    mrv::Choice*  _reel_choice;
    int           _value;

    CMedia::Mutex   _mtx;
    Fl_Tree_Item*    dragging;
    Fl_Tree_Item*    old_dragging;
    int lastX, lastY;

    ViewerUI* uiMain;
};

} // namespace mrv


#endif  // mrvImageBrowser_h

