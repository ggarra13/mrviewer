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

#ifndef ACESclipReader_h
#define ACESclipReader_h

#include <tinyxml2.h>

#include <locale.h>

#ifdef _WIN32
#include <stdlib.h>
#define locale_t _locale_t
#else
#include <xlocale.h>
#endif


#include "ACES_ASC_CDL.h"
#include "ACESTransform.h"
#include "ACESExport.h"


namespace ACES {
 
using namespace tinyxml2;

/**
 * ACESclip:  class encompasing an ACESclip.xml file
 *
 */
 
class ACES_EXPORT ACESclipReader
{
  public:
    enum ACESError
    {
    kAllOK = 0,
    kFileError,
    kNotAnAcesFile,
    kErrorParsingElement,
    kErrorVersion,
    kNoAcesInfo,
    kNoClipID,
    kNoConfig,
    kMissingSpaceConversion,
    kNoInputTransformList,
    kNoPreviewTransformList,
    kLastError
    };

   enum BitDepth
   {
   k10i,
   k12i,
   k16i,
   k16f,
   k32f,
   kLastBitDepth
   };

/**
 * Look Modification Transforms is a list
 */
    typedef std::vector< Transform > LMTransforms;
    typedef std::vector< std::string > GradeRefs;

  protected:
    std::string     date_time( const char* dt );
    TransformStatus get_status( const std::string& s );
    BitDepth        get_bit_depth( const std::string& s );
    void parse_V3( const char* s, float out[3] );

  public:
    ACESclipReader();
    ~ACESclipReader();

    const char* error_name( ACESError err ) const;

    ACESError header();
    ACESError info();
    ACESError clip_id();
    ACESError GradeRef();
    ACESError config();
    ACESError ITL();
    ACESError PTL();

    /** 
     * Load the XML file
     * 
     * @param filename  file to load xml from.  Add prefix and .xml suffix.
     * 
     * @return ACESError.
     */
    ACESError load( const char* filename );

  public:
    // aces:Info
    std::string application;
    std::string version;
    std::string comment;

    // aces:clipID
    std::string clip_name;
    std::string media_id;
    std::string clip_date;

    // aces:Config
    std::string timestamp;

    // aces::GradeRef
    TransformStatus graderef_status;
    std::string convert_to, convert_from;
    BitDepth in_bit_depth, out_bit_depth;
    GradeRefs grade_refs;
    ASC_CDL  sops;

    Transform IDT;
    LMTransforms LMT;
    Transform RRTODT, RRT, ODT;
    std::string link_ITL;
    std::string link_PTL;

  protected:
    tinyxml2::XMLDocument doc;
    XMLElement* element;
    XMLNode* root, *root2, *root3, *root4;
    locale_t loc;
};


}  // namespace ACES

#endif  // ACESclipReader_h
