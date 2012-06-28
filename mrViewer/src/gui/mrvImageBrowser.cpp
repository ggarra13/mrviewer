/**
 * @file   mrvImageBrowser.cpp
 * @author gga
 * @date   Sat Oct 14 11:53:10 2006
 * 
 * @brief  This class manages an image browser catalogue of all loaded images.
 * 
 * 
 */

#include <iostream>
#include <algorithm>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>  // for PRId64

#define BOOST_FILESYSTEM_VERSION 2
#include <boost/filesystem/path.hpp>
namespace fs = boost::filesystem;

#include <fltk/ask.h>
#include <fltk/damage.h>
#include <fltk/draw.h>
#include <fltk/events.h>
#include <fltk/Choice.h>
#include <fltk/Item.h>
#include <fltk/Menu.h>
#include <fltk/Window.h>
#include <fltk/Monitor.h>
#include <fltk/ProgressBar.h>
#include <fltk/run.h>

#include "core/stubImage.h"
#include "core/smpteImage.h"
#include "core/clonedImage.h"
#include "core/mrvColorBarsImage.h"
#include "core/slateImage.h"
#include "core/mrvLicensing.h"
#include "core/Sequence.h"
#include "core/mrvAudioEngine.h"
#include "core/mrvThread.h"

#include "db/mrvDatabase.h"

#include "gui/mrvIO.h"
#include "gui/mrvTimecode.h"
#include "gui/mrvTimeline.h"
#include "gui/mrvFileRequester.h"
#include "gui/mrvMainWindow.h"
#include "mrViewer.h"
#include "gui/mrvImageView.h"
#include "gui/mrvImageBrowser.h"

#if defined(WIN32) || defined(WIN64)
#  define snprintf _snprintf
#endif

namespace 
{
  const char* kModule = "db";
}


using namespace std;

extern void set_as_background_cb( fltk::Widget* o, mrv::ImageView* v );
extern void toggle_background_cb( fltk::Widget* o, mrv::ImageView* v );


void clone_all_cb( fltk::Widget* o, mrv::ImageBrowser* b )
{
  b->clone_all_current();
}

void clone_image_cb( fltk::Widget* o, mrv::ImageBrowser* b )
{
  b->clone_current();
}

namespace {

  void ntsc_color_bars_cb( fltk::Widget* o, mrv::ImageBrowser* b )
  {
    using mrv::ColorBarsImage;
    ColorBarsImage* img = new ColorBarsImage( ColorBarsImage::kSMPTE_NTSC );
    img->fetch(1);
    img->default_rendering_transform();
    b->add( img );
  }

  void pal_color_bars_cb( fltk::Widget* o, mrv::ImageBrowser* b )
  {
    using mrv::ColorBarsImage;
    ColorBarsImage* img = new ColorBarsImage( ColorBarsImage::kPAL );
    img->fetch(1);
    img->default_rendering_transform();
    b->add( img );
  }

  void ntsc_hdtv_color_bars_cb( fltk::Widget* o, mrv::ImageBrowser* b )
  {
    using mrv::ColorBarsImage;
    ColorBarsImage* img = new ColorBarsImage( ColorBarsImage::kSMPTE_NTSC_HDTV );
    img->fetch(1);
    img->default_rendering_transform();
    b->add( img );
  }

  void pal_hdtv_color_bars_cb( fltk::Widget* o, mrv::ImageBrowser* b )
  {
    using mrv::ColorBarsImage;
    ColorBarsImage* img = new ColorBarsImage( ColorBarsImage::kPAL_HDTV );
    img->fetch(1);
    img->default_rendering_transform();
    b->add( img );
  }

  static void gamma_chart( mrv::ImageBrowser* b, float g )
  {
    using mrv::smpteImage;

    fltk::Window* uiMain = b->main()->uiMain;
    const fltk::Monitor& m = fltk::Monitor::find(uiMain->x()+uiMain->w()/2, 
						 uiMain->y()+uiMain->h()/2);
    smpteImage* img = new smpteImage( smpteImage::kGammaChart, m.w(), m.h() );
    img->gamma(g);
    img->fetch(1);
    img->default_rendering_transform();
    img->gamma(1.0f);
    b->add( img );
  }

  void gamma_chart_14_cb( fltk::Widget* o, mrv::ImageBrowser* b )
  {
    gamma_chart(b, 1.4f);
  }
  void gamma_chart_18_cb( fltk::Widget* o, mrv::ImageBrowser* b )
  {
    gamma_chart(b, 1.8f);
  }
  void gamma_chart_22_cb( fltk::Widget* o, mrv::ImageBrowser* b )
  {
    gamma_chart(b, 2.2f);
  }
  void gamma_chart_24_cb( fltk::Widget* o, mrv::ImageBrowser* b )
  {
    gamma_chart(b, 2.4f);
  }

  void linear_gradient_cb( fltk::Widget* o, mrv::ImageBrowser* b )
  {
    using mrv::smpteImage;

    fltk::Window* uiMain = b->main()->uiMain;
    const fltk::Monitor& m = fltk::Monitor::find(uiMain->x()+uiMain->w()/2, 
						 uiMain->y()+uiMain->h()/2);
    smpteImage* img = new smpteImage( smpteImage::kLinearGradient, 
				      m.w(), m.h() );
    img->fetch(1);
    img->default_icc_profile();
    img->default_rendering_transform();
    b->add( img );
  }

  void luminance_gradient_cb( fltk::Widget* o, mrv::ImageBrowser* b )
  {
    using mrv::smpteImage;

    fltk::Window* uiMain = b->main()->uiMain;
    const fltk::Monitor& m = fltk::Monitor::find(uiMain->x()+uiMain->w()/2, 
						 uiMain->y()+uiMain->h()/2);
    smpteImage* img = new smpteImage( smpteImage::kLuminanceGradient, 
				      m.w(), m.h() );
    img->fetch(1);
    img->default_icc_profile();
    img->default_rendering_transform();
    b->add( img );
  }

  void checkered_cb( fltk::Widget* o, mrv::ImageBrowser* b )
  {
    using mrv::smpteImage;

    smpteImage* img = new smpteImage( smpteImage::kCheckered, 640, 480 );
    img->fetch(1);
    img->default_icc_profile();
    img->default_rendering_transform();
    b->add( img );
  }


  void slate_cb( fltk::Widget* o, mrv::ImageBrowser* b )
  {
    using mrv::slateImage;

    mrv::media cur = b->current_image();
    if ( !cur ) return;

    slateImage* img = new slateImage( cur->image() );
    img->fetch(1);
    img->default_icc_profile();
    img->default_rendering_transform();
    b->add( img );
  }

  /** 
   * Callback used to attach a color profile to the current image
   * 
   * @param o     ImageBrowser*
   * @param data  unused
   */
  static void attach_color_profile_cb( fltk::Widget* o, mrv::ImageBrowser* b )
  {
    b->attach_icc_profile();
  }


  static void attach_ctl_script_cb( fltk::Widget* o, mrv::ImageBrowser* b )
  {
     mrv::media fg = b->current_image();
     if ( ! fg ) return;
     
     attach_ctl_script( fg->image() );
  }
}

namespace mrv {

  class Element : public fltk::Item
  {
  public:
    Element( mrv::media& );
  };

  Element::Element( mrv::media& m ) :
    fltk::Item( m->image()->fileroot() )
  {
    m->create_thumbnail();
    image( m->thumbnail() );

    CMedia* img = m->image();

    char info[2048];
    if ( dynamic_cast< clonedImage* >( img ) != NULL )
      {
	sprintf( info, 
		 _("Cloned Image\n"
		   "Name: %s\n"
		   "Date: %s\n"
		   "Resolution: %dx%d"),
		 img->name().c_str(),
		 img->creation_date().c_str(),
		 img->width(),
		 img->height()
		 );
      }
    else
      {
	sprintf( info, 
		 _("Directory: %s\n"
		   "Name: %s\n"
		   "Resolution: %dx%d\n"
		   "Frames: %" PRId64 "-%" PRId64),
		 img->directory().c_str(),
		 img->name().c_str(),
		 img->width(),
		 img->height(),
		 img->first_frame(),
		 img->last_frame()
		 );
      }
    copy_label( info );
  }


