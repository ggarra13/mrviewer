
#ifndef mrvSave_h
#define mrvSave_h

class ViewerUI;

namespace mrv
{

void save_movie_or_sequence( const char* file, ViewerUI* uiMain,
                             const bool opengl );

bool save_xml( const CMedia* img, mrv::ImageOpts* iopts, const char* file );

} // namespace mrv

#endif
