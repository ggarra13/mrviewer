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

#include "SHA1/SHA1.h"
#include "md5/md5.h"
#include "SHA256.h"


#define _CRT_SECURE_NO_WARNINGS

#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp> // input-output
#include <boost/lexical_cast.hpp> // for casting uuid into std::string

 //#include <gnutls/openssl.h>

#include "AMFWriter.h"


namespace AMF {

static const double kVersion = 1.0;
static const double kContainerVersion = 1.0;

using namespace tinyxml2;


/**
 * Return a date and time information in AMF format.
 *
 * @param t time_t to convert to string.
 *
 * @return std::string with the formatted date and time.
 */
std::string AMFWriter::date_time( const time_t& t ) const
{
    char buf[64];
    struct tm* now = localtime( &t );
    sprintf( buf, "%d-%02d-%02dT%02d:%02d:%02d",
             now->tm_year+1900, now->tm_mon+1, now->tm_mday,
             now->tm_hour, now->tm_min, now->tm_sec );
    return buf;
}



    void AMFWriter::aces_dateTime( dateTimeType& dateTime,
                                   XMLNode*& root )
    {
        root = doc.NewElement("aces:dateTime");
        element = doc.NewElement( "aces:creationDateTime" );
        element->SetText( dateTime.creationDateTime.c_str() );
        root->InsertEndChild( element );
        element = doc.NewElement( "aces:modificationDateTime" );
        element->SetText( dateTime.modificationDateTime.c_str() );
        root->InsertEndChild( element );
    }

void AMFWriter::aces_uuid( std::string& uuid )
{
    boost::uuids::uuid id = boost::uuids::random_generator()();
    std::string UUID = boost::lexical_cast<std::string>(id);
    uuid = kUUID + UUID;
}

void AMFWriter::aces_applied( XMLElement*& element, bool applied )
 {
    if ( applied )
        element->SetAttribute( "applied", "true" );
    else
        element->SetAttribute( "applied", "false" );
 }
/**
 * Constructor
 *
 */
AMFWriter::AMFWriter()
{

}

void AMFWriter::header()
{
    XMLDeclaration* decl = doc.NewDeclaration( NULL );
    doc.InsertFirstChild( decl );  // ?xml version="1.0" encoding="UTF-8"

    element = doc.NewElement( "aces:acesMetadataFile" );
    element->SetAttribute( "xmlns:aces", "urn:ampas:aces:amf:v1.0");
    element->SetAttribute( "xsi:schemaLocation", "urn:ampas:aces:amf:v1.0 file:acesMetadataFile.xsd");
    element->SetAttribute( "xmlns:cdl", "urn:ASC:CDL:v1.01");
    element->SetAttribute( "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    element->SetAttribute( "version", aces.version );
    doc.InsertAfterChild( decl, element );
    root = element;
}

void AMFWriter::aces_author( const authorType& author, XMLNode*& r )
{
    if ( ! author.name.empty() ||
         ! author.emailAddress.empty() )
    {
        root6 = doc.NewElement( "aces:author" );
        if ( !author.name.empty() )
        {
            element = doc.NewElement( "aces:name" );
            element->SetText( author.name.c_str() );
            root6->InsertEndChild( element );
        }
        if ( !author.emailAddress.empty() )
        {
            element = doc.NewElement( "aces:emailAddress" );
            element->SetText( author.emailAddress.c_str() );
            root6->InsertEndChild( element );
        }
        r->InsertEndChild( root6 );
    }
}

/**
 * aces:Info portion.
 *
 */
void AMFWriter::info( infoType& amfInfo )
{
    root2 = doc.NewElement("aces:amfInfo");
    if ( ! amfInfo.description.empty() )
    {
        element = doc.NewElement( "aces:description" );
        element->SetText( amfInfo.description.c_str() );
        root2->InsertEndChild( element );
    }
    aces_author( amfInfo.author, root2 );
    aces_dateTime( amfInfo.dateTime, root3 );
    root2->InsertEndChild( root3 );

    element = doc.NewElement( "aces:uuid" );
    if ( amfInfo.uuid.empty() )
    {
        aces_uuid( amfInfo.uuid );
    }
    element->SetText( amfInfo.uuid.c_str() );
    root2->InsertEndChild( element );

    root->InsertEndChild( root2 );
}

void AMFWriter::aces_anyURI( XMLNode* r, std::string& file )
{
    if ( file.empty() ) return;

    element = doc.NewElement( "aces:file" );
    element->SetText( file.c_str() );
    r->InsertEndChild( element );
}

/**
 * amf:clipId information
 *
 */
void AMFWriter::clip_id( clipIdType& clipId )
{

    root2 = doc.NewElement( "aces:clipId" );

    element = doc.NewElement("aces:clipName");
    element->SetText( clipId.clipName.c_str() );
    root2->InsertEndChild( element );

    if ( ! clipId.sequence.idx.empty() )
    {
        element = doc.NewElement("aces:sequence");
        element->SetAttribute( "idx", clipId.sequence.idx.c_str() );
        char buf[16];
        sprintf( buf, "%d", clipId.sequence.min );
        element->SetAttribute( "min", buf );
        sprintf( buf, "%d", clipId.sequence.max );
        element->SetAttribute( "max", buf );
        std::string filename = clipId.sequence.file;
        element->SetText( filename.c_str() );
        root2->InsertEndChild( element );
    }

    aces_anyURI( root2, clipId.file );

    element = doc.NewElement( "aces:uuid" );
    if ( clipId.uuid.empty() )
    {
        aces_uuid( clipId.uuid );
    }
    element->SetText( clipId.uuid.c_str() );
    root2->InsertEndChild( element );

    root->InsertEndChild( root2 );
}


void AMFWriter::pipeline_info( pipelineInfoType& t )
{
    root3 = doc.NewElement( "aces:pipelineInfo" );
    {
        if ( ! t.description.empty() )
        {
            element = doc.NewElement( "aces:description" );
            element->SetText( t.description.c_str() );
            root3->InsertEndChild( element );
        }

        {
            aces_author( t.author, root3 );
            aces_dateTime( t.dateTime, root4 );
            root3->InsertEndChild( root4 );
        }

        element = doc.NewElement( "aces:uuid" );
        if ( t.uuid.empty() )
        {
            aces_uuid( t.uuid );
        }
        element->SetText( t.uuid.c_str() );
        root3->InsertEndChild( element );

        {
            versionType& s = t.systemVersion;
            root4 = doc.NewElement( "aces:systemVersion" );
            element = doc.NewElement( "aces:majorVersion" );
            element->SetText( s.majorVersion );
            root4->InsertEndChild( element );
            element = doc.NewElement( "aces:minorVersion" );
            element->SetText( s.minorVersion );
            root4->InsertEndChild( element );
            element = doc.NewElement( "aces:patchVersion" );
            element->SetText( s.patchVersion );
            root4->InsertEndChild( element );
        }
        root3->InsertEndChild( root4 );
    }
    root2->InsertEndChild( root3 );
}

/**
 * Pipeline
 */
void AMFWriter::default_pipeline( pipelineType& pipeline )
{
    root2 = doc.NewElement( "aces:pipeline" );
    {
        pipeline_info( pipeline.pipelineInfo );
        input_transform( pipeline.inputTransform );
        look_transform( pipeline.lookTransform );
        output_transform( pipeline.outputTransform );
    }
    root->InsertEndChild( root2 );
}

/**
 * Pipeline
 */
void AMFWriter::archived_pipeline( pipelineType& t )
{

    pipelineInfoType& p = t.pipelineInfo;
    inputTransformType& i = t.inputTransform;
    lookTransformType& l = t.lookTransform;
    outputTransformType& o = t.outputTransform;

    if ( ( p.description.empty() ) &&
         ( i.transformId.empty() ) &&
         ( i.inverseReferenceRenderingTransform.transformId.empty() ) &&
         ( i.inverseOutputTransform.transformId.empty() ) &&
         ( i.inverseOutputDeviceTransform.transformId.empty() ) &&
         ( l.transformId.empty() ) &&
         ( l.cdlWorkingSpace.fromCdlWorkingSpace.transformId.empty() ) &&
         ( l.cdlWorkingSpace.toCdlWorkingSpace.transformId.empty() ) &&
         ( o.transformId.empty() ) &&
         ( o.outputDeviceTransform.transformId.empty() ) &&
         ( o.referenceRenderingTransform.transformId.empty() )
        )
        return;

    root2 = doc.NewElement( "aces:archivedPipeline" );
    {
        pipeline_info( p );
        input_transform( i );
        look_transform( l );
        output_transform( o );
    }
    root->InsertEndChild( root2 );
}

std::string MD5( std::string str )
{
    md5( (uint8_t*)str.c_str(), str.size() );
    std::string sout;
    char buf[256];
    uint8_t* p;
    p = (uint8_t*)&h0;
    sprintf(buf, "%2.2x%2.2x%2.2x%2.2x", p[0], p[1], p[2], p[3]);
    sout = buf;
    p = (uint8_t*)&h1;
    sprintf(buf, "%2.2x%2.2x%2.2x%2.2x", p[0], p[1], p[2], p[3]);
    sout += buf;
    p = (uint8_t*)&h2;
    sprintf(buf, "%2.2x%2.2x%2.2x%2.2x", p[0], p[1], p[2], p[3]);
    sout += buf;
    p = (uint8_t*)&h3;
    sprintf(buf, "%2.2x%2.2x%2.2x%2.2x", p[0], p[1], p[2], p[3]);
    sout += buf;
    return sout;
}

void AMFWriter::aces_hash( hashType& t, XMLNode*& r, std::string text )
{
    // text = kTRANSFORM_ID + text;
    if ( t.algorithm.rfind( "sha256" ) != std::string::npos )
    {
        t.algorithm = "http://www.w3.org/2001/04/xmlenc#sha256";
        element = doc.NewElement( "aces:hash" );
        element->SetAttribute( "algorithm", t.algorithm.c_str() );
        t.hash = sha256( text );
        element->SetText( t.hash.c_str() );
        r->InsertEndChild( element );
    }
    else if ( t.algorithm.rfind( "sha1" ) != std::string::npos )
    {
        t.algorithm = "http://www.w3.org/2000/09/xmldsig#sha1";
        element = doc.NewElement( "aces:hash" );
        element->SetAttribute( "algorithm", t.algorithm.c_str() );
        SHA1 sha1;
        sha1.update( text );
        t.hash = sha1.final();
        element->SetText( t.hash.c_str() );
        r->InsertEndChild( element );
    }
    else if ( t.algorithm.rfind( "md5" ) != std::string::npos )
    {
        t.algorithm = "http://www.w3.org/2001/04/xmldsig-more\\#md5";
        element = doc.NewElement( "aces:hash" );
        element->SetAttribute( "algorithm", t.algorithm.c_str() );
        t.hash = MD5( text );
        element->SetText( t.hash.c_str() );
        r->InsertEndChild( element );
    }
}

void AMFWriter::aces_transform_id( std::string transformId,
                                   XMLElement*& element )
{
    if ( transformId.substr( 0, kTRANSFORM_ID.size() ) != kTRANSFORM_ID )
    {
        transformId = kTRANSFORM_ID + transformId;
    }
    element->SetText( transformId.c_str() );
}

void AMFWriter::inverse_output_transform( XMLNode* r,
                                          inverseOutputTransformType& t )
{
    if ( t.transformId.empty() ) return;

    XMLNode* root3 = doc.NewElement( "aces:inverseOutputTransform" );

    if ( ! t.description.empty() )
    {
        element = doc.NewElement( "aces:description" );
        element->SetText( t.description.c_str() );
        root3->InsertEndChild( element );
    }


    aces_anyURI( root3, t.file );

    if ( ! t.transformId.empty() )
    {
        aces_hash( t.hash, root3, t.transformId );
        element = doc.NewElement( "aces:uuid" );
        if ( t.uuid.empty() ) aces_uuid( t.uuid );
        element->SetText( t.uuid.c_str() );
        root3->InsertEndChild(element);

        element = doc.NewElement( "aces:transformId" );
        aces_transform_id( t.transformId, element );
        root3->InsertEndChild(element);
    }

    r->InsertEndChild( root3 );
}

void AMFWriter::inverse_reference_rendering_transform( XMLNode* r,
                                                       inverseReferenceRenderingTransformType& t )
{
    if ( t.transformId.empty() ) return;
    XMLNode* root3 = doc.NewElement( "aces:inverseReferenceRenderingTransform" );

    if ( ! t.description.empty() )
    {
        element = doc.NewElement( "aces:description" );
        element->SetText( t.description.c_str() );
        root3->InsertEndChild( element );
    }



    aces_anyURI( root3, t.file );

    aces_hash( t.hash, root3, t.transformId );
    element = doc.NewElement( "aces:uuid" );
    if ( t.uuid.empty() ) aces_uuid( t.uuid );
    element->SetText( t.uuid.c_str() );
    root3->InsertEndChild(element);

    element = doc.NewElement( "aces:transformId" );
    aces_transform_id( t.transformId, element );
    root3->InsertEndChild(element);

    r->InsertEndChild( root3 );
}

void AMFWriter::inverse_output_device_transform( XMLNode* r,
                                                 inverseOutputDeviceTransformType& t )
{
    if ( t.transformId.empty() ) return;

    XMLNode* root3 = doc.NewElement( "aces:inverseOutputDeviceTransform" );

    if ( ! t.description.empty() )
    {
        element = doc.NewElement( "aces:description" );
        element->SetText( t.description.c_str() );
        root3->InsertEndChild( element );
    }


    aces_anyURI( root3, t.file );

    aces_hash( t.hash, root3, t.transformId );
    element = doc.NewElement( "aces:uuid" );
    if ( t.uuid.empty() ) aces_uuid( t.uuid );
    element->SetText( t.uuid.c_str() );
    root3->InsertEndChild(element);

    element = doc.NewElement( "aces:transformId" );
    aces_transform_id( t.transformId, element );
    root3->InsertEndChild(element);

    r->InsertEndChild( root3 );
}

 void AMFWriter::from_cdl_working_space( XMLNode* r,
                                         fromCdlWorkingSpaceType& t )
{
    XMLNode* root3 = doc.NewElement( "aces:fromLookTransformWorkingSpace" );
    r->InsertEndChild( root3 );

    if ( ! t.description.empty() )
    {
        element = doc.NewElement( "aces:description" );
        element->SetText( t.description.c_str() );
        root3->InsertEndChild( element );
    }


    aces_anyURI( root3, t.file );


    if ( ! t.transformId.empty() )
    {
        aces_hash( t.hash, root3, t.transformId );
        element = doc.NewElement( "aces:uuid" );
        if ( t.uuid.empty() ) aces_uuid( t.uuid );
        element->SetText( t.uuid.c_str() );
        root3->InsertEndChild(element);
        element = doc.NewElement( "aces:transformId" );
        aces_transform_id( t.transformId, element );
        root3->InsertEndChild(element);
    }
}


void AMFWriter::color_correction_ref( ColorCorrectionRefType& t )
{
    if ( ! t.ref.empty() )
    {
        root4 = doc.NewElement( "cdl:ColorCorrectionRef" );
        element = doc.NewElement( "cdl:ref" );
        element->SetText( t.ref.c_str() );
        root4->InsertEndChild( element );
        root3->InsertEndChild( root4 );
    }
}

void AMFWriter::sat( XMLNode* r, SatNodeType& t )
{
    XMLNode* root2 = doc.NewElement( "cdl:SatNode" );
    element = doc.NewElement( "cdl:Saturation" );
    element->SetText( t );
    root2->InsertEndChild( element );
    r->InsertEndChild( root2 );
}

void AMFWriter::sop( XMLNode* r, SOPNodeType& t )
{
    XMLNode* root2 = doc.NewElement( "cdl:SOPNode" );
    char buf[64];
    element = doc.NewElement( "cdl:Slope" );
    sprintf( buf, "%g %g %g", t.slope[0], t.slope[1], t.slope[2] );
    element->SetText( buf );
    root2->InsertEndChild( element );
    element = doc.NewElement( "cdl:Offset" );
    sprintf( buf, "%g %g %g", t.offset[0], t.offset[1], t.offset[2] );
    element->SetText( buf );
    root2->InsertEndChild( element );
    element = doc.NewElement( "cdl:Power" );
    sprintf( buf, "%g %g %g", t.power[0], t.power[1], t.power[2] );
    element->SetText( buf );
    root2->InsertEndChild( element );
    r->InsertEndChild( root2 );
}

void AMFWriter::to_cdl_working_space( XMLNode* r, toCdlWorkingSpaceType& t )
{
    XMLNode* root3 = doc.NewElement( "aces:toLookTransformWorkingSpace" );
    r->InsertEndChild( root3 );

    if ( ! t.description.empty() )
    {
        element = doc.NewElement( "aces:description" );
        element->SetText( t.description.c_str() );
        root3->InsertEndChild( element );
    }


    aces_anyURI( root3, t.file );

    if ( ! t.transformId.empty() )
    {
        aces_hash( t.hash, root3, t.transformId );
        element = doc.NewElement( "aces:uuid" );
        if ( t.uuid.empty() ) aces_uuid( t.uuid );
        element->SetText( t.uuid.c_str() );
        root3->InsertEndChild(element);

        element = doc.NewElement( "aces:transformId" );
        aces_transform_id( t.transformId, element );
        root3->InsertEndChild(element);
    }
}

bool SOP_node( SOPNodeType& s )
{
    bool write = false;
    if ( s.slope[0] != 1.0f || s.slope[1] != 1.0f || s.slope[2] != 1.0f )
        write = true;
    if ( s.offset[0] > 0.0f || s.offset[1] > 0.0f || s.offset[2] > 0.0f )
        write = true;
    if ( s.power[0] != 1.0f || s.power[1] != 1.0f || s.power[2] != 1.0f )
        write = true;
    return write;
}

void AMFWriter::look_transform( lookTransformType& t )
{
    cdlWorkingSpaceType& c = t.cdlWorkingSpace;
    if ( t.transformId.empty() &&
         c.toCdlWorkingSpace.transformId.empty() &&
         c.fromCdlWorkingSpace.transformId.empty() )
        return;

    XMLNode* root3;
    root3 = element = doc.NewElement( "aces:lookTransform" );
    aces_applied( element, t.applied );
    root2->InsertEndChild( element );

   if ( ! t.description.empty() )
    {
        element = doc.NewElement( "aces:description" );
        element->SetText( t.description.c_str() );
        root3->InsertEndChild( element );
    }



    aces_anyURI( root3, t.file );

    if ( ! t.transformId.empty() )
    {
        aces_hash( t.hash, root3, t.transformId );
        element = doc.NewElement( "aces:uuid" );
        if ( t.uuid.empty() ) aces_uuid( t.uuid );
        element->SetText( t.uuid.c_str() );
        root3->InsertEndChild(element);

        element = doc.NewElement( "aces:transformId" );
        aces_transform_id( t.transformId, element );
        root3->InsertEndChild(element);
    }

    if ( !c.toCdlWorkingSpace.transformId.empty() ||
         !c.fromCdlWorkingSpace.transformId.empty() )
    {
        root4 = doc.NewElement( "aces:lookTransformWorkingSpace" );
        root3->InsertEndChild( root4 );

        if ( ! c.toCdlWorkingSpace.transformId.empty() )
        {
            to_cdl_working_space( root4, c.toCdlWorkingSpace );
        }
        if ( ! c.fromCdlWorkingSpace.transformId.empty() )
        {
            from_cdl_working_space( root4, c.fromCdlWorkingSpace );
        }
        sop( root3, t.SOPNode );
        sat( root3, t.SatNode );
    }


}

void AMFWriter::output_transform( outputTransformType& t )
{
    root3 = element = doc.NewElement( "aces:outputTransform" );

    if ( ! t.description.empty() )
    {
        element = doc.NewElement( "aces:description" );
        element->SetText( t.description.c_str() );
        root3->InsertEndChild( element );
    }



    if ( ! t.file.empty() )
    {
        element = doc.NewElement( "aces:file" );
        element->SetText( t.file.c_str() );
        root3->InsertEndChild(element);
    }

    if ( ! t.transformId.empty() )
    {
        aces_hash( t.hash, root3, t.transformId );
        element = doc.NewElement( "aces:uuid" );
        if ( t.uuid.empty() ) aces_uuid( t.uuid );
        element->SetText( t.uuid.c_str() );
        root3->InsertEndChild(element);

        element = doc.NewElement( "aces:transformId" );
        aces_transform_id( t.transformId, element );
        root3->InsertEndChild(element);
    }

    reference_rendering_transform( t.referenceRenderingTransform );
    output_device_transform( t.outputDeviceTransform );

    root2->InsertEndChild( root3 );
}

void AMFWriter::reference_rendering_transform( referenceRenderingTransformType& t )
{
    if ( t.transformId.empty() ) return;
    root4 = element = doc.NewElement( "aces:referenceRenderingTransform" );

    if ( ! t.description.empty() )
    {
        element = doc.NewElement( "aces:description" );
        element->SetText( t.description.c_str() );
        root4->InsertEndChild( element );
    }



    aces_anyURI( root4, t.file );


    if ( ! t.transformId.empty() )
    {
        aces_hash( t.hash, root4, t.transformId );
        element = doc.NewElement( "aces:uuid" );
        if ( t.uuid.empty() ) aces_uuid( t.uuid );
        element->SetText( t.uuid.c_str() );
        root4->InsertEndChild(element);

        element = doc.NewElement( "aces:transformId" );
        aces_transform_id( t.transformId, element );
        root4->InsertEndChild(element);
    }

    root3->InsertEndChild( root4 );
}

void AMFWriter::output_device_transform( outputDeviceTransformType& t )
{
    if ( t.transformId.empty() ) return;
    root4 = element = doc.NewElement( "aces:outputDeviceTransform" );

    if ( ! t.description.empty() )
    {
        element = doc.NewElement( "aces:description" );
        element->SetText( t.description.c_str() );
        root4->InsertEndChild( element );
    }



    aces_anyURI( root4, t.file );

    if ( ! t.transformId.empty() )
    {
        aces_hash( t.hash, root4, t.transformId );
        element = doc.NewElement( "aces:uuid" );
        if ( t.uuid.empty() ) aces_uuid( t.uuid );
        element->SetText( t.uuid.c_str() );
        root4->InsertEndChild(element);

        element = doc.NewElement( "aces:transformId" );
        aces_transform_id( t.transformId, element );
        root4->InsertEndChild(element);
    }

    root3->InsertEndChild( root4 );
}

/**
 * Start Input Transform List
 */
void AMFWriter::input_transform( inputTransformType& t)
{
    if ( t.transformId.empty() &&
         t.inverseOutputTransform.transformId.empty() &&
         t.inverseOutputDeviceTransform.transformId.empty() &&
         t.inverseReferenceRenderingTransform.transformId.empty() )
        return;

    root3 = element = doc.NewElement( "aces:inputTransform" );
    aces_applied( element, t.applied );
    root2->InsertEndChild( root3 );


    if ( ! t.description.empty() )
    {
        element = doc.NewElement( "aces:description" );
        element->SetText( t.description.c_str() );
        root3->InsertEndChild( element );
    }



    aces_anyURI( root3, t.file );

    if ( ! t.transformId.empty() )
    {
        aces_hash( t.hash, root3, t.transformId );
        element = doc.NewElement( "aces:uuid" );
        if ( t.uuid.empty() ) aces_uuid( t.uuid );
        element->SetText( t.uuid.c_str() );
        root3->InsertEndChild(element);

        element = doc.NewElement( "aces:transformId" );
        aces_transform_id( t.transformId, element );
        root3->InsertEndChild(element);
    }

    inverse_output_transform( root3, t.inverseOutputTransform );
    inverse_output_device_transform( root3, t.inverseOutputDeviceTransform );
    inverse_reference_rendering_transform( root3, t.inverseReferenceRenderingTransform );
}

/**
 * Last Step.  Save the XML file
 *
 * @param filename file to save the XML file under.
 *
 * @return true on success, false on failure
 */
bool AMFWriter::save( const char* filename )
{
    header();
    info( aces.amfInfo );
    clip_id( aces.clipId );
    default_pipeline( aces.pipeline);
    archived_pipeline( aces.archivedPipeline );

    XMLError err = doc.SaveFile( filename );
    if ( err != XML_SUCCESS ) return false;
    return true;
}


}  // namespace ACES
