/**
 * @file   mrvImageView.h
 * @author gga
 * @date   Sat Jul  7 13:49:25 2007
 * 
 * @brief  An image view class that draws an image of viewer
 * 
 * 
 */

#ifndef mrvImageView_h
#define mrvImageView_h

#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>


#include <fltk/GlWindow.h>  // this should be just Window

#include "core/mrvRectangle.h"
#include "core/mrvTimer.h"

#include "mrvChannelType.h"
#include "gui/mrvMedia.h"

#include "video/mrvGLShape.h"

namespace fltk {
  class Menu;
}


namespace mrv {

  class ViewerUI;
  class ImageBrowser;
  class Timeline;
  class DrawEngine;
  class Event;
  class Parser;

  class ImageView : public fltk::GlWindow
  {
  public:
    enum Actions {
      kMouseDown   = 1 << 0,
      kZoom        = 1 << 1,
      kMouseLeft   = 1 << 2,
      kMouseMiddle = 1 << 3,
      kMouseRight  = 1 << 4,
      kLeftAlt     = 1 << 5,
      kMouseMove   = 1 << 6,
    };

    enum Mode {
    kSelection = 1 << 0,
    kDraw      = 1 << 1,
    kErase     = 1 << 2,
    };

    enum Looping {
      kNoLoop   = 0,
      kLooping  = 1,
      kPingPong = 2,
    };

    enum Playback {
      kBackwards = -1,
      kStopped = 0,
      kForwards = 1,
      kScrubbing = 4,
    };

    enum FieldDisplay {
      kFrameDisplay,
      kTopField,
      kBottomField,
    };

    enum WipeDirection {
    kNoWipe = 0,
    kWipeVertical = 1,
    kWipeHorizontal = 2,
    kWipeFrozen = 4
    };

       enum FlipDirection {
       kFlipNone = 0,
       kFlipHorizontal = 1,
       kFlipVertical   = 2,
       };

    enum HudDisplay {
      kHudNone          = 0,
      kHudFilename      = 1 << 0,
      kHudDirectory     = 1 << 1,
      kHudFrame         = 1 << 2,
      kHudFrameRange    = 1 << 3,
      kHudResolution    = 1 << 4,
      kHudFPS           = 1 << 5,
      kHudIPTC          = 1 << 6,
      kHudAVDifference  = 1 << 7,
      kHudTimecode      = 1 << 8,
      kHudWipe          = 1 << 9,
    };

    enum PixelDisplay {
      kRGBA_Float,
      kRGBA_Hex,
      kRGBA_Decimal
    };


  public:
    ImageView(int X, int Y, int W, int H, const char *l=0);
  
    virtual ~ImageView();

    /// Handle an event in the window
    virtual int  handle(int);

    /// Draw the widget and its contents
    virtual void draw();

    // Set the display to frame or specific field
    void field( const FieldDisplay p );

    FieldDisplay field() const { return _field; }

    // Set the current frame rate for sequences in viewer
    void  fps( double x );

    /// Return the viewer's gamma
    float gamma() const { return _gamma; }

    /// Change the viewer's gamma
    void gamma(const float f);

    /// Return current gain (exposure) settings
    float gain() const { return _gain; }

    /// Set a new gain (exposure) setting
    void gain(const float f);

    /// Fit image in viewer's window, changing zoom.
    void fit_image();

    /// True if image needs updating, false if not.  fg image may change.
    bool should_update( mrv::media& fg );

    /// Change gain by changing exposure value.
    void exposure_change( float d );

    /// Returns current frame number in view
    int64_t frame() const;

    /// Change frame number to +/- N frames, respecting
    /// loop and timeline endings
    void step_frame( int64_t n );

    /// Change frame number to first frame of image
    void first_frame();

    /// Change frame number to last frame of image
    void last_frame();

    /// Change frame number to end of timeline
    void first_frame_timeline();

    /// Change frame number to end of timeline
    void last_frame_timeline();

    /// Seek to a specific frame (like frame, but may be more efficient)
    void seek( const int64_t frame );

    /// Change frame number for image
    void frame( const int64_t frame );

    /// Change the audio volume [0..1]
    void volume( float v );

