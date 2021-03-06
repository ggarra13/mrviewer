/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2020  Gonzalo Garramuño

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

#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Gl_Window.H>  // this should be just Window

#include <ImfAttribute.h>

#include "core/mrvRectangle.h"
#include "core/mrvTimer.h"
#include "core/mrvServer.h"
#include "core/mrvClient.h"
#include "core/Sequence.h"

#include "core/mrvChannelType.h"
#include "gui/mrvMedia.h"
#include "gui/mrvReel.h"

#include "video/mrvGLShape.h"

class ViewerUI;

void open_dir_cb( Fl_Widget* o, mrv::ImageBrowser* uiReelWindow );
void open_cb( Fl_Widget* o, mrv::ImageBrowser* uiReelWindow );
void open_single_cb( Fl_Widget* o, mrv::ImageBrowser* uiReelWindow );

namespace mrv {


class MainWindow;
class ImageBrowser;
class Timeline;
class DrawEngine;
class Event;
class Parser;
class server;

void modify_sop_sat_cb( Fl_Widget* w, mrv::ImageView* view );
void attach_ctl_idt_script_cb( Fl_Widget* o, ImageBrowser* v );
void attach_ctl_rrt_script_cb( Fl_Widget* o, ImageBrowser* v );

class ImageView : public Fl_Gl_Window
{
public:
    enum CommandType
    {
        kNoCommand = 0,
        kCreateReel = 1,
        kLoadImage  = 2,
        kInsertImage = 3,
        kChangeImage = 4,
        kBGImage     = 5,
        kFGReel      = 6,
        kBGReel      = 7,
        kStopVideo   = 8,
        kSeek        = 9,
        kPlayForwards = 10,
        kPlayBackwards = 11,
        kRemoveImage   = 12,
        kExchangeImage = 13,
        kCacheClear    = 14,
        kICS           = 15,
        kRT            = 16,
        kGAIN          = 17,
        kGAMMA         = 18,
        kChangeChannel = 19,
        kTimelineMin   = 20,
        kTimelineMax   = 21,
        kTimelineMinDisplay = 22,
        kTimelineMaxDisplay = 23,
        kFULLSCREEN    = 24,
        kPRESENTATION  = 25,
        kMEDIA_INFO_WINDOW_SHOW = 26,
        kMEDIA_INFO_WINDOW_HIDE = 27,
        kCOLOR_AREA_WINDOW_SHOW = 28,
        kCOLOR_AREA_WINDOW_HIDE = 29,
        k3D_VIEW_WINDOW_SHOW = 30,
        k3D_VIEW_WINDOW_HIDE = 31,
        kHISTOGRAM_WINDOW_SHOW = 32,
        kHISTOGRAM_WINDOW_HIDE = 33,
        kVECTORSCOPE_WINDOW_SHOW = 34,
        kVECTORSCOPE_WINDOW_HIDE = 35,
        kWAVEFORM_WINDOW_SHOW = 36,
        kWAVEFORM_WINDOW_HIDE = 37,
        kSTEREO_OPTIONS_WINDOW_SHOW = 38,
        kSTEREO_OPTIONS_WINDOW_HIDE = 39,
        kPAINT_TOOLS_WINDOW_SHOW = 40,
        kPAINT_TOOLS_WINDOW_HIDE = 41,
        kLUT_CHANGE = 42,
        kZoomChange = 43,
        kOCIOViewChange = 44,
        kSwitch_FG_BG = 45,
        kCOLOR_CONTROL_WINDOW_SHOW = 46,
        kCOLOR_CONTROL_WINDOW_HIDE = 47,
        kLastCommand
    };

    enum TextureFiltering
    {
        kPresentationOnly = 0,
        kBilinearFiltering = 1,
        kNearestNeighbor = 2
    };

    enum Actions {
        kMouseDown   = 1 << 0,
        kZoom        = 1 << 1,
        kGain        = 1 << 2,
        kMouseLeft   = 1 << 3,
        kMouseMiddle = 1 << 4,
        kMouseRight  = 1 << 5,
        kLeftAlt     = 1 << 6,
        kLeftShift   = 1 << 7,
        kLeftCtrl    = 1 << 8,
        kMouseMove   = 1 << 9,
        kGamma       = 1 << 10,
    };

