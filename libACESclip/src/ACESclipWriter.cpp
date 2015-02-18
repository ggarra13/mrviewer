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

#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp> // input-output
#include <boost/lexical_cast.hpp> // for casting uuid into std::string

#include "ACESclipWriter.h"


namespace ACES {

static const double kVersion = 1.0;
static const double kContainerVersion = 1.0;

using namespace tinyxml2;


/**
 * Return a date and time information in ACESclip format.
 *
 * @param t time_t to convert to string.
 * 
 * @return std::string with the formatted date and time.
 */
std::string ACESclipWriter::date_time( const time_t& t ) const
{
    char buf[24];
    struct tm* now = localtime( &t );
    sprintf( buf, "%d-%02d-%02dT%02d:%02d:%02d",
             now->tm_year+1900, now->tm_mon+1, now->tm_mday,
             now->tm_hour, now->tm_min, now->tm_sec );
    return buf;
}


/** 
 * Set status of a transform
 * 
 * @param s kPreview or kApplied
 */
void ACESclipWriter::set_status( TransformStatus s )
{
    const char* p;
    switch( s )
    {
        case kPreview:
            p = "preview"; break;
        case kApplied:
            p = "applied"; break;
        default:
            p = "unknown"; break;
    }
    element->SetAttribute( "status", p );
}

/** 
 * Constructor
 * 
 */
ACESclipWriter::ACESclipWriter()
{
    XMLDeclaration* decl = doc.NewDeclaration( NULL );
    doc.InsertFirstChild( decl );

    element = doc.NewElement( "aces:ACESmetadata" );
    element->SetAttribute( "xmlns:aces", 
                           "http://www.oscars.org/aces/ref/acesmetadata");
    doc.InsertAfterChild( decl, element );
    root = element;

    element = doc.NewElement("ContainerFormatVersion");
    element->SetText( kContainerVersion );
    root->InsertEndChild( element );

    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    std::string UUID = boost::lexical_cast<std::string>(uuid);

    element = doc.NewElement("UUID");
    element->SetText( UUID.c_str() );
    root->InsertEndChild( element );

    element = doc.NewElement("ModificationTime");

    time_t t = time(0);   // get time now
    const std::string& date = date_time( t );
    element->SetText( date.c_str() );
    root->InsertEndChild( element );
}

/** 
 * aces:Info portion.
 * 
 * @param application Application used to output the xml file. 
 * @param version     Version of the application.
 * @param comment     Some additional comment (optional)
 */
void ACESclipWriter::info( const std::string application,
                           const std::string version,
                           const std::string comment )
{
    root2 = doc.NewElement( "aces:Info" );
    root->InsertEndChild( root2 );

    element = doc.NewElement("Application");
    element->SetAttribute( "version", version.c_str() );
    element->SetText( application.c_str() );
    root2->InsertEndChild( element );

    if ( !comment.empty() )
    {
        element = doc.NewElement("Comment");
        element->SetText( comment.c_str() );
        root2->InsertEndChild( element );
    }
}

/** 
 * aces:ClipID information
 * 
 * @param clip_name name of clip (name of image, version, etc)
 * @param media_id  identification of location of clip ( shot, for example )
 * @param clip_date time of clip ( as returned from stat )
 */
void ACESclipWriter::clip_id( const std::string clip_name,
                              const std::string media_id,
                              const time_t clip_date )
{

    root2 = doc.NewElement( "aces:ClipID" );
    root->InsertEndChild( root2 );

    element = doc.NewElement("ClipName");
    element->SetText( clip_name.c_str() );
    root2->InsertEndChild( element );

    element = doc.NewElement("Source_MediaID");
    element->SetText( media_id.c_str() );
    root2->InsertEndChild( element );

    std::string date = date_time( clip_date );

    element = doc.NewElement("ClipDate");
    element->SetText( date.c_str() );
    root2->InsertEndChild( element );
}

/** 
 * aces:Config portion of file
 * 
 * @param xml_date time_t of date XML file was modified.
 */
void ACESclipWriter::config( const time_t xml_date )
{
    root2 = doc.NewElement( "aces:Config" );
    root->InsertEndChild( root2 );

    element = doc.NewElement("ACESrelease_Version");
    element->SetText( kVersion );
    root2->InsertEndChild( element );

    element = doc.NewElement("Timestamp");

    std::string date = date_time( xml_date );
    element->SetText( date.c_str() );
    root2->InsertEndChild( element );
}

/** 
 * Input Transform List start.
 * 
 * @param idt Name of Input Device Transform ( IDT ) (optional)
 */
void ACESclipWriter::ITL_start( TransformStatus status )
{
    element = doc.NewElement("aces:InputTransformList");
    set_status( status );
    root2->InsertEndChild( element );
    root3 = element;
}

/**
 * Add an Output Device Transform
 *
 * @param name    name of the ODT
 * @param status  kPreview or kApplied
 */
void ACESclipWriter::add_IDT( const std::string name, TransformStatus status )
{
    IDT.name = name;
    IDT.status = status;
}

/** 
 * Input Transform List end
 * 
 * @param it Link Input Transform (optional)
 */
void ACESclipWriter::ITL_end( const std::string it )
{
    if ( !IDT.name.empty() ) 
    {
        const std::string& name = IDT.name;

        element = doc.NewElement( "aces:IDTref" );
        element->SetAttribute( "TransformID", name.c_str() );
        set_status( IDT.status );

        root3->InsertEndChild( element );

        const std::string& link = IDT.link_transform;
        if ( ! link.empty() )
        {
            XMLNode* root4 = element;
            element = doc.NewElement( "LinkTransform" );
            element->SetText( link.c_str() );
            root4->InsertEndChild( element );
        }
    }

    if ( ! it.empty() )
    {
        element = doc.NewElement( "LinkInputTransformList" );
        element->SetText( it.c_str() );
        root3->InsertEndChild( element );
    }

}



/** 
 * Preview Transform List
 * 
 */
void ACESclipWriter::PTL_start()
{

    element = doc.NewElement("aces:PreviewTransformList");
    root2->InsertEndChild( element );
    root3 = element;

}

/** 
 * Add a Look Modification Transform to list
 * 
 * @param name    name of the LMT
 * @param status  kPreview or kApplied
 */
void ACESclipWriter::add_LMT( const std::string name, TransformStatus status,
                              const std::string link_transform )
{
    LMT.push_back( Transform( name, link_transform, status ) );
}


/** 
 * Add a Reference Rendering Transform
 * 
 * @param name    name of the RRT
 * @param status  kPreview or kApplied
 */
void ACESclipWriter::add_RRT( const std::string name, TransformStatus status )
{
    RRT.name = name;
    RRT.status = status;
}

void ACESclipWriter::add_RRTODT( const std::string name, 
                                 TransformStatus status )
{
    RRTODT.name = name;
    RRTODT.status = status;
}


/**
 * Add an Output Device Transform
 *
 * @param name    name of the ODT
 * @param status  kPreview or kApplied
 */
void ACESclipWriter::add_ODT( const std::string name, TransformStatus status,
                              const std::string link_transform )
{
    ODT.name = name;
    ODT.link_transform = link_transform;
    ODT.status = status;
}

/**
 * Preview Transform List end
 *
 * @param t Combined LMT_RRT_ODT ( optional )
 */
void ACESclipWriter::PTL_end( const std::string t )
{
    int count = 0;

    if ( ! LMT.empty() )
    {
        LMTransforms::const_iterator i = LMT.begin();
        LMTransforms::const_iterator e = LMT.end();
        for ( ; i != e; ++i, ++count )
        {
            const std::string& name = (*i).name;
            element = doc.NewElement( "aces:LMTref" );
            element->SetAttribute( "TransformID", name.c_str() );
            set_status( (*i).status );
            root3->InsertEndChild( element );


            const std::string& link = (*i).link_transform;
            if ( ! link.empty() )
            {
                XMLNode* root4 = element;
                element = doc.NewElement( "LinkTransform" );
                element->SetText( link.c_str() );
                root4->InsertEndChild( element );
            }
        }
    }

    if ( !RRT.name.empty() )
    {
        ++count;
        const std::string& name = RRT.name;
        element = doc.NewElement( "aces:RRTref" );
        element->SetAttribute( "TransformID", name.c_str() );
        set_status( RRT.status );
        root3->InsertEndChild( element );
    }

    if ( !RRTODT.name.empty() )
    {
        ++count;
        const std::string& name = RRTODT.name;
        element = doc.NewElement( "aces:RRTODTref" );
        element->SetAttribute( "TransformID", name.c_str() );
        set_status( RRTODT.status );
        root3->InsertEndChild( element );
    }


    if ( !ODT.name.empty() )
    {
        ++count;
        const std::string& name = ODT.name;

        element = doc.NewElement( "aces:ODTref" );
        element->SetAttribute( "TransformID", name.c_str() );
        set_status( ODT.status );
        root3->InsertEndChild( element );

        const std::string& link = ODT.link_transform;
        if ( ! link.empty() )
        {
            XMLNode* root4 = element;
            element = doc.NewElement( "LinkTransform" );
            element->SetText( link.c_str() );
            root4->InsertEndChild( element );
        }
    }

    if ( count > 1 && !t.empty() )
    {
        element = doc.NewElement( "LinkPreviewTransformList" );
        element->SetText( t.c_str() );
        root3->InsertEndChild( element );
    }
}

/** 
 * Last Step.  Save the XML file
 * 
 * @param filename file to save the XML file under. 
 * 
 * @return true on success, false on failure
 */
bool ACESclipWriter::save( const char* filename )
{
    XMLError err = doc.SaveFile( filename );
    if ( err != XML_NO_ERROR ) return false;
    return true;
}


}  // namespace ACES


