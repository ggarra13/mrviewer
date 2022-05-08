/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2022  Gonzalo Garramuño

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

#include "core/mrvFrame.h"

#include <vector>
#include <string>

#include <FL/Fl_Button.H>
#include <FL/Fl_Tree.H>

#include "core/Sequence.h"
#include "gui/mrvReelList.h"
#include "gui/mrvBrowser.h"
#include "gui/mrvChoice.h"
#include "gui/mrvMedia.h"
#include "video/mrvGLShape.h"

extern std::string retname;
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
    ImageBrowser(int x, int y, int w, int h);
    ~ImageBrowser();

    //! Clone all channels of a media
    void clone_all_current();

    //! Clone current channels of a media
    void clone_current();

    //! Ask for a directory and open all files in it
    void open_directory();

    //! Ask for filename and open sequence or movie file
    void open();

    //! Ask for filename and open AMF (ACES Metadata File)
    void open_amf();

    //! Ask for filename and open it as a stereo image for current
    //! selected image
    void open_stereo();

    //! Ask for filename and open it as a single image
    void open_single();

    //! Ask for filename and save image under it
    void save();

    //! Ask for filename and save sequence under it
    void save_sequence();

    //! Ask for filename and save all reels and a session file under it
    void save_session();

    //! Ask for filename and save reel under it
    void save_reel();

    //! Save session under a file name
    void save_session_( const std::string& file );

    //! Save reel under an otio file name
    void save_otio( mrv::Reel r, const std::string& file );

    //! Save reel under a reel or otio file name
    void save_reel_( mrv::Reel r, const std::string& file );

    //! Load an otio edit
    void load_otio( const LoadInfo& load );

    //! Load a reel from filename
    void load_reel( const LoadInfo& load );

    //! Load a session and all reels from filename
    void load_session( const char* filename );

    void open_session();

    //! Removes the current reel from reel list
    void remove_reel();

    //! Add the media to tree.  Returns true on success, false on failure
    bool add_to_tree( mrv::media m );

    //! Returns the number of reels
    size_t number_of_reels() const {
        return _reels.size();
    }
    //! Creates a new reel with name (or name #x if name already exists)
    mrv::Reel new_reel( const char* name = "reel" );
    //! Returns current reel
    mrv::Reel current_reel();
    //! Changes current reel to the one matching name
    mrv::Reel reel( const char* name );
    //! Changes current reel to reel index
    mrv::Reel reel( unsigned int idx );
    //! Returns the reel at a certain index, without changing the current reel
    mrv::Reel reel_at( unsigned int idx );
    //! Returns the current selected reel index
    unsigned reel_index() { return _reel; }

    //! Returns the current image being shown on gui
    mrv::media current_image();

    //! Sets the Fl_Choice for the reel
    void reel_choice( mrv::Choice* c ) {
        _reel_choice = c;
    }


    //! Remove current reel
    void remove_current();

    //! Sets the last image as current
    void last_image();

    //! Changes image to image index # i (internally does a lot of stuff)
    void real_change_image( int v, int i, CMedia::Playback play );

    //! Changes image to image index # i (wrapper around real_change_image() )
    void change_image( int i );

    //! Changes image version up or down by 1 or more.
    void image_version( int sum );

    //! Changes image version up or down by 1 or more.
    //! If max files is true, it goes to last version or first one.
    void image_version( size_t i, int sum, mrv::media fg,
                        bool max_files = false );


    //! Change image version to previous one (if found on disk )
    void previous_image_version();

    //! Change image version to next one ( if found on disk )
    void next_image_version();

    //! Changes to previous image on reel
    void previous_image();

    //! Changes to next image on reel and limits start/end frame to it
    void previous_image_limited();

    //! Changes to next image on reel
    void next_image();

    //! Changes to next image on reel and limits start/end frame to it
    void next_image_limited();

    //! Clears reel->images array and rebuilds it from the tree
    void match_tree_order();

    //! Inserts a media at a certain index in tree
    void insert( int idx, mrv::media m );

    //! Adds media to tree
    mrv::media add( const mrv::media m );

    //! Adds image to tree
    mrv::media add( CMedia* img );

    //! Addes filename as media to tree
    mrv::media add( const char* filename,
                    const int64_t start = -999999,
                    const int64_t end = -999999 );

    //! Loads files as media to tree
    void load( const LoadList& files, const bool stereo = false,
               std::string bgfile = "",
               const bool edl = false,
               const bool progressBar = false );

    //! Loads files as media to tree
    void load( const stringArray& files, const bool seqs = true,
               const bool stereo = false,
               const std::string bgfile = "",
               const bool edl = false,
               const bool progressBar = false );

    //! Replaces media at index idx in tree with media m.
    void replace( int idx, mrv::media m );

    //! Remove media m from reel image list and tree
    void remove( mrv::media m );

    //! Remove media at index idx from reel image list and tree
    void remove( int idx );

    //! Refresh media's icon
    void refresh( mrv::media img );

    //! Seek to a certain frame in reel EDL
    void seek( const int64_t f );

    //! Sets frame to a certain frame in reel EDL
    void frame( const int64_t f );

    //! Handles clearing, setting and toggling EDL
    void clear_edl();
    void set_edl();
    void toggle_edl();

    // Clear all reels
    void clear_reels();

    // Clear tree items
    void clear_items();

    //! Clear bg image
    void clear_bg();

    //! Set bg image to media bg
    void set_bg( mrv::media bg );

    //! Change background to nothing
    void change_background();

    //! Attach icc profile to current image
    void attach_icc_profile();

    //! Attach ctl script to current image
    void attach_ctl_script();

    //! Handle Drag and Drop of images into this tree's reel
    void handle_dnd();

    //! Sets and returns the current index of the selected image
    void value( int idx );
    inline int value() const { return _value; }

    virtual void draw();
    virtual int handle( int event );

    void main( ViewerUI* m ) {
        uiMain = m;
    }
    ViewerUI* main() {
        return uiMain;
    }

    void send_image( int idx );

    //! Exchange image in oldsel with image in sel
    void exchange( int oldsel, int sel );