  /** 
   * Constructor
   * 
   * @param x window's x position
   * @param y window's y position
   * @param w window's width
   * @param h window's height
   */
  ImageBrowser::ImageBrowser( int x, int y, int w, int h ) :
    fltk::Browser( x, y, w, h ),
    _reel( 0 ),
    dragging( NULL ),
    db( NULL )
  {
  }

  ImageBrowser::~ImageBrowser()
  {
    int num = children();
    for ( int j = 0; j < num; ++j )
      fltk::Browser::remove( child(0) );

    delete db;
  }

  mrv::Timeline* ImageBrowser::timeline()
  {
    assert( uiMain != NULL );
    return uiMain->uiTimeline;
  }

  /** 
   * @return current reel selected or NULL
   */
  mrv::Reel ImageBrowser::current_reel() const
  {
    assert( _reels.empty() || _reel < _reels.size() );
    return _reels.empty()? mrv::Reel() : _reels[ _reel ];
  }

  /** 
   * Change to a certain reel
   * 
   * @param idx reel index
   * 
   * @return new reel or NULL if invalid index 
   */
  mrv::Reel ImageBrowser::reel( const char* name )
  {
    mrv::ReelList::const_iterator i = _reels.begin();
    mrv::ReelList::const_iterator e = _reels.end();
    unsigned int idx = 0;
    for ( ; i != e; ++i, ++idx )
      {
	if ( (*i)->name == name ) {
	  _reel = idx;
	  change_reel();
	  return *i;
	}
      }
    return mrv::Reel();
  }


  /** 
   * Change to a certain reel
   * 
   * @param idx reel index
   * 
   * @return new reel or NULL if invalid index 
   */
  mrv::Reel ImageBrowser::reel( unsigned idx )
  {
    assert( idx < _reels.size() );
    if ( _reel == idx ) return _reels[ idx ];

    _reel = idx;
    change_reel();
    return _reels[ idx ];
  }

  /** 
   * @return current image selected or NULL
   */
  mrv::media ImageBrowser::current_image()
  {
    mrv::Reel reel = current_reel();
    if ( reel == NULL ) return mrv::media();

    int idx = value(); 
    if ( idx < 0 ) return mrv::media();

    return reel->images[idx];
  }

  /** 
   * @return current image selected or NULL
   */
  const mrv::media ImageBrowser::current_image() const
  {
    mrv::Reel reel = current_reel();
    if ( reel == NULL ) return mrv::media();

    int idx = value(); 
    if ( idx < 0 ) return mrv::media();

    return reel->images[idx];
  }

  /** 
   * Create a new reel, giving it a name
   * 
   * @param name Name of new reel
   * 
   * @return new reel created
   */
  mrv::Reel ImageBrowser::new_reel( const char* orig )
  {
    mrv::ReelList::const_iterator i = _reels.begin();
    mrv::ReelList::const_iterator e = _reels.end();
    std::string name = orig;
    int idx = 2;
    for ( ; i != e; ++i )
      {
	if ( (*i)->name == name )
	  {
	    name = orig; name += " #";
	    char buf[32];
	    sprintf( buf, "%d", idx );
	    name += buf;
	    // 	  name += itoa( idx, buf, 10 );
	    i = _reels.begin();
	    ++idx;
	  }
      }

    mrv::Reel reel( new mrv::Reel_t( name.c_str() ) );
    _reels.push_back( reel );
    assert( !_reels.empty() );

    _reel = (unsigned int) _reels.size() - 1;
    _reel_display->add( name.c_str() );
    _reel_display->value( _reel );
    if ( ! _reels.empty() ) change_reel();
    return reel;
  }

  /** 
   * Save current reel to a disk file
   * 
   */
  void ImageBrowser::save_reel()
  {
    mrv::Reel reel = current_reel();
    if ( !reel ) return;

    const char* file = mrv::save_reel();
    if ( !file ) return;

    std::string reelname( file );
    if ( reelname.substr(reelname.size()-5, 5) != ".reel" )
      {
	reelname += ".reel";
      }

    FILE* f = fopen( reelname.c_str(), "w" );

    fprintf( f, _("#\n"
		  "# mrViewer Reel \"%s\"\n"
		  "# \n"
		  "# Created with mrViewer\n"
		  "#\n\n"),
	     reel->name.c_str() );

    mrv::MediaList::iterator i = reel->images.begin();
    mrv::MediaList::iterator e = reel->images.end();
    for ( ; i != e; ++i )
      {
	const CMedia* img = (*i)->image();
	if ( img->is_sequence() )
	  {
	    fprintf( f, "%s %" PRId64 "-%" PRId64 "\n", img->fileroot(), 
		     img->first_frame(), img->last_frame() );
	    if ( img->has_audio() )
	       fprintf( f, "audio: %s\n", img->audio_file().c_str() );
	  }
	else
	  {
	    fprintf( f, "%s\n", img->fileroot() );
	  }
      }

    if ( reel->edl )
       fprintf( f, "EDL\n" );

    fclose(f);

  }

  /** 
   * Remove/Erase current reel
   * 
   */
  void ImageBrowser::remove_reel()
  {
    if ( _reels.empty() ) return;
    _reel_display->remove(_reel);
    _reels.erase( _reels.begin() + _reel );
    if ( _reels.empty() ) new_reel();
    if ( _reel >= (unsigned int)_reels.size() ) 
      _reel = (unsigned int)_reels.size() - 1;
    _reel_display->value( _reel );
    _reel_display->redraw();
    change_reel();
  }

  /** 
   * Create a new image browser's fltk::Item (widget line in browser) 
   * 
   * @param img image to create new fltk::Item for
   * 
   * @return fltk::Item*
   */
  Element* ImageBrowser::new_image( mrv::media& m )
  {
    Element* nw = new Element( m );

    nw->labelsize( 30 );
    nw->box( fltk::BORDER_BOX );
    return nw;
  }


  /** 
   * Insert a new image into image browser list
   * 
   * @param idx where to insert new image
   * @param img image to insert
   */
  void ImageBrowser::insert( unsigned idx, mrv::media& m )
  {
    if ( !m ) return;

    mrv::Reel reel = current_reel();
    if ( reel == NULL ) return;

    reel->images.insert( reel->images.begin() + idx, 
			 media( m ) );

    Element* nw = new_image( m );
    fltk::Browser::insert( *nw, idx );

    redraw();
  }