    enum Mode {
        kNoAction  = 0,
        kScrub     = 1 << 0,
        kSelection = 1 << 1,
        kTemporary = 1 << 2,
        kSelectionTemporary = kSelection | kTemporary,
        kDraw      = 1 << 3,
        kDrawTemporary      = kDraw | kTemporary,
        kErase     = 1 << 4,
        kEraseTemporary     = kErase | kTemporary,
        kText      = 1 << 5,
        kMovePicture = 1 << 6,
        kScalePicture = 1 << 7,
        kCircle       = 1 << 8,
        kArrow        = 1 << 9
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


    enum HudDisplay {
        kHudNone          = 0,
        kHudFilename      = 1 << 0,
        kHudDirectory     = 1 << 1,
        kHudFrame         = 1 << 2,
        kHudFrameRange    = 1 << 3,
        kHudResolution    = 1 << 4,
        kHudFPS           = 1 << 5,
        kHudAttributes    = 1 << 6,
        kHudAVDifference  = 1 << 7,
        kHudTimecode      = 1 << 8,
        kHudWipe          = 1 << 9,
        kHudMemoryUse     = 1 << 10,
        kHudCenter        = 1 << 11,
    };

    enum PixelValue {
        kRGBA_Full,
        kRGBA_Lut,
        kRGBA_Original
    };

    enum PixelDisplay {
        kRGBA_Float,
        kRGBA_Hex,
        kRGBA_Decimal
    };

    enum BlendMode {
        kBlendTraditional = 0,
        kBlendPremult = 1,
        kBlendTraditionalNonGamma = 2,
        kBlendPremultNonGamma = 3,
    };

    enum VRType {
        kNoVR = 0,
        kVRSphericalMap,
        kVRCubeMap,
    };

    enum WindowList
    {
        kReelWindow = 0,
        kMediaInfo = 1,
        kColorInfo = 2,
        kColorControls = 3,
        kEDLEdit = 4,
        kActionTools = 5,
        k3DStereoOptions = 6,
        k3dView = 7,
        kHistogram = 8,
        kVectorscope = 9,
        kWaveform = 10,
        kICCProfiles = 11,
        kConnections = 12,
        kPreferences = 13,
        kHotkeys = 14,
        kLogs = 15,
        kAbout = 16,
        kLastWindow
    };

public:
    ImageView(int X, int Y, int W, int H, const char *l=0);

    virtual ~ImageView();

    /// Handle an event in the window
    virtual int  handle(int);

    /// Draw the widget and its contents
    virtual void draw();

    // Set the texturing filtering
    void texture_filtering( const TextureFiltering p );

    inline TextureFiltering texture_filtering() const
        {
            return _texture_filtering;
        }

    // Set the display to frame or specific field
    void field( const FieldDisplay p );

    inline FieldDisplay field() const {
        return _field;
    }

    // Set the current frame rate for sequences in viewer
    void  fps( double x );

    double fps() const;

    inline int fg_reel() const {
        return _fg_reel;
    }
    inline int bg_reel() const {
        return _bg_reel;
    }


    void fg_reel(int idx);
    void bg_reel(int idx);

    /// Return the viewer's gamma
    inline float gamma() const {
        return _gamma;
    }

    /// Change the viewer's gamma
    void gamma(const float f);

    /// Return current gain (exposure) settings
    inline float gain() const {
        return _gain;
    }

    /// Set a new gain (exposure) setting
    void gain(const float f);

    /// Center image in viewer's window, without changing zoom.
    void center_image();

    /// Fit image in viewer's window, changing zoom.
    void fit_image();

    /// True if image needs updating, false if not.  fg image may change.
    bool should_update( mrv::media fg );

    // Handle vr mouse movements and rotate sphere/cube
    void handle_vr( double& delay );

    /// Change gain by changing exposure value.
    void exposure_change( float d );

    /// Returns current frame number in view
    int64_t frame() const;

    /// Change frame number to +/- N frames, respecting
    /// loop and timeline endings
    void step_frame( int64_t n );

    /// update ICS
    void update_ICS() const;

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

