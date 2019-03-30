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
#ifndef mrvEDLGroup_h
#define mrvEDLGroup_h

#include <FL/Fl_Browser.H>

#include "mrvMediaTrack.h"

class ViewerUI;

namespace mrv {

class Timeline;
class Element;
class ImageBrowser;
class ImageView;
class audio_track;
typedef audio_track* audio_track_ptr;

class EDLGroup : public Fl_Group
{
public:
    typedef std::vector< audio_track_ptr >  AudioTrack;

public:

    EDLGroup(int x, int y, int w, int h);
    ~EDLGroup();

    // Add a media track and return its index.
    // index can be negative 1 to indicate a missing reel
    unsigned add_media_track( int reel_idx );

    // Add an audio only track and return its index
    unsigned add_audio_track();

    // Return the number of media tracks
    unsigned number_of_media_tracks();

    // Return the number of audio only tracks
    unsigned number_of_audio_tracks();

    // Return a media track at index i
    mrv::media_track* media_track( int i )
    {
        if ( i < 0 || i >= children() ) return NULL;
	return (mrv::media_track*)this->child(i);
    }

    bool shift_audio( unsigned track_idx, std::string image,
                      boost::int64_t offset );

    bool shift_media_start( unsigned track_idx, std::string image,
                            boost::int64_t diff );

    bool shift_media_end( unsigned track_idx, std::string image,
                          boost::int64_t diff );

    // Return an audio track at index i
    audio_track_ptr& audio_track( unsigned i );

    // Remove a media track at index i
    void remove_media_track( unsigned i );

    // Remove an audio track at index i
    void remove_audio_track( unsigned i );

    void timeline( mrv::Timeline* t ) {
        _timeline = t;
    }
    mrv::Timeline* timeline() const {
        return _timeline;
    }

    void cut( boost::int64_t frame );

    void merge( boost::int64_t frame );

    void zoom( double x );

    void refresh();

    void main(ViewerUI* m) {
        uiMain = m;
    }
    ViewerUI* main() {
        return uiMain;
    }

    mrv::ImageBrowser* browser() const;
    mrv::ImageView* view() const;

    virtual int  handle( int event );
    virtual void draw();

    void pan(int value);

protected:
    double     _zoom;
    mrv::Element* _drag;
    int        _dragX;
    int        _dragY;
    int        _dragChild;
    AudioTrack _audio_track;
    ViewerUI* uiMain;
    mrv::Timeline* _timeline;
};

}

#endif  // mrvEDLGroup_h