  void ImageBrowser::add_video( const mrv::media& m )
  {
    if (!db) return;

    const CMedia* img = m->image();

    char buf[4096], created_at[256];

    std::string image_id;
    sprintf( buf, N_("SELECT id FROM images "
		     "WHERE directory='%s' AND filename='%s'"),
	     db->quote( img->directory() ).c_str(),
	     db->quote( img->name() ).c_str() );
    image_id = buf;

    time_t t = ::time( NULL );
    strftime( created_at, 256, N_("%F %H:%M:%S"), localtime( &t ) );

    unsigned num_video_streams = img->number_of_video_streams();
    for ( unsigned i = 0; i < num_video_streams; ++i )
      {
	 sprintf( buf, N_("SELECT id FROM videos "
			  "WHERE image_id=( %s ) AND stream=%d;"),
		 image_id.c_str(), i );
	if (! db->sql( buf ) )
	  {
	    LOG_ERROR( _("Could not find video '") << img->filename() 
		       << _("' in database '") << db->database() 
		       << "': " << endl << db->error() );
	    return;
	  }
	else
	  {
	    std::string result;
	    db->result( result );

	    // It already exists in database, do not add it again
	    if ( result != "" ) continue;
	  }

	const CMedia::video_info_t& s = img->video_info(i);


	std::string pixel_format_id = "NULL";
	if ( s.pixel_format.c_str() )
	{
	   char buf[4096];
	   sprintf( buf, N_("INSERT INTO pixel_formats(name)"
			    " VALUES "
			    "( '%s' );"), 
		    db->quote( s.pixel_format ).c_str() ); 
	   if ( db->sql( buf ) )
	   {
	      LOG_INFO( img->filename() << _(": added pixel format '") << 
			s.pixel_format << _("' to database '") 
			<< db->database() << _("'.") );
	   }
	   
	   pixel_format_id = N_("SELECT id FROM pixel_formats "
				"WHERE name='");
	   pixel_format_id += db->quote( s.pixel_format );
	   pixel_format_id += N_("'");
	}


	sprintf( buf, N_("INSERT INTO "
			 "videos"
			 "( image_id, stream, codec, "
			 "created_at, updated_at, "
			 "fourcc, pixel_format_id, fps, start, duration )"
			 " VALUES "
			 "( ( %s ), %d, '%s', "
			 "'%s', '%s', "
			 "'%s', ( %s ), %g, %g, %g );" ),
		 image_id.c_str(), i,
		 db->quote( s.codec_name ).c_str(),
		 created_at, created_at,
		 db->quote( s.fourcc ).c_str(),
		 pixel_format_id.c_str(),
		 s.fps,
		 s.start,
		 s.duration
		 );

	if ( ! db->sql( buf ) )
	  {
	    LOG_ERROR( _("Could not add video track #") << i 
		       << _(" for '") << img->filename() 
		       << _("' to database '") << db->database() 
		       << "': " << endl << db->error() );
	  }
	else
	{
	   LOG_INFO( img->filename() << _(": added video to database '") 
		     << db->database() << _("'.") );
	}
      }
  }


  void ImageBrowser::add_audio( const mrv::media& m )
  {
    if ( !db ) return;

    const CMedia* img = m->image();

    char buf[4096];
    char* login;
    std::string shot_id;
    db_envvars( login, shot_id );

    char date[256], created_at[256];
    time_t t = img->ctime();
    strftime( date, 256, N_("%F %H:%M:%S"), localtime( &t ) );

    t = ::time(NULL);
    strftime( created_at, 256, N_("%F %H:%M:%S"), localtime( &t ) );


    std::string image_id;
    sprintf( buf, N_("SELECT id FROM images "
		     "WHERE directory='%s' AND filename='%s'"),
	     db->quote( img->directory() ).c_str(), 
	     db->quote( img->name() ).c_str() );
    image_id = buf;

    unsigned num_audio_streams = img->number_of_audio_streams();
    for ( unsigned i = 0; i < num_audio_streams; ++i )
      {
	 sprintf( buf, N_("SELECT id FROM audios "
			  "WHERE directory='%s' AND filename='%s'"
			  " AND stream=%d;"),
		  db->quote( img->directory() ).c_str(), 
		  db->quote( img->name() ).c_str(), i+1 );
	if (! db->sql( buf ) )
	  {
	    LOG_ERROR( _("Could not find audio '") << img->filename() 
		       << _("' in database '") << db->database() 
		       << "': " << endl << db->error() );
	    return;
	  }
	else
	  {
	    std::string result;
	    db->result( result );

	    // It already exists in database, do not add it again
	    if ( result != "" ) continue;
	  }

	const CMedia::audio_info_t& s = img->audio_info(i);
	sprintf( buf, N_("INSERT INTO audios"
			 "( directory, filename, stream, image_id, creator, "
			 "created_at, updated_at, "
			 "disk_space, date, shot_id, codec, fourcc, "
			 "channels, frequency, bitrate, start, duration )"
			 " VALUES "
			 "('%s', '%s', %d, ( %s ), '%s', "
			 "'%s', '%s', "
			 "%u, '%s', ( %s ), '%s', '%s', %u, %u,"
			 "%u, %g, %g );" ),
		 db->quote( img->directory() ).c_str(), 
		 db->quote( img->name() ).c_str(), i+1,
		 image_id.c_str(),
		 login,
		 created_at, created_at,
		 0, date,
		 shot_id.c_str(),
		 db->quote( s.codec_name ).c_str(),
		 s.fourcc.c_str(),
		 s.channels,
		 s.frequency,
		 s.bitrate/1000,
		 s.start,
		 s.duration
		 );
	if ( ! db->sql( buf ) )
	  {
	    LOG_ERROR( _("Could not add audio track #") << i+1 
		       << _(" for '") << img->filename() 
		       << _("' to database '") << db->database() 
		       << "': " << endl << db->error() );
	  }
	else
	{
	   LOG_INFO( img->filename() << _(": added audio to database '") 
		     << db->database() << _("'.") );
	}
      }
  }

  void ImageBrowser::db_envvars( char*& login, std::string& shot_id )
  {
    char buf[4096];
    std::string seq_id;
    shot_id = N_("NULL");

    login = getenv(N_("USER"));
    if ( !login ) login = getenv(N_("USERNAME"));
    if ( !login ) login = (char*)"";

    const char* show = getenv( N_("SHOW") );
    if ( show && db )
      {
	 sprintf( buf, N_("INSERT INTO shows(name) VALUES ('%s');"), show ); 

	 if ( db->sql( buf ) )
	 {
	    LOG_INFO( _("Inserted show '") << show << _("'.") );
	 }

	const char* seq  = getenv( N_("SEQ") );

	const char* shot = getenv( N_("SHOT") );
	if ( !seq && shot )
	  {
	    seq_id = shot;
	    seq = seq_id.c_str();
	    int idx = 0;
	    for ( ; *seq != 0; ++seq, ++idx )
	      {
		if ( *seq >= '0' && *seq <= '9' ) break;
	      }

	    seq_id = seq_id.substr( 0, idx );
	    seq = seq_id.c_str();
	  }

	if ( seq )
	  {
	     sprintf( buf, N_("INSERT INTO sequences(name, show_id)"
			      " VALUES "
			      "('%s', "
			      "( SELECT id FROM shows WHERE name = '%s') );" ),
		     seq, show ); 
	    
	     if ( db->sql( buf ) )
	     {
		LOG_INFO( _("Inserted sequence '") << seq << _("' into show '")
			  << show << _("'." ) );
	     }

	    if ( shot )
	      {
		 sprintf( buf, N_( "INSERT INTO shots(name, sequence_id)"
				   " VALUES "
				   "( '%s', "
				   "  ( SELECT id FROM sequences "
				   "WHERE name = '%s')"
				   ");" ), 
			 shot, seq ); 

		 if ( db->sql( buf ) )
		 {
		    LOG_INFO( _("Added shot '") << shot << _("' to sequence '") 
			      << seq << _("'.") );
		 }


		shot_id = N_("SELECT id FROM shots WHERE name = '" );
		shot_id += shot;
		shot_id += N_("'");
	      }
	  }
      }

  }