    // Return audio volue [0..1]
    inline float volume() {
        return _volume;
    }

    /// Change channel shown in viewer
    void channel( Fl_Menu_Item* w ); // widget is one of the menus or submenus
    void channel( unsigned short c );
    inline unsigned short channel() const {
        return _channel;
    };

    char* get_layer_label(unsigned short c);

    void old_channel( unsigned short c ) {
        _old_channel = c;
    };
    inline unsigned short old_channel() const {
        return _old_channel;
    };

    /// Return current channel shown in viewer
    inline ChannelType channel_type() const {
        return _channelType;
    };

    /// Change stereo main image and attach B image
    void change_foreground();

    /// Change viewer's current foreground image
    void foreground( mrv::media img );

    /// Return viewer's current foreground image
    inline mrv::media foreground() const {
        return _fg;
    }

    /// Change viewer's current background image
    void background( mrv::media img );

    /// Return viewer's current background image
    inline mrv::media background() const {
        return _bg;
    };

    /// Toggle background image on and off
    void toggle_background();

    /// Toggle pixel ratio compensation on and off
    void toggle_pixel_ratio();

    bool show_pixel_ratio() const;
    void show_pixel_ratio( const bool b );

    void show_bars( ViewerUI* uiMain, bool showtop = true );

    /// Set a new zoom factor
    void zoom( float x );

    /// Return current zoom factor
    inline float zoom() const {
        return _real_zoom;
    };

    void resize( int X, int Y, int W, int H );
    inline void resize( int W, int H ) { resize( x(), y(), W, H ); }

    /// Resize main window to current displayed image
    void resize_main_window();

    /// Update the layer list
    void update_layers();

    /// Update the Display of Input Color Space of foreground image
    void update_ICS(mrv::media fg) const;

    /// Turn on or off safe areas
    void safe_areas( const bool t );

    /// Return status of safe areas
    inline bool safe_areas() {
        return _safeAreas;
    }

    /// Turn on or off display window
    void display_window( const bool t );

    /// Return status of safe areas
    inline bool display_window() const {
        return _displayWindow;
    }

    /// Turn on or off data window
    void data_window( const bool t );

    /// Return status of safe areas
    inline bool data_window() const {
        return _dataWindow;
    }

    /// Normalize value
    bool normalize() const;

    void normalize( const bool b );

    /// Toggle pixel normalization on and off
    void toggle_normalize();

    /// Normalize a pixel value
    void normalize( CMedia::Pixel& rgba, unsigned short idx = 0 ) const;

    /// Toggle 3D LUT on and off
    void toggle_lut();

    /// True if 3D LUT is on, false if not.
    inline bool use_lut() const {
        return _useLUT;
    }

    /// Set 3D LUT to on, false if not.
    inline void use_lut(const bool t) {
        _useLUT = !t;
        toggle_lut();
    }

    /// True if background is active
    inline bool show_background() const {
        return _showBG;
    }

    void show_background( const bool b );

    inline void update( bool t ) {
        _update = t;
    }

    /// Update the image window display
    void update_image_info() const;

    /// Update the color information display
    void update_color_info( const mrv::media fg ) const;
    void update_color_info() const;

    // Channel navigation (for hotkeys)
    void switch_channels();
    bool next_channel();
    bool previous_channel();

    /// Get Playback looping mode from timeline widget
    CMedia::Looping looping() const;

    /// Set Playback looping mode
    void  looping( CMedia::Looping x );

    /// Set Playback status/direction
    void playback( CMedia::Playback b );

    /// Return Playback status
    inline CMedia::Playback playback() const {
        return _playback;
    }

    void play( const CMedia::Playback dir );

    /// Play forwards
    void play_forwards();

    /// Play backwards
    void play_backwards();

    /// Scrub sequence
    void scrub( double dy );

    /// Stop
    void stop();

    /// Change audio stream
    void audio_stream( unsigned int idx );

    ///
    void vr( VRType t );
    inline VRType vr() const {
        return _vr;
    }

    float vr_angle() const;
    void vr_angle( const float t );

    /// Attaches main window class to this viewer
    void main( ViewerUI* b ) {
        uiMain = b;
    }

    /// Returns the main window class associated to this view
    inline ViewerUI* main()  {
        return uiMain;
    }

