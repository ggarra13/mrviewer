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
  stringArray open_image_file( const char* startfile = NULL );

  /** 
   * Opens a file requester to load a color profile
   * 
   * @param startfile start filename (directory)
   * 
   * @return opened color profile or null
   */
  const char* open_icc_profile( const char* startfile = NULL,
				const char* title = "Load Image's ICC Profile" );

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
   * Attach a CTL (Color Transform Language) script to an image for display.
   * 
   * @param image      already loaded image
   * @param startfile  start filename (directory)
   */
  void attach_ctl_script( CMedia* image, const char* ctlfile );
  void attach_ctl_script( CMedia* image );



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
  void save_image_file( const CMedia* image,
			const char* startdir = NULL );

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