  void ImageBrowser::add_image( const mrv::media& m )
  {
    const CMedia* img = m->image();

    char* login;
    std::string shot_id;
    db_envvars( login, shot_id );

    std::string icc_profile_id = N_("NULL");
    const char* icc_profile = img->icc_profile();
    if ( icc_profile )
      {
	char buf[4096];
	sprintf( buf, N_("INSERT INTO icc_profiles(name, filename)"
			 " VALUES "
			 "( '%s', '%s' );" ), icc_profile, icc_profile ); 
	if ( db->sql( buf ) )
	{
	   LOG_INFO( img->filename() << _(": added ICC ") << 
		     icc_profile << _(" to database '") 
		     << db->database() << _("'.") );
	}

	icc_profile_id = N_("SELECT id FROM icc_profiles WHERE filename = '" );
	icc_profile_id += icc_profile;
	icc_profile_id += N_("'");
      }

    std::string rendering_transform_id = "NULL";
    if ( img->rendering_transform() )
      {
	char buf[4096];
	sprintf( buf, N_("INSERT INTO render_transforms(name)"
			 " VALUES "
			 "( '%s' );" ), img->rendering_transform() );
	if ( db->sql( buf ) )
	{
	   LOG_INFO( img->filename() << _(": added render transform ") << 
		     img->rendering_transform() << _(" to database '") 
		     << db->database() << _("'.") );
	}

	rendering_transform_id = N_("SELECT id FROM render_transforms "
				    "WHERE name = '");
	rendering_transform_id += img->rendering_transform();
	rendering_transform_id += N_("'");
      }

    std::string look_mod_transform_id = "NULL";
    if ( img->look_mod_transform() )
      {
	char buf[4096];
	sprintf( buf, N_("INSERT INTO look_mod_transforms(name)"
			 " VALUES "
			 "( '%s' );"), img->look_mod_transform() ); 
	if ( db->sql( buf ) )
	{
	   LOG_INFO( img->filename() << _(": added look mod transform ") << 
		     img->look_mod_transform() << _(" to database '") 
		     << db->database() << _("'.") );
	}

	look_mod_transform_id = N_("SELECT id FROM look_mod_transforms "
				   "WHERE name = '");
	look_mod_transform_id += img->look_mod_transform();
	look_mod_transform_id += N_("'");
      }

    std::string pixel_format_id = "NULL";
    if ( img->pixel_format_name() )
      {
	char buf[4096];
	sprintf( buf, N_("INSERT INTO pixel_formats(name)"
			 " VALUES "
			 "( '%s' );"), img->pixel_format_name() ); 
	if ( db->sql( buf ) )
	{
	   LOG_INFO( img->filename() << _(": added pixel format ") << 
		     img->pixel_format_name() << _(" to database '") 
		     << db->database() << _("'.") );
	}

	pixel_format_id = N_("SELECT id FROM pixel_formats "
				   "WHERE name = '");
	pixel_format_id += img->pixel_format_name();
	pixel_format_id += N_("'");
      }

    float fstop = 8.0f;
    const char* fnumber = img->exif( "F Number" );
    if ( fnumber == NULL )
      fnumber = img->exif( "Aperture Value" );

    if ( fnumber )
      {
	stringArray tokens;
	mrv::split_string( tokens, fnumber, "/" );
	if ( tokens.size() == 2 )
	  {
	    int num = atoi( tokens[0].c_str() );
	    int den = atoi( tokens[1].c_str() );
	    fstop = (float) num / (float) den;
	  }
      }


    char date[256], created_at[256];
    time_t t = img->ctime();
    strftime( date, 255, "%F %H:%M:%S", localtime( &t ) );

    t = ::time(NULL);
    strftime( created_at, 256, "%F %H:%M:%S", localtime( &t ) );

    std::string format = db->quote( img->format() );

    // Quote thumbnail picture
    const fltk::Image* thumb = m->thumbnail();

    std::string thumbnail;

    if ( thumb )
      {
	thumbnail = db->binary( thumb->buffer(), 
				thumb->width() * thumb->height() * 4 );
      }

    char* buf = new char[ thumbnail.size() + 4096 ];

    sprintf( buf, N_("SELECT id FROM images "
		     "WHERE directory='%s' AND filename='%s';" ),
	     db->quote( img->directory() ).c_str(), 
	     db->quote( img->name() ).c_str() );
    if (! db->sql( buf ) )
      {
	LOG_ERROR( _("Could not find image table '") << img->filename() 
		   << _("' in database '") << db->database() 
		   << "':" << endl << db->error() );
	delete [] buf;
	return;
      }
    else
      {
	std::string result;
	db->result( result );

	// It already exists in database, do not add it again
	if ( result != "" ) return;
      }

    std::string layers;
    stringArray::const_iterator i = img->layers().begin();
    stringArray::const_iterator s = i;
    stringArray::const_iterator e = img->layers().end();
    for ( ; i != e; ++i )
      {
	if ( i != s ) layers += ", ";
	layers += *i;
      }


    sprintf( buf, "INSERT INTO images"
	     "( directory, filename, frame_start, frame_end, creator, "
	     "created_at, updated_at, "
	     "disk_space, date, shot_id, width, height, pixel_ratio, format, "
	     "fps, codec, icc_profile_id, render_transform_id, "
	     "look_mod_transform_id, pixel_format_id, "
	     "depth, num_channels, layers,"
	     "fstop, gamma, thumbnail, thumbnail_width, thumbnail_height )"
	     " VALUES "
	     "('%s', '%s', %" PRId64 ", %" PRId64 ", '%s', "
	     "'%s', '%s', "
	     "%lu, '%s', ( %s ), %u, %u, %g, '%s',"
	     " %g, '%s', ( %s ), ( %s ), "
	     "( %s ), ( %s ), %d, %d, '%s', "
	     "%g, %g, '%s', %d, %d);",
	     db->quote( img->directory() ).c_str(), 
	     db->quote( img->name() ).c_str(),
	     img->first_frame(), img->last_frame(),
	     login,
	     created_at, created_at, 
	     (unsigned long)img->disk_space(), date,
	     shot_id.c_str(),
	     img->width(), img->height(),
	     img->pixel_ratio(),
	     format.c_str(), img->fps(),
	     img->compression(),
	     icc_profile_id.c_str(),
	     rendering_transform_id.c_str(),
	     look_mod_transform_id.c_str(),
	     pixel_format_id.c_str(),
	     img->depth(), img->number_of_channels(),
	     layers.c_str(),
	     fstop, img->gamma(),
	     thumbnail.c_str(),
	     thumb->width(), thumb->height()
	     );
    
    if (! db->sql( buf ) )
      {
	LOG_ERROR( _("Could not add image '") << img->filename() 
		   << _("' to database '") << db->database() 
		   << "': " << endl << db->error() );
      }
    else
    {
       LOG_INFO( _("Added image '") << img->filename() 
		 << _("' to database '") << db->database()
		 << _("'.") );
    }

    delete [] buf;
  }

  mrv::media ImageBrowser::add( mrv::media& m )
  {
    mrv::Reel reel = current_reel();
    if ( !reel ) reel = new_reel();

    reel->images.push_back( m );

    Element* nw = new_image( m );

    fltk::Browser::add( nw );

    if ( reel->images.size() == 1 )
      {
	value(0); change_image();
      }

    if ( visible() ) relayout();

    const CMedia* img = m->image();

    if ( dynamic_cast< const stubImage* >( img ) ||
	 dynamic_cast< const smpteImage* >( img ) ||
	 dynamic_cast< const ColorBarsImage* >( img ) ||
	 dynamic_cast< const slateImage* >( img ) )
      return m; // no need to add to database


    //
    // Avoid adding images that are in temp directories to database
    //
    static const char* kTempDirs[] = {
      "/tmp",
      "/usr/tmp",
    };
    static const unsigned kNumTempDirs = sizeof(kTempDirs) / sizeof(char*);

    const std::string& dir = img->directory();
    for ( unsigned i = 0; i < kNumTempDirs; ++i )
      {
	if ( dir == kTempDirs[i] ) return m;
      }


    if (!db) {
      db = mrv::Database::factory();
      if ( !db )
	{
	  LOG_ERROR( img->name() << ": Could not open database driver" );
	  return m;
	}
    }

    if ( !db->connected() ) return m;


    if ( img->has_picture() )
      add_image( m );

    if ( img->has_video() )
      add_video( m );

    if ( img->has_audio() )
      add_audio( m );

    return m;
  }