    /// Returns the main window class associated to this view
    inline const ViewerUI* main() const {
        return uiMain;
    }

    /// Auxiliary function to return viewer's main fltk window
    MainWindow* fltk_main();

    /// Auxiliary function to return viewer's main fltk window
    const MainWindow* fltk_main() const;

    void toggle_window( const WindowList idx, const bool force = false );

    /// Auxiliary function to return reel list's browser for this view
    ImageBrowser* browser();

    /// Auxiliary function to return timeline for this view
    Timeline* timeline();

    /// Copy pixel values of image under cursor
    void copy_pixel() const;

    // Toggle between fullscreen and normal resolution
    void toggle_fullscreen();

    // Toggle between fullscreen presentation and normal resolution
    void toggle_presentation();

    void toggle_media_info(bool show);
    void toggle_color_control(bool show);
    void toggle_color_area(bool show);
    void toggle_stereo_options(bool show);
    void toggle_paint_tools(bool show);
    void toggle_3d_view(bool show);
    void toggle_histogram(bool show);
    void toggle_vectorscope(bool show);
    void toggle_waveform(bool show);

    void toggle_wait() {
        _wait ^= 1;
    }

    void stereo_input( CMedia::StereoInput x );
    void stereo_output( CMedia::StereoOutput x );
    CMedia::StereoInput stereo_input() const;
    CMedia::StereoOutput stereo_output() const;

    inline void offset_x( double x ) {
        xoffset = x;
    }
    inline void offset_y( double y ) {
        yoffset = y;
    }

    inline double offset_x() const {
        return xoffset;
    }
    inline double offset_y() const {
        return yoffset;
    }

    void rot_x( double x );
    void rot_y( double x );

    double rot_x() const;
    double rot_y() const;

    inline void spin_x( double x ) {
        spinx = x;
    }
    inline void spin_y( double x ) {
        spiny = x;
    }

    inline double spin_x() const {
        return spinx;
    }
    inline double spin_y() const {
        return spiny;
    }
    double pixel_ratio() const;

    DrawEngine* const engine() const {
        return _engine;
    }

    float masking() const {
        return _masking;
    }
    void masking( float f ) {
        _masking = f;
    }

    HudDisplay hud() const         {
        return _hud;
    }
    void hud( const HudDisplay x ) {
        _hud = x;
    }

    // Handle network commands encoded in stream
    void handle_commands();

    void handle_timeout();

    void timeout();

    /// Refresh the view images
    void refresh();

    CMedia* selected_image() const {
        return _selected_image;
    }
    void select_image(CMedia* img) {
        _selected_image = img;
        redraw();
    }

    void selection( const mrv::Rectd& r );
    const mrv::Rectd& selection() {
        return _selection;
    }

    /// Refresh audio tracks
    void refresh_audio_tracks() const;

    void wipe_direction( const WipeDirection& w ) {
        _wipe_dir = w;
    }
    WipeDirection wipe_direction() const {
        return _wipe_dir;
    }
    void wipe_amount(float w) {
        _wipe = w;
    }
    float wipe_amount() const {
        return _wipe;
    }



    // Auxiliary function to change x and y of a vector due to a
    // rotation in degrees
    static void rot2vec( double& x, double& y, const double r );

    int  keyDown(unsigned int);

    void text_mode();
    void scrub_mode();
    void selection_mode( bool temporary = false );
    void draw_mode( bool temporary = false );
    void erase_mode( bool temporary = false );
    void circle_mode();
    void arrow_mode();
    void move_pic_mode();
    void scale_pic_mode();

    bool has_redo() const;
    bool has_undo() const;

    void undo_draw();
    void redo_draw();

    bool network_active() const {
        return _network_active;
    }
    void network_active( const bool b ) {
        _network_active = b;
    }

    void send_network( std::string msg ) const;

    GLShapeList& shapes();

    void add_shape( shape_type_ptr shape );

    void ghost_previous( short x );

    void ghost_next( short x );

    short ghost_previous() const {
        return _ghost_previous;
    }
    short ghost_next()     const {
        return _ghost_next;
    }

    // Make pre-loading start from scratch
    void reset_caches();

