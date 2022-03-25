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

#ifndef AMFWriter_h
#define AMFWriter_h

#include "AMFExport.h"
#include "AMFReader.h"

namespace AMF {

static const char* kLibVersion = "0.1.0";


using namespace tinyxml2;

/**
 * AMF:  class encompasing an AMF.xml file
 *
 */

class AMF_EXPORT AMFWriter : public AMFReader
{

  protected:
/**
 * Look Modification Transforms is a list
 */

  protected:
    std::string date_time( const time_t& t ) const;
    void aces_dateTime( dateTimeType& dateTime, XMLNode*& root );
    void aces_author( const authorType& author, XMLNode*& root );
    void aces_hash( hashType& t, XMLNode*& root, std::string text );
    void aces_transform_id( std::string transformId, XMLElement*& element );
    void aces_uuid( std::string& uuid );
    void aces_applied( XMLElement*& e, bool applied );
    void aces_anyURI( XMLNode* r, std::string& file );

  public:
    AMFWriter();
    ~AMFWriter() {};


    /*
      File header
     */
    void header();

    /*
      info section
     */
    void info( infoType& t );

    /*
      clipId section
    */
    void clip_id( clipIdType& id );

    /*
      pipelineInfo section
    */
    void pipeline_info( pipelineInfoType& t );

    /*
      inverseReferenceRenderingTransform section
     */
    void inverse_reference_rendering_transform( XMLNode* r,
                                                inverseReferenceRenderingTransformType& t );

    /*
      inverseOutputTransform section
     */
    void inverse_output_transform( XMLNode* r,
                                   inverseOutputTransformType& t );

    /*
      inverseOutputDeviceTransform section
     */
    void inverse_output_device_transform( XMLNode* r,
                                          inverseOutputDeviceTransformType& t );

    /*
      inputTransform section
    */
    void input_transform( inputTransformType& t );

    /*
      lookTransform section
    */
    void look_transform( lookTransformType& t );

    void color_correction_ref( ColorCorrectionRefType& t );

    void sat( XMLNode* r, SatNodeType& t );
    void sop( XMLNode* r, SOPNodeType& s );

    void from_cdl_working_space( XMLNode* r, fromCdlWorkingSpaceType& t );
    void to_cdl_working_space( XMLNode* r, toCdlWorkingSpaceType& t );

    /*
      outputTransform section
    */
    void reference_rendering_transform( referenceRenderingTransformType& t );

    void output_device_transform( outputDeviceTransformType& t );

    void output_transform( outputTransformType& t );

    /*
      pipeline section
    */
    void default_pipeline(pipelineType& pipeline);

    /*
      archivedPipeline section
    */
    void archived_pipeline(pipelineType& pipeline);

    /**
     * Save the XML file to a certain filename
     *
     * @param filename  file to save xml into.  Add prefix and .xml suffix.
     *
     * @return true if success, false if not.
     */
    bool save( const char* filename );

  protected:
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement* element;
    tinyxml2::XMLNode* root, *root2, *root3, *root4, *root5, *root6, *root7;
};


}  // namespace AMF

#endif  // AMFWriter_h