  /** 
   * Add a new image at the end of image browser list
   * 
   * @param img  image to insert
   */
  mrv::media ImageBrowser::add( CMedia* img )
  {
    if ( img == NULL ) return media();

    mrv::media m( new mrv::gui::media(img) );
    return add( m );
  }


  mrv::media ImageBrowser::add( const char* filename, 
				const int64_t start, const int64_t end )
  {
    std::string file( filename );

#if defined(WIN32) || defined(WIN64)
    // Handle cygwin properly
    if ( strncmp( filename, "/cygdrive/", 10 ) == 0 )
      {
	file  = filename[10];
	file += ":/";
	file += filename + 12;
      }
#endif
    
    size_t len = file.size();
    if ( len > 5 && file.substr( len - 5, 5 ) == ".reel" )
      {
	load_reel( file.c_str() );
	return current_image();
      }
    else
      {
	return load_image( file.c_str(), start, end );
      }
  }

  /** 
   * Remove an image from image browser list
   * 
   * @param img image to remove
   */
  void ImageBrowser::remove( mrv::media& m )
  {
    mrv::Reel reel = current_reel();
    if ( reel == NULL ) return;

    mrv::MediaList::iterator i = std::find( reel->images.begin(),
					    reel->images.end(), m );
    if ( i == reel->images.end() )
      {
	LOG_ERROR( _("Image") << " " << m->image()->filename() 
		   << _(" not found in reel") );
	return;
      }
  
    m->image()->stop();

    if ( uiMain->uiView->background() == m )
      {
	uiMain->uiView->background( mrv::media() );
      }

    uiMain->uiView->foreground( mrv::media() );
    if ( uiMain->uiView->background() == m ) 
      uiMain->uiView->background( mrv::media() );

    reel->images.erase( i );
  }



  /** 
   * Replace current image with a new file.
   * 
   * @param file new file name  ( mray.1.exr )
   * @param root root file name ( mray.%l04d.exr )
   */
  mrv::media ImageBrowser::replace( const char* file, const char* root )
  {
    mrv::Reel reel = current_reel();
    if ( reel == NULL ) return mrv::media();

    int idx = value(); 
    if ( idx < 0 ) return mrv::media();

    CMedia* newImg = CMedia::guess_image( root );

    // did not recognize new image, keep old
    mrv::media m = reel->images[idx];
    if ( newImg == NULL ) return m;

    CMedia* img = m->image();
    int64_t frame = img->frame();

    CMedia::Playback playback = img->playback();

    mrv::media newm( new mrv::gui::media(newImg) );

    Element* nw = new_image( m );
    fltk::Group::replace( idx, *nw );

    this->remove( m );

    mrv::CMedia::Mutex& vpm = newImg->video_mutex();
    SCOPED_LOCK( vpm );
    newImg->fetch( frame );
    newImg->default_icc_profile();
    newImg->default_rendering_transform();
    newImg->decode_video( frame );
    newImg->find_image( frame );

    if ( playback != CMedia::kStopped )
      newImg->play( playback, uiMain );

    reel->images.insert( reel->images.begin() + idx, newm );

    // Make sure no alert message if printed
    mrv::alert( NULL );

    return newm;
  }


  /** 
   * Change image in image view display to reflect browser's change
   * 
   */
  void ImageBrowser::change_reel()
  {
    clear();

    mrv::Reel reel = current_reel();
    if ( reel == NULL ) return;

    relayout();
    if ( reel->images.empty() )
      {
	value(-1);
      }
    else
      {
	mrv::MediaList::iterator i = reel->images.begin();
	mrv::MediaList::iterator e = reel->images.end();
	for ( ; i != e; ++i )
	  {
	    Element* nw = new_image( *i );
	    fltk::Browser::add( nw );
	  }

	value(0);
      }

    if ( reel->edl )
    {
       set_edl();
    }
    else
    {
       clear_edl();
    }

    change_image();
  }

  /** 
   * Change image in image view display to reflect browser's change
   * 
   */
  void ImageBrowser::change_image()
  {
    int sel = value();
    if ( sel < 0 ) 
      {
	uiMain->uiView->foreground( mrv::media() );
	uiMain->uiView->background( mrv::media() );
	uiMain->uiView->redraw();
      }
    else
      {
	mrv::Reel reel = current_reel();
	if ( reel == NULL ) return;

	assert( (unsigned)sel < reel->images.size() );

	mrv::media m = reel->images[sel];
	assert( m != NULL );

	uiMain->uiView->foreground( m );
	adjust_timeline();
      }
  }


  void ImageBrowser::change_image(unsigned i)
  { 
     value(i); 
     change_image(); 
  }

  /** 
   * Change to last image in image browser
   * 
   */
  void ImageBrowser::last_image()
  { 
    mrv::Reel reel = current_reel();
    if ( reel == NULL ) return;

    if ( reel->images.empty() ) return;
    value( (int) reel->images.size()-1 ); 
    change_image(); 
  }

  /** 
   * Load an image
   * 
   * @param name name of image
   * @param start first frame to load
   * @param end   last frame to load
   * 
   * @return new image
   */
  mrv::media ImageBrowser::load_image( const char* name,
				       const boost::int64_t start, 
				       const boost::int64_t end )
  {
    mrv::Reel reel = current_reel();
    if ( !reel ) reel = new_reel();

    if ( start != mrv::kMinFrame ) uiMain->uiFrame->value( start );

    mrv::check_license_status();

    CMedia* img = CMedia::guess_image( name, NULL, 0, start, end );
    if ( img == NULL ) return mrv::media();

    {
       mrv::CMedia::Mutex& vpm = img->video_mutex();
       SCOPED_LOCK(vpm);

       img->fetch( img->first_frame() );
    }

    img->default_icc_profile();
    img->default_rendering_transform();

    PreferencesUI* prefs = ViewerUI::uiPrefs;
    img->audio_engine()->device( prefs->uiPrefsAudioDevice->value() );

    if ( img->is_sequence() )
      {
	img->first_frame( start );
	img->last_frame( end );
      }

    mrv::media m = this->add( img );

    adjust_timeline();
    return m;
  }

  /** 
   * Open new image, sequence or movie file(s) from a load list.
   * 
   */
  void ImageBrowser::load( const mrv::LoadList& files )
  {
    //
    // Create a progress window
    //
    mrv::Reel oldreel = current_reel();
    unsigned  numImages = 0;
    if ( oldreel ) numImages = oldreel->images.size();

    uiMain->uiView->stop();

    fltk::Window* main = uiMain->uiMain;

    fltk::Window* w = NULL;
    fltk::ProgressBar* progress = NULL;

    if ( files.size() > 1 )
      {
	w = new fltk::Window( main->x(), main->y() + main->h()/2, 
			      main->w(), 80 );
	w->child_of(main);
	w->clear_border();
	w->begin();
	progress = new fltk::ProgressBar( 0, 20, w->w(), w->h()-20 );
	progress->range( 0, files.size() );
	progress->align( fltk::ALIGN_TOP );
	progress->showtext(true);
	w->end();

	w->show();
	fltk::check();
      }

    mrv::LoadList::const_iterator i = files.begin();
    mrv::LoadList::const_iterator e = files.end();

    char buf[1024];
    for ( ; i != e; ++i )
      {
	const mrv::LoadInfo& load = *i;


	if ( w )
	  {
	    snprintf( buf, 1024, _("Loading \"%s\""), load.filename.c_str() );
	    progress->label(buf);
	    w->redraw();
	    fltk::check();
	  }

	if ( load.reel )
	  {
	    load_reel( load.filename.c_str() );
	  }
	else
	  {
	     mrv::media fg = load_image( load.filename.c_str(), 
					 load.start, load.end );
	     if (!fg) 
	     {
		LOG_ERROR( _("Could not load '") << load.filename.c_str() 
			   << N_("'") );
	     }
	     if ( load.audio != "" ) 
	     {
		fg->image()->audio_file( load.audio.c_str() );
	     }
	  }

	if ( w ) progress->step(1);
      }

    if ( w )
      {
	w->hide();
	w->destroy();
      }

    mrv::Reel reel = current_reel();
    if ( !reel || reel->images.empty() ) return;

    // If loading images to old non-empty reel, display last image.
    if ( reel == oldreel && numImages > 0 )
      {
	this->change_image( (unsigned int)reel->images.size()-1 );
	mrv::media m = current_image();
	uiMain->uiFrame->value( m->image()->first_frame() );
      }
    else
      {
	// display first image for good EDL playback
	this->change_image( 0 );

	if ( !reel->edl )
	  {
	    mrv::media m = current_image();
	    CMedia* img = m->image();
	    int64_t offset = timeline()->offset( img );
	    uiMain->uiFrame->value( offset + img->first_frame() );
	  }
	else
	  {
	    uiMain->uiFrame->value( 1 );
	  }
      }

    adjust_timeline();

    if ( uiMain->uiPrefs->uiPrefsAutoPlayback->value() )
      {
	mrv::media m = current_image();
	if ( m )
	  {
	    CMedia* img = m->image();
	    if ( reel->edl || 
		 img->first_frame() != img->last_frame() )
	      uiMain->uiView->play_forwards();
	  }
      }
  }


