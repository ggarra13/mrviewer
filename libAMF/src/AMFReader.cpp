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

#include <stdio.h>
#ifndef _WIN32
#include <locale.h>
#endif
#include <iostream>

#ifdef _WIN32
#include <time.h>
#include <stdlib.h>
#define locale_t _locale_t
#define strtod_l _strtod_l
#endif

#include "AMFReader.h"


namespace AMF {

using namespace tinyxml2;




/**
 */
std::string AMFReader::date_time( const char* i )
{
    if (!i) return "";

    std::string s = i;
    size_t e = s.find('T');
    std::string r = s.substr(0, e );
    r += " Time: ";
    r += s.substr( e+1, s.size() );
    return r;
}



/**
 * Parse a vector of 3 float numbers
 *
 * @param s    3 float numbers as a string separated by spaces
 * @param out  the 3 float numbers
 */
void AMFReader::parse_V3( const char* v3, float out[3]  )
{
    const char* s = v3;
    char* e;
    out[0] = (float) strtod( s, &e ); s = e;
    out[1] = (float) strtod( s, &e ); s = e;
    out[2] = (float) strtod( s, &e );
}

/**
 * Constructor
 *
 */
    AMFReader::AMFReader() :
        loc( NULL )
    {
        char* current = setlocale( LC_NUMERIC, NULL );
        if ( current ) loc = strdup( current );
        setlocale(LC_NUMERIC, "C");
    }

