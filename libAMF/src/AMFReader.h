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

#ifndef AMFReader_h
#define AMFReader_h

#include <tinyxml2.h>

#ifdef _WIN32
#include <stdlib.h>
#define locale_t _locale_t
#else
#include <locale.h>
#endif


#include "AMFExport.h"
#include "AMFBase.h"

namespace AMF {

using tinyxml2::XMLNode;
using tinyxml2::XMLElement;

/**
 * AMF:  class encompasing an AMF.xml file
 *
 */
class AMF_EXPORT AMFReader
{
  public:
    enum AMFError
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
    kAuthorError,
    kLastError
    };


/**
 * Look Modification Transforms is a list
 */

  protected:
    std::string     date_time( const char* dt );
    void parse_V3( const char* s, float out[3] );

    AMFError aces_author( XMLNode* root, authorType& a );
    AMFError aces_date_time( XMLNode* root, dateTimeType& a );
    AMFError aces_uuid( XMLNode* root, UUIDType& a );
    AMFError aces_hash( XMLNode* root, hashType& a );
    AMFError aces_description( XMLNode* root, std::string& d );
    AMFError aces_anyURI( XMLNode* root, std::string& d );
    AMFError aces_system_version( XMLNode* root2, versionType& t );
    AMFError aces_transform_id( XMLNode* r, std::string& t );
    void aces_applied( XMLElement* element, bool& t );
    void aces_enabled( XMLElement* element, bool& t );

  public:
    AMFReader();
    ~AMFReader();

    const char* error_name( AMFError err ) const;

    /**
     * Load the XML file
     *
     * @param filename  file to load xml from.  Add prefix and .xml suffix.
     *
     * @return AMFError.
     */
    AMFError load( const char* filename );

  public:
    acesMetadataFile aces;

    AMFError info();
    AMFError clip_id( clipIdType& c );
    AMFError reference_rendering_transform( XMLNode* r,
                                            referenceRenderingTransformType& t );
    AMFError output_device_transform( XMLNode* r,
                                      outputDeviceTransformType& t );
    AMFError output_transform( XMLNode* r,
                               outputTransformType& t );
    AMFError color_correction_ref( XMLNode* r, ColorCorrectionRefType& t );
    AMFError sat_node( XMLNode* r, SatNodeType& t );
    AMFError sop_node( XMLNode* r, SOPNodeType& t );
    AMFError to_cdl_working_space( XMLNode* r, toCdlWorkingSpaceType& c );
    AMFError from_cdl_working_space( XMLNode* r, fromCdlWorkingSpaceType& c );
    AMFError cdl_working_space( XMLNode* r, cdlWorkingSpaceType& c );
    AMFError look_transform( XMLNode* r, lookTransformType& t );

    AMFError inverse_reference_rendering_transform( XMLNode* r,
                                                    inverseReferenceRenderingTransformType& t );
    AMFError inverse_output_device_transform( XMLNode* r,
                                              inverseOutputDeviceTransformType& t );
    AMFError inverse_output_transform( XMLNode* r,
                                       inverseOutputTransformType& t );
    AMFError input_transform( XMLNode* r, inputTransformType& t );
    AMFError pipeline_info( XMLNode* r, pipelineInfoType& t );
    AMFError pipeline( pipelineType& t );

  protected:
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement* element;
    tinyxml2::XMLNode* root, *root2, *root3, *root4;
    char* loc;
};


}  // namespace AMF

#endif  // AMFReader_h