  /** 
   * Load an image reel
   * 
   * @param name name of reel to load
   */
  void ImageBrowser::load_reel( const char* name )
  {
     bool edl;
     mrv::LoadList sequences;
     parse_reel( sequences, edl, name );

     fs::path path( name, fs::native );
     std::string reelname = path.leaf().c_str();
     reelname = reelname.substr(0, reelname.size()-5);
     
     new_reel( reelname.c_str() );

     load( sequences );

     if ( edl )
     {
	set_edl();
     }
     else
     {
	clear_edl();
     }
  }

  void ImageBrowser::load( const stringArray& files )
  {
    stringArray::const_iterator i = files.begin();
    stringArray::const_iterator e = files.end();

    mrv::LoadList loadlist;

    for ( ; i != e; ++i )
      {
	std::string file = *i;

	if ( file.substr(0, 7) == "file://" )
	  file = file.substr( 7, file.size() );

	if ( file == "" ) continue;

	size_t len = file.size();
	if ( len > 5 && file.substr( len - 5, 5 ) == ".reel" )
	  {
	    loadlist.push_back( mrv::LoadInfo( file ) );
	  }
	else
	  {
	    int64_t start = mrv::kMinFrame;
	    int64_t end   = mrv::kMaxFrame;
	    mrv::get_sequence_limits( start, end, file );
	    loadlist.push_back( mrv::LoadInfo( file, start, end ) );
	  }
      }

    load( loadlist );
  }

  /** 
   * Open new image, sequence or movie file(s).
   * 
   */
  void ImageBrowser::open()
  {
    stringArray files = mrv::open_image_file();
    if (files.empty()) return;
    load( files );
  }

  /** 
   * Save current image buffer being displayed,
   * giving it a new dummy filename.
   * 
   */
  void ImageBrowser::save()
  {
    mrv::Reel reel = current_reel();
    if (!reel) return;

    int sel = value();
    if ( sel < 0 ) return;

    mrv::media orig = reel->images[sel];
    mrv::save_image_file( orig->image() );  // @todo: popup gui options
  }


  /** 
   * Clone current image buffer being displayed,
   * giving it a new dummy filename.
   * 
   */
  void ImageBrowser::clone_current()
  {
    mrv::Reel reel = current_reel();
    if (!reel) return;

    int sel = value();
    if ( sel < 0 ) return;

    mrv::media orig = reel->images[sel];
    clonedImage* copy = new clonedImage( orig->image() );

    mrv::media clone( new mrv::gui::media(copy) );
    this->insert( sel + 1, clone );

    adjust_timeline();
  }


  /** 
   * Clone current image buffer (with all its channels) being displayed,
   * giving it a new dummy filename.
   * 
   */
  void ImageBrowser::clone_all_current()
  {
    int sel = value();
    if ( sel < 0 ) return;

    mrv::Reel reel = current_reel();
    if (!reel) return;

    mrv::media orig = reel->images[sel];

    // @hmm.... this needs fixing
    stubImage* copy = new stubImage( orig->image() );

    mrv::media clone( new mrv::gui::media(copy) );
    this->insert( sel + 1, clone );

    adjust_timeline();
  }


  /** 
   * Remove and delete current image buffer being displayed.
   * 
   */
  void ImageBrowser::remove_current()
  {
    mrv::Reel reel = current_reel();
    if (!reel) return;

    int sel = value();
    if ( sel < 0 ) return;

    fltk::Browser::remove( sel );
    this->remove( reel->images[sel] );

    if ( sel < (int)reel->images.size() )
      {
	value( sel );
      }
    else
      {
	value( (int)reel->images.size()-1 );
      }

    change_image();
    redraw();
  }



  /** 
   * Remove all reel->images.
   * 
   */
  void ImageBrowser::remove_all()
  {
    mrv::Reel reel = current_reel();
    if (!reel) return;

    int ok = fltk::ask( _( "Are you sure you want to\n"
			   "remove all images from reel?" ) );
    if ( !ok ) return;

    unsigned i;
    size_t num = reel->images.size();
    for ( i = 0; i < num; ++i )
      {
	fltk::Browser::remove( 0 );
	this->remove( reel->images[0] );
      }

    value(-1);
    change_image();
  }


  /** 
   * Change background image to current image
   * 
   */
  void ImageBrowser::change_background()
  {
    mrv::Reel reel = current_reel();
    if (!reel) return;

    int sel = value();
    if ( sel < 0 ) return;

    if ( uiMain->uiView->background() == reel->images[sel] )
    {
       uiMain->uiView->background( mrv::media() );
    }
    else
    {
       uiMain->uiView->background( reel->images[sel] );
    }
  }


  /** 
   * Refresh an image's postage stamp widget
   * 
   * @param img image to refresh
   */
  void ImageBrowser::refresh( mrv::media& m )
  {
    if ( ! m ) return;

    fltk::Widget* ow = this->find( m->image()->filename() );
    if (ow) ow->relayout();
  }


  /** 
   * Go to next image in browser's list
   * 
   */
  void ImageBrowser::next_image()
  {
    mrv::Reel reel = current_reel();
    if ( reel == NULL ) return;

    mrv::media m = current_image();
    m->image()->stop();

    unsigned sel = value() + 1;
    if ( sel == 0 ) return;

    if ( sel >= reel->images.size() ) sel = 0;


    value(sel);
    change_image();

    if ( timeline()->edl() )
      {
	mrv::media m = reel->images[sel];
	assert( m );

	CMedia* img = m->image();

	int64_t offset = timeline()->offset( img );
	seek( offset + img->first_frame() );
      }
  }


  /** 
   * Go to previous image in browser's list
   * 
   */
  void ImageBrowser::previous_image()
  {
    mrv::Reel reel = current_reel();
    if ( reel == NULL ) return;

    mrv::media m = current_image();
    m->image()->stop();

    int sel = value() - 1;
    if ( sel < -1 ) return;
    if ( sel < 0 ) sel = (int)reel->images.size() - 1;

    value(sel);
    change_image();
    if ( timeline()->edl() )
      {
	mrv::media m = reel->images[sel];
	assert( m );

	CMedia* img = m->image();

	int64_t offset = timeline()->offset( img );
	seek( offset + img->first_frame() );
      }
  }


