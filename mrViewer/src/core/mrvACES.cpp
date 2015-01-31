
#include <vector>

#include <fltk/run.h>

#include "ACESclipWriter.h"

#include "core/CMedia.h"
#include "gui/mrvIO.h"
#include "gui/mrvVersion.h"
#include "gui/mrvPreferences.h"

namespace {
const char* kModule = "aces";
}

namespace mrv {

bool save_aces_xml( const CMedia* img, const char* filename )
{

    ACES::ACESclipWriter c;

    c.info( "mrViewer", mrv::version() );

    char buf[128];
    const char* show = getenv( "SHOW" );
    if ( !show ) show = "Unknown Show";

    const char* shot = getenv( "SHOT" );
    if ( !shot ) shot = "Unknown Shot";

    sprintf( buf, "%s-%s", show, shot );

    c.clip_id( img->filename(), buf);
    c.config();

    c.ITL_start();
    if ( img->idt_transform() )
        c.add_IDT( img->idt_transform() );
    c.ITL_end();

    c.PTL_start();

    size_t i = 0;
    size_t num = img->number_of_lmts();
    for ( ; i < num; ++i )
    {
        c.add_LMT( img->look_mod_transform(i) );
    }

    if ( img->rendering_transform() )
    {
        c.add_RRT( img->rendering_transform() );
    }

    if ( ! mrv::Preferences::ODT_CTL_transform.empty() )
        c.add_ODT( mrv::Preferences::ODT_CTL_transform );

    c.PTL_end();

    if ( ! c.save( filename ) )
        LOG_ERROR( "Could not save '" << filename << "'." );
    else
        LOG_INFO( "Saved ACES clip metadata file '" << filename << "'" );


    return true;
}

}