    /// Change channel shown in viewer
    void channel( unsigned short c );
    unsigned short channel() const { return _channel; };

    void old_channel( unsigned short c ) { _old_channel = c; };
    unsigned short old_channel() const { return _old_channel; };

    /// Return current channel shown in viewer
    ChannelType channel_type() const { return _channelType; };

    /// Change viewer's current foreground image
    void foreground( mrv::media img );

    /// Return viewer's current foreground image
    mrv::media foreground() const { return _fg; }

    /// Change viewer's current background image
    void background( mrv::media img );

    /// Return viewer's current background image
    mrv::media background() const { return _bg; };

    /// Toggle background image on and off
    void toggle_background();

    /// Toggle pixel ratio compensation on and off
    void toggle_pixel_ratio();

    /// Set a new zoom factor
    void zoom( float x );

    /// Return current zoom factor
    float zoom() const { return _zoom; };

    /// Resize main window to current displayed image
    void resize_main_window();

    /// Update the layer list
    void update_layers();

    /// Turn on or off safe areas
    void safe_areas( const bool t ) { _safeAreas = t; }

    /// Return status of safe areas
    bool safe_areas() { return _safeAreas; }
  
    /// Normalize value
    bool normalize() const;

    /// Toggle pixel normalization on and off
    void toggle_normalize();

    /// Toggle 3D LUT on and off
    void toggle_lut();

    /// True if 3D LUT is on, false if not.
    bool use_lut() const { return _useLUT; }

    /// True if 3D LUT is on, false if not.
    void use_lut(const bool t) { _useLUT = t; }

    /// True if background is active
    bool show_background() const { return _showBG; }

    /// Update the image window display
    void update_image_info() const;

    /// Update the color information display
    void update_color_info( const mrv::media& fg ) const;
    void update_color_info() const;

    /// Set Playback looping mode
    void  looping( Looping x );

    /// Return Playback looping mode
    Looping  looping() const { return _looping; }

    /// Set Playback status/direction
    void playback( Playback b );

    /// Return Playback status
    Playback playback() const { return _playback; }

    void play( const CMedia::Playback dir );

    /// Play forwards
    void play_forwards();

    /// Play backwards
    void play_backwards();

    /// Scrub sequence
    void scrub( float dy );

    /// Stop
    void stop();

    /// Change audio stream
    void audio_stream( unsigned int idx );

    /// Attaches main window class to this viewer
    void main( mrv::ViewerUI* b ) { uiMain = b; }

    /// Returns the main window class associated to this view
    mrv::ViewerUI* main() { return uiMain; }

    /// Returns the main window class associated to this view
    const mrv::ViewerUI* main() const { return uiMain; }

    /// Auxiliary function to return viewer's main fltk window
    fltk::Window* fltk_main();

    /// Auxiliary function to return viewer's main fltk window
    const fltk::Window* fltk_main() const;

    /// Auxiliary function to return reel list's browser for this view
    ImageBrowser* browser();

    /// Auxiliary function to return timeline for this view
    Timeline* timeline();

    /// Copy pixel values of image under cursor
    void copy_pixel() const;

    // Toggle between fullscreen and normal resolution
    void toggle_fullscreen();

    inline double offset_x() const { return xoffset; }
    inline double offset_y() const { return yoffset; }
    double pixel_ratio() const;

    DrawEngine* const engine() const { return _engine; }

    FlipDirection flip() const { return _flip; }

    float masking() const { return _masking; }
    void masking( float f ) { _masking = f; }

    HudDisplay hud() const         { return _hud; }
    void hud( const HudDisplay x ) { _hud = x; }

    void timeout();

    const mrv::Rectd& selection() { return _selection; }

    /// Refresh audio tracks
    void refresh_audio_tracks() const;

       WipeDirection wipe_direction() const { return _wipe_dir; }
       float wipe_amount() const { return _wipe; }


       void selection_mode() { _mode = kSelection; }
       void draw_mode()      { _mode = kDraw; }
       void erase_mode()     { _mode = kErase; }

       bool has_redo() { return (_undo_shapes.size() > 0); }
       bool has_undo() { return (_shapes.size() > 0); }

       void undo_draw();
       void redo_draw();