public:
    void dnd_text( const std::string text ) { _dnd_text = text; }

    //! Create a tree element from a media
    mrv::Element* new_item(mrv::media img);

    //! Adjust timeline of media clips when in EDL mode, returning first
    //! and last frame in timeline
    void adjust_timeline( int64_t& first, int64_t& last );

    void set_timeline( const int64_t& first, const int64_t& last );

    //! Returns GUI view window
    mrv::ImageView* view() const;

    //! Set item from a media or NULL if not found
    Fl_Tree_Item* media_to_item( const mrv::media m );

    //! Add all menus and submenus to RMB or to menu bar
    void add_menu( Fl_Menu_* menu );

protected:


    //! Set pathname from a media
    std::string media_to_pathname( const mrv::media m );

    //! These functions send reel, images o current image to network
    void send_reel( const mrv::Reel& r );
    void send_current_image( int64_t idx, const mrv::media& m );

    //! Change reel to new one selected
    void change_reel();

    //! Load a stereo image for fg
    void load_stereo( mrv::media& fg,
                      const char* name,
                      const int64_t first, const int64_t last,
                      const int64_t start, const int64_t end,
                      const double fps );

    //! Load an image
    mrv::CMedia* load_image( const char* name,
                             const int64_t first, const int64_t last,
                             const int64_t start, const int64_t end,
                             const double fps,
                             const bool avoid_seq = false );

    //! Load an image and store it at the end of current reel
    mrv::media load_image_in_reel( const char* name,
                                   const int64_t first, const int64_t last,
                                   const int64_t start, const int64_t end,
                                   const double fps,
                                   const bool avoid_seq = false );



    //! Handle mouse drag
    int mouseDrag( int x, int y );
    //! Handle mouse push
    int mousePush( int x, int y );
    //! Handle mouse release
    int mouseRelease( int x, int y );

    //! Returns GUI timeline
    mrv::Timeline* timeline();

    //! Returns GUI EDL group
    mrv::EDLGroup* edl_group() const;

    bool           _loading; // set to on when loading a reel or an otio file
    unsigned       _reel;
    mrv::ReelList  _reels;
    mrv::Choice*  _reel_choice;
    int           _value;
    std::string   _dnd_text;

    CMedia::Mutex   _mtx;
    Fl_Tree_Item*    dragging;
    Fl_Tree_Item*    old_dragging;
    int lastX, lastY;

    ViewerUI* uiMain;
};

} // namespace mrv

// Callbacks
void attach_ocio_ics_cb2( const std::string& ret, mrv::ImageBrowser* v );

#endif  // mrvImageBrowser_h