    /// Clear image sequence caches from reel idx
    void clear_reel_cache( size_t idx );

    /// Start preload image caches
    void preload_cache_start();

    int64_t preload_frame() const {
        return _preframe;
    }

    void preload_frame( int64_t f ) {
        _preframe = f;
    }

    bool grid() const { return _grid; }
    void grid(bool t) { _grid = t; redraw(); }

    unsigned grid_size() const { return _grid_size; }
    void grid_size(unsigned t) { _grid_size = t; redraw(); }

    /// Stop preload image caches
    void preload_cache_stop();

    // Return idle callback state for cache preload
    bool idle_callback() const {
        return _idle_callback;
    }

    /// Preload image caches
    void preload_caches();

    /// Clear image caches regardless of anything
    void clear_caches();

    /// Clear image sequence caches from reel idx if shadertype == kNone
    void flush_image( mrv::media fg );

    /// Clear image sequence caches if shadertype == kNone
    void flush_caches();

    // Preload an image into sequence cache
    bool preload();

    // Return if in presentation mode or not
    bool in_presentation() const;

    inline Mode action_mode() const {
        return _mode;
    }


    /// Fill menu based on context information
    void fill_menu( Fl_Menu_* menu );


public:
    // Auxiliary function to set the offsets after a rotation of x degrees.
    // This function is used in fit_image and center_image.
    static
    void zrotation_to_offsets( double& X, double& Y, const CMedia* img,
                               const int W, const int H );

public:
    typedef CMedia::Mutex Mutex;
    struct Command {
        Command() : type( kNoCommand ),
                    frame(AV_NOPTS_VALUE),
                    data(NULL),
                    linfo(NULL)
            {
            };
        Command( const Command& b ) : type( b.type ),
                                      frame( b.frame ), data( b.data ),
                                      linfo( b.linfo ) { };
        ~Command() {}

        inline Command& operator=( const Command& b )
            {
                type = b.type;
                frame = b.frame;
                data = b.data;
                linfo = b.linfo;
                return *this;
            }
        CommandType     type;
        int64_t         frame;
        Imf::Attribute* data;
        LoadInfo*       linfo;
    };

    Mutex                 commands_mutex;
    std::deque<Command>   commands;
    bool           _broadcast;
    CMedia::Mutex  _clients_mtx;
    ParserList     _clients;
    tcp_server_ptr _server;

protected:


    void pixel_processed( const CMedia* img, CMedia::Pixel& rgba ) const;

    void stop_playback();

    void create_timeout( double t );
    void delete_timeout();

    void damage_contents();

    int  leftMouseDown(int,int);
    void leftMouseUp(int,int);
    void mouseDrag(int,int);
    void mouseMove(int,int);
    int  keyUp(unsigned int);

    int update_shortcuts( const mrv::media fg, const char* channelName );

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
    void image_coordinates( const CMedia* const img,
                            double& x, double& y ) const;

    /// Given two window coordinates, return pixel coordinates
    /// in the data window (which may be offset)
    void data_window_coordinates( const CMedia* const img,
                                  double& x, double& y ) const;


    /// Given two window coordinates, return pixel coordinates
    /// in the returned picture (or outside set to true)
    void picture_coordinates( const CMedia* const img, const int x,
                              const int y, bool& outside,
                              mrv::image_type_ptr& pic,
                              int& xp, int& yp, int& w, int& h,
                              int off[2] ) const;

    /// Refresh only if not a hardware shader, otherwise just redraw
    void smart_refresh();

    // Ready preframe for next iteration
    bool ready_preframe( std::atomic<int64_t>& _preframe,
                         CMedia::Playback p, CMedia* const img,
                         const int64_t& first,
                         const int64_t& last );

    /// Resize background image to fit foreground image's dimensions
    void resize_background();

    /// Refresh the fstop display
    void refresh_fstop() const;

    void separate_layers( const CMedia* const img,
                          mrv::image_type_ptr& pic, int& xp, int& yp,
                          short& idx, bool& outside, int w, int h,
                          const mrv::Recti& dpw ) const;
    void top_bottom( const CMedia* const img,
                     mrv::image_type_ptr& pic, int& xp, int& yp,
                     short& idx, bool& outside, int w, int h ) const;
    void left_right( const CMedia* const img,
                     mrv::image_type_ptr& pic, int& xp, int& yp,
                     short& idx, bool& outside, int w, int h ) const;