     public:
       mrv::Parser*   _client;
       mrv::Parser*   _server;

  protected:

    void stop_playback();

    void create_timeout( double t );
    void delete_timeout();

    void damage_contents();

    void leftMouseDown(int,int);
    void leftMouseUp(int,int);
    void mouseDrag(int,int);
    void mouseMove(int,int);
    int  keyDown(unsigned int);
    int  keyUp(unsigned int);

       void draw_text( unsigned char r, unsigned char g, unsigned char b,
		       double x, double y, const char* text );
		       

    /// Create thumbnails for images
    void thumbnails();

    /// Initializes the draw engine (opengl, for example)
    void init_draw_engine();

    /// Set a new value for the fstop/exposure display
    void set_fstop_display( float exposure, float fstop ) const;

    /// Set a new zoom factor, but keep relative mouse position the same
    void zoom_under_mouse( float z, int x, int y );

    /// Prepare the foreground for opengl
    void bind_foreground();

    /// Prepare the background for opengl
    void bind_background();

    /// Calculate exposure value from gain
    float calculate_exposure() const;

    /// Calculate fstop value from exposure
    float calculate_fstop( float exposure ) const;

    /// Given two window coordinates, return pixel coordinates
    void image_coordinates( const mrv::image_type_ptr& img,
			    double& x, double& y ) const;

    /// Clear image sequence caches
    void flush_caches();

    /// Refresh only if not a hardware shader, otherwise just redraw
    void smart_refresh();

    /// Refresh the view images
    void refresh();

    /// Resize background image to fit foreground image's dimensions
    void resize_background();

    /// Refresh the fstop display
    void refresh_fstop() const;


  protected:
    mrv::ViewerUI* uiMain;
    mrv::DrawEngine*    _engine;


    bool         _normalize;   //<- normalize pixel values
    bool         _safeAreas;   //<- safe view/title area is active
    HudDisplay   _hud;         //<- hud display
    float        _masking;     //<- film masking ratio (top/bottom bars)
    WipeDirection _wipe_dir;   //<- wipe direction
    float         _wipe;       //<- wipe between A and B image [0..1]
  
    float        _gamma;      //<- display gamma
    float        _gain;       //<- display gain (exposure)
    float	 _zoom;       //<- display zoom
    double	 xoffset, yoffset; //<- display offsets




    //
    // Old state for actions
    //
    int           posX, posY;  //<- non-fullscreen window position
       double          X, Y;   //<- draw cursor coordinates
    int		lastX, lastY;  //<- last mouse coordinates
    int		       flags;  //<- flags containing current user action

    //! Channel index
    unsigned short     _channel;
    unsigned short     _old_channel; // previously selected color channel

    //! Channel type (RGB, R, G, B, A, L )
    mrv::ChannelType   _channelType;

    //! Flags for state of display - unneeded?, should use uiMain->uiLUT, etc.
    FieldDisplay  _field;
    bool          _showBG, _showPixelRatio, _useLUT;
    float         _volume;
    FlipDirection _flip;

    ///////////////////
    // Popup menu
    ///////////////////
    fltk::Menu*  _menu;

    // Event Timeout
    mrv::Event*  _timeout;

    ///////////////////
    // Foreground and background images in view
    ///////////////////
    mrv::media _fg;
    mrv::media _bg;

    ///////////////////
    // Rectangle selection
    ///////////////////
    Mode _mode;

    GLShapeList _undo_shapes;
    GLShapeList _shapes;
    mrv::Rectd _selection;

    ///////////////////
    // Playback state
    ///////////////////
    Playback   _playback;         //!< status of view
    Looping    _looping;          //!< looping mode of view -- unneeded
                                  //!< we should use uiMain->uiLoopMode instead

    ///////////////////
    // FPS calculation
    ///////////////////
    mrv::Timer   _timer;        //!< timer used to calculate fps
    double       _real_fps;     //!< current fps
    double       _last_fps;     //!< last fps
    int          _redraws_fps;  //!< # of redraws done for fps calculation
    int64_t      _lastFrame;    //!< last frame for fps calculation

    mrv::Timer   _dtimer;
  };

  void should_update_cb( ImageView* data );

} // namespace mrv




#endif // mrvImageView_h