  /** 
   * Handle a mouse push
   * 
   * @param x window's x coordinate
   * @param y window's y coordinate
   * 
   * @return 1 if handled, 0 if not
   */
  int ImageBrowser::mousePush( int x, int y )
  {
    int old_sel = value();

    int ok = fltk::Browser::handle( fltk::PUSH );

    uiMain->uiView->stop();

    int button = fltk::event_button();
    int sel = value();
    if ( button == 1 )
      {
	int clicks = fltk::event_clicks();
	if ( sel < 0 )
	  {
	    value( old_sel );
	  }
	else
	  {
	    if ( sel == old_sel && clicks > 0 )
	      {
		fltk::event_clicks(0);
		uiMain->uiImageInfo->uiMain->show();
		uiMain->uiView->update_image_info();
		return ok;
	      }

	    if ( sel == old_sel ) return ok;

	    lastX = x; lastY = y;
	    change_image();

	    mrv::media m = current_image();
	    if ( timeline()->edl() && m )
	      {
		CMedia* img = m->image();

		int64_t s = timeline()->location( img );
		seek( s );
	      }
	  }
	return 1;
      }
    else if ( button == 3 )
      {
	fltk::Menu menu(0,0,0,0);

	mrv::Reel reel = current_reel();
	if (!reel) {
	  new_reel("reel");
	  reel = current_reel();
	  if ( !reel ) return 0;
	}

	CMedia* img = NULL;
	bool valid = false;

	if ( sel >= 0 )
	  {
	    mrv::media m = reel->images[sel];
	    img = m->image();

	    valid = ( dynamic_cast< slateImage* >( img ) == NULL &&
		      dynamic_cast< smpteImage* >( img ) == NULL &&
		      dynamic_cast< clonedImage* >( img ) == NULL );

	    const stubImage* stub = dynamic_cast< const stubImage* >( img );
	    if ( stub )
	      {
		menu.add( _("Image/Clone"), 0, 
			 (fltk::Callback*)clone_image_cb, this);
		menu.add( _("Image/Clone All Channels"), 0, 
		     (fltk::Callback*)clone_all_cb, 
		     NULL, fltk::MENU_DIVIDER);
	      }
	    else
	      {
		if ( valid )
		  {
		    menu.add( _("Image/Clone"), 0, 
			      (fltk::Callback*)clone_image_cb, this, 
			      fltk::MENU_DIVIDER);
		  }
	      }

	    if ( valid )
	      {
		menu.add( _("Image/Attach ICC Color Profile"), 0, 
			  (fltk::Callback*)attach_color_profile_cb, this, 
			  fltk::MENU_DIVIDER);
		menu.add( _("Image/Attach CTL Script"), 0, 
			  (fltk::Callback*)attach_ctl_script_cb, this, 
			  fltk::MENU_DIVIDER);
	      }

	    menu.add( _("Image/Set as Background"), 0, 
		     (fltk::Callback*)set_as_background_cb, 
		     (void*) uiMain->uiView );

	    if ( valid )
	      {
		valid = ( dynamic_cast< stubImage*  >( img ) == NULL );
	      }
	  }

	menu.add( _("Create/Color Bars/SMPTE NTSC"), 0, 
		  (fltk::Callback*)ntsc_color_bars_cb, this);
	menu.add( _("Create/Color Bars/SMPTE NTSC HDTV"), 0, 
		  (fltk::Callback*)ntsc_hdtv_color_bars_cb, this);
	menu.add( _("Create/Color Bars/PAL"), 0, 
		  (fltk::Callback*)pal_color_bars_cb, this);
	menu.add( _("Create/Color Bars/PAL HDTV"), 0, 
		  (fltk::Callback*)pal_hdtv_color_bars_cb, this);
	menu.add( _("Create/Gamma Chart/1.4"), 0, 
		  (fltk::Callback*)gamma_chart_14_cb, this);
	menu.add( _("Create/Gamma Chart/1.8"), 0, 
		  (fltk::Callback*)gamma_chart_18_cb, this);
	menu.add( _("Create/Gamma Chart/2.2"), 0, 
		  (fltk::Callback*)gamma_chart_22_cb, this);
	menu.add( _("Create/Gamma Chart/2.4"), 0, 
		  (fltk::Callback*)gamma_chart_24_cb, this);
	menu.add( _("Create/Gradient/Linear"), 0, 
		  (fltk::Callback*)linear_gradient_cb, this);
	menu.add( _("Create/Gradient/Luminance"), 0, 
		  (fltk::Callback*)luminance_gradient_cb, this);
	menu.add( _("Create/Checkered"), 0, 
		  (fltk::Callback*)checkered_cb, this);

	if (valid)
	  menu.add("Create/Slate", 0, (fltk::Callback*)slate_cb, this);

	menu.popup( fltk::Rectangle( x, y, 80, 1) );
	return 1;
      }

    return ok;
  }

  /** 
   * Handle a Drag and Drop operation on this widget (or image view)
   * 
   */
  void ImageBrowser::handle_dnd()
  {
    std::string filenames = fltk::event_text();

    stringArray files;
    mrv::split_string( files, filenames, "\n" ); 

    stringArray::iterator i = files.begin();
    stringArray::iterator e = files.end();
    for ( ; i != e; ++i )
    {
       std::string file = *i;
       std::string root, frame, ext;
       bool ok = mrv::split_sequence( root, frame, ext, file );
       if ( root != "" && frame != "" && ext != N_(".jpg") && 
	    ext != N_(".JPG") )
       {
    	  file = root;
    	  for ( int i = 0; i < frame.size(); ++i )
    	     if ( frame[i] == '0' ) file += '@';
    	  file += '@';
    	  file += ext;
       }
       *i = file;
    }

    load( files );

    last_image();
  }

  /** 
   * Drag an element up and down image list
   * 
   * @param x new x position
   * @param y new y position
   * 
   * @return 1 if drag was success, 0 if not
   */
  int ImageBrowser::mouseDrag( int x, int y )
  {
    int sel = value();
    if (sel < 0) return 0;

    dragging = (Element*) child(sel);
    if (y < 0) lastY = 0;
    else       lastY = y;
    redraw();
    return 1;
  }


  int ImageBrowser::mouseRelease( int x, int y )
  {

    int oldsel = value();
    if (oldsel < 0 || !dragging ) 
      {
	redraw();
	return 0;
      }

    dragging = NULL;

    mrv::Reel reel = current_reel();
    if (!reel) 
      {
	redraw();
	return 0;
      }

    mrv::media m = reel->images[oldsel];
    CMedia* img = m->image();

    int64_t f = (int64_t) uiMain->uiFrame->value();
    int64_t t = timeline()->offset( img );
    f -= t;

    if ( y < 0 ) fltk::e_y = 0;

    fltk::Browser::handle( fltk::PUSH );

    int sel = value();
    if ( sel < 0 || sel == oldsel )
      {
	redraw();
	return 0;
      }


    Element* e = (Element*) child(oldsel);
    if ( sel > oldsel ) sel += 1;
    fltk::Browser::insert( *e, sel );

    redraw();

    reel->images.erase( reel->images.begin() + oldsel );
    if ( oldsel < sel ) sel -= 1;

    reel->images.insert( reel->images.begin() + sel, m );

    if ( timeline()->edl() )
      {
	int64_t t = timeline()->offset( img );
	f += t;
	uiMain->uiFrame->value( f );
	uiMain->uiFrame->redraw();
	timeline()->value(f);
	timeline()->redraw();
      }

    return 1;
  }

