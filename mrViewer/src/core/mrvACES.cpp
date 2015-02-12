
#include <vector>

#include <boost/filesystem.hpp>

#include <fltk/run.h>

#include "ACESclipWriter.h"
#include "ACESclipReader.h"

#include "core/CMedia.h"
#include "core/Sequence.h"
#include "gui/mrvIO.h"
#include "gui/mrvVersion.h"
#include "gui/mrvPreferences.h"

namespace {
const char* kModule = "aces";
}

namespace mrv {

std::string aces_xml_filename( const char* file )
{
    namespace fs = boost::filesystem;

    std::string root, frame, view, ext;
    mrv::split_sequence( root, frame, view, ext, file );

    fs::path f = root;
    std::string filename = f.filename().string();

    std::string xml;
    xml = f.parent_path().string();
    if ( ! xml.empty() ) xml += "/";
    xml += "ACESclip.";
    xml += filename;
    xml += "xml";

    return xml;
}


bool load_aces_xml( CMedia* img, const char* filename )
{
    ACES::ACESclipReader c;
    if ( ! c.load( filename ) )
        return false;

    if ( !c.link_ITL.empty() )
        img->idt_transform( c.link_ITL.c_str() );
    else if ( c.IDT.status == ACES::kPreview && !c.IDT.name.empty() )
        img->idt_transform( c.IDT.name.c_str() );

    size_t num = c.LMT.size();
    size_t i = 0;
    img->clear_look_mod_transform();
    for ( ; i < num; ++i )
    {
        if ( c.LMT[i].status != ACES::kPreview )
            continue;
        img->append_look_mod_transform( c.LMT[i].name.c_str() );
    }

    if ( c.RRT.status == ACES::kPreview && !c.RRT.name.empty() )
        img->rendering_transform( c.RRT.name.c_str() );

    if ( c.ODT.status == ACES::kPreview && !c.ODT.name.empty() )
        mrv::Preferences::ODT_CTL_transform = c.ODT.name;

    LOG_INFO( "Loaded ACES clip metadata file '" << filename << "'" );
    return true;
}

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

    c.clip_id( img->fileroot(), buf, img->mtime() );
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
