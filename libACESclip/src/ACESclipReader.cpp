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

#include <iostream>


#include "ACESclipReader.h"


namespace ACES {

using namespace tinyxml2;



/** 
 * Set status of a transform
 * 
 * @param s kPreview or kApplied
 */
std::string ACESclipReader::date_time( const char* i )
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
 * Set status of a transform
 * 
 * @param s kPreview or kApplied
 */
TransformStatus ACESclipReader::get_status( const std::string& s )
{
    if ( s == "applied" ) return kApplied;
    else return kPreview;
}

/** 
 * Constructor
 * 
 */
ACESclipReader::ACESclipReader()
{
}

XMLError ACESclipReader::header()
{
    root = doc.FirstChildElement("aces:ACESmetadata");
    if ( !root ) return XML_ERROR_FILE_READ_ERROR;

    element = root->FirstChildElement( "ContainerFormatVersion" );
    if ( !element ) return XML_ERROR_PARSING_ELEMENT;
    float version = atof( element->GetText() );
    if ( version > 1.0f )
        return XML_ERROR_FILE_READ_ERROR;

    return XML_NO_ERROR;
}

XMLError ACESclipReader::info()
{
    root2 = root->FirstChildElement( "aces:Info" );
    if (!root2) 
        return XML_ERROR_PARSING_ELEMENT;

    element = root2->FirstChildElement( "Application" );
    if ( element )
    {
        const char* tmp = element->GetText();
        if ( tmp ) application = tmp;

        tmp = element->Attribute("version");
        if ( tmp ) version = tmp;
    }

    element = root2->FirstChildElement( "Comment" );
    if ( element )
    {
        const char* tmp = element->GetText();
        if ( tmp ) comment = tmp;
    }

    return XML_NO_ERROR;
}

XMLError ACESclipReader::clip_id()
{
    root2 = root->FirstChildElement( "aces:ClipID" );
    if ( !root2 ) return XML_ERROR_PARSING_ELEMENT;

    element = root2->FirstChildElement( "ClipName" );
    if ( element )
    {
        const char* tmp = element->GetText();
        if ( tmp ) clip_name = tmp;
    }

    element = root2->FirstChildElement( "Source_MediaID" );
    if ( element )
    {
        const char* tmp = element->GetText();
        if ( tmp ) media_id = tmp;
    }

    element = root2->FirstChildElement( "ClipDate" );
    if ( element )
    {
        const char* tmp = element->GetText();
        if ( tmp ) clip_date = date_time( tmp );
    }

    return XML_NO_ERROR;
}

XMLError ACESclipReader::config()
{
    root2 = root->FirstChildElement( "aces:Config" );
    if ( !root2 ) return XML_ERROR_PARSING_ELEMENT;

    element = root2->FirstChildElement( "ACESrelease_Version" );
    if ( element )
    {
        const char* tmp = element->GetText();
        if ( tmp )
        {
            double version = atof( tmp );
            if ( version > 1.0 )
            {
                return XML_ERROR_FILE_READ_ERROR;
            }
        }
    }

    element = root2->FirstChildElement( "ClipDate" );
    if ( element )
    {
        const char* tmp = element->GetText();
        if ( tmp ) timestamp = tmp;
    }

    return XML_NO_ERROR;
}

XMLError ACESclipReader::ITL()
{
    root3 = root2->FirstChildElement( "InputTransformList" );
    if ( !root3 ) return XML_ERROR_PARSING_ELEMENT;

    std::string name;
    TransformStatus status = kPreview;
    element = root3->FirstChildElement( "aces:IDTref" );
    if ( element )
    {
        const char* tmp = element->Attribute( "name" );
        if ( tmp ) name = tmp;

        tmp = element->Attribute( "status" );
        if ( tmp ) status = get_status( tmp );
    }

    element = root3->FirstChildElement( "LinkTransform" );
    if ( element )
    {
        const char* tmp = element->GetText();
        if ( tmp ) name = tmp;
    }

    IDT.name = name;
    IDT.status = status;

    element = root3->FirstChildElement( "LinkInputTransformList" );
    if ( element )
    {
        const char* tmp = element->GetText();
        if ( tmp ) link_ITL = tmp;
    }

    return XML_NO_ERROR;
}

XMLError ACESclipReader::PTL()
{
    root3 = root2->FirstChildElement( "PreviewTransformList" );
    if ( !root3 ) return XML_ERROR_PARSING_ELEMENT;


    element = root3->FirstChildElement( "aces:LMTref" );
    while( element )
    {
        std::string name;
        TransformStatus status = kPreview;
        if ( element )
        {
            const char* tmp = element->Attribute( "name" );
            if ( tmp ) name = tmp;

            tmp = element->Attribute( "status" );
            if ( tmp ) status = get_status( tmp );
        }

        XMLElement* elem = root3->FirstChildElement( "LinkTransform" );
        if ( elem )
        {
            const char* tmp = elem->GetText();
            if ( tmp ) name = tmp;
        }

        LMT.push_back( Transform( name, status ) );

        element = element->NextSiblingElement( "aces:LMTref" );
    }

    {
        std::string name;
        TransformStatus status = kPreview;
        element = root3->FirstChildElement( "aces:RRTref" );
        if ( element )
        {
            const char* tmp = element->Attribute( "name" );
            if ( tmp ) name = tmp;

            tmp = element->Attribute( "status" );
            if ( tmp ) status = get_status( tmp );
        }

        element = root3->FirstChildElement( "LinkTransform" );
        if ( element )
        {
            const char* tmp = element->GetText();
            if ( tmp ) name = tmp;
        }

        RRT.name = name;
        RRT.status = status;
    }

    {
        std::string name;
        TransformStatus status = kPreview;
        element = root3->FirstChildElement( "aces:ODTref" );
        if ( element )
        {
            const char* tmp = element->Attribute( "name" );
            if ( tmp ) name = tmp;

            tmp = element->Attribute( "status" );
            if ( tmp ) status = get_status( tmp );
        }

        element = root3->FirstChildElement( "LinkTransform" );
        if ( element )
        {
            const char* tmp = element->GetText();
            if ( tmp ) name = tmp;
        }

        ODT.name = name;
        ODT.status = status;
    }


    element = root3->FirstChildElement( "LinkPreviewTransformList" );
    if ( element )
    {
        const char* tmp = element->GetText();
        if ( tmp ) link_PTL = tmp;
    }

    return XML_NO_ERROR;
}

/** 
 * First Step.  Load the XML file.
 * 
 * @param filename file to load the XML file from. 
 * 
 * @return true on success, false on failure
 */
bool ACESclipReader::load( const char* filename )
{
    XMLError err = doc.LoadFile( filename );
    if ( err != XML_NO_ERROR ) return false;

    err = header();
    if ( err != XML_NO_ERROR ) return false;

    err = info();
    if ( err != XML_NO_ERROR ) return false;

    err = clip_id();
    if ( err != XML_NO_ERROR ) return false;

    err = config();
    if ( err != XML_NO_ERROR ) return false;

    err = ITL();
    if ( err != XML_NO_ERROR ) return false;

    err = PTL();
    if ( err != XML_NO_ERROR ) return false;

    return true;
}


}  // namespace ACES


