/* 
Copyright (c) 2015, Gonzalo Garramuño
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer. 
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies, 
either expressed or implied, of the FreeBSD Project.
*/

#ifndef ACESclipWriter_h
#define ACESclipWriter_h

#include <time.h>

#include <tinyxml2.h>

#include "ACESExport.h"
#include "ACESTransform.h"

namespace ACES {

static const char* kLibVersion = "0.2.1";


using namespace tinyxml2;

/**
 * ACESclip:  class encompasing an ACESclip.xml file
 *
 */
 
class ACES_EXPORT ACESclipWriter
{

  protected:
/**
 * Look Modification Transforms is a list
 */
    typedef std::vector< Transform > LMTransforms;

  protected:
    std::string date_time( const time_t& t ) const;
    void set_status( TransformStatus s );

  public:
    ACESclipWriter();
    ~ACESclipWriter() {};

    /** 
     * aces:Info section
     * 
     * @param application application used to save xml file. 
     * @param version     version of application used.
     * @param comment     some useful comment
     */
    void info( const std::string application = "ACESclipLib",
               const std::string version = kLibVersion,
               const std::string comment = "" );

    /** 
     * aces:clipID section
     * 
     * @param clip_name name of the clip (image name, for example)
     * @param media_id  media id ( show,shot,take, or reel for example )
     * @param clip_date date of clip as returned by stat
     */
    void clip_id( const std::string clip_name,
                  const std::string media_id,
                  const time_t clip_date = time(0) );

    /** 
     * aces:Config section
     * 
     * @param xml_date date of xml creation
     */
    void config( const time_t xml_date = time(0) );

    /** 
     * Input Transform List beginnings.
     * 
     */
    void ITL_start( TransformStatus status = kPreview );

    /** 
     * Add Input Device Transform (IDT) to ITL.
     * 
     * @param name    name of the transform (without .ctl extension)
     * @param status  status of transform (preview or applied)
     */
    void add_IDT( const std::string name, 
                  TransformStatus status = kPreview );

    /** 
     * Input Transform List ending
     * 
     * @param it pointer to combined process list
     */
    void ITL_end( const std::string it = "" );


    /** 
     * Preview Transform List beginning.
     * 
     */
    void PTL_start();

    /** 
     * Append one Look Modification Transform to PTL.  Multiple
     * calls on this function are allowed.
     * 
     * @param name     name of the transform (without .ctl extension)
     * @param status   status of transform (preview or applied)
     */
    void add_LMT( const std::string name, 
                  TransformStatus status = kPreview,
                  const std::string link_transform = "" );

    /** 
     * Add a Reference Rendering Transform to PTL.  Only a single
     * call to it is accepted.
     * 
     * @param name    name of the transform (without .ctl extension)
     * @param status  status of transform (preview or applied)
     */
    void add_RRT( const std::string name, 
                  TransformStatus status = kPreview );

    /** 
     * Add an Output Device Transform to PTL.  Only a single call
     * to it is accepted.
     * 
     * @param name    name of the transform (without .ctl extension)
     * @param status  status of transform (preview or applied)
     */

    void add_ODT( const std::string name, 
                  TransformStatus status = kPreview,
                  const std::string link_transform = "" );

    /** 
     * Add a Reference Rendering Transform and a combined ODT to PTL.
     * Only a single call to it is accepted.
     * 
     * @param name    name of the transform (without .ctl extension)
     * @param status  status of transform (preview or applied)
     */
    void add_RRTODT( const std::string name, 
                     TransformStatus status = kPreview );

    /** 
     * Preview Transform List ending.
     * 
     * @param t Combined LMTs + RRT + ODT (optional)
     */
    void PTL_end( const std::string t = "" );


    /** 
     * Save the XML file to a certain filename
     * 
     * @param filename  file to save xml into.  Add prefix and .xml suffix.
     * 
     * @return true if success, false if not.
     */
    bool save( const char* filename );

  protected:
    XMLDocument doc;
    XMLElement* element;
    XMLNode* root, *root2, *root3;

    Transform cvt_to_workspace, cvt_from_workspace;


    LMTransforms LMT;
    Transform IDT, RRT, RRTODT, ODT;
};


}  // namespace ACES

#endif  // ACESclipWriter_h