  /** 
   * Event handler for the image browser window
   * 
   * @param event fltk::event enumeration
   * 
   * @return 1 if handled, 0 if not
   */
  int ImageBrowser::handle( int event )
  {
    switch( event )
      {
      case fltk::ENTER:
	take_focus();
	window()->show();
	return 1;
      case fltk::PUSH:
	return mousePush( fltk::event_x(), fltk::event_y() );
      case fltk::DRAG:
	return mouseDrag( fltk::event_x(), fltk::event_y() );
      case fltk::RELEASE:
	return mouseRelease( fltk::event_x(), fltk::event_y() );
      case fltk::DND_ENTER:
      case fltk::DND_LEAVE:
      case fltk::DND_DRAG:
      case fltk::DND_RELEASE:
	return 1;
      case fltk::PASTE:
	handle_dnd();
	return 1;
      }
    int old_sel = value();
    int ok = fltk::Browser::handle( event );
    if ( value() != old_sel ) change_image();
    return ok;
  }

  /** 
   * Switch to a new frame on one image if in EDL mode or
   * in all images of reel if not.
   * 
   * @param f new frame
   */
  void ImageBrowser::seek( const int64_t tframe )
  {
     boost::int64_t f = tframe;

     uiMain->uiFrame->value( tframe );
     uiMain->uiFrame->redraw();

    timeline()->value( tframe );
    timeline()->redraw();


    if ( timeline()->edl() )
      {
	ImageView::Playback playback = uiMain->uiView->playback();

	// Check if we need to change to a new sequence based on frame
	 mrv::media m = timeline()->media_at( f );
	 if (! m ) return;

	CMedia* img = m->image();

	boost::int64_t pos = timeline()->offset( img );
	int size = img->last_frame() - img->first_frame() + 1;


	if ( f < timeline()->minimum() )
	  {
	    f = int64_t(timeline()->maximum() - timeline()->minimum() - f) + 1;
	  }
	else if ( f > timeline()->maximum() )
	  {
	    f = int64_t(timeline()->minimum() + f - timeline()->maximum()) - 1;
	  }


	if ( m != uiMain->uiView->foreground() )
	  {
	     uiMain->uiView->stop();

	     unsigned int i = timeline()->index( f );
	     f = timeline()->global_to_local( f );
	     img->seek( f );
	     change_image(i);
	     if ( playback != ImageView::kStopped )
	     {
	       uiMain->uiView->play( (CMedia::Playback)playback);
	     }

	  }
	else
	  {
	     f = timeline()->global_to_local( f );
	     img->stop();
	     img->seek( f );
	     if ( playback != ImageView::kStopped )
	     {
	       img->play( (CMedia::Playback)playback, uiMain);
	     }
	  }
      }
    else
      {
	mrv::Reel reel = current_reel();
	if (!reel) return;

	mrv::MediaList::iterator i = reel->images.begin();
	mrv::MediaList::iterator e = reel->images.end();
	for ( ; i != e; ++i )
	  {
	    CMedia* img = (*i)->image();
	    img->seek( f );
	  }
      }




    mrv::ImageView* view = uiMain->uiView;

    if ( view->playback() != ImageView::kStopped ) return;

    //
    // If stopped, play the audio sample for the frame of both
    // the fg and bg images.
    //
    mrv::media fg = view->foreground();
    if ( fg )
      {
	CMedia* img = fg->image();

	if ( timeline()->edl() )
	  {
	    f = tframe;
	    f -= timeline()->location( img );
	    f += img->first_frame();
	  }

	if ( img->has_audio() )
	  {
	    img->find_audio( f );
	  }
	if ( img->has_video() )
	  {
	    img->find_image( f );
	  }
      }

    mrv::media bg = view->background();
    if ( bg )
      {
	CMedia* img = bg->image();

	if ( timeline()->edl() )
	  {
	    f = tframe;
	    f -= timeline()->location( img );
	    f += img->first_frame();
	  }

	if ( img->has_audio() )
	  {
	    img->find_audio( f );
	  }
	if ( img->has_video() )
	  {
	    img->find_image( f );
	  }
      }

    view->redraw();
    redraw();

  }


  /** 
   * Switch to a new frame.
   * 
   * @param f new frame
   */
  void ImageBrowser::frame( const int64_t f )
  {
    uiMain->uiFrame->value( f );
    uiMain->uiFrame->redraw();
  }

  void ImageBrowser::clear_edl()
  {
     mrv::Reel reel = current_reel();
     reel->edl = false;

    mrv::media m = current_image();
    if (!m) return;

    CMedia* img = m->image();
    int64_t frame = img->frame();

    timeline()->edl( false );
    timeline()->redraw();
    if ( !img ) return;

    uiMain->uiFrame->value( frame );
    timeline()->value( frame );
    adjust_timeline();
  }

  void ImageBrowser::set_edl()
  {
     mrv::Reel reel = current_reel();
     reel->edl = true;

    mrv::media m = current_image();
    if (!m) return;

    CMedia* img = m->image();
    int64_t frame = img->frame();

    timeline()->edl( true );
    timeline()->redraw();
    if ( !img ) return;

    int64_t f = frame - img->first_frame() + timeline()->location( img );
    uiMain->uiFrame->value( f );
    timeline()->value( f );
  }

  void ImageBrowser::toggle_edl()
  {
    mrv::media m = current_image();
    if (!m) return;

    CMedia* img = m->image();
    int64_t frame = img->frame();

    timeline()->edl( !timeline()->edl() );
    mrv::Reel reel = current_reel();
    reel->edl = timeline()->edl();
    timeline()->redraw();
    if ( !img ) return;

    if ( !timeline()->edl() )
      {
	uiMain->uiFrame->value( frame );
	timeline()->value( frame );
	adjust_timeline();
      }
    else
      {
	int64_t f = frame - img->first_frame() + timeline()->location( img );
	uiMain->uiFrame->value( f );
	timeline()->value( f );
      }
  }
 

  void ImageBrowser::attach_icc_profile()
  {
    mrv::media m = current_image();
    if ( ! m ) return;

    CMedia* img = m->image();
    mrv::attach_icc_profile( img );
  }

  void ImageBrowser::attach_ctl_script()
  {
    mrv::media m = current_image();
    if ( ! m ) return;

    CMedia* img = m->image();
    mrv::attach_ctl_script( img );
  }



  void ImageBrowser::adjust_timeline()
  {
     boost::int64_t first, last, frame;
     
     if ( ! timeline()->edl() ) 
     {
	mrv::media m = current_image();
	if ( ! m ) return;
	CMedia* img = m->image();

	first = img->first_frame();
	frame = img->frame();
	last  = img->last_frame();

	if (frame > last ) frame = last;
	if (frame < first ) frame = first;
      }
    else
      {
	mrv::Reel reel = current_reel();
	if ( !reel || reel->images.empty() ) return;

	first = 1;

	mrv::media m = current_image();
	if (! m ) return;

	CMedia* img = m->image();
	frame = timeline()->offset( img ) + img->frame();

	m = reel->images.back();
	if (! m ) return;

	img = m->image();
	last  = ( timeline()->offset( img ) + 
		  img->last_frame() - img->first_frame() + 1 );

      }


    timeline()->minimum( first );
    timeline()->maximum( last );
    timeline()->value( frame );
    uiMain->uiStartFrame->value( first );
    uiMain->uiEndFrame->value( last );
    uiMain->uiFrame->value( frame );

    timeline()->redraw();
  }


  void ImageBrowser::draw()
  {
    fltk::Browser::draw();

    mrv::Reel reel = current_reel();
    if (!reel) return;

    mrv::MediaList::iterator i = reel->images.begin();
    mrv::MediaList::iterator e = reel->images.end();
    for ( ; i != e; ++i )
      {
	mrv::media m = *i;
	CMedia* img = m->image();
	if ( img->image_damage() & CMedia::kDamageThumbnail )
	  {
	    m->create_thumbnail();
	  }
      }


    if ( dragging )
      {
	fltk::push_matrix();
	fltk::translate( 0, lastY );
	dragging->draw();
	fltk::pop_matrix();
      }
  }


} // namespace mrv
