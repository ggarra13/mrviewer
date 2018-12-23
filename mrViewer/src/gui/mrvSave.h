
#ifndef mrvSave_h
#define mrvSave_h

namespace mrv
{
class ViewerUI;

void save_movie_or_sequence( const char* file, const mrv::ViewerUI* uiMain,
                             const bool opengl );

bool save_xml( const CMedia* img, ImageOpts* iopts, const char* file );

} // namespace mrv

#endif
