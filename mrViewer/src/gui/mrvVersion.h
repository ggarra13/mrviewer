/**
 * @file   mrvVersion.h
 * @author gga
 * @date   Wed Oct 25 01:45:43 2006
 * 
 * @brief  Versioning information for mrViewer
 * 
 * 
 */

#ifndef mrvVersion_h
#define mrvVersion_h

#include <string>

namespace fltk
{
class Browser;
}

namespace mrv
{
  class ViewerUI;

void ffmpeg_formats( fltk::Browser& b );
void ffmpeg_video_codecs( fltk::Browser& b );
void ffmpeg_audio_codecs( fltk::Browser& b  );
void ffmpeg_subtitle_codecs( fltk::Browser& b);
std::string ffmpeg_protocols();
std::string ffmpeg_motion_estimation_methods();


  const char* version();

  std::string about_message();

  std::string cpu_information();
  std::string gpu_information( mrv::ViewerUI* uiMain );

} // namespace mrv


#endif // mrvVersion_h

