#include <vector>

#include <boost/filesystem.hpp>

#include <FL/Fl.H>
#include <FL/fl_utf8.h>

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

    if ( !fs::exists( f ) )
        return "";

    std::string xml;
    fs::path p = fs::absolute( f ).parent_path();
#ifdef _WIN32
    xml = p.string();
#else
    xml = fs::canonical( p ).string();
#endif
    if ( ! xml.empty() ) xml += "/";
    xml += "ACESclip.";
    xml += filename;
    xml += "xml";


    return xml;
}


bool load_aces_xml( CMedia* img, const char* filename )
{
    return true;
}

bool save_aces_xml( const CMedia* img, const char* filename )
{
    return true;
}

}
