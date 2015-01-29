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
 * @file   mrvFileRequester.h
 * @author gga
 * @date   Fri Jul  6 17:38:52 2007
 * 
 * @brief  This file defines several helper functions for file requesters.
 * 
 * 
 */

#ifndef mrvFileRequester_h
#define mrvFileRequester_h

#include "mrvString.h"


namespace mrv
{

  class CMedia;

  /** 
   * Opens a file requester to load a reel
   * 
   * @param startfile directory to start from
   * 
   * @return opened reel(s)
   */
  stringArray open_reel( const char* startfile = NULL );

  /** 
   * Opens a file requester to load an image
   * 
   * @param startfile start filename (directory)
   * 
   * @return opened filename(s)
   */
stringArray open_image_file( const char* startfile = NULL,
			     const bool compact_files = true );

  /** 
   * Opens a file requester to load a color profile
   * 
   * @param startfile start filename (directory)
   * 
   * @return opened color profile or null
   */
  const char* open_icc_profile( const char* startfile = NULL,
				const char* title = "Load Image's ICC Profile" );

  const char* open_ctl_script( const char* startfile = NULL,
                               const char* title = "Load CTL Script" );

  /** 
   * Attach an ICC/ICM color profile to a loaded image
   * 
   * @param image      already loaded image
   * @param startfile  start filename (directory)
   */
  void attach_icc_profile( CMedia* image, const char* startfile );
  void attach_icc_profile( CMedia* image );


  /** 
   * Attach a CTL display script to a monitor for display
   * 
   * @param index monitor index
   */
  void monitor_ctl_script( const unsigned index = 0, 
			   const char* startfile = NULL );

  /** 
   * Attach an ICC/ICM color profile to a monitor display
   * 
   * @param index monitor index
   */
  void monitor_icc_profile( const unsigned index = 0 );


  /** 
   * Attach a CTL (Color Transform Language) RT script to an image for display.
   * 
   * @param image      already loaded image
   * @param startfile  start filename (directory)
   */
  void attach_ctl_script( CMedia* image, const char* ctlfile );
  void attach_ctl_script( CMedia* image );

  /**
   * Attach a CTL (Color Transform Language) LMT script to an image for display.
   * 
   * @param image      already loaded image
   * @param startfile  start filename (directory)
   */
  void attach_ctl_lmt_script( CMedia* image, const char* ctlfile );
  void attach_ctl_lmt_script( CMedia* image );

  /**
   * Attach a CTL (Color Transform Language) IDT script to an image for display.
   * 
   * @param image      already loaded image
   * @param startfile  start filename (directory)
   */
  void attach_ctl_idt_script( CMedia* image, const char* ctlfile );
  void attach_ctl_idt_script( CMedia* image );

  /** 
   * Opens a file requester to load audio files
   * 
   * @param startfile  start filename (directory)
   * 
   * @return  opened audio file or null
   */
  const char* open_audio_file( const char* startfile = NULL );

  /** 
   * Attach a new audio file to loaded sequence
   * 
   * @param image      already loaded image
   * @param startfile  start filename (directory)
   */
  void attach_audio( CMedia* image, const char* startfile );

  /** 
   * Save an image under a new filename
   * 
   * @param image      image to save (directory)
   * @param startdir   start directory to save to
   */
  void save_image_file( CMedia* image,
			const char* startdir = NULL );
  /** 
   * Save an image under a new filename
   * 
   * @param uiMain     main widget class
   * @param startdir   start directory to save to
   * @param opengl     use opengl snapshots
   */
  void save_sequence_file( const mrv::ViewerUI* uiMain,
			   const char* startdir = NULL,
                           bool opengl = false );

  /** 
   * Save a reel under a new filename
   * 
   * @param startdir start directory to save to
   * 
   * @return reel to save or NULL
   */
  const char* save_reel( const char* startdir = NULL );

} // namespace mrv


#endif // mrvFileRequester