    AMFReader::~AMFReader()
    {
        setlocale( LC_NUMERIC, loc );
        if ( loc ) free( loc );
    }


const char* AMFReader::error_name( AMFReader::AMFError err ) const
{
    switch( err )
    {
        case kAllOK:
            return "ALL OK";
        case kFileError:
            return "File Error";
        case kNotAnAcesFile:
            return "Not an AMF clip file";
        case kErrorParsingElement:
            return "Error Parsing Element";
        case kErrorVersion:
            return "Not a supported version";
        case kNoAcesInfo:
            return "No aces:Info";
        case kNoClipID:
            return "No aces:clipID";
        case kNoConfig:
            return "No aces:Config";
        case kMissingSpaceConversion:
            return "Missing color space conversion";
        case kNoInputTransformList:
            return "No InputTransformList";
        case kNoPreviewTransformList:
            return "No PreviewTransformList";
        case kAuthorError:
            return "aces:author Error";
        case kLastError:
        default:
            return "Unknown Error";
    };

}

AMFReader::AMFError AMFReader::aces_date_time( XMLNode* root, dateTimeType& a )
{
    XMLNode* r = root->FirstChildElement( "aces:dateTime" );
    if ( !r ) return kAllOK;

    element = r->FirstChildElement( "aces:creationDateTime" );
    if ( !element ) return kErrorParsingElement;
    a.creationDateTime = element->GetText();

    element = r->FirstChildElement( "aces:modificationDateTime" );
    if ( !element ) return kErrorParsingElement;
    a.modificationDateTime = element->GetText();

    return kAllOK;
}

AMFReader::AMFError AMFReader::aces_author( XMLNode* root, authorType& a )
{
    XMLNode* r = root->FirstChildElement( "aces:author" );
    if ( !r ) return kAllOK;

    element = r->FirstChildElement( "aces:name" );
    if ( !element ) return kAuthorError;
    a.name = element->GetText();

    element = r->FirstChildElement( "aces:emailAddress" );
    if ( !element ) return kAllOK;
    a.emailAddress = element->GetText();

    return kAllOK;
}

AMFReader::AMFError AMFReader::aces_hash( XMLNode* r, hashType& h )
{
    element = r->FirstChildElement( "aces:hash" );
    if ( !element ) return kAllOK;

    const char* algo = element->Attribute( "algorithm" );
    if ( algo ) h.algorithm = algo;
    else return kErrorParsingElement;

    h.hash = element->GetText();

    return kAllOK;
}

AMFReader::AMFError AMFReader::aces_uuid( XMLNode* r, UUIDType& a )
{
    element = r->FirstChildElement( "aces:uuid" );
    if ( !element ) return kAllOK;

    std::string uuid = element->GetText();
    a = uuid.replace( uuid.begin(), uuid.begin()+kUUID.size(), "" );

    return kAllOK;
}

AMFReader::AMFError AMFReader::aces_description( XMLNode* r,
                                                 std::string& d )
{
    element = r->FirstChildElement( "aces:description" );
    if ( !element || !element->GetText() ) return kAllOK;

    d = element->GetText();
    return kAllOK;
}

AMFReader::AMFError AMFReader::aces_anyURI( XMLNode* root,
                                            std::string& d )
{
    element = root->FirstChildElement( "aces:file" );
    if ( !element || !element->GetText() ) return kAllOK;

    d = element->GetText();
    return kAllOK;
}

AMFReader::AMFError AMFReader::info()
{
    root = element = doc.FirstChildElement( "aces:acesMetadataFile" );
    if ( !root ) return kNotAnAcesFile;

    aces.version = element->FloatAttribute( "version" );

    XMLNode* root2 = root->FirstChildElement( "aces:amfInfo" );
    if ( !root2 ) return kNotAnAcesFile;

    AMFError err = kAllOK;
    aces_description( root2, aces.amfInfo.description );
    err = aces_author( root2, aces.amfInfo.author );
    if ( err != kAllOK ) return err;
    err = aces_date_time( root2, aces.amfInfo.dateTime );
    if ( err != kAllOK ) return err;
    aces_uuid( root2, aces.amfInfo.uuid );
    return err;
}

AMFReader::AMFError AMFReader::clip_id( clipIdType& c )
{
    XMLNode* root2 = root->FirstChildElement( "aces:clipId" );
    if ( !root2 ) return kAllOK;

    element = root2->FirstChildElement( "aces:clipName" );
    if ( !element ) return kErrorParsingElement;
    if ( !element->GetText() ) return kAllOK;
    c.clipName = element->GetText();
    element = root2->FirstChildElement( "aces:sequence" );
    if ( element ) {
        const char* idx = element->Attribute( "idx" );
        if ( !idx ) return kErrorParsingElement;
        c.sequence.idx = idx;
        int min = element->IntAttribute( "min" );
        c.sequence.min = min;
        int max = element->IntAttribute( "max" );
        c.sequence.max = max;
        if ( element->GetText() )
            c.sequence.file = element->GetText();
    }
    aces_uuid( root2, c.uuid );
    aces_anyURI( root2, c.file );

    return kAllOK;
}

AMFReader::AMFError AMFReader::aces_transform_id( XMLNode* r, std::string& t )
{
    element = r->FirstChildElement( "aces:transformId" );
    if ( !element ) return kErrorVersion;

    t = element->GetText();
    t = t.replace( t.begin(), t.begin()+kTRANSFORM_ID.size(), "" );
    return kAllOK;
}

AMFReader::AMFError AMFReader::aces_system_version( XMLNode* r, versionType& t )
{
    XMLNode* root2 = r->FirstChildElement( "aces:systemVersion" );
    if ( !root2 ) return kErrorVersion;

    element = root2->FirstChildElement( "aces:majorVersion" );
    if ( !element ) return kErrorVersion;
    t.majorVersion = atoi( element->GetText() );

    element = root2->FirstChildElement( "aces:minorVersion" );
    if ( !element ) return kErrorVersion;
    t.minorVersion = atoi( element->GetText() );

    element = root2->FirstChildElement( "aces:patchVersion" );
    if ( !element ) return kErrorVersion;
    t.patchVersion = atoi( element->GetText() );

    return kAllOK;
}

void AMFReader::aces_applied( XMLElement* element, bool& t )
{
    const char* applied = element->Attribute( "applied" );
    if ( applied )
    {
        std::string a = applied;
        if ( a == "true" || a == "1" )
            t = true;
        else
            t = false;
    }
}

void AMFReader::aces_enabled( XMLElement* element, bool& t )
{
    t = true;
    const char* enabled = element->Attribute( "enabled" );
    if ( enabled )
    {
        std::string a = enabled;
        if ( a == "true" || a == "1" )
            t = true;
        else
            t = false;
    }
}

AMFReader::AMFError
AMFReader::output_device_transform( XMLNode* r,
                                    outputDeviceTransformType& t )
{
    XMLNode* root2 = element =
                     r->FirstChildElement( "aces:outputDeviceTransform" );
    if ( !root2 ) return kAllOK;
    aces_description( root2, t.description );
    aces_uuid( root2, t.uuid );
    aces_hash( root2, t.hash );
    aces_anyURI( root2, t.file );
    aces_transform_id( root2, t.transformId );
    return kAllOK;
}

AMFReader::AMFError
AMFReader::inverse_output_device_transform( XMLNode* r,
                                            inverseOutputDeviceTransformType& t )
{
    XMLNode* root2 = element =
                     r->FirstChildElement( "aces:inverseOutputDeviceTransform" );
    if ( !root2 ) return kErrorParsingElement;
    aces_description( root2, t.description );
    aces_uuid( root2, t.uuid );
    aces_hash( root2, t.hash );
    aces_anyURI( root2, t.file );
    aces_transform_id( root2, t.transformId );
    return kAllOK;
}

AMFReader::AMFError
AMFReader::reference_rendering_transform( XMLNode* r,
                                          referenceRenderingTransformType& t )
{
    XMLNode* root2 = element =
                     r->FirstChildElement( "aces:referenceRenderingTransform" );
    if ( !root2 ) return kAllOK;
    aces_description( root2, t.description );
    aces_uuid( root2, t.uuid );
    aces_hash( root2, t.hash );
    aces_anyURI( root2, t.file );
    aces_transform_id( root2, t.transformId );
    return kAllOK;
}

AMFReader::AMFError
AMFReader::inverse_reference_rendering_transform( XMLNode* r,
                                                  inverseReferenceRenderingTransformType& t )
{
    XMLNode* root2 = element =
                     r->FirstChildElement( "aces:inverseReferenceRenderingTransform" );
    if ( !root2 ) return kAllOK;
    aces_description( root2, t.description );
    aces_uuid( root2, t.uuid );
    aces_hash( root2, t.hash );
    aces_anyURI( root2, t.file );
    aces_transform_id( root2, t.transformId );
    return kAllOK;
}

AMFReader::AMFError
AMFReader::inverse_output_transform( XMLNode* r,
                                     inverseOutputTransformType& t )
{
    XMLNode* root2 = element =
                     r->FirstChildElement( "aces:inverseOutputTransform" );
    if ( !root2 ) return kAllOK;
    aces_description( root2, t.description );
    aces_uuid( root2, t.uuid );
    aces_hash( root2, t.hash );
    aces_anyURI( root2, t.file );
    aces_transform_id( root2, t.transformId );
    return kAllOK;
}

AMFReader::AMFError AMFReader::output_transform( XMLNode* r,
                                                 outputTransformType& t )
{
    XMLNode* root2 = element = r->FirstChildElement( "aces:outputTransform" );
    if ( !root2 ) return kAllOK;


    AMFError err = kAllOK;
    aces_description( root2, t.description );
    aces_uuid( root2, t.uuid );
    aces_hash( root2, t.hash );
    aces_anyURI( root2, t.file );
    aces_transform_id( root2, t.transformId );



    err = reference_rendering_transform( root2, t.referenceRenderingTransform );
    if ( err != kAllOK ) return err;


    err = output_device_transform( root2, t.outputDeviceTransform );

    return err;
}

AMFReader::AMFError
AMFReader::from_cdl_working_space( XMLNode* r,
                                   fromCdlWorkingSpaceType& t )
{
    XMLNode* root2 = r->FirstChildElement( "aces:fromLookTransformWorkingSpace" );
    if ( !root2 ) return kAllOK;

    aces_description( root2, t.description );
    aces_uuid( root2, t.uuid );
    aces_hash( root2, t.hash );
    aces_anyURI( root2, t.file );
    aces_transform_id( root2, t.transformId );


    return kAllOK;
}

AMFReader::AMFError
AMFReader::to_cdl_working_space( XMLNode* r, toCdlWorkingSpaceType& t )
{
    XMLNode* root2 = r->FirstChildElement( "aces:toLookTransformWorkingSpace" );
    if ( !root2 ) return kAllOK;

    aces_description( root2, t.description );
    aces_uuid( root2, t.uuid );
    aces_hash( root2, t.hash );
    aces_anyURI( root2, t.file );
    aces_transform_id( root2, t.transformId );

    return kAllOK;
}

AMFReader::AMFError
AMFReader::cdl_working_space( XMLNode* r, cdlWorkingSpaceType& c )
{
    XMLNode* root2 = r->FirstChildElement( "aces:lookTransformWorkingSpace" );
    if ( !root2 ) return kAllOK;

    AMFError err;
    err = to_cdl_working_space( root2, c.toCdlWorkingSpace );
    if ( err != kAllOK ) return err;
    err = from_cdl_working_space( root2, c.fromCdlWorkingSpace );
    if ( err != kAllOK ) return err;
    return kAllOK;

}

AMFReader::AMFError AMFReader::sat_node( XMLNode* r,
                                         SatNodeType& t )
{
    XMLNode* root = r->FirstChildElement( "cdl:SatNode" );
    if ( !root ) return kAllOK;
    element = root->FirstChildElement( "cdl:Saturation" );
    if ( !element || !element->GetText() ) return kErrorParsingElement;

    t = atof( element->GetText() );
    return kAllOK;
}

AMFReader::AMFError AMFReader::sop_node( XMLNode* r,
                                         SOPNodeType& t )
{
    XMLNode* root2 = r->FirstChildElement( "cdl:SOPNode" );
    if ( !root2 ) return kAllOK;

    element = root2->FirstChildElement( "cdl:Slope" );
    if ( !element || !element->GetText() ) return kErrorParsingElement;

    sscanf( element->GetText(), "%g %g %g", &t.slope[0], &t.slope[1],
            &t.slope[2] );

    element = root2->FirstChildElement( "cdl:Offset" );
    if ( !element || !element->GetText() ) return kErrorParsingElement;

    sscanf( element->GetText(), "%g %g %g", &t.offset[0], &t.offset[1],
            &t.offset[2] );

    element = root2->FirstChildElement( "cdl:Power" );
    if ( !element || !element->GetText() ) return kErrorParsingElement;

    sscanf( element->GetText(), "%g %g %g", &t.power[0], &t.power[1],
            &t.power[2] );

    return kAllOK;
}

AMFReader::AMFError AMFReader::color_correction_ref( XMLNode* r,
                                                     ColorCorrectionRefType& t )
{
    XMLNode* root2 = element = r->FirstChildElement( "aces:colorCorrectionRef" );
    if ( !root2 ) return kAllOK;

    element = root2->FirstChildElement( "ref" );
    if ( !element || !element->GetText() ) return kErrorParsingElement;

    t.ref = element->GetText();
    return kAllOK;
}

AMFReader::AMFError AMFReader::look_transform( XMLNode* r,
                                               lookTransformType& t )
{
    XMLNode* root2 = element = r->FirstChildElement( "aces:lookTransform" );
    if ( !root2 ) return kAllOK;


    AMFError err = kAllOK;
    aces_applied( element, t.applied );
    aces_enabled( element, t.enabled );
    aces_description( root2, t.description );
    aces_uuid( root2, t.uuid );
    aces_hash( root2, t.hash );
    aces_anyURI( root2, t.file );
    aces_transform_id( root2, t.transformId );

    err = cdl_working_space( root2, t.cdlWorkingSpace );
    if ( err != kAllOK ) return err;

    sop_node( root2, t.SOPNode );
    sat_node( root2, t.SatNode );

    color_correction_ref( root2, t.ColorCorrectionRef );

    r->InsertEndChild( root2 );
    return kAllOK;
}


AMFReader::AMFError AMFReader::input_transform( XMLNode* r,
                                                inputTransformType& t )
{
    XMLNode* root2 = element = r->FirstChildElement( "aces:inputTransform" );
    if ( !root2 ) return kAllOK;


    AMFError err = kAllOK;
    aces_applied( element, t.applied );
    aces_enabled( element, t.enabled );
    aces_description( root2, t.description );
    aces_uuid( root2, t.uuid );
    aces_hash( root2, t.hash );
    aces_anyURI( root2, t.file );
    aces_transform_id( root2, t.transformId );

    inverse_output_transform( root2, t.inverseOutputTransform );
    inverse_output_device_transform( root2, t.inverseOutputDeviceTransform );
    inverse_reference_rendering_transform( root2,
                                           t.inverseReferenceRenderingTransform );

    return kAllOK;
}

AMFReader::AMFError AMFReader::pipeline_info( XMLNode* r, pipelineInfoType& t )
{
    XMLNode* root2 = r->FirstChildElement( "aces:pipelineInfo" );
    if ( !root2 ) return kNotAnAcesFile;

    AMFError err = kAllOK;
    aces_description( root2, t.description );
    err = aces_author( root2, t.author );
    if ( err != kAllOK ) return err;
    err = aces_date_time( root2, t.dateTime );
    if ( err != kAllOK ) return err;
    aces_uuid( root2, t.uuid );

    aces_system_version( root2, t.systemVersion );

    return kAllOK;
};

AMFReader::AMFError AMFReader::pipeline( pipelineType& t )
{
    XMLNode* root2 = root->FirstChildElement( "aces:pipeline" );


    AMFError err = pipeline_info( root2, t.pipelineInfo );
    if ( err != kAllOK ) return err;


    err = input_transform( root2, t.inputTransform );
    if ( err != kAllOK ) return err;

    err = look_transform( root2, t.lookTransform );
    if ( err != kAllOK ) return err;

    err = output_transform( root2, t.outputTransform );
    if ( err != kAllOK ) return err;

    return kAllOK;
};


/**
 * First Step.  Load the XML file.
 *
 * @param filename file to load the XML file from.
 *
 * @return true on success, false on failure
 */
AMFReader::AMFError AMFReader::load( const char* filename )
{
    XMLError e = doc.LoadFile( filename );
    if ( e != XML_SUCCESS ) return kFileError;


    AMFError err = info();
    if ( err != kAllOK ) return err;

    err = clip_id( aces.clipId );
    if ( err != kAllOK ) return err;

    err = pipeline( aces.pipeline );
    if ( err != kAllOK ) return err;

    return kAllOK;
}

}  // namespace AMF
