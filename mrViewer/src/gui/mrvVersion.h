/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo Garramuño

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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