    void restore_locale() const;

    void log() const;

protected:
    ViewerUI* uiMain;
    mrv::DrawEngine*    _engine;

    bool         _update;    //<- Freeze opengl updates when not set
    bool         _wait;
    bool         _normalize;   //<- normalize pixel values
    bool         _safeAreas;   //<- safe view/title area is active
    bool         _grid;        //<- grid is active
    bool         _zoom_grid;     //<- zoom grid is active
    unsigned     _grid_size;   //<- size of grid in pixels
    HudDisplay   _hud;         //<- hud display
    float        _masking;     //<- film masking ratio (top/bottom bars)
    WipeDirection _wipe_dir;   //<- wipe direction
    float         _wipe;       //<- wipe between A and B image [0..1]

    float        _gamma;      //<- display gamma
    float        _gain;       //<- display gain (exposure)
    float        _zoom;       //<- display zoom
    float        _real_zoom;  //<- real display zoom ( with screen factor )
    double       xoffset, yoffset; //<- display offsets
    double     spinx, spiny;   //<- VR's rotation offsets


    //
    // Old state for actions
    //
    int           posX, posY;  //<- non-fullscreen window position
    int           sizeX, sizeY; //<- non-fullscreen window size

    int           widthX, heightY; // <- size of opengl view

    double          X, Y;   //<- draw cursor coordinates
    int		lastX, lastY;  //<- last mouse coordinates
    int                flags;  //<- flags containing current user action

    short       _ghost_previous;
    short       _ghost_next;

    //! Channel index
    unsigned short     _channel;
    unsigned short     _old_channel; // previously selected color channel

    //! Channel type (RGB, R, G, B, A, L )
    mrv::ChannelType   _channelType;

    //! Flags for state of display - unneeded?, should use uiMain->uiLUT, etc.
    FieldDisplay  _field;
    TextureFiltering _texture_filtering;
    bool          _displayWindow, _dataWindow, _showBG;
    bool          _showPixelRatio, _useLUT;
    float         _volume;
    unsigned      _reel;      // <- reel of preframe
    std::atomic< int64_t >      _preframe;
    int64_t       _old_fg_frame;  // <- old frame used to stat fileroot's fg
    int64_t       _old_bg_frame;  // <- old frame used to stat fileroot's bg
    bool          _idle_callback;

    VRType        _vr;  // Cube/Spherical VR 360



    ///////////////////
    // Popup menu
    ///////////////////
    Fl_Menu_Button* menu;

    // Event Timeout
    mrv::Event*  _timeout;

    ///////////////////
    // Foreground and background images in view
    ///////////////////

    mrv::media _fg;
    CMedia* _old_fg;
    mrv::media _bg;

    int   _fg_reel;
    int   _bg_reel;

    ///////////////////
    // Rectangle selection
    ///////////////////
    Mode _mode;

    bool     _scale; // boolean to indicate whether move tool is scaling or
    // moving
    CMedia*  _selected_image;
    mrv::Rectd _selection;

    ///////////////////
    // Playback state
    ///////////////////
    std::atomic<CMedia::Playback>   _playback;         //!< status of view
    CMedia::Playback   _orig_playback;    //!< play direction once started

    bool _network_active;  //<- whether to send commands across the network
    bool _interactive;     //<- whether fltk should update (Fl::check)

    ///////////////////
    // FPS calculation
    ///////////////////
    mrv::Timer   _timer;        //!< timer used to calculate fps
    double       _real_fps;     //!< current fps
    double       _last_fps;     //!< last fps
    int          _redraws_fps;  //!< # of redraws done for fps calculation
    int64_t      _frame;        //!< current frame in viewer
    int64_t      _lastFrame;    //!< last frame for fps calculation
    bool       _do_seek;
    CMedia::Mutex _shortcut_mutex;
    CMedia::Mutex _draw_mutex;


    mrv::Timer   _dtimer;
};

void should_update_cb( ImageView* data );

} // namespace mrv




#endif // mrvImageView_h
