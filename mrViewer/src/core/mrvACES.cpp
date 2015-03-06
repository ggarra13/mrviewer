
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

namespace fs = boost::filesystem;

namespace mrv {

std::string aces_xml_filename( const char* file )
{

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
    using namespace ACES;

    if(! fs::exists(filename) )
        return false;

    ACESclipReader c;
    ACESclipReader::ACESError err = c.load(filename);
    if ( err != ACESclipReader::kAllOK )
    {
        LOG_ERROR( filename << _(" failed. ") << c.error_name( err ) );
        return false;
    }

    if ( !c.link_ITL.empty() )
      {
        img->idt_transform( c.link_ITL.c_str() );
      }
    else if ( c.IDT.status == ACES::kPreview && !c.IDT.name.empty() )
      {
        img->idt_transform( c.IDT.name.c_str() );
      }

    size_t num = c.LMT.size();
    size_t i = 0;
    img->clear_look_mod_transform();

    if ( c.graderef_status == ACES::kPreview )
    {
        if ( c.convert_to != "" )
        {
            img->append_look_mod_transform( c.convert_to.c_str() );

            unsigned num = c.grade_refs.size();
            img->asc_cdl( c.sops );
            img->grade_refs( c.grade_refs );

            for ( unsigned i = 0; i < num; ++i )
            {
                std::string name = "LMT." + c.grade_refs[i];
                name += N_(".a1.0.0");
                img->append_look_mod_transform( name.c_str() );
            }
        }

        if ( c.convert_from != "" )
        {
            img->append_look_mod_transform( c.convert_from.c_str() );
        }
    }


    for ( ; i < num; ++i )
      {
        if ( c.LMT[i].status != ACES::kPreview )
	  continue;
        img->append_look_mod_transform( c.LMT[i].name.c_str() );
      }
    
    if ( c.RRT.status == ACES::kPreview && !c.RRT.name.empty() )
      {
        img->rendering_transform( c.RRT.name.c_str() );
      }

    if ( c.ODT.status == ACES::kPreview && !c.ODT.name.empty() )
        mrv::Preferences::ODT_CTL_transform = c.ODT.name;

    LOG_INFO( _("Loaded ACES clip metadata file '") << filename << "'" );
    return true;
}

bool save_aces_xml( const CMedia* img, const char* filename )
{

    ACES::ACESclipWriter c;

    setlocale( LC_NUMERIC, "C" );

    c.info( "mrViewer", mrv::version() );

    char buf[128];
    const char* show = getenv( N_("SHOW") );
    if ( !show ) show = getenv( _("SHOW") );
    if ( !show ) show = _("Unknown Show");

    const char* shot = getenv( N_("SHOT") );
    if ( !shot ) shot = getenv( _("SHOT") );
    if ( !shot ) shot = _("Unknown Shot");

    sprintf( buf, "%s-%s", show, shot );

    c.clip_id( img->fileroot(), buf, img->mtime() );
    c.config();
    c.ITL_start();

    if ( img->idt_transform() )
        c.add_IDT( img->idt_transform() );

    size_t i = 0;
    const ACES::ASC_CDL& t = img->asc_cdl();
    size_t num = img->number_of_lmts();
    size_t num_graderefs = img->number_of_grade_refs();

    if ( num_graderefs > 0 )
    {
        i = num_graderefs + 2;
        if ( i > num )
        {
            LOG_ERROR( _("Missing transforms for aces:GradeRef") );
            i = num;
        }

        c.gradeRef_start( img->look_mod_transform(0) );

        for ( unsigned j = 0; j < num_graderefs; ++j )
        {
            const std::string& gr = img->grade_ref( j );
            if ( gr == N_("SOPNode") )
            {
                c.gradeRef_SOPNode( t );
            }
            else if ( gr == N_("SatNode") )
            {
                c.gradeRef_SatNode( t );
            }
            else
            {
                LOG_ERROR( _("Unknown node in GradeRef") );
            }
        }

        c.gradeRef_end( img->look_mod_transform(i-1) );

    }

    c.ITL_end();

    c.PTL_start();

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
        LOG_ERROR( _("Could not save '") << filename << "'." );
    else
        LOG_INFO( _("Saved ACES clip metadata file '") << filename << "'" );


    setlocale( LC_NUMERIC, "" );

    return true;
}

}
