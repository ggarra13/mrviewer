
#include <string>

namespace mrv {

class CMedia;

std::string aces_xml_filename( const char* file );
bool load_aces_xml( CMedia* img, const char* filename );
bool save_aces_xml( const CMedia* img, const char* filename );

}
