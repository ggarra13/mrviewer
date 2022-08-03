#include <vector>


#include <FL/Fl.H>
#include <FL/fl_utf8.h>

#include "ACESclipWriter.h"
#include "ACESclipReader.h"
#include "AMFReader.h"
#include "AMFWriter.h"

#include <boost/filesystem.hpp>

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

std::string aces_amf_filename( const char* file )
{

    std::string root, frame, view, ext;
    mrv::split_sequence( root, frame, view, ext, file );


    fs::path f = root;
    std::string filename = f.filename().string();

    std::string xml;
    fs::path p = fs::absolute( f ).parent_path();
    xml = p.string();

    if ( ! xml.empty() ) xml += "/";

    xml += filename;
    xml += "amf";

    return xml;
}


std::string aces_xml_filename( const char* file )
{

    std::string root, frame, view, ext;
    mrv::split_sequence( root, frame, view, ext, file );

    fs::path f = root;
    std::string filename = f.filename().string();

    std::string xml;
    fs::path p = fs::absolute( f ).parent_path();
    xml = p.string();

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
        if ( c.convert_to != "" && c.convert_from != "" )
        {
            img->append_look_mod_transform( c.convert_to.c_str() );

            size_t num = c.grade_refs.size();
            img->asc_cdl( c.sops );

            for ( size_t i = 0; i < num; ++i )
            {
                std::string name = "LMT." + c.grade_refs[i];
                img->append_look_mod_transform( name.c_str() );
            }
        }

        if ( c.convert_from != "" )
        {
            img->append_look_mod_transform( c.convert_from.c_str() );
        }
        else
        {
            LOG_ERROR( _("Missing Convert_from_WorkSpace.  "
                         "LUT will probably look wrong.") );
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

bool load_amf( CMedia* img, const char* filename )
{
    AMF::AMFReader::AMFError err;
    AMF::AMFReader r;
    if ( ! fs::exists( filename ) ) return false;
    err = r.load( filename );
    if ( err != AMF::AMFReader::kAllOK ) {
        LOG_ERROR( "Could not load " << filename );
        return false;
    }

    pipelineType& p = r.aces.pipeline;
    inputTransformType& i = p.inputTransform;
    if ( !i.transformId.empty() )
        img->idt_transform( i.transformId.c_str() );
    img->idt_transform_id().enabled = i.enabled;
    img->idt_transform_id().applied = i.applied;

    inverseOutputDeviceTransformType& iodt = i.inverseOutputDeviceTransform;
    if ( !iodt.transformId.empty() )
        img->inverse_odt_transform( iodt.transformId.c_str() );
    // img->inverse_odt_transform_id().enabled = iodt.enabled;
    // img->inverse_odt_transform_id().applied = iodt.applied;



    inverseOutputTransformType& iot = i.inverseOutputTransform;
    if ( !iot.transformId.empty() )
        img->inverse_ot_transform( iot.transformId.c_str() );
    // img->inverse_ot_transform_id().enabled = iot.enabled;
    // img->inverse_ot_transform_id().applied = iot.applied;

    inverseReferenceRenderingTransformType& irrt =
        i.inverseReferenceRenderingTransform;
    if ( !irrt.transformId.empty() )
        img->inverse_rrt_transform( irrt.transformId.c_str() );
    // img->inverse_rrt_transform_id().enabled = irrt.enabled;
    // img->inverse_rrt_transform_id().applied = irrt.applied;

    lookTransformType& l = p.lookTransform;
    if ( ! l.transformId.empty() )
        img->append_look_mod_transform( l.transformId.c_str() );

    cdlWorkingSpaceType& c = l.cdlWorkingSpace;

    toCdlWorkingSpaceType& to = c.toCdlWorkingSpace;
    if ( ! to.transformId.empty() )
        img->append_look_mod_transform( to.transformId.c_str() );

    ACES::ASC_CDL sops;

    SOPNodeType& s = l.SOPNode;
    sops.slope( s.slope[0], s.slope[1], s.slope[2] );
    sops.offset( s.offset[0], s.offset[1], s.offset[2] );
    sops.power( s.power[0], s.power[1], s.power[2] );
    sops.saturation( l.SatNode );

    img->asc_cdl( sops );

    if ( s.slope[0] != 1.0f || s.slope[1] != 1.0f || s.slope[2] != 1.0f ||
         s.offset[0] != 0.0f || s.offset[1] != 0.0f || s.offset[2] != 0.0f ||
         s.power[0] != 1.0f || s.power[1] != 1.0f || s.power[2] != 1.0f )
    {
        img->append_look_mod_transform( "LMT.SOPNode" );
    }
    if ( l.SatNode != 1.0f )
    {
        img->append_look_mod_transform( "LMT.SatNode" );
    }
    fromCdlWorkingSpaceType& from = c.fromCdlWorkingSpace;
    if ( ! from.transformId.empty() )
        img->append_look_mod_transform( from.transformId.c_str() );

    outputTransformType& o = p.outputTransform;
    referenceRenderingTransformType& rrt = o.referenceRenderingTransform;
    outputDeviceTransformType& odt = o.outputDeviceTransform;
    img->rendering_transform( rrt.transformId.c_str() );
    mrv::Preferences::ODT_CTL_transform = odt.transformId.c_str();

    return true;
}

bool save_amf( const CMedia* img, const char* filename )
{
    char* oldloc = av_strdup( setlocale( LC_NUMERIC, NULL ) );
    setlocale( LC_NUMERIC, "C" ); // Make floating point values be like 1.2

    AMF::AMFWriter w;
    acesMetadataFile& aces = w.aces;

    aces.version = 1.0;

    char buf[128];
    const char* show = fl_getenv( N_("SHOW") );
    if ( !show ) show = fl_getenv( _("SHOW") );
    if ( !show ) show = _("Unknown Show");

    const char* shot = fl_getenv( N_("SHOT") );
    if ( !shot ) shot = fl_getenv( _("SHOT") );
    if ( !shot ) shot = _("Unknown Shot");

    sprintf( buf, "%s-%s", show, shot );

    clipIdType& c = aces.clipId;
    c.clipName = buf;
    c.file = img->fileroot();
    size_t pos = c.file.rfind( '%' );
    if ( pos != std::string::npos )
    {
        size_t pos2 = c.file.rfind( 'd', pos );
        int digits = atoi( c.file.substr( pos+1, pos2-1 ).c_str() );
        c.sequence.file = c.file.substr(0, pos-1 );
        while ( digits-- )
            c.sequence.file += '#';
        c.sequence.file += c.file.substr( pos2+1, c.file.size() );
        c.sequence.idx = "#";
        c.sequence.min = img->first_frame();
        c.sequence.max = img->last_frame();
    }

    pipelineType& p = aces.pipeline;

    if ( img->idt_transform() )
    {
        inputTransformType& i = p.inputTransform;
        i.applied = false;
        i.enabled = true;
        i.transformId = img->idt_transform();
    }

    size_t i = 0;
    const ACES::ASC_CDL& t = img->asc_cdl();
    size_t num = img->number_of_lmts();
    size_t num_graderefs = img->number_of_grade_refs();

    lookTransformType& l = p.lookTransform;
    l.applied = false;
    l.enabled = true;

    //
    // GradeRefs are transformed into Look Mod Transforms.
    // Here we extract them back again into the xml file.
    //
    if ( num_graderefs > 0 )
    {
        i = num_graderefs + 2;
        if ( i > num )
        {
            LOG_ERROR( _("Missing transforms for aces:GradeRef in '") <<
                       img->name() << "'" );
            i = num;
        }

        cdlWorkingSpaceType& c = l.cdlWorkingSpace;
        toCdlWorkingSpaceType& to = c.toCdlWorkingSpace;
        to.transformId = img->look_mod_transform(0);

        for ( unsigned j = 0; j < num_graderefs; ++j )
        {
            const std::string& g = img->look_mod_transform( j+1 );
            if ( g.substr(4, 7) == N_("SOPNode") )
            {
                SOPNodeType& s = l.SOPNode;
                s.slope[0] = t.slope(0); s.slope[1] = t.slope(1);
                s.slope[2] = t.slope(2);
                s.offset[0] = t.offset(0); s.offset[1] = t.offset(1);
                s.offset[2] = t.offset(2);
                s.power[0] = t.power(0); s.power[1] = t.power(1);
                s.power[2] = t.power(2);
            }
            else if ( g.substr(4, 7) == N_("SatNode") )
            {
                l.SatNode = t.saturation();
            }
            else
            {
                LOG_ERROR( _("Unknown node in CDL") );
            }
        }

        fromCdlWorkingSpaceType& f = c.fromCdlWorkingSpace;
        f.transformId = img->look_mod_transform(i-1);
    }

    if ( img->look_mod_transform(i) )
        l.transformId = img->look_mod_transform(i);

    outputTransformType& o = p.outputTransform;
    if ( img->rendering_transform() )
        o.referenceRenderingTransform.transformId = img->rendering_transform();

    if ( ! mrv::Preferences::ODT_CTL_transform.empty() )
        o.outputDeviceTransform.transformId =
            mrv::Preferences::ODT_CTL_transform;

    if ( ! w.save( filename ) )
        LOG_ERROR( _("Could not save AMF clip data file '") << filename << "'." );
    else
        LOG_INFO( _("Saved AMF clip metadata file '") << filename << "'" );


    setlocale( LC_NUMERIC, oldloc );  // Return floating point form to our locale
    av_free( oldloc );

    return true;
}

bool save_aces_xml( const CMedia* img, const char* filename )
{
    char* oldloc = av_strdup( setlocale( LC_NUMERIC, NULL ) );
    setlocale( LC_NUMERIC, "C" ); // Make floating point values be like 1.2

    ACES::ACESclipWriter c;

    c.info( "mrViewer", mrv::version() );

    char buf[128];
    const char* show = fl_getenv( N_("SHOW") );
    if ( !show ) show = fl_getenv( _("SHOW") );
    if ( !show ) show = _("Unknown Show");

    const char* shot = fl_getenv( N_("SHOT") );
    if ( !shot ) shot = fl_getenv( _("SHOT") );
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

    //
    // GradeRefs are transformed into Look Mod Transforms.
    // Here we extract them back again into the xml file.
    //
    if ( num_graderefs > 0 )
    {
        i = num_graderefs + 2;
        if ( i > num )
        {
            LOG_ERROR( _("Missing transforms for aces:GradeRef in '") <<
                       img->name() << "'" );
            i = num;
        }

        c.gradeRef_start( img->look_mod_transform(0) );

        for ( unsigned j = 0; j < num_graderefs; ++j )
        {
            const std::string& g = img->look_mod_transform( j+1 );
            if ( g.substr(4, 7) == N_("SOPNode") )
            {
                c.gradeRef_SOPNode( t );
            }
            else if ( g.substr(4, 7) == N_("SatNode") )
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


    setlocale( LC_NUMERIC, oldloc );  // Return floating point form to our locale
    av_free( oldloc );

    return true;
}

}
